 /**********************************************************************  
 *<
	FILE: EditPolyModes.cpp

	DESCRIPTION: Edit Poly Modifier - Command modes

	CREATED BY: Steve Anderson

	HISTORY: created April 2004

 *>	Copyright (c) 2004, All Rights Reserved.
 **********************************************************************/

#include "EPoly.h"
#include "EditPoly.h"
#include "decomp.h"
#include "spline3d.h"
#include "splshape.h"
#include "shape.h"

#define EPSILON .0001f

CommandMode * EditPolyMod::getCommandMode (int mode) {
	switch (mode) {
	case ep_mode_create_vertex: return createVertMode;
	case ep_mode_create_edge: return createEdgeMode;
	case ep_mode_create_face: return createFaceMode;
	case ep_mode_divide_edge: return divideEdgeMode;
	case ep_mode_divide_face: return divideFaceMode;
	case ep_mode_bridge_border: return bridgeBorderMode;
	case ep_mode_bridge_polygon: return bridgePolyMode;
	case ep_mode_bridge_edge: return bridgeEdgeMode;


	case ep_mode_extrude_vertex:
	case ep_mode_extrude_edge:
		return extrudeVEMode;
	case ep_mode_extrude_face: return extrudeMode;
	case ep_mode_chamfer_vertex:
	case ep_mode_chamfer_edge:
		return chamferMode;
	case ep_mode_bevel: return bevelMode;
	case ep_mode_inset_face: return insetMode;
	case ep_mode_outline: return outlineMode;
	case ep_mode_cut: return cutMode;
	case ep_mode_quickslice: return quickSliceMode;
	case ep_mode_weld: return weldMode;
	case ep_mode_hinge_from_edge: return hingeFromEdgeMode;
	case ep_mode_pick_hinge: return pickHingeMode;
	case ep_mode_pick_bridge_1: return pickBridgeTarget1;
	case ep_mode_pick_bridge_2: return pickBridgeTarget2;
	case ep_mode_edit_tri: return editTriMode;
	case ep_mode_turn_edge: return turnEdgeMode;
	case ep_mode_edit_ss: return editSSMode;
	}
	return NULL;
}

int EditPolyMod::EpModGetCommandMode () {
	if (mSliceMode) return ep_mode_sliceplane;

	if (!ip) return -1;
	CommandMode *currentMode = ip->GetCommandMode ();

	if (currentMode == createVertMode) return ep_mode_create_vertex;
	if (currentMode == createEdgeMode) return ep_mode_create_edge;
	if (currentMode == createFaceMode) return ep_mode_create_face;
	if (currentMode == divideEdgeMode) return ep_mode_divide_edge;
	if (currentMode == divideFaceMode) return ep_mode_divide_face;
	if (currentMode == bridgeBorderMode) return ep_mode_bridge_border;
	if (currentMode == bridgePolyMode) return ep_mode_bridge_polygon;
	if (currentMode == bridgeEdgeMode) return ep_mode_bridge_edge;
	if (currentMode == extrudeVEMode) {
		if (meshSelLevel[selLevel] == MNM_SL_EDGE) return ep_mode_extrude_edge;
		else return ep_mode_extrude_vertex;
	}
	if (currentMode == extrudeMode) return ep_mode_extrude_face;
	if (currentMode == chamferMode) {
		if (meshSelLevel[selLevel] == MNM_SL_EDGE) return ep_mode_chamfer_edge;
		else return ep_mode_chamfer_vertex;
	}
	if (currentMode == bevelMode) return ep_mode_bevel;
	if (currentMode == insetMode) return ep_mode_inset_face;
	if (currentMode == outlineMode) return ep_mode_outline;
	if (currentMode == cutMode) return ep_mode_cut;
	if (currentMode == quickSliceMode) return ep_mode_quickslice;
	if (currentMode == weldMode) return ep_mode_weld;
	if (currentMode == hingeFromEdgeMode) return ep_mode_hinge_from_edge;
	if (currentMode == pickHingeMode) return ep_mode_pick_hinge;
	if (currentMode == pickBridgeTarget1) return ep_mode_pick_bridge_1;
	if (currentMode == pickBridgeTarget2) return ep_mode_pick_bridge_2;
	if (currentMode == editTriMode) return ep_mode_edit_tri;
	if (currentMode == turnEdgeMode) return ep_mode_turn_edge;
	if(currentMode == editSSMode) return ep_mode_edit_ss;
	return -1;
}

void EditPolyMod::EpModToggleCommandMode (int mode) {
	if (!ip) return;
	if ((mode == ep_mode_sliceplane) && (selLevel == EPM_SL_OBJECT)) return;

	if (mode == ep_mode_sliceplane) {
		// Special case.
		ip->ClearPickMode();
		if (mSliceMode) ExitSliceMode();
		else {
			EpModCloseOperationDialog ();	// Cannot have slice mode, settings at same time.
			// Exit any EditPoly command mode we may currently be in:
			ExitAllCommandModes (false, false);
			EnterSliceMode();
		}
		return;
	}

	// Otherwise, make sure we're not in Slice mode:
	if (mSliceMode) ExitSliceMode ();

	CommandMode *cmd = getCommandMode (mode);
	if (cmd==NULL) return;
	CommandMode *currentMode = ip->GetCommandMode ();

	switch (mode) {
	case ep_mode_extrude_vertex:
	case ep_mode_chamfer_vertex:
		// Special case - only use DeleteMode to exit mode if in correct SO level,
		// Otherwise use EpModEnterCommandMode to switch SO levels and enter mode again.
		if ((currentMode==cmd) && (selLevel==EPM_SL_VERTEX)) ip->DeleteMode (cmd);
		else {
			EpModCloseOperationDialog ();	// Cannot have command mode, settings at same time.
			EpModEnterCommandMode (mode);
		}
		break;

	case ep_mode_extrude_edge:
	case ep_mode_chamfer_edge:
		// Special case - only use DeleteMode to exit mode if in correct SO level,
		// Otherwise use EpModEnterCommandMode to switch SO levels and enter mode again.
		if ((currentMode==cmd) && (meshSelLevel[selLevel]==MNM_SL_EDGE)) ip->DeleteMode (cmd);
		else {
			EpModCloseOperationDialog ();	// Cannot have command mode, settings at same time.
			EpModEnterCommandMode (mode);
		}
		break;

	case ep_mode_pick_hinge:
	case ep_mode_pick_bridge_1:
	case ep_mode_pick_bridge_2:
		// Special case - we do not want to end our preview or close the dialog,
		// since this command mode is controlled from the dialog.
		if (currentMode == cmd) ip->DeleteMode (cmd);
		else EpModEnterCommandMode (mode);
		break;

	default:
		if (currentMode == cmd) ip->DeleteMode (cmd);
		else {
			EpModCloseOperationDialog ();	// Cannot have command mode, settings at same time.
			EpModEnterCommandMode (mode);
		}
		break;
	}
}

void EditPolyMod::EpModEnterCommandMode (int mode) {
	if (!ip) return;

	// First of all, we don't want to pile up our command modes:
	ExitAllCommandModes (false, false);

	switch (mode) {
	case ep_mode_create_vertex:
		if (selLevel != EPM_SL_VERTEX) ip->SetSubObjectLevel (EPM_SL_VERTEX);
		ip->PushCommandMode(createVertMode);
		break;

	case ep_mode_create_edge:
		if (meshSelLevel[selLevel] != MNM_SL_EDGE) ip->SetSubObjectLevel (EPM_SL_EDGE);
		ip->PushCommandMode (createEdgeMode);
		break;

	case ep_mode_create_face:
	
		if (selLevel != EPM_SL_FACE)  ip->SetSubObjectLevel (EPM_SL_FACE);
		ip->PushCommandMode (createFaceMode);
		break;

	case ep_mode_divide_edge:
		if (meshSelLevel[selLevel] != MNM_SL_EDGE) ip->SetSubObjectLevel (EPM_SL_EDGE);
		ip->PushCommandMode (divideEdgeMode);
		break;

	case ep_mode_divide_face:
		if (selLevel < EPM_SL_FACE) ip->SetSubObjectLevel (EPM_SL_FACE);
		ip->PushCommandMode (divideFaceMode);
		break;

	case ep_mode_bridge_border:
		if (selLevel != EPM_SL_BORDER) ip->SetSubObjectLevel (EPM_SL_BORDER);
		ip->PushCommandMode (bridgeBorderMode);
		break;

	case ep_mode_bridge_polygon:
		if (selLevel != EPM_SL_FACE) ip->SetSubObjectLevel (EPM_SL_FACE);
		ip->PushCommandMode (bridgePolyMode);
		break;

	case ep_mode_bridge_edge:
		if (selLevel != EPM_SL_EDGE) ip->SetSubObjectLevel (EPM_SL_EDGE);
		ip->PushCommandMode (bridgeEdgeMode);
		break;

	case ep_mode_extrude_vertex:
		if (selLevel != EPM_SL_VERTEX) ip->SetSubObjectLevel (EPM_SL_VERTEX);
	
		ip->PushCommandMode (extrudeVEMode);
		break;

	case ep_mode_extrude_edge:
		if (meshSelLevel[selLevel] != MNM_SL_EDGE) ip->SetSubObjectLevel (EPM_SL_EDGE);
	
		ip->PushCommandMode (extrudeVEMode);
		break;

	case ep_mode_extrude_face:
		if (selLevel < EPM_SL_FACE) ip->SetSubObjectLevel (EPM_SL_FACE);
		ip->PushCommandMode (extrudeMode);
		break;

	case ep_mode_chamfer_vertex:
		if (selLevel != EPM_SL_VERTEX) ip->SetSubObjectLevel (EPM_SL_VERTEX);
		ip->PushCommandMode (chamferMode);
		break;

	case ep_mode_chamfer_edge:
		if (meshSelLevel[selLevel] != MNM_SL_EDGE) ip->SetSubObjectLevel (EPM_SL_EDGE);
		ip->PushCommandMode (chamferMode);
		break;

	case ep_mode_bevel:
		if (selLevel < EPM_SL_FACE) ip->SetSubObjectLevel (EPM_SL_FACE);
		ip->PushCommandMode (bevelMode);
		break;
	case ep_mode_edit_ss:
		ip->PushCommandMode (editSSMode);
		break;
	case ep_mode_inset_face:
		if (meshSelLevel[selLevel] != MNM_SL_FACE) ip->SetSubObjectLevel (EPM_SL_FACE);
		ip->PushCommandMode (insetMode);
		break;

	case ep_mode_outline:
		if (meshSelLevel[selLevel] != MNM_SL_FACE) ip->SetSubObjectLevel (EPM_SL_FACE);
		ip->PushCommandMode (outlineMode);
		break;

	case ep_mode_cut:
		ip->PushCommandMode (cutMode);
		break;

	case ep_mode_quickslice:
		ip->PushCommandMode (quickSliceMode);
		break;

	case ep_mode_weld:
		if (selLevel > EPM_SL_BORDER) ip->SetSubObjectLevel (EPM_SL_VERTEX);
		if (selLevel == EPM_SL_BORDER) ip->SetSubObjectLevel (EPM_SL_EDGE);
		ip->PushCommandMode (weldMode);
		break;

	case ep_mode_hinge_from_edge:
		if (selLevel != EPM_SL_FACE) ip->SetSubObjectLevel (EPM_SL_FACE);
		ip->PushCommandMode (hingeFromEdgeMode);
		break;

	case ep_mode_pick_hinge:
		ip->PushCommandMode (pickHingeMode);
		break;

	case ep_mode_pick_bridge_1:
		ip->PushCommandMode (pickBridgeTarget1);
		break;

	case ep_mode_pick_bridge_2:
		ip->PushCommandMode (pickBridgeTarget2);
		break;

	case ep_mode_edit_tri:
		if (selLevel < EPM_SL_EDGE) ip->SetSubObjectLevel (EPM_SL_FACE);
		ip->PushCommandMode (editTriMode);
		break;

	case ep_mode_turn_edge:
		if (selLevel < EPM_SL_EDGE) ip->SetSubObjectLevel (EPM_SL_EDGE);
		ip->PushCommandMode (turnEdgeMode);
		break;
	}
}

int EditPolyMod::EpModGetPickMode () {
	if (!ip) return -1;
	PickModeCallback *currentMode = ip->GetCurPickMode ();

	if (currentMode == attachPickMode) return ep_mode_attach;
	if (currentMode == mpShapePicker) return ep_mode_pick_shape;

	return -1;
}

void EditPolyMod::EpModEnterPickMode (int mode) {
	if (!ip) return;

	// Make sure we're not in Slice mode:
	if (mSliceMode) ExitSliceMode ();

	switch (mode) {
	case ep_mode_attach:
		ip->SetPickMode (attachPickMode);
		break;
	case ep_mode_pick_shape:
		ip->SetPickMode (mpShapePicker);
		break;
	}
}

//------------Command modes & Mouse procs----------------------

HitRecord *EditPolyPickEdgeMouseProc::HitTestEdges (IPoint2 &m, ViewExp *vpt, float *prop, 
											Point3 *snapPoint) {
	vpt->ClearSubObjHitList();

	if (HitTestResult ()) mpEPoly->SetHitTestResult ();
	if (mpEPoly->GetEPolySelLevel() != EPM_SL_BORDER)
		mpEPoly->SetHitLevelOverride (SUBHIT_MNEDGES);
	ip->SubObHitTest(ip->GetTime(),HITTYPE_POINT,0, 0, &m, vpt);
	if (mpEPoly->GetEPolySelLevel() != EPM_SL_BORDER)
		mpEPoly->ClearHitLevelOverride ();
	mpEPoly->ClearHitTestResult ();

	if (!vpt->NumSubObjHits()) return NULL;
	HitLog& hitLog = vpt->GetSubObjHitList();
	HitRecord *hr = hitLog.ClosestHit();
	if (!hr) return hr;
	if (!prop) return hr;

	// Find where along this edge we hit
	// Strategy:
	// Get Mouse click, plus viewport z-direction at mouse click, in object space.
	// Then find the direction of the edge in a plane orthogonal to z, and see how far
	// along that edge we are.

	DWORD ee = hr->hitInfo;
	Matrix3 obj2world = hr->nodeRef->GetObjectTM (ip->GetTime ());

	Ray r;
	vpt->MapScreenToWorldRay ((float)m.x, (float)m.y, r);
	if (!snapPoint) snapPoint = &(r.p);
	Point3 Zdir = Normalize (r.dir);

	MNMesh *pMesh = HitTestResult() ? mpEPoly->EpModGetOutputMesh (hr->nodeRef)
		: mpEPoly->EpModGetMesh (hr->nodeRef);
	if (pMesh == NULL) return NULL;

	MNMesh & mm = *pMesh;
	Point3 A = obj2world * mm.v[mm.e[ee].v1].p;
	Point3 B = obj2world * mm.v[mm.e[ee].v2].p;
	Point3 Xdir = B-A;
	Xdir -= DotProd(Xdir, Zdir)*Zdir;
	*prop = DotProd (Xdir, *snapPoint-A) / LengthSquared (Xdir);
	if (*prop<.0001f) *prop=0;
	if (*prop>.9999f) *prop=1;
	return hr;
}

int EditPolyPickEdgeMouseProc::proc (HWND hwnd, int msg, int point, int flags, IPoint2 m) {
	ViewExp* vpt = NULL;
	HitRecord* hr = NULL;
	float prop(0.0f);
	Point3 snapPoint(0.0f,0.0f,0.0f);

	switch (msg) {
	case MOUSE_PROPCLICK:
		ip->PopCommandMode ();
		break;

	case MOUSE_POINT:
		ip->SetActiveViewport(hwnd);
		vpt = ip->GetViewport(hwnd);
		snapPoint = vpt->SnapPoint (m, m, NULL);
		snapPoint = vpt->MapCPToWorld (snapPoint);
		hr = HitTestEdges (m, vpt, &prop, &snapPoint);
		if (vpt) ip->ReleaseViewport(vpt);
	
		if (!hr) break;
		if (!EdgePick (hr, prop)) {
			// False return value indicates exit mode.
			ip->PopCommandMode ();
			return false;
		}
		return true;

	case MOUSE_MOVE:
	case MOUSE_FREEMOVE:
		vpt = ip->GetViewport(hwnd);
		vpt->SnapPreview (m, m, NULL, SNAP_FORCE_3D_RESULT);//|SNAP_SEL_OBJS_ONLY);
		if (HitTestEdges(m,vpt,NULL,NULL)) SetCursor(ip->GetSysCursor(SYSCUR_SELECT));
		else SetCursor(LoadCursor(NULL,IDC_ARROW));
		if (vpt) ip->ReleaseViewport(vpt);
		break;
	}

	return TRUE;
}

// --------------------------------------------------------

HitRecord *EditPolyPickFaceMouseProc::HitTestFaces (IPoint2 &m, ViewExp *vpt) {
	vpt->ClearSubObjHitList();

	mpEPoly->SetHitLevelOverride (SUBHIT_MNFACES);
	if (HitTestResult()) mpEPoly->SetHitTestResult ();
	ip->SubObHitTest(ip->GetTime(),HITTYPE_POINT,0, 0, &m, vpt);
	mpEPoly->ClearHitTestResult();
	mpEPoly->ClearHitLevelOverride ();
	if (!vpt->NumSubObjHits()) return NULL;
	HitLog& hitLog = vpt->GetSubObjHitList();
	HitRecord *hr = hitLog.ClosestHit();
	return hr;
}

void EditPolyPickFaceMouseProc::ProjectHitToFace (IPoint2 &m, ViewExp *vpt,
										  HitRecord *hr, Point3 *snapPoint) {
	if (!hr) return ;

	// Find subtriangle, barycentric coordinates of hit in face.
	face = hr->hitInfo;
	Matrix3 obj2world = hr->nodeRef->GetObjectTM (ip->GetTime ());

	Ray r;
	vpt->MapScreenToWorldRay ((float)m.x, (float)m.y, r);
	if (!snapPoint) snapPoint = &(r.p);
	Point3 Zdir = Normalize (r.dir);

	// Find an approximate location for the point on the surface we hit:
	// Get the average normal for the face, thus the plane, and intersect.
	Point3 intersect;
	MNMesh *pMesh = HitTestResult() ? mpEPoly->EpModGetOutputMesh () : mpEPoly->EpModGetMesh();
	if (pMesh == NULL) return;

	MNMesh & mm = *pMesh;
	Point3 planeNormal = mm.GetFaceNormal (face, TRUE);
	planeNormal = Normalize (obj2world.VectorTransform (planeNormal));
	float planeOffset=0.0f;
	for (int i=0; i<mm.f[face].deg; i++)
		planeOffset += DotProd (planeNormal, obj2world*mm.v[mm.f[face].vtx[i]].p);
	planeOffset = planeOffset/float(mm.f[face].deg);

	// Now we intersect the snapPoint + r.dir*t line with the
	// DotProd (planeNormal, X) = planeOffset plane.
	float rayPlane = DotProd (r.dir, planeNormal);
	float firstPointOffset = planeOffset - DotProd (planeNormal, *snapPoint);
	if (fabsf(rayPlane) > EPSILON) {
		float amount = firstPointOffset / rayPlane;
		intersect = *snapPoint + amount*r.dir;
	} else {
		intersect = *snapPoint;
	}

	Matrix3 world2obj = Inverse (obj2world);
	intersect = world2obj * intersect;

	mm.FacePointBary (face, intersect, bary);
}

int EditPolyPickFaceMouseProc::proc (HWND hwnd, int msg, int point, int flags, IPoint2 m) {
	ViewExp* vpt = NULL;
	HitRecord* hr = NULL;
	Point3 snapPoint(0.0f,0.0f,0.0f);

	switch (msg) {
	case MOUSE_PROPCLICK:
		ip->PopCommandMode ();
		break;

	case MOUSE_POINT:
		ip->SetActiveViewport(hwnd);
		vpt = ip->GetViewport(hwnd);
		snapPoint = vpt->SnapPoint (m, m, NULL);
		snapPoint = vpt->MapCPToWorld (snapPoint);
		hr = HitTestFaces (m, vpt);
		ProjectHitToFace (m, vpt, hr, &snapPoint);
		if (vpt) ip->ReleaseViewport(vpt);
		if (hr) FacePick ();
		break;

	case MOUSE_MOVE:
	case MOUSE_FREEMOVE:
		vpt = ip->GetViewport(hwnd);
		vpt->SnapPreview (m, m, NULL, SNAP_FORCE_3D_RESULT);
		if (HitTestFaces(m,vpt)) SetCursor(ip->GetSysCursor(SYSCUR_SELECT));
		else SetCursor(LoadCursor(NULL,IDC_ARROW));
		if (vpt) ip->ReleaseViewport(vpt);
		break;
	}

	return TRUE;
}

// -------------------------------------------------------

HitRecord *EditPolyConnectVertsMouseProc::HitTestVertices (IPoint2 & m, ViewExp *vpt) {
	vpt->ClearSubObjHitList();

	mpEPoly->SetHitLevelOverride (SUBHIT_MNVERTS);
	ip->SubObHitTest(ip->GetTime(),HITTYPE_POINT,0, 0, &m, vpt);
	mpEPoly->ClearHitLevelOverride ();
	if (!vpt->NumSubObjHits()) return NULL;

	HitLog& hitLog = vpt->GetSubObjHitList();
	HitRecord *bestHit = NULL;

	for (HitRecord *hr = hitLog.First(); hr; hr = hr->Next())
	{
		if (v1>-1)
		{
			// Check that this hit vertex is on the right mesh, and if it's a neighbor of v1.
			if (hr->nodeRef != mpEPoly->EpModGetPrimaryNode()) continue;
         int i;
			for (i=0; i<neighbors.Count(); i++) if (neighbors[i] == hr->hitInfo) break;
			if (i>=neighbors.Count()) continue;
		}
		else
		{
			// can't accept vertices on no faces.
			if (hr->modContext->localData == NULL) continue;
			EditPolyData *pEditData = (EditPolyData *) hr->modContext->localData;
			if (!pEditData->GetMeshOutput()) continue;	// Shouldn't happen.
			if (pEditData->GetMeshOutput()->vfac[hr->hitInfo].Count() == 0) continue;
		}
		if ((bestHit == NULL) || (bestHit->distance>hr->distance)) bestHit = hr;
	}
	return bestHit;
}

void EditPolyConnectVertsMouseProc::DrawDiag (HWND hWnd, const IPoint2 & m) {
	if (v1<0) return;


	IPoint2 points[2];
	points[0]  = m1;
	points[1]  = m;
	XORDottedPolyline(hWnd, 2, points);
}

void EditPolyConnectVertsMouseProc::SetV1 (int vv) {
	v1 = vv;
	neighbors.ZeroCount();
	MNMesh *pMesh = mpEPoly->EpModGetOutputMesh ();
	if (!pMesh) return;
	Tab<int> & vf = pMesh->vfac[vv];
	Tab<int> & ve = pMesh->vedg[vv];
	// Add to neighbors all the vertices that share faces (but no edges) with this one:
	int i,j,k;
	for (i=0; i<vf.Count(); i++) {
		MNFace & mf = pMesh->f[vf[i]];
		for (j=0; j<mf.deg; j++) {
			// Do not consider v1 a neighbor:
			if (mf.vtx[j] == v1) continue;

			// Filter out those that share an edge with v1:
			for (k=0; k<ve.Count(); k++) {
				if (pMesh->e[ve[k]].OtherVert (vv) == mf.vtx[j]) break;
			}
			if (k<ve.Count()) continue;

			// Add to neighbor list.
			neighbors.Append (1, &(mf.vtx[j]), 4);
		}
	}
}

int EditPolyConnectVertsMouseProc::proc (HWND hwnd, int msg, int point, int flags, IPoint2 m) {
	ViewExp *vpt = NULL;
	HitRecord *hr = NULL;
	Point3 snapPoint;

	switch (msg) {
	case MOUSE_INIT:
		v1 = v2 = -1;
		neighbors.ZeroCount ();
		break;

	case MOUSE_PROPCLICK:
		ip->PopCommandMode ();
		break;

	case MOUSE_POINT:
		ip->SetActiveViewport(hwnd);
		vpt = ip->GetViewport(hwnd);
		snapPoint = vpt->SnapPoint (m, m, NULL);
		snapPoint = vpt->MapCPToWorld (snapPoint);
		hr = HitTestVertices (m, vpt);
		ip->ReleaseViewport(vpt);
		if (!hr) break;
		if (v1<0) {
			mpEPoly->EpModSetPrimaryNode (hr->nodeRef);
			SetV1 (hr->hitInfo);
			m1 = m;
			lastm = m;
			DrawDiag (hwnd, m);
			break;
		}
		// Otherwise, we've found a connection.
		DrawDiag (hwnd, lastm);	// erase last dotted line.
		v2 = hr->hitInfo;
		VertConnect ();
		v1 = -1;
		return FALSE;	// Done with this connection.

	case MOUSE_MOVE:
	case MOUSE_FREEMOVE:
		vpt = ip->GetViewport(hwnd);
		vpt->SnapPreview (m, m, NULL, SNAP_FORCE_3D_RESULT);
		hr=HitTestVertices(m,vpt);
		if (hr!=NULL) SetCursor(ip->GetSysCursor(SYSCUR_SELECT));
		else SetCursor(LoadCursor(NULL,IDC_ARROW));
		if (vpt) ip->ReleaseViewport(vpt);
		DrawDiag (hwnd, lastm);
		DrawDiag (hwnd, m);
		lastm = m;
		break;
	}

	return TRUE;
}

// -------------------------------------------------------

static HCURSOR hCurCreateVert = NULL;

void EditPolyCreateVertCMode::EnterMode() {
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom);
	if (!hGeom) return;
	ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_CREATE));
	if (but) {
		but->SetCheck(TRUE);
		ReleaseICustButton(but);
	}
}

void EditPolyCreateVertCMode::ExitMode() {
	// Here's where we auto-commit to the "Create" operation.
	if (mpEditPoly->GetPolyOperationID () == ep_op_create)
	{
		theHold.Begin();
		mpEditPoly->EpModCommitUnlessAnimating (TimeValue(0));
		theHold.Accept (GetString (IDS_EDITPOLY_COMMIT));
	}

	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom);
	if (!hGeom) return;
	ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_CREATE));
	if (but) {
		but->SetCheck(FALSE);
		ReleaseICustButton(but);
	}
}

int EditPolyCreateVertMouseProc::proc (HWND hwnd, int msg, int point, int flags, IPoint2 m) {
	if (!hCurCreateVert) hCurCreateVert = LoadCursor(hInstance,MAKEINTRESOURCE(IDC_ADDVERTCUR)); 

	ViewExp *vpt = ip->GetViewport (hwnd);
	Matrix3 ctm, nodeTM;
	Point3 pt;
	IPoint2 m2;

	switch (msg) {
	case MOUSE_PROPCLICK:
		ip->PopCommandMode ();
		break;

	case MOUSE_POINT:
		ip->SetActiveViewport(hwnd);

		// Get the point in world-space:
		vpt->GetConstructionTM(ctm);
		pt = vpt->SnapPoint (m, m2, &ctm);
		pt = pt * ctm;

		// Put the point into object space:
		// (This chooses a primary node if one is not already chosen.)
		nodeTM = mpEPoly->EpModGetNodeTM (ip->GetTime());
		nodeTM.Invert ();
		pt = pt * nodeTM;

		// Set the operation in the mod and in the localdata, and add the vertex.
		theHold.Begin ();
		mpEPoly->EpModCreateVertex (pt);
		theHold.Accept (GetString (IDS_CREATE_VERTEX));

		mpEPoly->EpModRefreshScreen ();
		break;

	case MOUSE_FREEMOVE:
		SetCursor(hCurCreateVert);
		vpt->SnapPreview(m, m, NULL, SNAP_FORCE_3D_RESULT);
		break;
	}

	if (vpt) ip->ReleaseViewport(vpt);
	return TRUE;
}

//----------------------------------------------------------

void EditPolyCreateEdgeCMode::EnterMode () {
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom);
	if (hGeom) {
		ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_CREATE));
		but->SetCheck(TRUE);
		ReleaseICustButton(but);
	}
	mpEditPoly->SetDisplayLevelOverride (MNDISP_VERTTICKS);
	mpEditPoly->EpModLocalDataChanged (PART_DISPLAY);
	mpEditPoly->EpModRefreshScreen ();
}

void EditPolyCreateEdgeCMode::ExitMode() {
	// Here's where we auto-commit to the "Create" operation.
	if (mpEditPoly->GetPolyOperationID () == ep_op_create_edge)
	{
		theHold.Begin();
		mpEditPoly->EpModCommitUnlessAnimating (TimeValue(0));
		theHold.Accept (GetString (IDS_EDITPOLY_COMMIT));
	}

	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom);
	if (hGeom) {
		ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_CREATE));
		but->SetCheck (FALSE);
		ReleaseICustButton(but);
	}
	mpEditPoly->ClearDisplayLevelOverride ();
	mpEditPoly->EpModLocalDataChanged (PART_DISPLAY);
	mpEditPoly->EpModRefreshScreen ();
}

void EditPolyCreateEdgeMouseProc::VertConnect () {
	theHold.Begin();
	mpEPoly->EpModCreateEdge (v1, v2);
	theHold.Accept (GetString (IDS_CREATE_EDGE));
	mpEPoly->EpModRefreshScreen ();
}

//----------------------------------------------------------

void EditPolyCreateFaceCMode::EnterMode() {
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom);
	if (hGeom) {
		ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_CREATE));
		but->SetCheck(TRUE);
		ReleaseICustButton(but);
	}
	mpEditPoly->SetDisplayLevelOverride (MNDISP_VERTTICKS);
	mpEditPoly->SetHitTestResult ();
	mpEditPoly->EpModLocalDataChanged (PART_DISPLAY);
	mpEditPoly->EpModRefreshScreen ();
}

void EditPolyCreateFaceCMode::ExitMode() {
	// Here's where we auto-commit to the "Create" operation.
	if (mpEditPoly->GetPolyOperationID () == ep_op_create)
	{
		theHold.Begin();
		mpEditPoly->EpModCommitUnlessAnimating (TimeValue(0));
		theHold.Accept (GetString (IDS_EDITPOLY_COMMIT));
	}

	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom);
	if (hGeom) {
		ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_CREATE));
		but->SetCheck (FALSE);
		ReleaseICustButton(but);
	}
	mpEditPoly->ClearDisplayLevelOverride ();
	mpEditPoly->ClearHitTestResult ();
	mpEditPoly->EpModLocalDataChanged (PART_DISPLAY);
	mpEditPoly->EpModRefreshScreen ();
}

void EditPolyCreateFaceCMode::Display (GraphicsWindow *gw) {
	proc.DrawEstablishedFace(gw);
}

// -----------------------------------------
EditPolyCreateFaceMouseProc::EditPolyCreateFaceMouseProc( EPolyMod* in_e, IObjParam* in_i ) 
: CreateFaceMouseProcTemplate(in_i), m_EPoly(in_e) {
	if (!hCurCreateVert) hCurCreateVert = LoadCursor(hInstance,MAKEINTRESOURCE(IDC_ADDVERTCUR));
}

bool EditPolyCreateFaceMouseProc::CreateVertex(const Point3& in_worldLocation, int& out_vertex) {
	if (HasPrimaryNode()) {
		m_EPoly->EpModSetPrimaryNode(GetPrimaryNode());
	} else {
		m_EPoly->EpModSetPrimaryNode(0);
	}
	const Matrix3& l_world2Object = Inverse(m_EPoly->EpModGetPrimaryNode()->GetObjectTM(GetInterface()->GetTime()));
	out_vertex = m_EPoly->EpModCreateVertex(in_worldLocation * l_world2Object);
	return out_vertex >= 0;
}

bool EditPolyCreateFaceMouseProc::CreateFace(MaxSDK::Array<int>& in_vertices) {
	if (HasPrimaryNode()) {
		m_EPoly->EpModSetPrimaryNode(GetPrimaryNode());
	} else {
		m_EPoly->EpModSetPrimaryNode(0);
	}
	Tab<int> l_vertices;
	l_vertices.SetCount((int)in_vertices.length());
	for (int i = 0; i < in_vertices.length(); i++) l_vertices[i] = in_vertices[i];
	return m_EPoly->EpModCreateFace(&l_vertices) >= 0;
}

Point3 EditPolyCreateFaceMouseProc::GetVertexWP(int in_vertex)
{
	DbgAssert(m_EPoly->EpModGetPrimaryNode());
	MNMesh *pMesh = m_EPoly->EpModGetOutputMesh (m_EPoly->EpModGetPrimaryNode());
	
	if(pMesh&&in_vertex>=0&&in_vertex< pMesh->VNum())
		return m_EPoly->EpModGetPrimaryNode()->GetObjectTM(GetInterface()->GetTime()) * pMesh->P(in_vertex);
	return Point3(0.0f,0.0f,0.0f);
}

bool EditPolyCreateFaceMouseProc::HitTestVertex(IPoint2 in_mouse, ViewExp* in_viewport, INode*& out_node, int& out_vertex) {
	in_viewport->ClearSubObjHitList();

	// why SUBHIT_OPENONLY? In an MNMesh, an edge can belong to at most two faces. If two vertices are selected
	// in sequence that specify an edge that belongs to two faces, then a new face containing this subsequence 
	// of vertices cannot be created. Unfortunately, in this situation, EpfnCreateFace will return successfully 
	// but no new face will actually be created. To prevent this odd behaviour, only vertices on edges with less 
	// than two faces can be selected (i.e., vertices on "open" edges can be selected).
	m_EPoly->SetHitLevelOverride(SUBHIT_MNVERTS|SUBHIT_OPENONLY);
	//#error PW sees something wrong here ... need to figure out how this relates to faces only being created on first object
	GetInterface()->SubObHitTest(GetInterface()->GetTime(),HITTYPE_POINT,0, 0, &in_mouse, in_viewport);
	m_EPoly->ClearHitLevelOverride();

	HitRecord* l_bestHit = 0; 
	// Find the closest vertex on the primary node. If the primary node hasn't been specified, 
	// then use the closest hit from all nodes. 
	for (HitRecord *hr = in_viewport->GetSubObjHitList().First(); hr != 0; hr = hr->Next()) {
		if (HasPrimaryNode() && hr->nodeRef != GetPrimaryNode()) {
			continue;
		}
		if (l_bestHit == 0 || l_bestHit->distance > hr->distance) {
			l_bestHit = hr;
		}
	}

	if (l_bestHit != 0) {
		out_vertex = l_bestHit->hitInfo;
		out_node = l_bestHit->nodeRef;
	}
	return l_bestHit != 0;
}

void EditPolyCreateFaceMouseProc::ShowCreateVertexCursor() {
	SetCursor(hCurCreateVert);
}

//-----------------------------------------------------------------------/

BOOL EditPolyAttachPickMode::Filter(INode *node) {
	if (!mpEditPoly) return false;
	if (!ip) return false;
	if (!node) return FALSE;

	// Make sure the node does not depend on us
	Modifier *pMod = mpEditPoly->GetModifier();
	node->BeginDependencyTest();
	pMod->NotifyDependents(FOREVER,0,REFMSG_TEST_DEPENDENCY);
	if (node->EndDependencyTest()) return FALSE;

	ObjectState os = node->GetObjectRef()->Eval(ip->GetTime());
	if (os.obj->IsSubClassOf(polyObjectClassID)) return TRUE;
	if (os.obj->CanConvertToType(polyObjectClassID)) return TRUE;
	return FALSE;
}

BOOL EditPolyAttachPickMode::HitTest(IObjParam *ip, HWND hWnd, ViewExp *vpt, IPoint2 m,int flags) {
	if (!mpEditPoly) return false;
	if (!ip) return false;
	return ip->PickNode(hWnd,m,this) ? TRUE : FALSE;
}

// Note: we always return false, because we always want to be done picking.
BOOL EditPolyAttachPickMode::Pick(IObjParam *ip,ViewExp *vpt) {
	if (!mpEditPoly) return false;
	INode *node = vpt->GetClosestHit();
	if (!Filter(node)) return false;

	INode *pMyNode = mpEditPoly->EpModGetPrimaryNode();
	BOOL ret = TRUE;
	if (pMyNode->GetMtl() && node->GetMtl() && (pMyNode->GetMtl() != node->GetMtl())) {
		ret = DoAttachMatOptionDialog (ip, mpEditPoly);
	}
	if (!ret) return false;

	theHold.Begin ();
	mpEditPoly->EpModAttach (node, pMyNode, ip->GetTime());
	theHold.Accept (GetString (IDS_ATTACH));
	mpEditPoly->EpModRefreshScreen ();
	return false;
}

void EditPolyAttachPickMode::EnterMode(IObjParam *ip) {
	if (!mpEditPoly) return;
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom);
	if (!hGeom) return;
	ICustButton *but = GetICustButton (GetDlgItem (hGeom, IDC_ATTACH));
	but->SetCheck(TRUE);
	ReleaseICustButton(but);
}

void EditPolyAttachPickMode::ExitMode(IObjParam *ip) {
	if (!mpEditPoly) return;

	// If we're still in Attach mode, and the popup dialog isn't engaged, leave attach mode:
	if ((mpEditPoly->GetPolyOperationID() == ep_op_attach) && !mpEditPoly->EpModShowingOperationDialog()) {
		theHold.Begin ();
		mpEditPoly->EpModCommitUnlessAnimating (ip->GetTime());
		theHold.Accept (GetString (IDS_EDITPOLY_COMMIT));
	}

	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom);
	if (!hGeom) return;
	ICustButton *but = GetICustButton (GetDlgItem (hGeom, IDC_ATTACH));
	but->SetCheck(FALSE);
	ReleaseICustButton(but);
}

// -----------------------------

int EditPolyAttachHitByName::filter(INode *node) {
	if (!node) return FALSE;

	// Make sure the node does not depend on this modifier.
	node->BeginDependencyTest();
	mpEditPoly->NotifyDependents(FOREVER,0,REFMSG_TEST_DEPENDENCY);
	if (node->EndDependencyTest()) return FALSE;

	ObjectState os = node->GetObjectRef()->Eval(ip->GetTime());
	if (os.obj->IsSubClassOf(polyObjectClassID)) return TRUE;
	if (os.obj->CanConvertToType(polyObjectClassID)) return TRUE;
	return FALSE;
}

void EditPolyAttachHitByName::proc (INodeTab & nodeTab) {
	INode *pMyNode = mpEditPoly->EpModGetPrimaryNode();
	BOOL ret = TRUE;
	if (pMyNode->GetMtl()) {
      int i;
		for (i=0; i<nodeTab.Count(); i++) {
			if (nodeTab[i]->GetMtl() && (pMyNode->GetMtl()!=nodeTab[i]->GetMtl())) break;
		}
		if (i<nodeTab.Count()) ret = DoAttachMatOptionDialog (ip, mpEditPoly);
	}
	if (!ret) return;

	theHold.Begin();
	mpEditPoly->EpModMultiAttach (nodeTab, pMyNode, ip->GetTime ());
	theHold.Accept (GetString (IDS_ATTACH_LIST));
}

//--- EditPolyShapePickMode ---------------------------------

BOOL EditPolyShapePickMode::HitTest(IObjParam *ip, HWND hWnd, ViewExp *vpt, IPoint2 m,int flags) {
	if (!mpMod) return false;
	return ip->PickNode(hWnd,m,this) ? TRUE : FALSE;
}

BOOL EditPolyShapePickMode::Pick(IObjParam *ip,ViewExp *vpt) {
	if (!mpMod) return false;
	INode *node = vpt->GetClosestHit();
	if (!Filter(node)) return false;

	Matrix3 world2obj(true);
	ModContextList list;
	INodeTab nodes;
	ip->GetModContexts (list, nodes);
	if (list.Count() > 0) {
		Interval foo(FOREVER);
		Matrix3 obj2world = nodes[0]->GetObjectTM (ip->GetTime(), &foo);
		Matrix3 mctm = *(list[0]->tm);
		world2obj = Inverse (obj2world);
		world2obj = world2obj * mctm;
	}

	bool inPopup = (mpMod->GetPolyOperationID() == ep_op_extrude_along_spline) && mpMod->EpModShowingOperationDialog();

	if (!inPopup) {
		theHold.Begin ();
		mpMod->EpModSetOperation (ep_op_extrude_along_spline);
		mpMod->getParamBlock()->SetValue (epm_extrude_spline_node, ip->GetTime(), node);
		mpMod->getParamBlock()->SetValue (epm_world_to_object_transform, ip->GetTime(), world2obj);
		mpMod->EpModCommitUnlessAnimating (ip->GetTime());
		theHold.Accept (GetString (IDS_EXTRUDE_ALONG_SPLINE));
	} else {
		theHold.Begin ();
		mpMod->getParamBlock()->SetValue (epm_extrude_spline_node, ip->GetTime(), node);
		mpMod->getParamBlock()->SetValue (epm_world_to_object_transform, ip->GetTime(), world2obj);
		theHold.Accept (GetString (IDS_EDITPOLY_PICK_SPLINE));
	}
	return true;
}

void EditPolyShapePickMode::EnterMode(IObjParam *ip) {
	mpMod->RegisterCMChangedForGrip();
	ICustButton *but = NULL;
	HWND hWnd = mpMod->GetDlgHandle (ep_settings);
	if (hWnd && (but = GetICustButton (GetDlgItem (hWnd, IDC_EXTRUDE_PICK_SPLINE))) != NULL) {
		but->SetCheck(true);
		ReleaseICustButton(but);
		but = NULL;
	}

	hWnd = mpMod->GetDlgHandle (ep_subobj);
	if (hWnd && (but = GetICustButton (GetDlgItem (hWnd, IDC_EXTRUDE_ALONG_SPLINE))) != NULL) {
		but->SetCheck(true);
		ReleaseICustButton(but);
		but = NULL;
	}
}

void EditPolyShapePickMode::ExitMode(IObjParam *ip) {
	ICustButton *but = NULL;
	HWND hWnd = mpMod->GetDlgHandle (ep_settings);
	if (hWnd && (but = GetICustButton (GetDlgItem (hWnd, IDC_EXTRUDE_PICK_SPLINE))) != NULL) {
		but->SetCheck(false);
		ReleaseICustButton(but);
		but = NULL;
	}

	hWnd = mpMod->GetDlgHandle (ep_subobj);
	if (hWnd && (but = GetICustButton (GetDlgItem (hWnd, IDC_EXTRUDE_ALONG_SPLINE))) != NULL) {
		but->SetCheck(false);
		ReleaseICustButton(but);
		but = NULL;
	}
}

BOOL EditPolyShapePickMode::Filter(INode *node) {
	if (!mpMod) return false;
	if (!ip) return false;
	if (!node) return false;

	// Make sure the node does not depend on us
	Modifier *pMod = mpMod->GetModifier();
	node->BeginDependencyTest();
	pMod->NotifyDependents(FOREVER,0,REFMSG_TEST_DEPENDENCY);
	if (node->EndDependencyTest()) return FALSE;

	ObjectState os = node->GetObjectRef()->Eval(ip->GetTime());
	if (os.obj->IsSubClassOf(splineShapeClassID)) return true;
	if (os.obj->CanConvertToType(splineShapeClassID)) return true;
	return false;
}


//----------------------------------------------------------

// Divide edge modifies two faces; creates a new vertex and a new edge.
bool EditPolyDivideEdgeProc::EdgePick (HitRecord *hr, float prop) {
	theHold.Begin ();
	mpEPoly->EpModDivideEdge (hr->hitInfo, prop, hr->nodeRef);
	theHold.Accept (GetString (IDS_INSERT_VERTEX));
	mpEPoly->EpModRefreshScreen ();
	return true;	// false = exit mode when done; true = stay in mode.
}

void EditPolyDivideEdgeCMode::EnterMode() {
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_subobj);
	if (hGeom) {
		ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_INSERT_VERTEX));
		but->SetCheck(TRUE);
		ReleaseICustButton(but);
	}
	mpEditPoly->SetDisplayLevelOverride (MNDISP_VERTTICKS);
	mpEditPoly->EpModLocalDataChanged (PART_DISPLAY);
	mpEditPoly->EpModRefreshScreen();
}

void EditPolyDivideEdgeCMode::ExitMode() {
	if (mpEditPoly->GetPolyOperationID() == ep_op_insert_vertex_edge) {
		// This is where we commit, if we did any face dividing.
		theHold.Begin ();
		mpEditPoly->EpModCommit (mpEditPoly->ip->GetTime(), true, true);
		theHold.Accept (GetString (IDS_EDITPOLY_COMMIT));
	}

	HWND hGeom = mpEditPoly->GetDlgHandle (ep_subobj);
	if (hGeom) {
		ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_INSERT_VERTEX));
		but->SetCheck(FALSE);
		ReleaseICustButton(but);
	}
	mpEditPoly->ClearDisplayLevelOverride ();
	mpEditPoly->EpModLocalDataChanged (PART_DISPLAY);
	mpEditPoly->EpModRefreshScreen();
}

//----------------------------------------------------------

void EditPolyDivideFaceProc::FacePick () {
	theHold.Begin ();
	mpEPoly->EpModDivideFace (face, &bary);
	theHold.Accept (GetString (IDS_INSERT_VERTEX));
	mpEPoly->EpModRefreshScreen ();
}

void EditPolyDivideFaceCMode::EnterMode() {
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_subobj);
	if (hGeom) {
		ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_INSERT_VERTEX));
		if (but) {
			but->SetCheck(TRUE);
			ReleaseICustButton(but);
		}
	}
	mpEditPoly->SetDisplayLevelOverride (MNDISP_VERTTICKS);
	mpEditPoly->EpModLocalDataChanged (PART_DISPLAY);
	mpEditPoly->EpModRefreshScreen();
}

void EditPolyDivideFaceCMode::ExitMode() {
	if (mpEditPoly->GetPolyOperationID() == ep_op_insert_vertex_face) {
		// This is where we commit, if we did any face dividing.
		theHold.Begin ();
		mpEditPoly->EpModCommit (mpEditPoly->ip->GetTime(), true, true);
		theHold.Accept (GetString (IDS_EDITPOLY_COMMIT));
	}

	HWND hGeom = mpEditPoly->GetDlgHandle (ep_subobj);
	if (!hGeom) return;
	ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_INSERT_VERTEX));
	but->SetCheck(FALSE);
	ReleaseICustButton(but);
	mpEditPoly->ClearDisplayLevelOverride ();
	mpEditPoly->EpModLocalDataChanged (PART_DISPLAY);
	mpEditPoly->EpModRefreshScreen();
}

// ------------------------------------------------------

HitRecord *EditPolyBridgeMouseProc::HitTest (IPoint2 & m, ViewExp *vpt) {
	vpt->ClearSubObjHitList();

	mpEPoly->SetHitLevelOverride (mHitLevel);
	ip->SubObHitTest(ip->GetTime(),HITTYPE_POINT,0, 0, &m, vpt);
	mpEPoly->ClearHitLevelOverride ();
	if (!vpt->NumSubObjHits()) return NULL;

	HitLog& hitLog = vpt->GetSubObjHitList();
	HitRecord *bestHit = NULL;

	for (HitRecord *hr = hitLog.First(); hr; hr = hr->Next())
	{
		if (hit1>-1) {
			// The only requirement we have for the second hit
			// is that it's not the same as the first.
			if (hr->hitInfo == hit1) continue;
		}
		if ((bestHit == NULL) || (bestHit->distance>hr->distance)) bestHit = hr;
	}
	return bestHit;
}

void EditPolyBridgeMouseProc::DrawDiag (HWND hWnd, const IPoint2 & m) {
	if (hit1<0) return;

	IPoint2 points[2];
	points[0]  = m1;
	points[1]  = m;
	XORDottedPolyline(hWnd, 2, points);

}

int EditPolyBridgeMouseProc::proc (HWND hwnd, int msg, int point, int flags, IPoint2 m) {
	ViewExp* vpt = NULL;
	HitRecord* hr = NULL;
	Point3 snapPoint(0.0f,0.0f,0.0f);

	switch (msg) {
	case MOUSE_INIT:
		hit1 = hit2 = -1;
		break;

	case MOUSE_PROPCLICK:
		ip->PopCommandMode ();
		break;

	case MOUSE_POINT:
		if (point==1) break;

		ip->SetActiveViewport(hwnd);
		vpt = ip->GetViewport(hwnd);
		snapPoint = vpt->SnapPoint (m, m, NULL);
		snapPoint = vpt->MapCPToWorld (snapPoint);
		hr = HitTest (m, vpt);
		ip->ReleaseViewport(vpt);
		if (!hr) break;
		if (hit1<0) {
			mpEPoly->EpModSetPrimaryNode (hr->nodeRef);
			hit1 = hr->hitInfo;
			m1 = m;
			lastm = m;
			DrawDiag (hwnd, m);
			break;
		}
		// Otherwise, we've found a connection.
		DrawDiag (hwnd, lastm);	// erase last dotted line.
		hit2 = hr->hitInfo;
		Bridge ();
		hit1 = -1;
		return FALSE;	// Done with this connection.

	case MOUSE_MOVE:
	case MOUSE_FREEMOVE:
		vpt = ip->GetViewport(hwnd);
		vpt->SnapPreview (m, m, NULL, SNAP_FORCE_3D_RESULT);
		hr=HitTest(m,vpt);
		if (hr!=NULL) SetCursor(ip->GetSysCursor(SYSCUR_SELECT));
		else SetCursor(LoadCursor(NULL,IDC_ARROW));
		if (vpt) ip->ReleaseViewport(vpt);
		DrawDiag (hwnd, lastm);
		DrawDiag (hwnd, m);
		lastm = m;
		break;
	}

	return TRUE;
}

void EditPolyBridgeBorderProc::Bridge () {
	theHold.Begin ();
	mpEPoly->EpModBridgeBorders (hit1, hit2);
	mpEPoly->EpModCommitUnlessAnimating (ip->GetTime());
	theHold.Accept (GetString (IDS_BRIDGE_BORDERS));
	mpEPoly->EpModRefreshScreen ();
}

void EditPolyBridgeBorderCMode::EnterMode () {
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_subobj);
	if (hGeom) {
		ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_BRIDGE));
		if (but) {
			but->SetCheck(TRUE);
			ReleaseICustButton(but);
		}
	}
}

void EditPolyBridgeBorderCMode::ExitMode () {
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_subobj);
	if (hGeom) {
		ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_BRIDGE));
		if (but) {
			but->SetCheck (false);
			ReleaseICustButton(but);
		}
	}
}

// ------------------------------------------------------
void EditPolyBridgeEdgeProc::Bridge () {
	theHold.Begin ();
	mpEPoly->EpModBridgeEdges (hit1, hit2);
	mpEPoly->EpModCommitUnlessAnimating (ip->GetTime());
	theHold.Accept (GetString (IDS_BRIDGE_EDGES));
	mpEPoly->EpModRefreshScreen ();
}

void EditPolyBridgeEdgeCMode::EnterMode () {
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_subobj);
	if (hGeom) {
		ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_BRIDGE));
		if (but) {
			but->SetCheck(TRUE);
			ReleaseICustButton(but);
		}
	}
}

void EditPolyBridgeEdgeCMode::ExitMode () {
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_subobj);
	if (hGeom) {
		ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_BRIDGE));
		if (but) {
			but->SetCheck (false);
			ReleaseICustButton(but);
		}
	}
}


// ------------------------------------------------------

void EditPolyBridgePolygonProc::Bridge () {
	theHold.Begin ();
	mpEPoly->EpModBridgePolygons (hit1, hit2);
	mpEPoly->EpModCommitUnlessAnimating (ip->GetTime());
	theHold.Accept (GetString (IDS_BRIDGE_POLYGONS));
	mpEPoly->EpModRefreshScreen ();
}

void EditPolyBridgePolygonCMode::EnterMode () {
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_subobj);
	if (hGeom) {
		ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_BRIDGE));
		if (but) {
			but->SetCheck(TRUE);
			ReleaseICustButton(but);
		}
	}
}

void EditPolyBridgePolygonCMode::ExitMode () {
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_subobj);
	if (hGeom) {
		ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_BRIDGE));
		if (but) {
			but->SetCheck (false);
			ReleaseICustButton(but);
		}
	}
}

// ------------------------------------------------------

void EditPolyExtrudeProc::BeginExtrude (TimeValue t)
{
	if (mInExtrude) return;
	mInExtrude = true;

	theHold.Begin ();
	int type = mpEditPoly->GetPolyOperationID ();
	if (type != ep_op_extrude_face) {
		mpEditPoly->EpModSetOperation (ep_op_extrude_face);
		mStartHeight = 0.0f;

		SuspendAnimate();
		AnimateOff();
		SuspendSetKeyMode(); // AF (5/13/03)
		mpEditPoly->getParamBlock()->SetValue (epm_extrude_face_height, t, 0.0f);
		ResumeSetKeyMode();
		ResumeAnimate ();
	} else {
		mStartHeight = mpEditPoly->getParamBlock()->GetFloat (epm_extrude_face_height, t);
	}
}

void EditPolyExtrudeProc::EndExtrude (TimeValue t, bool accept)
{
	if (!mInExtrude) return;
	mInExtrude = FALSE;

	if (accept)
	{
		mpEditPoly->EpModCommitUnlessAnimating (t);
		theHold.Accept (GetString(IDS_EXTRUDE));

		if (mpEditPoly->getParamBlock()->GetInt(epm_animation_mode)) {
			mpEditPoly->SetHitTestResult (true);
		}
	}
	else theHold.Cancel ();
}

void EditPolyExtrudeProc::DragExtrude (TimeValue t, float value)
{
	if (!mInExtrude) return;
	mpEditPoly->getParamBlock()->SetValue (epm_extrude_face_height, t, value+mStartHeight);
}

int EditPolyExtrudeProc::proc (HWND hwnd, int msg, int point, int flags, IPoint2 m ) {	
	ViewExp *vpt=ip->GetViewport (hwnd);
	Point3 p0, p1;
	IPoint2 m2;
	float height;

	switch (msg) {
	case MOUSE_PROPCLICK:
		ip->PopCommandMode ();
		break;

	case MOUSE_POINT:
		if (!point) {
			BeginExtrude (ip->GetTime());
			om = m;
		} else {
			ip->RedrawViews(ip->GetTime(),REDRAW_END);
			EndExtrude (ip->GetTime(), true);
		}
		break;

	case MOUSE_MOVE:
		p0 = vpt->MapScreenToView (om,float(-200));
		m2.x = om.x;
		m2.y = m.y;
		p1 = vpt->MapScreenToView (m2,float(-200));
		height = Length (p1-p0);
		if (m.y > om.y) height *= -1.0f;
		DragExtrude (ip->GetTime(), height);
		ip->RedrawViews(ip->GetTime(),REDRAW_INTERACTIVE);
		break;

	case MOUSE_ABORT:
		EndExtrude (ip->GetTime (), false);
		ip->RedrawViews (ip->GetTime(), REDRAW_END);
		break;
	}

	if (vpt) ip->ReleaseViewport(vpt);
	return TRUE;
}

HCURSOR EditPolyExtrudeSelectionProcessor::GetTransformCursor() { 
	static HCURSOR hCur = NULL;
	if ( !hCur ) hCur = LoadCursor(hInstance,MAKEINTRESOURCE(IDC_EXTRUDECUR));
	return hCur; 
}

void EditPolyExtrudeCMode::EnterMode() {
	if (mpEditPoly->GetPolyOperationID() == ep_op_extrude_face) {
		mpEditPoly->SetHitTestResult (true);
	} else {
		mpEditPoly->SetHitTestResult ();
	}

	HWND hGeom = mpEditPoly->GetDlgHandle (ep_subobj);
	if (!hGeom) return;
	ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_EXTRUDE));
	if (but) {
		but->SetCheck (true);
		ReleaseICustButton(but);
	} else {
		DebugPrint ("Edit Poly: we're entering Extrude mode, but we can't find the extrude button!\n");
		DbgAssert (0);
	}
}

void EditPolyExtrudeCMode::ExitMode() {
	mpEditPoly->ClearHitTestResult();

	HWND hGeom = mpEditPoly->GetDlgHandle (ep_subobj);
	if (!hGeom) return;
	ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_EXTRUDE));
	if (but) {
		but->SetCheck(FALSE);
		ReleaseICustButton(but);
	} else {
		DebugPrint ("Edit Poly: we're exiting Extrude mode, but we can't find the extrude button!\n");
		DbgAssert (0);
	}
}

// ------------------------------------------------------

void EditPolyExtrudeVEMouseProc::BeginExtrude (TimeValue t)
{
	if (mInExtrude) return;
	mInExtrude = true;

	int type = mpEditPoly->GetPolyOperationID ();
	theHold.Begin ();

	switch (mpEditPoly->GetMNSelLevel()) {
	case MNM_SL_VERTEX:
		if (type != ep_op_extrude_vertex) {
			mpEditPoly->EpModSetOperation (ep_op_extrude_vertex);
			mpEditPoly->getParamBlock()->SetValue (epm_extrude_vertex_height, TimeValue(0), 0.0f);
			mpEditPoly->getParamBlock()->SetValue (epm_extrude_vertex_width, TimeValue(0), 0.0f);
			mStartHeight = 0.0f;
			mStartWidth = 0.0f;
		} else {
			mStartHeight = mpEditPoly->getParamBlock()->GetFloat (epm_extrude_vertex_height);
			mStartWidth = mpEditPoly->getParamBlock()->GetFloat (epm_extrude_vertex_width);
		}
		break;

	case MNM_SL_EDGE:
		if (type != ep_op_extrude_edge) {
			mpEditPoly->EpModSetOperation (ep_op_extrude_edge);
			mpEditPoly->getParamBlock()->SetValue (epm_extrude_edge_height, TimeValue(0), 0.0f);
			mpEditPoly->getParamBlock()->SetValue (epm_extrude_edge_width, TimeValue(0), 0.0f);
			mStartHeight = 0.0f;
			mStartWidth = 0.0f;
		} else {
			mStartHeight = mpEditPoly->getParamBlock()->GetFloat (epm_extrude_edge_height);
			mStartWidth = mpEditPoly->getParamBlock()->GetFloat (epm_extrude_edge_width);
		}
		break;

	default:
		mInExtrude = false;
		theHold.Cancel ();
		break;
	}
}

void EditPolyExtrudeVEMouseProc::EndExtrude (TimeValue t, bool accept)
{
	if (!mInExtrude) return;
	mInExtrude = FALSE;

	if (accept)
	{
		mpEditPoly->EpModCommitUnlessAnimating (ip->GetTime());
		switch (mpEditPoly->GetMNSelLevel()) {
			case MNM_SL_VERTEX:
				theHold.Accept (GetString (IDS_EDITPOLY_EXTRUDE_VERTEX));
				break;
			case MNM_SL_EDGE:
				theHold.Accept (GetString (IDS_EDITPOLY_EXTRUDE_EDGE));
				break;
		}

		if (mpEditPoly->getParamBlock()->GetInt(epm_animation_mode)) {
			mpEditPoly->SetHitTestResult (true);
		}
	}
	else theHold.Cancel ();
}

void EditPolyExtrudeVEMouseProc::DragExtrude (TimeValue t, float height, float width)
{
	if (!mInExtrude) return;

	height += mStartHeight;
	width += mStartWidth;
	if (width<0) width=0;

	switch (mpEditPoly->GetMNSelLevel()) {
	case MNM_SL_VERTEX:
		mpEditPoly->getParamBlock()->SetValue (epm_extrude_vertex_width, ip->GetTime(), width);
		mpEditPoly->getParamBlock()->SetValue (epm_extrude_vertex_height, ip->GetTime(), height);
		break;
	case MNM_SL_EDGE:
		mpEditPoly->getParamBlock()->SetValue (epm_extrude_edge_width, ip->GetTime(), width);
		mpEditPoly->getParamBlock()->SetValue (epm_extrude_edge_height, ip->GetTime(), height);
		break;
	}
}

int EditPolyExtrudeVEMouseProc::proc (HWND hwnd, int msg, int point, int flags, IPoint2 m ) {
	ViewExp *vpt=ip->GetViewport (hwnd);
	Point3 p0, p1, p2;
	IPoint2 m1, m2;
	float width, height;

	switch (msg) {
	case MOUSE_PROPCLICK:
		if (mInExtrude) EndExtrude (ip->GetTime(), false);
		ip->PopCommandMode ();
		break;

	case MOUSE_POINT:
		switch (point) {
		case 0:
			m0 = m;
			BeginExtrude (ip->GetTime());
			break;
		case 1:
			EndExtrude (ip->GetTime(), true);
			ip->RedrawViews(ip->GetTime());
			break;
		}
		break;

	case MOUSE_MOVE:
		if (!mInExtrude) break;
		p0 = vpt->MapScreenToView (m0, float(-200));
		m1.x = m.x;
		m1.y = m0.y;
		p1 = vpt->MapScreenToView (m1, float(-200));
		m2.x = m0.x;
		m2.y = m.y;
		p2 = vpt->MapScreenToView (m2, float(-200));
		width = Length (p0 - p1);
		height = Length (p0 - p2);
		if (m.y > m0.y) height *= -1.0f;
		if ((mStartWidth>0) && (m.x < m0.x)) width = -width;

		DragExtrude (ip->GetTime(), height, width);
		ip->RedrawViews(ip->GetTime(),REDRAW_INTERACTIVE);
		break;

	case MOUSE_ABORT:
		if (mInExtrude) EndExtrude (ip->GetTime(), false);
		ip->RedrawViews (ip->GetTime(), REDRAW_END);
		break;
	}

	if (vpt) ip->ReleaseViewport(vpt);
	return TRUE;
}

HCURSOR EditPolyExtrudeVESelectionProcessor::GetTransformCursor() { 
	static HCURSOR hCur = NULL;
	if (!hCur) {
		if (mpEPoly->GetMNSelLevel() == MNM_SL_EDGE) hCur = LoadCursor(hInstance,MAKEINTRESOURCE(IDC_EXTRUDE_EDGE_CUR));
		else hCur = LoadCursor(hInstance,MAKEINTRESOURCE(IDC_EXTRUDE_VERTEX_CUR));
	}
	return hCur; 
}

void EditPolyExtrudeVECMode::EnterMode() {
	if ((mpEditPoly->GetPolyOperationID() == ep_op_extrude_vertex) ||
		(mpEditPoly->GetPolyOperationID() == ep_op_extrude_edge)) {
		mpEditPoly->SetHitTestResult (true);
	}

	HWND hGeom = mpEditPoly->GetDlgHandle (ep_subobj);
	if (!hGeom) return;
	ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_EXTRUDE));
	but->SetCheck(TRUE);
	ReleaseICustButton(but);
}

void EditPolyExtrudeVECMode::ExitMode() {
	mpEditPoly->ClearHitTestResult();

	HWND hGeom = mpEditPoly->GetDlgHandle (ep_subobj);
	if (!hGeom) return;
	ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_EXTRUDE));
	but->SetCheck(FALSE);
	ReleaseICustButton(but);
}

// ------------------------------------------------------

void EditPolyBevelMouseProc::Begin (TimeValue t)
{
	if (mInBevel) return;
	mInBevel = true;

	theHold.Begin ();
	int type = mpEditPoly->GetPolyOperationID ();
	if (type != ep_op_bevel) {
		mStartHeight = 0.0f;
		mStartOutline = 0.0f;
		mpEditPoly->EpModSetOperation (ep_op_bevel);

		SuspendAnimate();
		AnimateOff();
		SuspendSetKeyMode(); // AF (5/13/03)
		mpEditPoly->getParamBlock()->SetValue (epm_bevel_height, TimeValue(0), 0.0f);
		mpEditPoly->getParamBlock()->SetValue (epm_bevel_outline, TimeValue(0), 0.0f);
		ResumeSetKeyMode();
		ResumeAnimate ();
	} else {
		mStartHeight = mpEditPoly->getParamBlock()->GetFloat (epm_bevel_height, t);
		mStartOutline = mpEditPoly->getParamBlock()->GetFloat (epm_bevel_outline, t);
	}
}

void EditPolyBevelMouseProc::End (TimeValue t, bool accept)
{
	if (!mInBevel) return;
	mInBevel = FALSE;

	if (accept)
	{
		mpEditPoly->EpModCommitUnlessAnimating (t);
		theHold.Accept (GetString(IDS_BEVEL));

		if (mpEditPoly->getParamBlock()->GetInt(epm_animation_mode)) {
			mpEditPoly->SetHitTestResult (true);
		}
	}
	else theHold.Cancel ();
}

void EditPolyBevelMouseProc::DragHeight (TimeValue t, float height)
{
	if (!mInBevel) return;
	mpEditPoly->getParamBlock()->SetValue (epm_bevel_height, t, height+mStartHeight);
}

void EditPolyBevelMouseProc::DragOutline (TimeValue t, float outline)
{
	if (!mInBevel) return;
	mpEditPoly->getParamBlock()->SetValue (epm_bevel_outline, t, outline+mStartOutline);
}

int EditPolyBevelMouseProc::proc (HWND hwnd, int msg, int point, int flags, IPoint2 m ) {	
	
	ViewExp *vpt=ip->GetViewport (hwnd);
	Point3 p0, p1;
	IPoint2 m2;

	switch (msg) {
	case MOUSE_PROPCLICK:
		ip->PopCommandMode ();
		break;

	case MOUSE_POINT:
		switch (point) {
		case 0:
			m0 = m;
			m0set = TRUE;
			// Flag ignored in Edit Poly.
			Begin (ip->GetTime());
			break;
		case 1:
			m1 = m;
			m1set = TRUE;
			p0 = vpt->MapScreenToView (m0, float(-200));
			m2.x = m0.x;
			m2.y = m.y;
			p1 = vpt->MapScreenToView (m2, float(-200));
			height = Length (p0-p1);
			if (m1.y > m0.y) height *= -1.0f;
			DragHeight (ip->GetTime(), height);
			break;
		case 2:
			ip->RedrawViews(ip->GetTime(),REDRAW_END);
			End (ip->GetTime(), true);
			m1set = m0set = FALSE;
			break;
		}
		break;

	case MOUSE_MOVE:
		if (!m0set) break;
		m2.y = m.y;
		if (!m1set) {
			p0 = vpt->MapScreenToView (m0, float(-200));
			m2.x = m0.x;
			p1 = vpt->MapScreenToView (m2, float(-200));
			height = Length (p1-p0);
			if (m.y > m0.y) height *= -1.0f;
			DragHeight (ip->GetTime(), height);
		} else {
			p0 = vpt->MapScreenToView (m1, float(-200));
			m2.x = m1.x;
			p1 = vpt->MapScreenToView (m2, float(-200));
			float outline = Length (p1-p0);
			if (m.y > m1.y) outline *= -1.0f;
			DragOutline (ip->GetTime(), outline);
		}

		ip->RedrawViews(ip->GetTime(),REDRAW_INTERACTIVE);
		break;

	case MOUSE_ABORT:
		End (ip->GetTime(), false);
		ip->RedrawViews(ip->GetTime(),REDRAW_END);
		m1set = m0set = FALSE;
		break;
	}

	if (vpt) ip->ReleaseViewport(vpt);
	return TRUE;
}

HCURSOR EditPolyBevelSelectionProcessor::GetTransformCursor() { 
	static HCURSOR hCur = NULL;
	if ( !hCur ) hCur = LoadCursor(hInstance,MAKEINTRESOURCE(IDC_BEVELCUR));
	return hCur;
}

void EditPolyBevelCMode::EnterMode() {
	if (mpEditPoly->GetPolyOperationID() == ep_op_bevel) {
		mpEditPoly->SetHitTestResult (true);
	} else {
		mpEditPoly->SetHitTestResult ();
	}

	HWND hGeom = mpEditPoly->GetDlgHandle (ep_subobj);
	if (!hGeom) return;
	ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_BEVEL));
	but->SetCheck(TRUE);
	ReleaseICustButton(but);
}

void EditPolyBevelCMode::ExitMode() {
	mpEditPoly->ClearHitTestResult ();

	HWND hGeom = mpEditPoly->GetDlgHandle (ep_subobj);
	if (!hGeom) return;
	ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_BEVEL));
	but->SetCheck(FALSE);
	ReleaseICustButton(but);
}







// ------------------------------------------------------

void EditPolyInsetMouseProc::Begin (TimeValue t)
{
	if (mInInset) return;
	mInInset = true;

	theHold.Begin ();
	int type = mpEditPoly->GetPolyOperationID ();
	if (type != ep_op_inset) {
		mpEditPoly->EpModSetOperation (ep_op_inset);
		mStartAmount = 0.0f;

		SuspendAnimate();
		AnimateOff();
		SuspendSetKeyMode(); // AF (5/13/03)
		mpEditPoly->getParamBlock()->SetValue (epm_inset, t, 0.0f);
		ResumeSetKeyMode();
		ResumeAnimate ();
	} else {
		mStartAmount = mpEditPoly->getParamBlock()->GetFloat (epm_inset, t);
	}
}

void EditPolyInsetMouseProc::End (TimeValue t, bool accept)
{
	if (!mInInset) return;
	mInInset = FALSE;

	if (accept)
	{
		mpEditPoly->EpModCommitUnlessAnimating (t);
		theHold.Accept (GetString(IDS_INSET));

		if (mpEditPoly->getParamBlock()->GetInt(epm_animation_mode)) {
			mpEditPoly->SetHitTestResult (true);
		}
	}
	else theHold.Cancel ();
}

void EditPolyInsetMouseProc::Drag (TimeValue t, float amount)
{
	if (!mInInset) return;
	amount += mStartAmount;
	if (amount<0) amount=0;
	mpEditPoly->getParamBlock()->SetValue (epm_inset, t, amount);
}

int EditPolyInsetMouseProc::proc (HWND hwnd, int msg, int point, int flags, IPoint2 m ) {
	ViewExp *vpt=ip->GetViewport (hwnd);
	Point3 p0, p1;
	IPoint2 m2;
	ISpinnerControl *spin=NULL;
	float inset;

	switch (msg) {
	case MOUSE_PROPCLICK:
		ip->PopCommandMode ();
		break;

	case MOUSE_POINT:
		switch (point) {
		case 0:
			m0 = m;
			m0set = TRUE;
			// Flag ignored in Edit Poly.
			Begin (ip->GetTime());
			break;
		case 1:
			ip->RedrawViews(ip->GetTime(),REDRAW_END);
			End (ip->GetTime(), true);
			m0set = FALSE;
			break;
		}
		break;

	case MOUSE_MOVE:
		if (!m0set) break;
		p0 = vpt->MapScreenToView (m0,float(-200));
		m2 = IPoint2(m0.x, m.y);
		p1 = vpt->MapScreenToView (m2,float(-200));
		inset = Length (p1-p0);
		if ((m.y > m0.y) && (mStartAmount>0)) inset *= -1.0f;
		Drag (ip->GetTime(), inset);
		ip->RedrawViews(ip->GetTime(),REDRAW_INTERACTIVE);
		break;

	case MOUSE_ABORT:
		End (ip->GetTime(), false);
		ip->RedrawViews(ip->GetTime(),REDRAW_END);
		m0set = FALSE;
		break;
	}

	if (vpt) ip->ReleaseViewport(vpt);
	return TRUE;
}

HCURSOR EditPolyInsetSelectionProcessor::GetTransformCursor() { 
	static HCURSOR hCur = NULL;
	if ( !hCur ) hCur = LoadCursor(hInstance,MAKEINTRESOURCE(IDC_INSETCUR));
	return hCur;
}

void EditPolyInsetCMode::EnterMode() {
	if (mpEditPoly->GetPolyOperationID() == ep_op_inset) {
		mpEditPoly->SetHitTestResult (true);
	} else {
		mpEditPoly->SetHitTestResult ();
	}

	HWND hGeom = mpEditPoly->GetDlgHandle (ep_subobj);
	if (!hGeom) return;
	ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_INSET));
	but->SetCheck(TRUE);
	ReleaseICustButton(but);
}

void EditPolyInsetCMode::ExitMode() {
	mpEditPoly->ClearHitTestResult ();

	HWND hGeom = mpEditPoly->GetDlgHandle (ep_subobj);
	if (!hGeom) return;
	ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_INSET));
	but->SetCheck(FALSE);
	ReleaseICustButton(but);
}

// ------------------------------------------------------

void EditPolyOutlineMouseProc::Begin (TimeValue t)
{
	if (mInOutline) return;
	mInOutline = true;

	theHold.Begin ();
	int type = mpEditPoly->GetPolyOperationID ();
	if (type != ep_op_outline) {
		mpEditPoly->EpModSetOperation (ep_op_outline);
		mStartAmount = 0.0f;

		SuspendAnimate();
		AnimateOff();
		SuspendSetKeyMode(); // AF (5/13/03)
		mpEditPoly->getParamBlock()->SetValue (epm_outline, t, 0.0f);
		ResumeSetKeyMode();
		ResumeAnimate ();
	} else {
		mStartAmount = mpEditPoly->getParamBlock()->GetFloat (epm_outline, t);
	}
}

void EditPolyOutlineMouseProc::Accept (TimeValue t)
{
	if (!mInOutline) return;
	mInOutline = false;

	mpEditPoly->EpModCommitUnlessAnimating (t);
	theHold.Accept (GetString(IDS_OUTLINE));

	if (mpEditPoly->getParamBlock()->GetInt(epm_animation_mode))
		mpEditPoly->SetHitTestResult (true);

	mpEditPoly->EpModRefreshScreen();
}

void EditPolyOutlineMouseProc::Cancel () {
	if (!mInOutline) return;
	mInOutline = false;
	theHold.Cancel ();
	mpEditPoly->EpModRefreshScreen();
}

void EditPolyOutlineMouseProc::Drag (TimeValue t, float amount)
{
	if (!mInOutline) return;
	mpEditPoly->getParamBlock()->SetValue (epm_outline, t, amount+mStartAmount);
}

int EditPolyOutlineMouseProc::proc (HWND hwnd, int msg, int point, int flags, IPoint2 m ) {
	ViewExp *vpt=ip->GetViewport (hwnd);
	Point3 p0, p1;
	IPoint2 m1;
	ISpinnerControl *spin=NULL;
	float outline;

	switch (msg) {
	case MOUSE_PROPCLICK:
		if (mInOutline) {
			Cancel ();
			ip->RedrawViews (ip->GetTime(), REDRAW_END);
		}
		ip->PopCommandMode ();
		break;

	case MOUSE_POINT:
		switch (point) {
		case 0:
			m0 = m;
			Begin (ip->GetTime());
			ip->RedrawViews(ip->GetTime(),REDRAW_INTERACTIVE);
			break;
		case 1:
			Accept (ip->GetTime());
			ip->RedrawViews(ip->GetTime(),REDRAW_END);
			break;
		}
		break;

	case MOUSE_MOVE:
		if (!mInOutline) break;

		// Get signed right/left distance from original point:
		p0 = vpt->MapScreenToView (m0, float(-200));
		m1.y = m.y;
		m1.x = m0.x;
		p1 = vpt->MapScreenToView (m1, float(-200));
		outline = Length (p1-p0);
		if (m.y > m0.y) outline *= -1.0f;

		Drag (ip->GetTime(), outline);
		ip->RedrawViews(ip->GetTime(),REDRAW_INTERACTIVE);
		break;

	case MOUSE_ABORT:
		Cancel ();
		ip->RedrawViews(ip->GetTime(),REDRAW_END);
		break;
	}

	if (vpt) ip->ReleaseViewport(vpt);
	return TRUE;
}

HCURSOR EditPolyOutlineSelectionProcessor::GetTransformCursor() { 
	static HCURSOR hCur = NULL;
	if ( !hCur ) hCur = LoadCursor(hInstance,MAKEINTRESOURCE(IDC_INSETCUR));	// STEVE: need new cursor?
	return hCur;
}

void EditPolyOutlineCMode::EnterMode() {
	if ((mpEditPoly->GetPolyOperationID() == ep_op_outline)) {
		mpEditPoly->SetHitTestResult (true);
	} else {
		mpEditPoly->SetHitTestResult ();
	}

	HWND hGeom = mpEditPoly->GetDlgHandle (ep_subobj);
	if (!hGeom) return;
	ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_OUTLINE));
	but->SetCheck(TRUE);
	ReleaseICustButton(but);
}

void EditPolyOutlineCMode::ExitMode() {
	mpEditPoly->ClearHitTestResult ();

	HWND hGeom = mpEditPoly->GetDlgHandle (ep_subobj);
	if (!hGeom) return;
	ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_OUTLINE));
	but->SetCheck(FALSE);
	ReleaseICustButton(but);
}

// ------------------------------------------------------

void EditPolyChamferMouseProc::Begin (TimeValue t, int meshSelLevel)
{
	if (mInChamfer) return;
	mInChamfer = true;

	mChamferOperation = (meshSelLevel == MNM_SL_EDGE) ? ep_op_chamfer_edge : ep_op_chamfer_vertex;

	theHold.Begin ();
	int type = mpEditPoly->GetPolyOperationID ();
	if (type != mChamferOperation) {
		mpEditPoly->EpModSetOperation (mChamferOperation);
		mStartAmount = 0.0f;

		SuspendAnimate();
		AnimateOff();
		SuspendSetKeyMode(); // AF (5/13/03)
		if (mChamferOperation==ep_op_chamfer_edge)
			mpEditPoly->getParamBlock()->SetValue (epm_chamfer_edge, t, 0.0f);
		else
			mpEditPoly->getParamBlock()->SetValue (epm_chamfer_vertex, t, 0.0f);
		ResumeSetKeyMode();
		ResumeAnimate ();
	} else {
		if (mChamferOperation==ep_op_chamfer_edge)
			mStartAmount = mpEditPoly->getParamBlock()->GetFloat (epm_chamfer_edge, t);
		else
			mStartAmount = mpEditPoly->getParamBlock()->GetFloat (epm_chamfer_vertex, t);
	}
}

void EditPolyChamferMouseProc::End (TimeValue t, bool accept)
{
	if (!mInChamfer) return;
	mInChamfer = FALSE;

	if (accept)
	{
		mpEditPoly->EpModCommitUnlessAnimating (t);
		theHold.Accept (GetString(IDS_CHAMFER));

		if (mpEditPoly->getParamBlock()->GetInt(epm_animation_mode)) {
			mpEditPoly->SetHitTestResult (true);
		}
	}
	else theHold.Cancel ();
}

void EditPolyChamferMouseProc::Drag (TimeValue t, float amount)
{
	if (!mInChamfer) return;
	amount += mStartAmount;
	if (amount<0) amount = 0;

	if (mChamferOperation==ep_op_chamfer_edge)
		mpEditPoly->getParamBlock()->SetValue (epm_chamfer_edge, t, amount);
	else mpEditPoly->getParamBlock()->SetValue (epm_chamfer_vertex, t, amount);
}

int EditPolyChamferMouseProc::proc (HWND hwnd, int msg, int point, int flags, IPoint2 m ) {	
	ViewExp *vpt=ip->GetViewport (hwnd);
	Point3 p0, p1;
	IPoint2 m2;
	float chamfer;

	switch (msg) {
	case MOUSE_PROPCLICK:
		ip->PopCommandMode ();
		break;

	case MOUSE_POINT:
		if (!point) {
			Begin (ip->GetTime(), mpEditPoly->GetMNSelLevel());
			om = m;
		} else {
			End (ip->GetTime(), true);
			ip->RedrawViews (ip->GetTime(),REDRAW_END);
		}
		break;

	case MOUSE_MOVE:
		p0 = vpt->MapScreenToView (om,float(-200));
		m2 = IPoint2(om.x, m.y);
		p1 = vpt->MapScreenToView (m2,float(-200));
		chamfer = Length (p1-p0);
		if ((mStartAmount>0) && (m.y > om.y)) chamfer *= -1.0f;

		Drag (ip->GetTime(), chamfer);
		ip->RedrawViews(ip->GetTime(),REDRAW_INTERACTIVE);
		break;

	case MOUSE_ABORT:
		End (ip->GetTime(), false);			
		ip->RedrawViews (ip->GetTime(),REDRAW_END);
		break;
	}

	if (vpt) ip->ReleaseViewport(vpt);
	return TRUE;
}

HCURSOR EditPolyChamferSelectionProcessor::GetTransformCursor() { 
	static HCURSOR hECur = NULL;
	static HCURSOR hVCur = NULL;
	if (mpEditPoly->GetEPolySelLevel() == EPM_SL_VERTEX) {
		if ( !hVCur ) hVCur = LoadCursor(hInstance,MAKEINTRESOURCE(IDC_VCHAMFERCUR));
		return hVCur;
	}
	if ( !hECur ) hECur = LoadCursor(hInstance,MAKEINTRESOURCE(IDC_ECHAMFERCUR));
	return hECur;
}

void EditPolyChamferCMode::EnterMode() {
	if ((mpEditPoly->GetPolyOperationID() == ep_op_chamfer_vertex) ||
		(mpEditPoly->GetPolyOperationID() == ep_op_chamfer_edge)) {
		mpEditPoly->SetHitTestResult (true);
	}

	HWND hGeom = mpEditPoly->GetDlgHandle (ep_subobj);
	if (hGeom) {
		ICustButton *but = GetICustButton(GetDlgItem(hGeom,IDC_CHAMFER));
		but->SetCheck(TRUE);
		ReleaseICustButton(but);
	}

	eproc.mInChamfer = false;
}

void EditPolyChamferCMode::ExitMode() {
	mpEditPoly->ClearHitTestResult ();

	if (eproc.mInChamfer) eproc.End (TimeValue(0), false);

	HWND hGeom = mpEditPoly->GetDlgHandle (ep_subobj);
	if (!hGeom) return;
	ICustButton *but = GetICustButton(GetDlgItem(hGeom,IDC_CHAMFER));
	but->SetCheck(FALSE);
	ReleaseICustButton(but);
}

// -- Cut proc/mode -------------------------------------

HitRecord *EditPolyCutProc::HitTestVerts (IPoint2 &m, ViewExp *vpt, bool completeAnalysis) {
	vpt->ClearSubObjHitList();

	mpEPoly->SetHitLevelOverride (SUBHIT_MNVERTS);
	ip->SubObHitTest (ip->GetTime(), HITTYPE_POINT, 0, 0, &m, vpt);
	mpEPoly->ClearHitLevelOverride ();
	if (!vpt->NumSubObjHits()) return NULL;
	HitLog& hitLog = vpt->GetSubObjHitList();

	// Ok, if we haven't yet started a cut, and we don't want to know the hit location,
	// we're done:
	if ((mStartIndex<0) && !completeAnalysis) return hitLog.First();

	// Ok, now we need to find the closest eligible point:
	// First we grab our node's transform from the first hit:
	mObj2world = hitLog.First()->nodeRef->GetObjectTM(ip->GetTime());

	int bestHitIndex = -1;
	int bestHitDistance = 0;
	Point3 bestHitPoint(0.0f,0.0f,0.0f);
	HitRecord *ret = NULL;
	MNMesh* pMesh = NULL;

	for (HitRecord *hitRec=hitLog.First(); hitRec != NULL; hitRec=hitRec->Next()) {
		// Always want the closest hit pixelwise:
		if ((bestHitIndex>-1) && (bestHitDistance < hitRec->distance)) continue;

		if (mStartIndex >= 0)
		{
			// Check that our hit doesn't touch starting component,
			// and that it's on the right mesh.
			if (hitRec->nodeRef != mpEPoly->EpModGetPrimaryNode ()) continue;
			pMesh = mpEPoly->EpModGetOutputMesh ();
			if (!pMesh) continue;
			switch (mStartLevel) {
			case MNM_SL_VERTEX:
				if (mStartIndex == int(hitRec->hitInfo)) continue;
				break;
			case MNM_SL_EDGE:
				if (pMesh->e[mStartIndex].v1 == hitRec->hitInfo) continue;
				if (pMesh->e[mStartIndex].v2 == hitRec->hitInfo) continue;
				break;
			case MNM_SL_FACE:
				// Any face is suitable.
				break;
			}
		} else pMesh = mpEPoly->EpModGetOutputMesh (hitRec->nodeRef);
		if (!pMesh) continue;

		Point3 & p = pMesh->P(hitRec->hitInfo);

		if (bestHitIndex>-1)
		{
			Point3 diff = p - bestHitPoint;
			diff = mObj2world.VectorTransform(diff);
			if (DotProd (diff, mLastHitDirection) > 0) continue;	// this vertex clearly further away.
		}

		bestHitIndex = hitRec->hitInfo;
		bestHitDistance = hitRec->distance;
		bestHitPoint = p;
		ret = hitRec;
	}

	if (bestHitIndex>-1)
	{
		mLastHitLevel = MNM_SL_VERTEX;
		mLastHitIndex = bestHitIndex;
		mLastHitPoint = bestHitPoint;
	}

	return ret;
}

HitRecord *EditPolyCutProc::HitTestEdges (IPoint2 &m, ViewExp *vpt, bool completeAnalysis) {
	vpt->ClearSubObjHitList();

	mpEPoly->SetHitLevelOverride (SUBHIT_MNEDGES);
	ip->SubObHitTest (ip->GetTime(), HITTYPE_POINT, 0, 0, &m, vpt);
	mpEPoly->ClearHitLevelOverride ();
	int numHits = vpt->NumSubObjHits();
	if (numHits == 0) return NULL;

	HitLog& hitLog = vpt->GetSubObjHitList();

	// Ok, if we haven't yet started a cut, and we don't want to know the actual hit location,
	// we're done:
	if ((mStartIndex<0) && !completeAnalysis) return hitLog.First ();

	// Ok, now we want the edge with the closest hit point.
	// So we form a list of all the edges and their hit points.

	// First we grab our node's transform from the first hit:
	mObj2world = hitLog.First()->nodeRef->GetObjectTM(ip->GetTime());

	int bestHitIndex = -1;
	int bestHitDistance = 0;
	Point3 bestHitPoint(0.0f,0.0f,0.0f);
	HitRecord *ret = NULL;

	for (HitRecord *hitRec=hitLog.First(); hitRec != NULL; hitRec=hitRec->Next()) {
		// Always want the closest hit pixelwise:
		if ((bestHitIndex>-1) && (bestHitDistance < hitRec->distance)) continue;

		MNMesh* pMesh = NULL;
		if (mStartIndex>-1) 	{
			// Check that our hit doesn't touch starting component,
			// and that it's on the right mesh.
			if (hitRec->nodeRef != mpEPoly->EpModGetPrimaryNode ()) continue;
			pMesh = mpEPoly->EpModGetOutputMesh ();
			if (!pMesh) continue;
			switch (mStartLevel) {
			case MNM_SL_VERTEX:
				if (pMesh->e[hitRec->hitInfo].v1 == mStartIndex) continue;
				if (pMesh->e[hitRec->hitInfo].v2 == mStartIndex) continue;
				break;
			case MNM_SL_EDGE:
				if (mStartIndex == int(hitRec->hitInfo)) continue;
				break;
				// (any face is acceptable)
			}
		} else pMesh = mpEPoly->EpModGetOutputMesh (hitRec->nodeRef);
		if (!pMesh) continue;

		// Get endpoints in world space:
		Point3 Aobj = pMesh->P(pMesh->e[hitRec->hitInfo].v1);
		Point3 Bobj = pMesh->P(pMesh->e[hitRec->hitInfo].v2);
		Point3 A = mObj2world * Aobj;
		Point3 B = mObj2world * Bobj;

		// Find proportion of our nominal hit point along this edge:
		Point3 Xdir = B-A;
		Xdir -= DotProd(Xdir, mLastHitDirection)*mLastHitDirection;	// make orthogonal to mLastHitDirection.
		float prop = DotProd (Xdir, mLastHitPoint-A) / LengthSquared (Xdir);
		if (prop<.0001f) prop=0;
		if (prop>.9999f) prop=1;

		// Find hit point in object space:
		Point3 p = Aobj*(1.0f-prop) + Bobj*prop;

		if (bestHitIndex>-1) {
			Point3 diff = p - bestHitPoint;
			diff = mObj2world.VectorTransform(diff);
			if (DotProd (diff, mLastHitDirection)>0) continue; // this edge clearly further away.
		}
		bestHitIndex = hitRec->hitInfo;
		bestHitDistance = hitRec->distance;
		bestHitPoint = p;
		ret = hitRec;
	}

	if (bestHitIndex>-1)
	{
		mLastHitLevel = MNM_SL_EDGE;
		mLastHitIndex = bestHitIndex;
		mLastHitPoint = bestHitPoint;
	}

	return ret;
}

HitRecord *EditPolyCutProc::HitTestFaces (IPoint2 &m, ViewExp *vpt, bool completeAnalysis) {
	vpt->ClearSubObjHitList();

	mpEPoly->SetHitLevelOverride (SUBHIT_MNFACES);
	ip->SubObHitTest (ip->GetTime(), HITTYPE_POINT, 0, 0, &m, vpt);
	mpEPoly->ClearHitLevelOverride ();
	if (!vpt->NumSubObjHits()) return NULL;
	HitLog& hitLog = vpt->GetSubObjHitList();

	// We don't bother to choose the view-direction-closest face,
	// since generally that's the only one we expect to get.
	if (!completeAnalysis) return hitLog.First();

	// Find the closest hit on the right mesh:
	Point3 bestHitPoint;
	HitRecord *ret = NULL;
	for (HitRecord *hitRec=hitLog.First(); hitRec != NULL; hitRec=hitRec->Next()) {
		// Always want the closest hit pixelwise:
		if ((ret!=NULL) && (ret->distance < hitRec->distance)) continue;

		if (mStartIndex>-1) 	{
			// Check that the hit is on the right mesh.
			if (hitRec->nodeRef != mpEPoly->EpModGetPrimaryNode ()) continue;
		}
		ret = hitRec;
	}
	if (ret == NULL) return ret;

	mLastHitIndex = ret->hitInfo;
	mObj2world = ret->nodeRef->GetObjectTM (ip->GetTime ());
	
	// Get the average normal for the face, thus the plane, and intersect.
	MNMesh *pMesh = mpEPoly->EpModGetOutputMesh(ret->nodeRef);
	if (!pMesh) return NULL;
	Point3 planeNormal = pMesh->GetFaceNormal (mLastHitIndex, TRUE);
	planeNormal = Normalize (mObj2world.VectorTransform (planeNormal));
	float planeOffset=0.0f;
	for (int i=0; i<pMesh->f[mLastHitIndex].deg; i++)
		planeOffset += DotProd (planeNormal, mObj2world*pMesh->v[pMesh->f[mLastHitIndex].vtx[i]].p);
	planeOffset = planeOffset/float(pMesh->f[mLastHitIndex].deg);

	// Now we intersect the mLastHitPoint + mLastHitDirection*t line with the
	// DotProd (planeNormal, X) = planeOffset plane.
	float rayPlane = DotProd (mLastHitDirection, planeNormal);
	float firstPointOffset = planeOffset - DotProd (planeNormal, mLastHitPoint);
	if (fabsf(rayPlane) > EPSILON) {
		float amount = firstPointOffset / rayPlane;
		mLastHitPoint += amount*mLastHitDirection;
	}

	// Put hitPoint in object space:
	Matrix3 world2obj = Inverse (mObj2world);
	mLastHitPoint = world2obj * mLastHitPoint;
	mLastHitLevel = MNM_SL_FACE;

	return ret;
}

HitRecord *EditPolyCutProc::HitTestAll (IPoint2 & m, ViewExp *vpt, int flags, bool completeAnalysis) {
	HitRecord *hr = NULL;
	mLastHitLevel = MNM_SL_OBJECT;	// no hit.

	mpEPoly->ForceIgnoreBackfacing (true);
	mpEPoly->SetHitTestResult ();

	if (!(flags & (MOUSE_SHIFT|MOUSE_CTRL))) {
		hr = HitTestVerts (m,vpt, completeAnalysis);
		if (hr) mLastHitLevel = MNM_SL_VERTEX;
	}
	if (!hr && !(flags & MOUSE_SHIFT)) {
		hr = HitTestEdges (m, vpt, completeAnalysis);
		if (hr) mLastHitLevel = MNM_SL_EDGE;
	}
	if (!hr) {
		hr = HitTestFaces (m, vpt, completeAnalysis);
		if (hr) mLastHitLevel = MNM_SL_FACE;
	}

	mpEPoly->ClearHitTestResult();
	mpEPoly->ForceIgnoreBackfacing (false);

	return hr;
}

void EditPolyCutProc::DrawCutter (HWND hWnd) {
	if (mStartIndex<0) return;


	IPoint2 points[2];
	points[0]  = mMouse1;
	points[1]  = mMouse2;
	XORDottedPolyline(hWnd, 2, points);

}

void EditPolyCutProc::Start ()
{
	mStartIndex = mLastHitIndex;
	mStartLevel = mLastHitLevel;

	if (ip->GetSnapState() || true) {
		// Must suspend "fully interactive" while snapping in Cut mode.
		mSuspendPreview = true;
		//ip->PushPrompt (GetString (IDS_CUT_SNAP_PREVIEW_WARNING));
		mStartPoint = mLastHitPoint;
	} else {
		mSuspendPreview = false;
		theHold.Begin ();
		mpEPoly->EpModCut (mStartLevel, mStartIndex, mLastHitPoint, -mLastHitDirection);
	}
}

void EditPolyCutProc::Cancel ()
{
	if (mStartIndex<0) return;

	if (mSuspendPreview) {
		//ip->PopPrompt ();
	} else {
		theHold.Cancel ();
		ip->RedrawViews (ip->GetTime());
	}
	mpEPoly->EpModClearLastCutEnd ();

	mStartIndex = -1;
}

void EditPolyCutProc::Update ()
{
	if ((mStartIndex<0) || mSuspendPreview) return;
	mpEPoly->EpModSetCutEnd (mLastHitPoint);
}

void EditPolyCutProc::Accept ()
{
	if (mStartIndex<0) return;

	if (mSuspendPreview) {
		//ip->PopPrompt();
		theHold.Begin ();
		mpEPoly->EpModCut (mStartLevel, mStartIndex, mStartPoint, -mLastHitDirection);
	}
	mpEPoly->EpModSetCutEnd (mLastHitPoint);
	theHold.Accept (GetString (IDS_CUT));

	mStartIndex = -1;
}

// Qilin.Ren Defect 932406
// The class is used for encapsulating my algorithms to fix the bug, which is 
// caused by float point precision problems when mouse ray is almost parallel 
// to the XOY plane.
class ViewSnapPlane
{
public:
	// Create a matrix of a new plane, which is orthogonal to the mouse ray
	ViewSnapPlane(IN ViewExp* pView, IN const IPoint2& mousePosition);

	const Point3& GetHitPoint() const;
	const IPoint2& GetBetterMousePosition() const;
	// If the mouse ray did not hit any of the polygons, then we should create
	// another plane, compute the intersection of that plane and the ray, in 
	// order to satisfy MNMesh::Cut alrogithm.
	void HitTestFarPlane(
		OUT Point3& hitPoint, 
		IN EPolyMod* pEditPoly, 
		IN const Matrix3& obj2World);

private:
	const Point3& GetXAxis() const;
	const Point3& GetYAxis() const;
	const Point3& GetZAxis() const;
	const Point3& GetEyePosition() const;

	const Matrix3& GetMatrix() const;
	const Matrix3& GetInvertMatrix() const;

private:
	Point3 mHitPoint;
	IPoint2 mBetterMousePosition;

	Point3 mXAxis;
	Point3 mYAxis;
	Point3 mZAxis;
	Point3 mEyePosition;
	Matrix3 mMatrix;
	Matrix3 mInvertMatrix;
};

ViewSnapPlane::ViewSnapPlane(
	IN ViewExp* pView, 
	IN const IPoint2& mousePosition)
{
	DbgAssert(NULL != pView);

	Ray ray;
	pView->MapScreenToWorldRay(mousePosition.x, mousePosition.y, ray);

	mEyePosition = ray.p;

	mZAxis = -Normalize(ray.dir);
	Point3 up = mZAxis;
	up[mZAxis.MinComponent()] = +2.0f + mZAxis[mZAxis.MaxComponent()]*2;
	up[mZAxis.MaxComponent()] = -2.0f + mZAxis[mZAxis.MinComponent()]/2;
	mXAxis = Normalize(CrossProd(up, mZAxis));
	mYAxis = CrossProd(mZAxis, mXAxis);

	mMatrix.SetColumn(0, Point4(
		mXAxis.x, 
		mXAxis.y, 
		mXAxis.z, 
		-DotProd(mXAxis, mEyePosition)));
	mMatrix.SetColumn(1, Point4(
		mYAxis.x, 
		mYAxis.y, 
		mYAxis.z, 
		-DotProd(mYAxis, mEyePosition)));
	mMatrix.SetColumn(2, Point4(
		mZAxis.x, 
		mZAxis.y, 
		mZAxis.z, 
		-DotProd(mZAxis, mEyePosition)));
	mInvertMatrix = mMatrix;
	mInvertMatrix.Invert();

	mHitPoint = pView->SnapPoint (
		mousePosition, 
		mBetterMousePosition, 
		&mInvertMatrix);
	mHitPoint = mInvertMatrix.PointTransform(mHitPoint);
}

inline const Point3& ViewSnapPlane::GetHitPoint() const
{
	return mHitPoint;
}

inline const IPoint2& ViewSnapPlane::GetBetterMousePosition() const
{
	return mBetterMousePosition;
}


inline const Point3& ViewSnapPlane::GetXAxis() const
{
	return mXAxis;
}

inline const Point3& ViewSnapPlane::GetYAxis() const
{
	return mYAxis;
}

inline const Point3& ViewSnapPlane::GetZAxis() const
{
	return mZAxis;
}

inline const Point3& ViewSnapPlane::GetEyePosition() const
{
	return mEyePosition;
}


inline const Matrix3& ViewSnapPlane::GetMatrix() const
{
	return mMatrix;
}

inline const Matrix3& ViewSnapPlane::GetInvertMatrix() const
{
	return mInvertMatrix;
}

void ViewSnapPlane::HitTestFarPlane(
	OUT Point3& hitPoint, 
	IN EPolyMod* pEditPoly, 
	IN const Matrix3& obj2World)
{
	DbgAssert(NULL != pEditPoly);

	Box3 bbox;
	Tab<Box3> boxes;
	IObjParam* l_ip = pEditPoly->EpModGetIP();
	if (NULL != l_ip)
	{
		// combine all bounding box together, then we are safe
		int i;
		ModContextList l_list;
		INodeTab l_nodes;	
		l_ip->GetModContexts(l_list, l_nodes);
		for (i = 0; i < l_list.Count(); ++i)
		{
			EditPolyData* l_pData = (EditPolyData *)(l_list[i]->localData);
			if (NULL == l_pData)
			{
				continue;
			}
			MNMesh* pMesh = l_pData->GetMesh();
			if (NULL == pMesh)
			{
				continue;
			}
			pMesh->BBox(bbox);
			boxes.Append(1, &bbox);
		}
		bbox.Init();
		for (int i = 0; i < boxes.Count(); ++i)
		{
			bbox += boxes[i];
		}
	}
	if (boxes.Count() == 0)
	{
		// We still need to transform the hit point into object space.
		Matrix3 l_world2obj = Inverse (obj2World);
		hitPoint = l_world2obj * hitPoint;
		return;
	}

	// convert bbox to bounding sphere
	Point3 l_boundCenter = bbox.Center();
	float l_boundRadius = ((bbox.Width()/2)*obj2World).Length();

	// convert bounding sphere from object space to world space
	l_boundCenter = obj2World * l_boundCenter;

	// create a plane
	Point3 v1 = l_boundCenter - GetZAxis() * l_boundRadius;
	const Point3& planeNormal = GetZAxis();
	float d = DotProd(planeNormal, v1);

	// get the intersection of (mLastHitPoint, mLastHitPoint-zAxis) and the plane
	Point3 A = mHitPoint;
	Point3 B = mHitPoint - GetZAxis();
	float dA = DotProd(planeNormal, A) - d;
	float dB = DotProd(planeNormal, B) - d;
	float scale = dA / (dA - dB);
	Point3 C = B-A;
	C *= scale;
	A += C;

	hitPoint = A;

	// We still need to transform the hit point into object space.
	Matrix3 l_world2obj = Inverse (obj2World);
	hitPoint = l_world2obj * hitPoint;
}

int EditPolyCutProc::proc (HWND hwnd, int msg, int point, int flags, IPoint2 m ) {
	ViewExp* vpt = NULL;
	IPoint2 betterM(0,0);
	Matrix3 world2obj;
	HitRecord* hr = NULL;
	Ray r;
	static HCURSOR hCutVertCur = NULL, hCutEdgeCur=NULL, hCutFaceCur = NULL;

	if (!hCutVertCur) {
		hCutVertCur = LoadCursor (hInstance, MAKEINTRESOURCE(IDC_CUT_VERT_CUR));
		hCutEdgeCur = LoadCursor (hInstance, MAKEINTRESOURCE(IDC_CUT_EDGE_CUR));
		hCutFaceCur = LoadCursor (hInstance, MAKEINTRESOURCE(IDC_CUT_FACE_CUR));
	}

	switch (msg) {
	case MOUSE_ABORT:
		if (mStartIndex>-1) {
			DrawCutter (hwnd);
			Cancel ();
		}
		return FALSE;

	case MOUSE_PROPCLICK:
		if (mStartIndex>-1) {
			DrawCutter (hwnd);
			Cancel ();
		}
		ip->PopCommandMode ();
		return FALSE;

	case MOUSE_DBLCLICK:
		return false;

	case MOUSE_POINT:
		if (point==1) break;		// Want click-move-click behavior.
		ip->SetActiveViewport (hwnd);
		vpt = ip->GetViewport (hwnd);

		// Get a worldspace hit point and view direction:
		{
			// Qilin.Ren Defect 932406
			// Fixed the problem while mouse ray is almost parallel to the relative xoy plane
			ViewSnapPlane snapPlane(vpt, m);
			mLastHitPoint = snapPlane.GetHitPoint();
			betterM = snapPlane.GetBetterMousePosition();
			vpt->MapScreenToWorldRay ((float)betterM.x, (float)betterM.y, r);
			mLastHitDirection = Normalize (r.dir);	// in world space

			// Hit test all subobject levels:
			hr = HitTestAll (m, vpt, flags, true);
			if (NULL == hr)
			{
				snapPlane.HitTestFarPlane(mLastHitPoint, mpEPoly, mObj2world);
			}
		}
		ip->ReleaseViewport (vpt);

		if (mLastHitLevel == MNM_SL_OBJECT) return true;

		// Get hit directon in object space:
		world2obj = Inverse (mObj2world);
		mLastHitDirection = Normalize (VectorTransform (world2obj, mLastHitDirection));

		if (mStartIndex<0) {
			mpEPoly->EpModSetPrimaryNode (hr->nodeRef);
			Start ();

			mMouse1 = betterM;
			mMouse2 = betterM;
			DrawCutter (hwnd);
			return true;
		}

		// Erase last cutter line:
		DrawCutter (hwnd);

		// Do the cut:
		Accept ();

		// Refresh, so that the "LastCutEnd" data is updated along with everything else:
		mpEPoly->UpdateCache (ip->GetTime());
		ip->RedrawViews (ip->GetTime());

		int cutEnd;
		cutEnd = mpEPoly->EpModGetLastCutEnd ();
		if (cutEnd>-1) {
			// Last cut was successful - start the next cut:
			mLastHitIndex = cutEnd;
			mLastHitLevel = MNM_SL_VERTEX;
			Start ();
			mMouse1 = betterM;
			mMouse2 = betterM;
			DrawCutter (hwnd);
			return true;
		}

		// Otherwise, we had to stop somewhere, so we discontinue cutting:
		return false;

	case MOUSE_MOVE:
		vpt = ip->GetViewport (hwnd);

		// Show snap preview:
		vpt->SnapPreview (m, m, NULL, SNAP_FORCE_3D_RESULT);
		//ip->RedrawViews (ip->GetTime(), REDRAW_INTERACTIVE);	// hey - why are we doing this?

		// Find 3D point in object space:
		{
			// Qilin.Ren Defect 932406
			// Fixed the problem while mouse ray is almost parallel to the relative xoy plane
			ViewSnapPlane snapPlane(vpt, m);
			mLastHitPoint = snapPlane.GetHitPoint();
			betterM = snapPlane.GetBetterMousePosition();
			vpt->MapScreenToWorldRay ((float)betterM.x, (float)betterM.y, r);
			mLastHitDirection = Normalize (r.dir);	// in world space

			if (NULL == HitTestAll (betterM, vpt, flags, true))
			{
				snapPlane.HitTestFarPlane(mLastHitPoint, mpEPoly, mObj2world);
			}
		}

		if (mStartIndex>-1) {
			// Erase last dotted line
			DrawCutter (hwnd);

			// Set the cut preview to use the point we just hit on:
			Update ();

			// Draw new dotted line
			mMouse2 = betterM;
			DrawCutter (hwnd);
		}

		// Set cursor based on best subobject match:
		switch (mLastHitLevel) {
		case MNM_SL_VERTEX: SetCursor (hCutVertCur); break;
		case MNM_SL_EDGE: SetCursor (hCutEdgeCur); break;
		case MNM_SL_FACE: SetCursor (hCutFaceCur); break;
		default: SetCursor (LoadCursor (NULL, IDC_ARROW));
		}

		ip->ReleaseViewport (vpt);
		ip->RedrawViews (ip->GetTime());
		break;

	case MOUSE_FREEMOVE:
		vpt = ip->GetViewport (hwnd);
		vpt->SnapPreview(m,m,NULL);
		HitTestAll (m, vpt, flags, false);
		ip->ReleaseViewport (vpt);

		// Set cursor based on best subobject match:
		switch (mLastHitLevel) {
		case MNM_SL_VERTEX: SetCursor (hCutVertCur); break;
		case MNM_SL_EDGE: SetCursor (hCutEdgeCur); break;
		case MNM_SL_FACE: SetCursor (hCutFaceCur); break;
		default: SetCursor (LoadCursor (NULL, IDC_ARROW));
		}
		break;
	}

	return TRUE;
}

void EditPolyCutCMode::EnterMode() {
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom);
	if (!hGeom) return;
	ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_CUT));
	but->SetCheck(TRUE);
	ReleaseICustButton(but);
	proc.mStartIndex = -1;
}

void EditPolyCutCMode::ExitMode() {
	// Lets make double-extra-sure that Cut's suspension of the preview system doesn't leak out...
	// (This line is actually necessary in the case where the user uses a shortcut key to jump-start
	// another command mode.)
	if (proc.mStartIndex>-1) proc.Cancel ();

	// Commit if needed:
	if (mpEditPoly->GetPolyOperationID () == ep_op_cut)
	{
		theHold.Begin();
		mpEditPoly->EpModCommit (TimeValue(0));
		theHold.Accept (GetString (IDS_EDITPOLY_COMMIT));
	}

	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom);
	if (!hGeom) return;
	ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_CUT));
	but->SetCheck(FALSE);
	ReleaseICustButton(but);
}

//------- QuickSlice proc/mode------------------------

void EditPolyQuickSliceProc::DrawSlicer (HWND hWnd) {
	static int callCount = 0;
	if (!mSlicing) return;

	callCount++;

	IPoint2 points[2];
	points[0]  = mMouse1;
	points[1]  = mMouse2;
	XORDottedPolyline(hWnd, 2, points,0,true);

}

// Given two points and a view direction (in obj space), find slice plane:
void EditPolyQuickSliceProc::ComputeSliceParams () {
	Point3 Xdir = mLocal2 - mLocal1;
	Xdir = Xdir - mZDir * DotProd (Xdir, mZDir);	// make orthogonal to view
	float len = Length(Xdir);
	Point3 planeNormal;
	if (len<.001f) {
		// Xdir is insignificant, but mZDir is valid.
		// Choose an arbitrary Xdir that'll work:
		Xdir = Point3(1,0,0);
		Xdir = Xdir - mZDir * DotProd (Xdir, mZDir);
		len = Length(Xdir);
	}

	if (len<.001f) {
		// straight X-direction didn't work; therefore straight Y-direction should:
		Xdir = Point3(0,1,0);
		Xdir = Xdir - mZDir * DotProd (Xdir, mZDir);
		len = Length (Xdir);
	}

	// Now guaranteed to have some valid Xdir:
	Xdir = Xdir/len;
	planeNormal = mZDir^Xdir;

	float dp = DotProd (planeNormal, mLocal1-mBoxCenter);
	Point3 planeCtr = mBoxCenter + dp * planeNormal;

	planeNormal = FNormalize (mWorld2ModContext.VectorTransform (planeNormal));
	mpEPoly->EpSetSlicePlane (planeNormal, mWorld2ModContext*planeCtr, mpInterface->GetTime());
}

static HCURSOR hCurQuickSlice = NULL;

void EditPolyQuickSliceProc::Start ()
{
	if (mpInterface->GetSnapState()) {
		// Must suspend "fully interactive" while snapping in Quickslice mode.
		mSuspendPreview = true;
		mpInterface->PushPrompt (GetString (IDS_QUICKSLICE_SNAP_PREVIEW_WARNING));
	} else {
		mSuspendPreview = false;
		theHold.Begin ();
		ComputeSliceParams ();
		if (mpEPoly->GetMNSelLevel()==MNM_SL_FACE) mpEPoly->EpModSetOperation (ep_op_slice_face);
		else mpEPoly->EpModSetOperation (ep_op_slice);
	}

	// Get the current world2obj and mod context transforms for node 0:
	ModContextList list;
	INodeTab nodes;
	mpInterface->GetModContexts (list, nodes);
	Matrix3 objTM = nodes[0]->GetObjectTM (mpInterface->GetTime());
	nodes.DisposeTemporary();
	mWorld2ModContext = (*(list[0]->tm)) * Inverse(objTM);

	mBoxCenter = objTM * list[0]->box->Center();

	mSlicing = true;
}

void EditPolyQuickSliceProc::Cancel ()
{
	if (!mSlicing) return;

	if (mSuspendPreview) {
		mpInterface->PopPrompt ();
	} else {
		mpEPoly->EpModCancel ();
		theHold.Cancel ();
		mpEPoly->EpModRefreshScreen();
	}

	mSlicing = false;
}

void EditPolyQuickSliceProc::Update ()
{
	if (!mSlicing || mSuspendPreview) return;

	ComputeSliceParams ();
	mpEPoly->EpModRefreshScreen();
}

void EditPolyQuickSliceProc::Accept ()
{
	if (!mSlicing) return;

	if (mSuspendPreview) {
		mpInterface->PopPrompt();
		theHold.Begin ();
		if (mpEPoly->GetMNSelLevel()==MNM_SL_FACE) mpEPoly->EpModSetOperation (ep_op_slice_face);
		else mpEPoly->EpModSetOperation (ep_op_slice);
	}
	ComputeSliceParams ();
	mpEPoly->EpModCommit (mpInterface->GetTime());
	theHold.Accept (GetString (IDS_SLICE));

	mSlicing = false;
}

int EditPolyQuickSliceProc::proc (HWND hwnd, int msg, int point, int flags, IPoint2 m ) {
	if (!hCurQuickSlice)
		hCurQuickSlice = LoadCursor(hInstance,MAKEINTRESOURCE(IDC_QUICKSLICE_CURSOR));

	ViewExp* vpt = NULL;
	IPoint2 betterM(0,0);
	Point3 snapPoint(0.0f,0.0f,0.0f);
	Ray r;

	switch (msg) {
	case MOUSE_ABORT:
		if (mSlicing) {
			if (mSuspendPreview) DrawSlicer (hwnd);	// Erase last slice line.
			Cancel ();
		}
		return false;

	case MOUSE_PROPCLICK:
		if (mSlicing) {
			if (mSuspendPreview) DrawSlicer (hwnd);	// Erase last slice line.
			Cancel ();
		}
		mpInterface->PopCommandMode ();
		return false;

	case MOUSE_DBLCLICK:
		return false;

	case MOUSE_POINT:
		if (point==1) break;	// don't want to get a notification on first mouse-click release.
		mpInterface->SetActiveViewport (hwnd);

		// Find where we hit, in world space, on the construction plane or snap location:
		vpt = mpInterface->GetViewport (hwnd);
		snapPoint = vpt->SnapPoint (m, betterM, NULL);
		snapPoint = vpt->MapCPToWorld (snapPoint);

		if (!mSlicing) {
			// Initialize our endpoints:
			mLocal2 = mLocal1 = snapPoint;

			// We'll also need to find our Z direction:
			vpt->MapScreenToWorldRay ((float)betterM.x, (float)betterM.y, r);
			mZDir = Normalize (r.dir);

			// Begin slicing:
			Start ();

			mMouse1 = betterM;
			mMouse2 = betterM;
			DrawSlicer (hwnd);
		} else {
			DrawSlicer (hwnd);	// Erase last slice line.
			
			// second point:
			mLocal2 = snapPoint;

			// Finish Slicing:
			Accept ();
			GetCOREInterface()->ForceCompleteRedraw();
		}
		mpInterface->ReleaseViewport (vpt);
		return true;

	case MOUSE_MOVE:
		if (!mSlicing) break;	// Nothing to do if we haven't clicked first point yet

		SetCursor (hCurQuickSlice);
		DrawSlicer (hwnd);	// Erase last slice line.

		// Find where we hit, in world space, on the construction plane or snap location:
		vpt = mpInterface->GetViewport (hwnd);
		snapPoint = vpt->SnapPoint (m, mMouse2, NULL);
		snapPoint = vpt->MapCPToWorld (snapPoint);
		mpInterface->ReleaseViewport (vpt);

		// Find object-space equivalent for mLocal2:
		mLocal2 = snapPoint;

		Update ();

		DrawSlicer (hwnd);	// Draw this slice line.
		break;

	case MOUSE_FREEMOVE:
		SetCursor (hCurQuickSlice);
		vpt = mpInterface->GetViewport (hwnd);
		// Show any snapping:
		vpt->SnapPreview(m,m,NULL);//, SNAP_SEL_OBJS_ONLY);
		mpInterface->ReleaseViewport (vpt);
		break;
	}

	return TRUE;
}

void EditPolyQuickSliceCMode::EnterMode() {
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom);
	if (!hGeom) return;
	ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_QUICKSLICE));
	but->SetCheck(TRUE);
	ReleaseICustButton(but);
	proc.mSlicing = false;
}

void EditPolyQuickSliceCMode::ExitMode() {
	// Lets make double-extra-sure that QuickSlice's suspension of the preview system doesn't leak out...
	// (This line is actually necessary in the case where the user uses a shortcut key to jump-start
	// another command mode.)
	if (proc.mSlicing) proc.Cancel ();

	// Commit if needed:
	if ((mpEditPoly->GetPolyOperationID () == ep_op_slice) ||
		(mpEditPoly->GetPolyOperationID () == ep_op_slice_face))
	{
		theHold.Begin();
		mpEditPoly->EpModCommitUnlessAnimating (mpEditPoly->ip->GetTime());
		theHold.Accept (GetString (IDS_EDITPOLY_COMMIT));
	}

	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom);
	if (!hGeom) return;
	ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_QUICKSLICE));
	but->SetCheck(FALSE);
	ReleaseICustButton(but);
}

//------- HingeFromEdge proc/mode------------------------

HitRecord *EditPolyHingeFromEdgeProc::HitTestEdges (ViewExp *vpt) {
	vpt->ClearSubObjHitList();
	mpEPoly->SetHitLevelOverride (SUBHIT_MNEDGES);
	mpEPoly->ForceIgnoreBackfacing (true);
	ip->SubObHitTest (ip->GetTime(), HITTYPE_POINT, 0, 0, &mCurrentPoint, vpt);
	mpEPoly->ForceIgnoreBackfacing (false);
	mpEPoly->ClearHitLevelOverride ();
	if (!vpt->NumSubObjHits()) return NULL;
	HitLog& hitLog = vpt->GetSubObjHitList();
	return hitLog.ClosestHit();	// (may be NULL.)
}

void EditPolyHingeFromEdgeProc::Start ()
{
	if (mEdgeFound) return;
	mEdgeFound = true;

	int oldEdge = mpEPoly->EpModGetHingeEdge (mpHitRec->nodeRef);
	if (oldEdge == mpHitRec->hitInfo)
	{
		// We hit the same edge - so we're just adjusting the hinge angle.
		mStartAngle = mpEPoly->getParamBlock()->GetFloat (epm_hinge_angle, ip->GetTime());
		theHold.Begin ();
	}
	else
	{
		mStartAngle = 0.0f;
		theHold.Begin ();
		mpEPoly->EpModSetHingeEdge (mpHitRec->hitInfo, 
			mpHitRec->modContext->tm ? *(mpHitRec->modContext->tm) : Matrix3(true),
			mpHitRec->nodeRef);

		SuspendAnimate();
		AnimateOff();
		SuspendSetKeyMode(); // AF (5/13/03)
		mpEPoly->getParamBlock()->SetValue (epm_hinge_angle, TimeValue(0), 0.0f);
		ResumeSetKeyMode();
		ResumeAnimate ();
	}

	mFirstPoint = mCurrentPoint;

	ip->RedrawViews (ip->GetTime());
}

void EditPolyHingeFromEdgeProc::Update ()
{
	if (!mEdgeFound) return;

	IPoint2 diff = mCurrentPoint - mFirstPoint;
	// (this arbirtrarily scales each pixel to one degree.)
	float angle = diff.y * PI / 180.0f;
	mpEPoly->getParamBlock()->SetValue (epm_hinge_angle, ip->GetTime(), angle + mStartAngle);
}

void EditPolyHingeFromEdgeProc::Cancel ()
{
	if (!mEdgeFound) return;
	theHold.Cancel ();
	ip->RedrawViews (ip->GetTime());
	mEdgeFound = false;
}

void EditPolyHingeFromEdgeProc::Accept ()
{
	if (!mEdgeFound) return;

	Update ();
	mpEPoly->EpModCommitUnlessAnimating (ip->GetTime());
	theHold.Accept (GetString (IDS_HINGE_FROM_EDGE));
	ip->RedrawViews (ip->GetTime());
	mEdgeFound = false;
}

// Mouse interaction:
// - user clicks on hinge edge
// - user drags angle.

int EditPolyHingeFromEdgeProc::proc (HWND hwnd, int msg, int point, int flags, IPoint2 m ) {
	ViewExp* vpt = NULL;
	Point3 snapPoint(0.0f,0.0f,0.0f);
	IPoint2 diff(0,0);

	switch (msg) {
	case MOUSE_ABORT:
		Cancel ();
		return FALSE;

	case MOUSE_PROPCLICK:
		Cancel ();
		ip->PopCommandMode ();
		return FALSE;

	case MOUSE_DBLCLICK:
		return false;

	case MOUSE_POINT:
		ip->SetActiveViewport (hwnd);
		vpt = ip->GetViewport (hwnd);
		snapPoint = vpt->SnapPoint (m, m, NULL);
		snapPoint = vpt->MapCPToWorld (snapPoint);
		mCurrentPoint = m;

		if (!mEdgeFound) {
			mpHitRec = HitTestEdges (vpt);
			if (!mpHitRec) return true;
			Start ();
		} else {
			Accept ();
			return false;
		}
		if (vpt) ip->ReleaseViewport (vpt);
		return true;

	case MOUSE_MOVE:
		// Find where we hit, in world space, on the construction plane or snap location:
		vpt = ip->GetViewport (hwnd);
		snapPoint = vpt->SnapPoint (m, m, NULL);
		snapPoint = vpt->MapCPToWorld (snapPoint);
		mCurrentPoint = m;

		if (!mEdgeFound) {
			// Just set cursor depending on presence of edge:
			if (HitTestEdges(vpt)) SetCursor(ip->GetSysCursor(SYSCUR_SELECT));
			else SetCursor(LoadCursor(NULL,IDC_ARROW));
			if (vpt) ip->ReleaseViewport(vpt);
			break;
		}
		ip->ReleaseViewport (vpt);

		Update ();
		ip->RedrawViews (ip->GetTime());
		break;

	case MOUSE_FREEMOVE:
		vpt = ip->GetViewport (hwnd);
		snapPoint = vpt->SnapPoint (m, m, NULL);
		snapPoint = vpt->MapCPToWorld (snapPoint);
		mCurrentPoint = m;

		if (!mEdgeFound) {
			// Just set cursor depending on presence of edge:
			if (HitTestEdges (vpt)) SetCursor(ip->GetSysCursor(SYSCUR_SELECT));
			else SetCursor(LoadCursor(NULL,IDC_ARROW));
		}
		ip->ReleaseViewport (vpt);
		break;
	}

	return TRUE;
}

void EditPolyHingeFromEdgeCMode::EnterMode() {
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_subobj);
	if (!hGeom) return;
	ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_HINGE_FROM_EDGE));
	but->SetCheck(TRUE);
	ReleaseICustButton(but);
}

void EditPolyHingeFromEdgeCMode::ExitMode() {
	proc.Reset ();
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_subobj);
	if (!hGeom) return;
	ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_HINGE_FROM_EDGE));
	but->SetCheck(FALSE);
	ReleaseICustButton(but);
}

//-------------------------------------------------------

bool EditPolyPickHingeProc::EdgePick (HitRecord *hr, float prop) {
	theHold.Begin ();
	mpEPoly->EpModSetHingeEdge (hr->hitInfo, 
		hr->modContext->tm ? *(hr->modContext->tm) : Matrix3(true), hr->nodeRef);
	theHold.Accept (GetString (IDS_PICK_HINGE));

	ip->RedrawViews (ip->GetTime());
	return false;	// false = exit mode when done; true = stay in mode.
}

void EditPolyPickHingeCMode::EnterMode () {
	mpEditPoly->RegisterCMChangedForGrip();
	HWND hSettings = mpEditPoly->GetDlgHandle (ep_settings);
	if (!hSettings) return;
	ICustButton *but = GetICustButton (GetDlgItem (hSettings,IDC_HINGE_PICK_EDGE));
	but->SetCheck(true);
	ReleaseICustButton(but);
}

void EditPolyPickHingeCMode::ExitMode () {
	HWND hSettings = mpEditPoly->GetDlgHandle (ep_settings);
	if (!hSettings) return;
	ICustButton *but = GetICustButton (GetDlgItem (hSettings,IDC_HINGE_PICK_EDGE));
	but->SetCheck(false);
	ReleaseICustButton(but);
}

// --------------------------------------------------------

HitRecord *EditPolyPickBridgeTargetProc::HitTest (IPoint2 &m, ViewExp *vpt) {
	vpt->ClearSubObjHitList();
	ip->SubObHitTest(ip->GetTime(),HITTYPE_POINT,0, 0, &m, vpt);
	if (!vpt->NumSubObjHits()) return NULL;

	HitLog& hitLog = vpt->GetSubObjHitList();
	mpBridgeNode = mpEPoly->EpModGetBridgeNode();

	HitRecord *ret = NULL;
	for (HitRecord *hitRec=hitLog.First(); hitRec != NULL; hitRec=hitRec->Next()) {
		// If we've established a bridge node, filter out anything not on it.
		if (mpBridgeNode && (hitRec->nodeRef != mpBridgeNode)) continue;

		// TODO: Would be nice if we could filter out edges in the other border,
		// and polygons neighboring the other polygon.

		// Always want the closest hit pixelwise:
		if ((ret==NULL) || (ret->distance > hitRec->distance)) ret = hitRec;
	}
	return ret;
}

int EditPolyPickBridgeTargetProc::proc (HWND hwnd, int msg, int point, int flags, IPoint2 m) {
	ViewExp* vpt = NULL;
	HitRecord* hr = NULL;
	Point3 snapPoint(0.0f,0.0f,0.0f);

	switch (msg) {
	case MOUSE_PROPCLICK:
		ip->PopCommandMode ();
		break;

	case MOUSE_POINT:
		ip->SetActiveViewport(hwnd);
		vpt = ip->GetViewport(hwnd);
		snapPoint = vpt->SnapPoint (m, m, NULL);
		snapPoint = vpt->MapCPToWorld (snapPoint);

		// Check to see if we've hit any of the subobjects we're looking for:
		hr = HitTest (m, vpt);

		// Done with the viewport:
		ip->ReleaseViewport(vpt);
	
		if (!hr) break;

		theHold.Begin ();
		{
			// if mWhichEnd is 0, we're setting targ1 based on this hit, and leaving targ2 as it is;
			// otherwise, we do the reverse.
			int targ1 = mWhichEnd ? mpEPoly->getParamBlock()->GetInt(epm_bridge_target_1)-1 : hr->hitInfo;
			int targ2 = mWhichEnd ? hr->hitInfo : mpEPoly->getParamBlock()->GetInt (epm_bridge_target_2)-1;

			// These methods set both the node as well as the parameters for the targets at each end.
			// We use a different method for borders and polygons.
			if (mpEPoly->GetEPolySelLevel() == EPM_SL_BORDER) 
				mpEPoly->EpModBridgeBorders (targ1, targ2, hr->nodeRef);
			else if (mpEPoly->GetEPolySelLevel() == EPM_SL_FACE )
				mpEPoly->EpModBridgePolygons (targ1, targ2, hr->nodeRef);
			else if ( mpEPoly->GetEPolySelLevel() == EPM_SL_EDGE )
				mpEPoly->EpModBridgeEdges (targ1, targ2, hr->nodeRef);
		}


		// Set the undo string as appropriate for the end and subobject level we're picking.
		theHold.Accept (GetString ((mpEPoly->GetEPolySelLevel()==EPM_SL_BORDER) ?
			(mWhichEnd ? IDS_BRIDGE_PICK_EDGE_2 : IDS_BRIDGE_PICK_EDGE_1) :
			(mWhichEnd ? IDS_BRIDGE_PICK_POLYGON_2 : IDS_BRIDGE_PICK_POLYGON_1)));

		// Update the view.
		mpEPoly->EpModRefreshScreen();

		// All done picking.
		ip->PopCommandMode ();
		return false;

	case MOUSE_MOVE:
	case MOUSE_FREEMOVE:
		vpt = ip->GetViewport(hwnd);
		vpt->SnapPreview (m, m, NULL, SNAP_FORCE_3D_RESULT);
		if (HitTest(m,vpt)) SetCursor(ip->GetSysCursor(SYSCUR_SELECT));
		else SetCursor(LoadCursor(NULL,IDC_ARROW));
		if (vpt) ip->ReleaseViewport(vpt);
		break;
	}

	return true;
}

void EditPolyPickBridge1CMode::EnterMode () {
	mpEditPoly->RegisterCMChangedForGrip();
	HWND hSettings = mpEditPoly->GetDlgHandle (ep_settings);
	if (!hSettings) return;
	ICustButton *but = GetICustButton (GetDlgItem (hSettings,IDC_BRIDGE_PICK_TARG1));
	if (but) {
		but->SetCheck(true);
		ReleaseICustButton(but);
	}
}

void EditPolyPickBridge1CMode::ExitMode () {
	HWND hSettings = mpEditPoly->GetDlgHandle (ep_settings);
	if (!hSettings) return;
	ICustButton *but = GetICustButton (GetDlgItem (hSettings,IDC_BRIDGE_PICK_TARG1));
	if (but) {
		but->SetCheck(false);
		ReleaseICustButton(but);
	}
}

void EditPolyPickBridge2CMode::EnterMode () {
	mpEditPoly->RegisterCMChangedForGrip();
	HWND hSettings = mpEditPoly->GetDlgHandle (ep_settings);
	if (!hSettings) return;
	ICustButton *but = GetICustButton (GetDlgItem (hSettings,IDC_BRIDGE_PICK_TARG2));
	if (but) {
		but->SetCheck(true);
		ReleaseICustButton(but);
	}
}

void EditPolyPickBridge2CMode::ExitMode () {
	HWND hSettings = mpEditPoly->GetDlgHandle (ep_settings);
	if (!hSettings) return;
	ICustButton *but = GetICustButton (GetDlgItem (hSettings,IDC_BRIDGE_PICK_TARG2));
	if (but) {
		but->SetCheck(false);
		ReleaseICustButton(but);
	}
}

//-------------------------------------------------------

void EditPolyWeldCMode::EnterMode () {
	mproc.Reset();
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_subobj);
	if (!hGeom) return;
	ICustButton *but = GetICustButton (GetDlgItem (hGeom, IDC_TARGET_WELD));
	but->SetCheck(TRUE);
	ReleaseICustButton(but);
}

void EditPolyWeldCMode::ExitMode () {
	// Commit if needed:
	if ((mpEditPoly->GetPolyOperationID () == ep_op_target_weld_vertex) ||
		(mpEditPoly->GetPolyOperationID () == ep_op_target_weld_edge))
	{
		theHold.Begin();
		mpEditPoly->EpModCommit (TimeValue(0));	// non-animatable operation.
		theHold.Accept (GetString (IDS_EDITPOLY_COMMIT));
	}

	HWND hGeom = mpEditPoly->GetDlgHandle (ep_subobj);
	if (!hGeom) return;
	ICustButton *but = GetICustButton (GetDlgItem (hGeom, IDC_TARGET_WELD));
	but->SetCheck(FALSE);
	ReleaseICustButton(but);
}

void EditPolyWeldMouseProc::DrawWeldLine (HWND hWnd,IPoint2 &m) {
	if (targ1<0) return;

	IPoint2 points[2];
	points[0]  = m1;
	points[1]  = m;
	XORDottedPolyline(hWnd, 2, points);

}

bool EditPolyWeldMouseProc::CanWeldVertices (int v1, int v2) {
	MNMesh *pMesh = mpEditPoly->EpModGetOutputMesh ();
	if (pMesh == NULL) return false;

	MNMesh & mm = *pMesh;
	// If vertices v1 and v2 share an edge, then take a collapse type approach;
	// Otherwise, weld them if they're suitable (open verts, etc.)
	int i;
	for (i=0; i<mm.vedg[v1].Count(); i++) {
		if (mm.e[mm.vedg[v1][i]].OtherVert(v1) == v2) break;
	}
	if (i<mm.vedg[v1].Count()) {
		int ee = mm.vedg[v1][i];
		if (mm.e[ee].f1 == mm.e[ee].f2) return false;
		// there are other conditions, but they're complex....
	} else {
		if (mm.vedg[v1].Count() && (mm.vedg[v1].Count() <= mm.vfac[v1].Count())) return false;
		for (i=0; i<mm.vedg[v1].Count(); i++) {
			for (int j=0; j<mm.vedg[v2].Count(); j++) {
				int e1 = mm.vedg[v1][i];
				int e2 = mm.vedg[v2][j];
				int ov = mm.e[e1].OtherVert (v1);
				if (ov != mm.e[e2].OtherVert (v2)) continue;
				// Edges from these vertices connect to the same other vert.
				// That means we have additional conditions:
				if (((mm.e[e1].v1 == ov) && (mm.e[e2].v1 == ov)) ||
					((mm.e[e1].v2 == ov) && (mm.e[e2].v2 == ov))) return false;	// edges trace same direction, so cannot be merged.
				if (mm.e[e1].f2 > -1) return false;
				if (mm.e[e2].f2 > -1) return false;
				if (mm.vedg[ov].Count() <= mm.vfac[ov].Count()) return false;
			}
		}
	}
	return true;
}

bool EditPolyWeldMouseProc::CanWeldEdges (int e1, int e2) {
	MNMesh *pMesh = mpEditPoly->EpModGetOutputMesh ();
	if (pMesh == NULL) return false;

	MNMesh & mm = *pMesh;
	if (mm.e[e1].f2 > -1) return false;
	if (mm.e[e2].f2 > -1) return false;
	if (mm.e[e1].f1 == mm.e[e2].f1) return false;
	if (mm.e[e1].v1 == mm.e[e2].v1) return false;
	if (mm.e[e1].v2 == mm.e[e2].v2) return false;
	return true;
}

HitRecord *EditPolyWeldMouseProc::HitTest (IPoint2 &m, ViewExp *vpt) {
	vpt->ClearSubObjHitList();
	// Use the default pixel value - no one wanted the old weld_pixels spinner.
	if (mpEditPoly->GetMNSelLevel()==MNM_SL_EDGE) {
		mpEditPoly->SetHitLevelOverride (SUBHIT_EDGES|SUBHIT_OPENONLY);
	}
	mpEditPoly->SetHitTestResult ();
	ip->SubObHitTest (ip->GetTime(), HITTYPE_POINT, 0, 0, &m, vpt);
	mpEditPoly->ClearHitLevelOverride ();
	mpEditPoly->ClearHitTestResult ();
	if (!vpt->NumSubObjHits()) return NULL;

	HitLog& hitLog = vpt->GetSubObjHitList();
	HitRecord *bestHit = NULL;
	if (targ1>-1)
	{
		for (HitRecord *hr = hitLog.First(); hr; hr=hr->Next())
		{
			if (hr->nodeRef->GetActualINode() != mpEditPoly->EpModGetPrimaryNode()) continue;
			if (targ1 == hr->hitInfo) continue;
			if (mpEditPoly->GetMNSelLevel() == MNM_SL_EDGE) {
				if (!CanWeldEdges (targ1, hr->hitInfo)) continue;
			} else {
				if (!CanWeldVertices (targ1, hr->hitInfo)) continue;
			}
			if (!bestHit || (bestHit->distance > hr->distance)) bestHit = hr;
		}
		return bestHit;
	}

	return hitLog.ClosestHit();
}

int EditPolyWeldMouseProc::proc (HWND hwnd, int msg, int point, int flags, IPoint2 m) {
	ViewExp *vpt = NULL;
	int res = TRUE;
	HitRecord* hr = NULL;
	MNMesh *pMesh = NULL;
	int stringID = 0;

	switch (msg) {
	case MOUSE_ABORT:
		// Erase last weld line:
		if (targ1>-1) DrawWeldLine (hwnd, oldm2);
		targ1 = -1;
		return FALSE;

	case MOUSE_PROPCLICK:
		// Erase last weld line:
		if (targ1>-1) DrawWeldLine (hwnd, oldm2);
		ip->PopCommandMode ();
		return FALSE;

	case MOUSE_DBLCLICK:
		return false;

	case MOUSE_POINT:
		ip->SetActiveViewport (hwnd);
		vpt = ip->GetViewport (hwnd);
		hr = HitTest (m, vpt);
		if (!hr) break;
		if (targ1 < 0) {
			targ1 = hr->hitInfo;
			mpEditPoly->EpModSetPrimaryNode (hr->nodeRef);
			m1 = m;
			DrawWeldLine (hwnd, m);
			oldm2 = m;
			break;
		}

		// Otherwise, we're completing the weld.
		// Erase the last weld-line:
		DrawWeldLine (hwnd, oldm2);

		//pMesh = mpEditPoly->EpModGetOutputMesh ();
		//if (pMesh == NULL) return false;

		// Do the weld:
		theHold.Begin();

		switch (mpEditPoly->GetMNSelLevel()) {
		case MNM_SL_VERTEX:
			//Point3 destination = pMesh->P(hr->hitInfo);
			mpEditPoly->EpModWeldVerts (targ1, hr->hitInfo);
			stringID = IDS_WELD_VERTS;
			break;
		case MNM_SL_EDGE:
			mpEditPoly->EpModWeldEdges (targ1, hr->hitInfo);
			stringID = IDS_WELD_EDGES;
			break;
		}

		theHold.Accept (GetString(stringID));
		mpEditPoly->EpModRefreshScreen ();
		targ1 = -1;
		ip->ReleaseViewport(vpt);
		return false;

	case MOUSE_MOVE:
	case MOUSE_FREEMOVE:
		vpt = ip->GetViewport(hwnd);
		if (targ1 > -1) {
			DrawWeldLine (hwnd, oldm2);
			oldm2 = m;
		}
		if (HitTest (m,vpt)) SetCursor(ip->GetSysCursor(SYSCUR_SELECT));
		else SetCursor (LoadCursor (NULL, IDC_ARROW));
		ip->RedrawViews (ip->GetTime());
		if (targ1 > -1) DrawWeldLine (hwnd, m);
		break;
	}
	
	if (vpt) ip->ReleaseViewport(vpt);
	return true;	
}

//-------------------------------------------------------

void EditPolyEditTriCMode::EnterMode() {
	HWND hSurf = mpEditPoly->GetDlgHandle (ep_subobj);
	if (hSurf) {
		ICustButton *but = GetICustButton (GetDlgItem (hSurf, IDC_FS_EDIT_TRI));
		but->SetCheck(TRUE);
		ReleaseICustButton(but);
	}
	mpEditPoly->SetDisplayLevelOverride (MNDISP_DIAGONALS);
	mpEditPoly->EpModLocalDataChanged (PART_DISPLAY);
	mpEditPoly->EpModRefreshScreen ();
}

void EditPolyEditTriCMode::ExitMode() {
	// Auto-commit if needed:
	if (mpEditPoly->GetPolyOperationID () == ep_op_edit_triangulation)
	{
		theHold.Begin();
		mpEditPoly->EpModCommitUnlessAnimating (TimeValue(0));
		theHold.Accept (GetString (IDS_EDITPOLY_COMMIT));
	}

	HWND hSurf = mpEditPoly->GetDlgHandle (ep_subobj);
	if (hSurf) {
		ICustButton *but = GetICustButton (GetDlgItem (hSurf, IDC_FS_EDIT_TRI));
		but->SetCheck (FALSE);
		ReleaseICustButton(but);
	}
	mpEditPoly->ClearDisplayLevelOverride();
	mpEditPoly->EpModLocalDataChanged (PART_DISPLAY);
	mpEditPoly->EpModRefreshScreen ();
}

void EditPolyEditTriProc::VertConnect () {
	theHold.Begin();
	mpEPoly->EpModSetDiagonal (v1, v2);
	theHold.Accept (GetString (IDS_EDIT_TRIANGULATION));
	mpEPoly->EpModRefreshScreen ();
}

//-------------------------------------------------------

void EditPolyTurnEdgeCMode::EnterMode() {
	HWND hSurf = mpEditPoly->GetDlgHandle (ep_subobj);
	if (hSurf) {
		ICustButton *but = GetICustButton (GetDlgItem (hSurf, IDC_TURN_EDGE));
		if (but) {
			but->SetCheck(true);
			ReleaseICustButton(but);
		}
	}
	mpEditPoly->SetDisplayLevelOverride (MNDISP_DIAGONALS);
	mpEditPoly->EpModLocalDataChanged (PART_DISPLAY);
	mpEditPoly->EpModRefreshScreen ();
}

void EditPolyTurnEdgeCMode::ExitMode() {
	// Auto-commit if needed:
	if (mpEditPoly->GetPolyOperationID () == ep_op_turn_edge)
	{
		theHold.Begin();
		mpEditPoly->EpModCommitUnlessAnimating (TimeValue(0));
		theHold.Accept (GetString (IDS_EDITPOLY_COMMIT));
	}

	HWND hSurf = mpEditPoly->GetDlgHandle (ep_subobj);
	if (hSurf) {
		ICustButton *but = GetICustButton (GetDlgItem (hSurf, IDC_TURN_EDGE));
		if (but) {
			but->SetCheck (false);
			ReleaseICustButton(but);
		}
	}
	mpEditPoly->ClearDisplayLevelOverride();
	mpEditPoly->EpModLocalDataChanged (PART_DISPLAY);
	mpEditPoly->EpModRefreshScreen ();
}

HitRecord *EditPolyTurnEdgeProc::HitTestEdges (IPoint2 & m, ViewExp *vpt) {
	vpt->ClearSubObjHitList();

	mpEPoly->SetHitTestResult ();
	mpEPoly->ForceIgnoreBackfacing (true);
	ip->SubObHitTest (ip->GetTime(), HITTYPE_POINT, 0, 0, &m, vpt);
	mpEPoly->ForceIgnoreBackfacing (false);
	mpEPoly->ClearHitTestResult ();

	if (!vpt->NumSubObjHits()) return NULL;
	HitLog& hitLog = vpt->GetSubObjHitList();

	// Find the closest hit that is not a border edge:
	HitRecord *hr = NULL;
	for (HitRecord *hitRec=hitLog.First(); hitRec != NULL; hitRec=hitRec->Next()) {
		// Always want the closest hit pixelwise:
		if (hr && (hr->distance < hitRec->distance)) continue;
		MNMesh *pMesh = mpEPoly->EpModGetOutputMesh (hitRec->nodeRef);
		if (pMesh==NULL) continue;
		if (pMesh->e[hitRec->hitInfo].f2<0) continue;
		hr = hitRec;
	}

	return hr;	// (may be NULL.)
}

HitRecord *EditPolyTurnEdgeProc::HitTestDiagonals (IPoint2 & m, ViewExp *vpt) {
	vpt->ClearSubObjHitList();

	mpEPoly->SetHitTestResult ();
	mpEPoly->SetHitLevelOverride (SUBHIT_MNDIAGONALS);
	mpEPoly->ForceIgnoreBackfacing (true);
	ip->SubObHitTest (ip->GetTime(), HITTYPE_POINT, 0, 0, &m, vpt);
	mpEPoly->ForceIgnoreBackfacing (false);
	mpEPoly->ClearHitLevelOverride ();
	mpEPoly->ClearHitTestResult ();

	if (!vpt->NumSubObjHits()) return NULL;
	HitLog& hitLog = vpt->GetSubObjHitList();
	return hitLog.ClosestHit ();
}

int EditPolyTurnEdgeProc::proc (HWND hwnd, int msg, int point, int flags, IPoint2 m) {
	ViewExp* vpt = NULL;
	HitRecord* hr = NULL;
	Point3 snapPoint(0.0f,0.0f,0.0f);
	MNDiagonalHitData *hitDiag=NULL;

	switch (msg) {
	case MOUSE_PROPCLICK:
		ip->PopCommandMode ();
		break;

	case MOUSE_POINT:
		if (point==1) break;

		ip->SetActiveViewport(hwnd);
		vpt = ip->GetViewport(hwnd);
		snapPoint = vpt->SnapPoint (m, m, NULL);
		snapPoint = vpt->MapCPToWorld (snapPoint);
		//hr = HitTestEdges (m, vpt);
		//if (!hr) {
			hr = HitTestDiagonals (m, vpt);
			if (hr) hitDiag = (MNDiagonalHitData *) hr->hitData;
		//}
		ip->ReleaseViewport(vpt);
	
		if (!hr) break;

		theHold.Begin ();
		if (hitDiag) mpEPoly->EpModTurnDiagonal (hitDiag->mFace, hitDiag->mDiag, hr->nodeRef);
		//else mpEPoly->EpModTurnEdge (hr->hitInfo, hr->nodeRef);
		theHold.Accept (GetString (IDS_TURN_EDGE));
		mpEPoly->EpModRefreshScreen();
		break;

	case MOUSE_MOVE:
	case MOUSE_FREEMOVE:
		vpt = ip->GetViewport(hwnd);
		vpt->SnapPreview (m, m, NULL, SNAP_FORCE_3D_RESULT);
		if (/*HitTestEdges(m,vpt) ||*/ HitTestDiagonals(m,vpt)) SetCursor(ip->GetSysCursor(SYSCUR_SELECT));
		else SetCursor(LoadCursor(NULL,IDC_ARROW));
		if (vpt) ip->ReleaseViewport(vpt);
		break;
	}

	return true;
}

//////////////////////////////////////////

void EditPolyMod::DoAccept(TimeValue t)
{
	EpModCommitUnlessAnimating (t);
}

//for EditSSCB 
void EditPolyMod::SetPinch(TimeValue t, float pinch)
{
	getParamBlock()->SetValue (epm_ss_pinch, t, pinch);
}
void EditPolyMod::SetBubble(TimeValue t, float bubble)
{
	getParamBlock()->SetValue (epm_ss_bubble, t, bubble);
}
float EditPolyMod::GetPinch(TimeValue t)
{
	return getParamBlock()->GetFloat (epm_ss_pinch, t);

}
float EditPolyMod::GetBubble(TimeValue t)
{
	return getParamBlock()->GetFloat (epm_ss_bubble, t);
}
float EditPolyMod::GetFalloff(TimeValue t)
{
	return getParamBlock()->GetFloat (epm_ss_falloff, t);
}

void EditPolyMod::SetFalloff(TimeValue t, float falloff)
{
	getParamBlock()->SetValue (epm_ss_falloff, t, falloff);
			//not needed I don't think!
	EpModCommitUnlessAnimating (t);

}


