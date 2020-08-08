 /**********************************************************************  
 *<
	FILE: PolyModes.cpp

	DESCRIPTION: Editable Polygon Mesh Object - Command modes

	CREATED BY: Steve Anderson

	HISTORY: created April 2000

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

#include "EPoly.h"
#include "PolyEdit.h"
#include "macrorec.h"
#include "decomp.h"
#include "spline3d.h"
#include "splshape.h"
#include "shape.h"
#include "winutil.h"

#define EPSILON .0001f

CommandMode * EditPolyObject::getCommandMode (int mode) {
	switch (mode) {
	case epmode_create_vertex: return createVertMode;
	case epmode_create_edge: return createEdgeMode;
	case epmode_create_face: return createFaceMode;
	case epmode_divide_edge: return divideEdgeMode;
	case epmode_divide_face: return divideFaceMode;

	case epmode_extrude_vertex:
	case epmode_extrude_edge:
		return extrudeVEMode;
	case epmode_extrude_face: return extrudeMode;
	case epmode_chamfer_vertex:
	case epmode_chamfer_edge:
		return chamferMode;
	case epmode_bevel: return bevelMode;
	case epmode_inset_face: return insetMode;
	case epmode_outline: return outlineMode;
	case epmode_cut_vertex: return cutMode;
	case epmode_cut_edge: return cutMode;
	case epmode_cut_face: return cutMode;
	case epmode_quickslice: return quickSliceMode;
	case epmode_weld: return weldMode;
	case epmode_lift_from_edge: return liftFromEdgeMode;
	case epmode_pick_lift_edge: return pickLiftEdgeMode;
	case epmode_edit_tri: return editTriMode;
	case epmode_bridge_border: return bridgeBorderMode;
	case epmode_bridge_polygon: return bridgePolyMode;
	case epmode_bridge_edge: return bridgeEdgeMode;
	case epmode_pick_bridge_1: return pickBridgeTarget1;
	case epmode_pick_bridge_2: return pickBridgeTarget2;
	case epmode_turn_edge: return turnEdgeMode;
	case epmode_edit_ss:  return editSSMode;
	}
	return NULL;
}
int EditPolyObject::EpActionGetCommandMode()
{

	if (sliceMode) return epmode_sliceplane;

	if (!ip) return -1;
	CommandMode *currentMode = ip->GetCommandMode ();

	if (currentMode == createVertMode) return epmode_create_vertex;
	if (currentMode == createEdgeMode) return epmode_create_edge;
	if (currentMode == createFaceMode) return epmode_create_face;
	if (currentMode == divideEdgeMode) return epmode_divide_edge;
	if (currentMode == divideFaceMode) return epmode_divide_face;
	if (currentMode == bridgeBorderMode) return epmode_bridge_border;
	if (currentMode == bridgePolyMode) return epmode_bridge_polygon;
	if (currentMode == bridgeEdgeMode) return epmode_bridge_edge;
	if (currentMode == extrudeVEMode) {
		if (meshSelLevel[selLevel] == MNM_SL_EDGE) return epmode_extrude_edge;
		else return epmode_extrude_vertex;
	}
	if (currentMode == extrudeMode) return epmode_extrude_face;
	if (currentMode == chamferMode) {
		if (meshSelLevel[selLevel] == MNM_SL_EDGE) return epmode_chamfer_edge;
		else return epmode_chamfer_vertex;
	}
	if (currentMode == bevelMode) return epmode_bevel;
	if (currentMode == insetMode) return epmode_inset_face;
	if (currentMode == outlineMode) return epmode_outline;
	if (currentMode == cutMode)
	{
		if (meshSelLevel[selLevel] == MNM_SL_VERTEX)
			return epmode_cut_vertex;
		else if(meshSelLevel[selLevel]==MNM_SL_EDGE)
			return epmode_cut_edge;
		else if(meshSelLevel[selLevel]==MNM_SL_FACE)
			return epmode_cut_face;
	}
	if (currentMode == quickSliceMode) return epmode_quickslice;
	if (currentMode == weldMode) return epmode_weld;
	if (currentMode == pickBridgeTarget1) return epmode_pick_bridge_1;
	if (currentMode == pickBridgeTarget2) return epmode_pick_bridge_2;
	if (currentMode == editTriMode) return epmode_edit_tri;
	if (currentMode == turnEdgeMode) return epmode_turn_edge;
	if (currentMode==editSSMode) return epmode_edit_ss;

	return -1;

}
void EditPolyObject::EpActionToggleCommandMode (int mode) {
	if (!ip) return;
	if ((mode == epmode_sliceplane) && (selLevel == EP_SL_OBJECT)) return;

#ifdef EPOLY_MACRO_RECORD_MODES_AND_DIALOGS
	macroRecorder->FunctionCall(_T("$.EditablePoly.toggleCommandMode"), 1, 0, mr_int, mode);
	macroRecorder->EmitScript ();
#endif

	if (mode == epmode_sliceplane) {
		// Special case.
		ip->ClearPickMode();
		if (sliceMode) ExitSliceMode();
		else {
			EpPreviewCancel ();
			EpfnClosePopupDialog ();	// Cannot have slice mode, settings at same time.
			EnterSliceMode();
		}
		return;
	}

	// Otherwise, make sure we're not in Slice mode:
	if (sliceMode) ExitSliceMode ();

	CommandMode *cmd = getCommandMode (mode);
	if (cmd==NULL) return;
	CommandMode *currentMode = ip->GetCommandMode ();

	switch (mode) {
	case epmode_extrude_vertex:
	case epmode_chamfer_vertex:
		// Special case - only use DeleteMode to exit mode if in correct SO level,
		// Otherwise use EnterCommandMode to switch SO levels and enter mode again.
		if ((currentMode==cmd) && (selLevel==EP_SL_VERTEX)) ip->DeleteMode (cmd);
		else {
			EpPreviewCancel ();
			EpfnClosePopupDialog ();	// Cannot have command mode, settings at same time.
			EnterCommandMode (mode);
		}
		break;
	case epmode_extrude_edge:
	case epmode_chamfer_edge:
		// Special case - only use DeleteMode to exit mode if in correct SO level,
		// Otherwise use EnterCommandMode to switch SO levels and enter mode again.
		if ((currentMode==cmd) && (meshSelLevel[selLevel]==MNM_SL_EDGE)) ip->DeleteMode (cmd);
		else {
			EpPreviewCancel ();
			EpfnClosePopupDialog ();	// Cannot have command mode, settings at same time.
			EnterCommandMode (mode);
		}
		break;
	case epmode_pick_lift_edge:
		// Special case - we do not want to end our preview or close the dialog,
		// since this command mode is controlled from the dialog.
		if (currentMode == cmd) ip->DeleteMode (cmd);
		else EnterCommandMode (mode);
		break;

	case epmode_pick_bridge_1:
	case epmode_pick_bridge_2:
		// Another special case - we do not want to end our preview or close the dialog,
		// since this command mode is controlled from the dialog.
		if (currentMode == cmd) ip->DeleteMode (cmd);
		else EnterCommandMode (mode);
		break;

	default:
		if (currentMode == cmd) ip->DeleteMode (cmd);
		else {
			EpPreviewCancel ();
			EpfnClosePopupDialog ();	// Cannot have command mode, settings at same time.
			EnterCommandMode (mode);
		}
		break;
	}
}

void EditPolyObject::EnterCommandMode(int mode) {
	if (!ip) return;

	// First of all, we don't want to pile up our command modes:
	ExitAllCommandModes (false, false);

	switch (mode) {
	case epmode_create_vertex:
		if (selLevel != EP_SL_VERTEX) ip->SetSubObjectLevel (EP_SL_VERTEX);
		ip->PushCommandMode(createVertMode);
		break;

	case epmode_create_edge:
		if (meshSelLevel[selLevel] != MNM_SL_EDGE) ip->SetSubObjectLevel (EP_SL_EDGE);
		ip->PushCommandMode (createEdgeMode);
		break;

	case epmode_create_face:
	
		if (selLevel != EP_SL_FACE) ip->SetSubObjectLevel (EP_SL_FACE);
		ip->PushCommandMode (createFaceMode);
		break;

	case epmode_divide_edge:
		if (meshSelLevel[selLevel] != MNM_SL_EDGE) ip->SetSubObjectLevel (EP_SL_EDGE);
		ip->PushCommandMode (divideEdgeMode);
		break;

	case epmode_divide_face:
		if (selLevel < EP_SL_FACE) ip->SetSubObjectLevel (EP_SL_FACE);
		ip->PushCommandMode (divideFaceMode);
		break;

	case epmode_extrude_vertex:
		if (selLevel != EP_SL_VERTEX) ip->SetSubObjectLevel (EP_SL_VERTEX);
	
		ip->PushCommandMode (extrudeVEMode);
		break;

	case epmode_extrude_edge:
		if (meshSelLevel[selLevel] != MNM_SL_EDGE) ip->SetSubObjectLevel (EP_SL_EDGE);
	
		ip->PushCommandMode (extrudeVEMode);
		break;

	case epmode_extrude_face:
		if (selLevel < EP_SL_FACE) ip->SetSubObjectLevel (EP_SL_FACE);
		ip->PushCommandMode (extrudeMode);
		break;

	case epmode_chamfer_vertex:
		if (selLevel != EP_SL_VERTEX) ip->SetSubObjectLevel (EP_SL_VERTEX);
		ip->PushCommandMode (chamferMode);
		break;

	case epmode_chamfer_edge:
		if (meshSelLevel[selLevel] != MNM_SL_EDGE) ip->SetSubObjectLevel (EP_SL_EDGE);
		ip->PushCommandMode (chamferMode);
		break;

	case epmode_bevel:
		if (selLevel < EP_SL_FACE) ip->SetSubObjectLevel (EP_SL_FACE);
		ip->PushCommandMode (bevelMode);
		break;


	case epmode_inset_face:
		if (meshSelLevel[selLevel] != MNM_SL_FACE) ip->SetSubObjectLevel (EP_SL_FACE);
		ip->PushCommandMode (insetMode);
		break;

	case epmode_outline:
		if (meshSelLevel[selLevel] != MNM_SL_FACE) ip->SetSubObjectLevel (EP_SL_FACE);
		ip->PushCommandMode (outlineMode);
		break;
	case epmode_edit_ss:
		ip->PushCommandMode (editSSMode);
		break;

	//(Should never have been 3 of these.)
	case epmode_cut_vertex:
	case epmode_cut_edge:
	case epmode_cut_face:
		ip->PushCommandMode (cutMode);
		break;


	case epmode_quickslice:
		ip->PushCommandMode (quickSliceMode);
		break;

	case epmode_weld:
		if (selLevel > EP_SL_BORDER) ip->SetSubObjectLevel (EP_SL_VERTEX);
		if (selLevel == EP_SL_BORDER) ip->SetSubObjectLevel (EP_SL_EDGE);
		ip->PushCommandMode (weldMode);
		break;


	case epmode_lift_from_edge:
		if (selLevel != EP_SL_FACE) ip->SetSubObjectLevel (EP_SL_FACE);
		ip->PushCommandMode (liftFromEdgeMode);
		break;


	case epmode_pick_lift_edge:
		ip->PushCommandMode (pickLiftEdgeMode);
		break;

	case epmode_edit_tri:
		if (selLevel < EP_SL_EDGE) ip->SetSubObjectLevel (EP_SL_FACE);
		ip->PushCommandMode (editTriMode);
		break;

	case epmode_turn_edge:
		if (selLevel<EP_SL_EDGE) ip->SetSubObjectLevel (EP_SL_FACE);
		ip->PushCommandMode (turnEdgeMode);
		break;

	case epmode_bridge_border:
		if (selLevel != EP_SL_BORDER) ip->SetSubObjectLevel (EP_SL_BORDER);
		ip->PushCommandMode (bridgeBorderMode);
		break;

	case epmode_bridge_edge:
		if (selLevel != EP_SL_EDGE) 
			ip->SetSubObjectLevel (EP_SL_EDGE);
		ip->PushCommandMode (bridgeEdgeMode);
		break;
		
	case epmode_bridge_polygon:
		if (selLevel != EP_SL_FACE) ip->SetSubObjectLevel (EP_SL_FACE);
		ip->PushCommandMode (bridgePolyMode);
		break;

	case epmode_pick_bridge_1:
		ip->PushCommandMode (pickBridgeTarget1);
		break;
		
	case epmode_pick_bridge_2:
		ip->PushCommandMode (pickBridgeTarget2);
		break;
	}
}

void EditPolyObject::EpActionEnterPickMode (int mode) {
	if (!ip) return;

	// Make sure we're not in Slice mode:
	if (sliceMode) ExitSliceMode ();

#ifdef EPOLY_MACRO_RECORD_MODES_AND_DIALOGS
	macroRecorder->FunctionCall(_T("$.EditablePoly.enterPickMode"), 1, 0, mr_int, mode);
	macroRecorder->EmitScript ();
#endif

	switch (mode) {
	case epmode_attach:
		ip->SetPickMode (attachPickMode);
		break;
	case epmode_pick_shape:
		if (GetMNSelLevel () != MNM_SL_FACE) return;
	
		ip->SetPickMode (shapePickMode);
		break;
	}
}

//------------Command modes & Mouse procs----------------------

HitRecord *PickEdgeMouseProc::HitTestEdges (IPoint2 &m, ViewExp *vpt, float *prop, 
											Point3 *snapPoint) {
	vpt->ClearSubObjHitList();

	EditPolyObject *pEditPoly = (EditPolyObject *) mpEPoly;	// STEVE: make go away someday.
	if (pEditPoly->GetEPolySelLevel() != EP_SL_BORDER)
		pEditPoly->SetHitLevelOverride (SUBHIT_MNEDGES);
	ip->SubObHitTest(ip->GetTime(),HITTYPE_POINT,0, 0, &m, vpt);
	if (pEditPoly->GetEPolySelLevel() != EP_SL_BORDER)
		pEditPoly->ClearHitLevelOverride ();
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

	MNMesh & mm = *(mpEPoly->GetMeshPtr());
	Point3 A = obj2world * mm.v[mm.e[ee].v1].p;
	Point3 B = obj2world * mm.v[mm.e[ee].v2].p;
	Point3 Xdir = B-A;
	Xdir -= DotProd(Xdir, Zdir)*Zdir;

	// OLP Avoid division by zero
	float XdirLength = LengthSquared(Xdir);
	if (XdirLength == 0.0f) {
		*prop = 1;
		return hr;
	}
	*prop = DotProd (Xdir, *snapPoint-A) / XdirLength;

	if (*prop<.0001f) *prop=0;
	if (*prop>.9999f) *prop=1;
	return hr;
}

int PickEdgeMouseProc::proc (HWND hwnd, int msg, int point, int flags, IPoint2 m) {
	ViewExp* vpt = NULL;
	HitRecord* hr = NULL;
	float prop;
	Point3 snapPoint;

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
		if (!EdgePick (hr->hitInfo, prop)) {
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

HitRecord *PickFaceMouseProc::HitTestFaces (IPoint2 &m, ViewExp *vpt) {
	vpt->ClearSubObjHitList();

	EditPolyObject *pEditPoly = (EditPolyObject *) mpEPoly;	// STEVE: make go away someday.
	pEditPoly->SetHitLevelOverride (SUBHIT_MNFACES);
	ip->SubObHitTest(ip->GetTime(),HITTYPE_POINT,0, 0, &m, vpt);
	pEditPoly->ClearHitLevelOverride ();
	if (!vpt->NumSubObjHits()) return NULL;
	HitLog& hitLog = vpt->GetSubObjHitList();
	HitRecord *hr = hitLog.ClosestHit();
	return hr;
}

void PickFaceMouseProc::ProjectHitToFace (IPoint2 &m, ViewExp *vpt,
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
	MNMesh & mm = *(mpEPoly->GetMeshPtr());
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

int PickFaceMouseProc::proc (HWND hwnd, int msg, int point, int flags, IPoint2 m) {
	ViewExp* vpt = NULL;
	HitRecord* hr = NULL;
	Point3 snapPoint;

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

HitRecord *ConnectVertsMouseProc::HitTestVertices (IPoint2 & m, ViewExp *vpt) {
	vpt->ClearSubObjHitList();

	EditPolyObject *pEditPoly = (EditPolyObject *) mpEPoly;	// STEVE: make go away someday.
	pEditPoly->SetHitLevelOverride (SUBHIT_MNVERTS);
	ip->SubObHitTest(ip->GetTime(),HITTYPE_POINT,0, 0, &m, vpt);
	pEditPoly->ClearHitLevelOverride ();
	if (!vpt->NumSubObjHits()) return NULL;
	HitLog& hitLog = vpt->GetSubObjHitList();
	HitRecord *hr = hitLog.ClosestHit();
	if (v1<0) {
		// can't accept vertices on no faces.
		if (mpEPoly->GetMeshPtr()->vfac[hr->hitInfo].Count() == 0) return NULL;
		return hr;
	}

	// Otherwise, we're looking for a vertex on v1's faces - these are listed in neighbors.
	for (int i=0; i<neighbors.Count(); i++) 
	{
		if (neighbors[i] == hr->hitInfo) 
			return hr;
	}

	return NULL;
}

void ConnectVertsMouseProc::DrawDiag (HWND hWnd, const IPoint2 & m, bool bErase, bool bPresent) {
	if (v1<0) return;

	IPoint2 pts[2];
	pts[0].x = m1.x;
	pts[0].y = m1.y;
	pts[1].x = m.x;
	pts[1].y = m.y;
	XORDottedPolyline(hWnd, 2, pts, 0, bErase, !bPresent);
}

void ConnectVertsMouseProc::SetV1 (int vv) {
	v1 = vv;
	neighbors.ZeroCount();
	Tab<int> & vf = mpEPoly->GetMeshPtr()->vfac[vv];
	Tab<int> & ve = mpEPoly->GetMeshPtr()->vedg[vv];
	// Add to neighbors all the vertices that share faces (but no edges) with this one:
	int i,j,k;
	for (i=0; i<vf.Count(); i++) {
		MNFace & mf = mpEPoly->GetMeshPtr()->f[vf[i]];
		for (j=0; j<mf.deg; j++) {
			// Do not consider v1 a neighbor:
			if (mf.vtx[j] == v1) continue;

			// Filter out those that share an edge with v1:
			for (k=0; k<ve.Count(); k++) {
				if (mpEPoly->GetMeshPtr()->e[ve[k]].OtherVert (vv) == mf.vtx[j]) break;
			}
			if (k<ve.Count()) continue;

			// Add to neighbor list.
			neighbors.Append (1, &(mf.vtx[j]), 4);
		}
	}
}

int ConnectVertsMouseProc::proc (HWND hwnd, int msg, int point, int flags, IPoint2 m) {
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
			SetV1 (hr->hitInfo);
			m1 = m;
			lastm = m;
			DrawDiag (hwnd, m, false, true);
			break;
		}
		// Otherwise, we've found a connection.
		DrawDiag (hwnd, lastm, true, true);	// erase last dotted line.
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
		DrawDiag (hwnd, lastm, true, false);
		DrawDiag (hwnd, m, false, true);
		lastm = m;
		break;
	}

	return TRUE;
}

// -------------------------------------------------------

static HCURSOR hCurCreateVert = NULL;

void CreateVertCMode::EnterMode() {
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom);
	if (!hGeom) return;
	ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_CREATE));
	but->SetCheck(TRUE);
	ReleaseICustButton(but);
}

void CreateVertCMode::ExitMode() {
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom);
	if (!hGeom) return;
	ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_CREATE));
	but->SetCheck(FALSE);
	ReleaseICustButton(but);
}

int CreateVertMouseProc::proc (HWND hwnd, int msg, int point, int flags, IPoint2 m) {
	if (!hCurCreateVert) hCurCreateVert = LoadCursor(hInstance,MAKEINTRESOURCE(IDC_ADDVERTCUR)); 

	ViewExp *vpt = ip->GetViewport (hwnd);
	Matrix3 ctm;
	Point3 pt;
	IPoint2 m2;

	switch (msg) {
	case MOUSE_PROPCLICK:
		ip->PopCommandMode ();
		break;

	case MOUSE_POINT:
		ip->SetActiveViewport(hwnd);
		vpt->GetConstructionTM(ctm);
		pt = vpt->SnapPoint (m, m2, &ctm);
		pt = pt * ctm;
		theHold.Begin ();
		if (mpEPoly->EpfnCreateVertex(pt)<0) {
			theHold.Cancel ();
		} else {
			theHold.Accept (GetString (IDS_CREATE_VERTEX));
			mpEPoly->RefreshScreen ();
		}
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

void CreateEdgeCMode::EnterMode () {
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom);
	if (hGeom) {
		ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_CREATE));
		but->SetCheck(TRUE);
		ReleaseICustButton(but);
	}
	mpEditPoly->SetDisplayLevelOverride (MNDISP_VERTTICKS);
	mpEditPoly->NotifyDependents (FOREVER, PART_DISPLAY, REFMSG_CHANGE);
	mpEditPoly->ip->RedrawViews(mpEditPoly->ip->GetTime());
}

void CreateEdgeCMode::ExitMode() {
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom);
	if (hGeom) {
		ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_CREATE));
		but->SetCheck (FALSE);
		ReleaseICustButton(but);
	}
	mpEditPoly->ClearDisplayLevelOverride ();
	mpEditPoly->NotifyDependents (FOREVER, PART_DISPLAY, REFMSG_CHANGE);
	mpEditPoly->ip->RedrawViews(mpEditPoly->ip->GetTime());
}

void CreateEdgeMouseProc::VertConnect () {
	theHold.Begin();
	if (mpEPoly->EpfnCreateEdge (v1, v2) < 0) {
		theHold.Cancel ();
		return;
	}
	theHold.Accept (GetString (IDS_CREATE_EDGE));
	mpEPoly->RefreshScreen ();
}

//----------------------------------------------------------

void CreateFaceCMode::EnterMode() {
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom);
	if (hGeom) {
		ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_CREATE));
		but->SetCheck(TRUE);
		ReleaseICustButton(but);
	}
	mpEditPoly->SetDisplayLevelOverride (MNDISP_VERTTICKS);
	mpEditPoly->NotifyDependents (FOREVER, PART_DISPLAY, REFMSG_CHANGE);
	mpEditPoly->ip->RedrawViews(mpEditPoly->ip->GetTime());
}

void CreateFaceCMode::ExitMode() {
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom);
	if (hGeom) {
		ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_CREATE));
		but->SetCheck (FALSE);
		ReleaseICustButton(but);
	}
	mpEditPoly->ClearDisplayLevelOverride ();
	mpEditPoly->NotifyDependents (FOREVER, PART_DISPLAY, REFMSG_CHANGE);
	mpEditPoly->ip->RedrawViews(mpEditPoly->ip->GetTime());
}

void CreateFaceCMode::Display (GraphicsWindow *gw) {
	proc.DrawEstablishedFace(gw);
}
Point3 CreateFaceMouseProc::GetVertexWP(int in_vertex)
{
	ModContextList mcList;
	INodeTab nodes;
	GetInterface()->GetModContexts(mcList,nodes);

	MNMesh *pMesh = &m_EPoly->mm;

	if(nodes[0]&&in_vertex>=0&&in_vertex< pMesh->VNum())
		return nodes[0]->GetObjectTM(GetInterface()->GetTime()) * pMesh->P(in_vertex);
	return Point3(0.0f,0.0f,0.0f);
}
bool CreateFaceMouseProc::HitTestVertex(IPoint2 in_mouse, ViewExp* in_viewport, INode*& out_node, int& out_vertex) {
	in_viewport->ClearSubObjHitList();
	// why SUBHIT_OPENONLY? In an MNMesh, an edge can belong to at most two faces. If two vertices are selected
	// in sequence that specify an edge that belongs to two faces, then a new face containing this subsequence 
	// of vertices cannot be created. Unfortunately, in this situation, EpfnCreateFace will return successfully 
	// but no new face will actually be created. To prevent this odd behaviour, only vertices on edges with less 
	// than two faces can be selected (i.e., vertices on "open" edges can be selected).
	m_EPoly->SetHitLevelOverride (SUBHIT_MNVERTS|SUBHIT_OPENONLY);
	GetInterface()->SubObHitTest(GetInterface()->GetTime(), HITTYPE_POINT, 0, 0, &in_mouse, in_viewport);
	m_EPoly->ClearHitLevelOverride ();

	HitRecord* l_closest = in_viewport->GetSubObjHitList().ClosestHit();
	if (l_closest != 0) { 
		out_vertex = l_closest->hitInfo;
		out_node = l_closest->nodeRef;
	}
	return l_closest != 0;
}

bool CreateFaceMouseProc::CreateVertex(const Point3& in_worldLocation, int& out_vertex) {
	out_vertex = m_EPoly->EpfnCreateVertex(in_worldLocation, false);
	return out_vertex >= 0;
}

bool CreateFaceMouseProc::CreateFace(MaxSDK::Array<int>& in_vertices) {
	return m_EPoly->EpfnCreateFace(in_vertices.asArrayPtr(), static_cast<int>(in_vertices.length())) >= 0;
}

CreateFaceMouseProc::CreateFaceMouseProc( EditPolyObject* in_e, IObjParam* in_i ) 
: CreateFaceMouseProcTemplate(in_i), m_EPoly(in_e) {
	if (!hCurCreateVert) hCurCreateVert = LoadCursor(hInstance,MAKEINTRESOURCE(IDC_ADDVERTCUR));
}

void CreateFaceMouseProc::ShowCreateVertexCursor() {
	SetCursor(hCurCreateVert);
}

BOOL AttachPickMode::Filter(INode *node) {
	if (!mpEditPoly) return false;
	if (!ip) return false;
	if (!node) return FALSE;

	// Make sure the node does not depend on us
	node->BeginDependencyTest();
	mpEditPoly->NotifyDependents(FOREVER,0,REFMSG_TEST_DEPENDENCY);
	if (node->EndDependencyTest()) return FALSE;

	ObjectState os = node->GetObjectRef()->Eval(ip->GetTime());
	if (os.obj->IsSubClassOf(polyObjectClassID)) return TRUE;
	if (os.obj->CanConvertToType(polyObjectClassID)) return TRUE;
	return FALSE;
}

BOOL AttachPickMode::HitTest(IObjParam *ip, HWND hWnd, ViewExp *vpt, IPoint2 m,int flags) {
	if (!mpEditPoly) return false;
	if (!ip) return false;
	return ip->PickNode(hWnd,m,this) ? TRUE : FALSE;
}

BOOL AttachPickMode::Pick(IObjParam *ip,ViewExp *vpt) {
	if (!mpEditPoly) return false;
	INode *node = vpt->GetClosestHit();
	if (!Filter(node)) return FALSE;

	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);

	BOOL ret = TRUE;
	if (nodes[0]->GetMtl() && node->GetMtl() && (nodes[0]->GetMtl()!=node->GetMtl())) {
		ret = DoAttachMatOptionDialog (ip, mpEditPoly);
	}
	if (!ret) {
		nodes.DisposeTemporary ();
		return FALSE;
	}

	bool canUndo = TRUE;
	mpEditPoly->EpfnAttach (node, canUndo, nodes[0], ip->GetTime());
	if (!canUndo) GetSystemSetting (SYSSET_CLEAR_UNDO);
	mpEditPoly->RefreshScreen ();
	nodes.DisposeTemporary ();
	return FALSE;
}

void AttachPickMode::EnterMode(IObjParam *ip) {
	if (!mpEditPoly) return;
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom);
	if (!hGeom) return;
	ICustButton *but = GetICustButton (GetDlgItem (hGeom, IDC_ATTACH));
	but->SetCheck(TRUE);
	ReleaseICustButton(but);
}

void AttachPickMode::ExitMode(IObjParam *ip) {
	if (!mpEditPoly) return;
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom);
	if (!hGeom) return;
	ICustButton *but = GetICustButton (GetDlgItem (hGeom, IDC_ATTACH));
	but->SetCheck(FALSE);
	ReleaseICustButton(but);
}

// -----------------------------

int AttachHitByName::filter(INode *node) {
	if (!node) return FALSE;

	// Make sure the node does not depend on this modifier.
	node->BeginDependencyTest();
	mpEditPoly->NotifyDependents(FOREVER,0,REFMSG_TEST_DEPENDENCY);
	if (node->EndDependencyTest()) return FALSE;

	ObjectState os = node->GetObjectRef()->Eval(mpEditPoly->ip->GetTime());
	if (os.obj->IsSubClassOf(polyObjectClassID)) return TRUE;
	if (os.obj->CanConvertToType(polyObjectClassID)) return TRUE;
	return FALSE;
}

void AttachHitByName::proc(INodeTab &nodeTab) {
	if (inProc) return;
	if (!mpEditPoly->ip) return;
	inProc = TRUE;
	ModContextList mcList;
	INodeTab nodes;
	mpEditPoly->ip->GetModContexts (mcList, nodes);
	BOOL ret = TRUE;
	if (nodes[0]->GetMtl()) {
      int i;
		for (i=0; i<nodeTab.Count(); i++) {
			if (nodeTab[i]->GetMtl() && (nodes[0]->GetMtl()!=nodeTab[i]->GetMtl())) break;
		}
		if (i<nodeTab.Count()) ret = DoAttachMatOptionDialog ((IObjParam *)mpEditPoly->ip, mpEditPoly);
		if (!mpEditPoly->ip) ret = FALSE;
	}
	inProc = FALSE;
	if (ret) mpEditPoly->EpfnMultiAttach (nodeTab, nodes[0], mpEditPoly->ip->GetTime ());
	nodes.DisposeTemporary ();
}

//-----------------------------------------------------------------------/

BOOL ShapePickMode::Filter(INode *node) {
	if (!mpEditPoly) return false;
	if (!ip) return false;
	if (!node) return FALSE;

	// Make sure the node does not depend on us
	node->BeginDependencyTest();
	mpEditPoly->NotifyDependents(FOREVER,0,REFMSG_TEST_DEPENDENCY);
	if (node->EndDependencyTest()) return FALSE;

	ObjectState os = node->GetObjectRef()->Eval(ip->GetTime());
	if (os.obj->IsSubClassOf(splineShapeClassID)) return TRUE;
	if (os.obj->CanConvertToType(splineShapeClassID)) return TRUE;
	return FALSE;
}

BOOL ShapePickMode::HitTest(IObjParam *ip, HWND hWnd, ViewExp *vpt, IPoint2 m,int flags) {
	if (!mpEditPoly) return false;
	if (!ip) return false;
	return ip->PickNode(hWnd,m,this) ? TRUE : FALSE;
}

BOOL ShapePickMode::Pick(IObjParam *ip,ViewExp *vpt) {
	if (!mpEditPoly) return false;
	INode *node = vpt->GetClosestHit();
	if (!Filter(node)) return FALSE;
	mpEditPoly->getParamBlock()->SetValue (ep_extrude_spline_node, ip->GetTime(), node);
	if (!mpEditPoly->EpPreviewOn()) {
		// We're not in preview mode - do the extrusion.
		mpEditPoly->EpActionButtonOp (epop_extrude_along_spline);
	} else {
	mpEditPoly->RefreshScreen ();
	}
	return TRUE;
}

void ShapePickMode::EnterMode(IObjParam *ip) {
	HWND hOp = mpEditPoly->GetDlgHandle (ep_settings);
	ICustButton *but = NULL;
	if (hOp) but = GetICustButton (GetDlgItem (hOp, IDC_EXTRUDE_PICK_SPLINE));
	if (but) {
		but->SetCheck(TRUE);
		ReleaseICustButton(but);
		but = NULL;
	}
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom_face);
	if (hGeom) but = GetICustButton (GetDlgItem (hGeom, IDC_EXTRUDE_ALONG_SPLINE));
	if (but) {
		but->SetCheck(TRUE);
		ReleaseICustButton(but);
		but = NULL;
	}
}

void ShapePickMode::ExitMode(IObjParam *ip) {
	HWND hOp = mpEditPoly->GetDlgHandle (ep_settings);
	ICustButton *but = NULL;
	if (hOp) but = GetICustButton (GetDlgItem (hOp, IDC_EXTRUDE_PICK_SPLINE));
	if (but) {
		but->SetCheck(false);
		ReleaseICustButton(but);
		but = NULL;
	}
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom_face);
	if (hGeom) but = GetICustButton (GetDlgItem (hGeom, IDC_EXTRUDE_ALONG_SPLINE));
	if (but) {
		but->SetCheck(false);
		ReleaseICustButton(but);
		but = NULL;
	}
}

//----------------------------------------------------------

// Divide edge modifies two faces; creates a new vertex and a new edge.
bool DivideEdgeProc::EdgePick (int edge, float prop) {
	theHold.Begin ();
	mpEPoly->EpfnDivideEdge (edge, prop);
	theHold.Accept (GetString (IDS_INSERT_VERTEX));
	mpEPoly->RefreshScreen ();
	return true;	// false = exit mode when done; true = stay in mode.
}

void DivideEdgeCMode::EnterMode() {
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom_edge);
	if (hGeom) {
		ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_INSERT_VERTEX));
		but->SetCheck(TRUE);
		ReleaseICustButton(but);
	}
	mpEditPoly->SetDisplayLevelOverride (MNDISP_VERTTICKS);
	mpEditPoly->NotifyDependents (FOREVER, PART_DISPLAY, REFMSG_CHANGE);
	mpEditPoly->ip->RedrawViews(mpEditPoly->ip->GetTime());
}

void DivideEdgeCMode::ExitMode() {
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom_edge);
	if (hGeom) {
		ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_INSERT_VERTEX));
		but->SetCheck(FALSE);
		ReleaseICustButton(but);
	}
	mpEditPoly->ClearDisplayLevelOverride ();
	mpEditPoly->NotifyDependents (FOREVER, PART_DISPLAY, REFMSG_CHANGE);
	mpEditPoly->ip->RedrawViews(mpEditPoly->ip->GetTime());
}

//----------------------------------------------------------

void DivideFaceProc::FacePick () {
	theHold.Begin ();
	mpEPoly->EpfnDivideFace (face, bary);
	theHold.Accept (GetString (IDS_INSERT_VERTEX));
	mpEPoly->RefreshScreen ();
}

void DivideFaceCMode::EnterMode() {
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom_face);
	if (!hGeom) return;
	ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_INSERT_VERTEX));
	but->SetCheck(TRUE);
	ReleaseICustButton(but);		
	mpEditPoly->SetDisplayLevelOverride (MNDISP_VERTTICKS);
	mpEditPoly->NotifyDependents (FOREVER, PART_DISPLAY, REFMSG_CHANGE);
	mpEditPoly->ip->RedrawViews(mpEditPoly->ip->GetTime());
}

void DivideFaceCMode::ExitMode() {
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom_face);
	if (!hGeom) return;
	ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_INSERT_VERTEX));
	but->SetCheck(FALSE);
	ReleaseICustButton(but);
	mpEditPoly->ClearDisplayLevelOverride ();
	mpEditPoly->NotifyDependents (FOREVER, PART_DISPLAY, REFMSG_CHANGE);
	mpEditPoly->ip->RedrawViews(mpEditPoly->ip->GetTime());
}

// ------------------------------------------------------

int ExtrudeProc::proc (HWND hwnd, int msg, int point, int flags, IPoint2 m ) {	
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
			mpEditPoly->EpfnBeginExtrude(mpEditPoly->GetMNSelLevel(), MN_SEL, ip->GetTime());
			om = m;
		} else {
			ip->RedrawViews(ip->GetTime(),REDRAW_END);
			mpEditPoly->EpfnEndExtrude (true, ip->GetTime());
		}
		break;

	case MOUSE_MOVE:
		p0 = vpt->MapScreenToView (om,float(-200));
		m2.x = om.x;
		m2.y = m.y;
		p1 = vpt->MapScreenToView (m2,float(-200));
		height = Length (p1-p0);
		if (m.y > om.y) height *= -1.0f;
		mpEditPoly->EpfnDragExtrude (height, ip->GetTime());
		mpEditPoly->getParamBlock()->SetValue (ep_face_extrude_height, ip->GetTime(), height);
		ip->RedrawViews(ip->GetTime(),REDRAW_INTERACTIVE);
		break;

	case MOUSE_ABORT:
		mpEditPoly->EpfnEndExtrude (false, ip->GetTime ());
		ip->RedrawViews (ip->GetTime(), REDRAW_END);
		break;
	}

	if (vpt) ip->ReleaseViewport(vpt);
	return TRUE;
}

HCURSOR ExtrudeSelectionProcessor::GetTransformCursor() { 
	static HCURSOR hCur = NULL;
	if ( !hCur ) hCur = LoadCursor(hInstance,MAKEINTRESOURCE(IDC_EXTRUDECUR));
	return hCur; 
}

void ExtrudeCMode::EnterMode() {
	mpEditPoly->SuspendContraints (true);
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom_face);
	if (!hGeom) return;
	ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_EXTRUDE));
	if (but) {
		but->SetCheck (true);
		ReleaseICustButton(but);
	} else {
		DebugPrint ("Editable Poly: we're entering Extrude mode, but we can't find the extrude button!\n");
		DbgAssert (0);
	}
}

void ExtrudeCMode::ExitMode() {
	mpEditPoly->SuspendContraints (false);
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom_face);
	if (!hGeom) return;
	ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_EXTRUDE));
	if (but) {
		but->SetCheck(FALSE);
		ReleaseICustButton(but);
	} else {
		DebugPrint ("Editable Poly: we're exiting Extrude mode, but we can't find the extrude button!\n");
		DbgAssert (0);
	}
}

// ------------------------------------------------------

int ExtrudeVEMouseProc::proc (HWND hwnd, int msg, int point, int flags, IPoint2 m ) {
	ViewExp *vpt=ip->GetViewport (hwnd);
	Point3 p0, p1, p2;
	IPoint2 m1, m2;
	float width, height;

	switch (msg) {
	case MOUSE_PROPCLICK:
		ip->PopCommandMode ();
		break;

	case MOUSE_POINT:
		switch (point) {
		case 0:
			m0 = m;
			m0set = TRUE;
			switch (mpEditPoly->GetMNSelLevel()) {
			case MNM_SL_VERTEX:
				mpEditPoly->getParamBlock()->SetValue (ep_vertex_extrude_height, TimeValue(0), 0.0f);
				mpEditPoly->getParamBlock()->SetValue (ep_vertex_extrude_width, TimeValue(0), 0.0f);
				break;
			case MNM_SL_EDGE:
				mpEditPoly->getParamBlock()->SetValue (ep_edge_extrude_height, TimeValue(0), 0.0f);
				mpEditPoly->getParamBlock()->SetValue (ep_edge_extrude_width, TimeValue(0), 0.0f);
				break;
			}
			mpEditPoly->EpPreviewBegin (epop_extrude);
			break;
		case 1:
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
			switch (mpEditPoly->GetMNSelLevel()) {
			case MNM_SL_VERTEX:
				mpEditPoly->getParamBlock()->SetValue (ep_vertex_extrude_width, ip->GetTime(), width);
				mpEditPoly->getParamBlock()->SetValue (ep_vertex_extrude_height, ip->GetTime(), height);
				break;
			case MNM_SL_EDGE:
				mpEditPoly->getParamBlock()->SetValue (ep_edge_extrude_width, ip->GetTime(), width);
				mpEditPoly->getParamBlock()->SetValue (ep_edge_extrude_height, ip->GetTime(), height);
				break;
			}
			mpEditPoly->EpPreviewAccept ();
			ip->RedrawViews(ip->GetTime());
			break;
		}
		break;

	case MOUSE_MOVE:
		if (!m0set) break;
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
		switch (mpEditPoly->GetMNSelLevel()) {
		case MNM_SL_VERTEX:
			mpEditPoly->getParamBlock()->SetValue (ep_vertex_extrude_width, ip->GetTime(), width);
			mpEditPoly->getParamBlock()->SetValue (ep_vertex_extrude_height, ip->GetTime(), height);
			break;
		case MNM_SL_EDGE:
			mpEditPoly->getParamBlock()->SetValue (ep_edge_extrude_width, ip->GetTime(), width);
			mpEditPoly->getParamBlock()->SetValue (ep_edge_extrude_height, ip->GetTime(), height);
			break;
		}
		mpEditPoly->EpPreviewSetDragging (true);
		mpEditPoly->EpPreviewInvalidate ();
		mpEditPoly->EpPreviewSetDragging (false);

		ip->RedrawViews(ip->GetTime(),REDRAW_INTERACTIVE);
		break;

	case MOUSE_ABORT:
		mpEditPoly->EpPreviewCancel ();
		ip->RedrawViews (ip->GetTime(), REDRAW_END);
		m0set = FALSE;
		break;
	}

	if (vpt) ip->ReleaseViewport(vpt);
	return TRUE;
}

HCURSOR ExtrudeVESelectionProcessor::GetTransformCursor() { 
	static HCURSOR hCur = NULL;
	if (!hCur) {
		if (mpEPoly->GetMNSelLevel() == MNM_SL_EDGE) hCur = LoadCursor(hInstance,MAKEINTRESOURCE(IDC_EXTRUDE_EDGE_CUR));
		else hCur = LoadCursor(hInstance,MAKEINTRESOURCE(IDC_EXTRUDE_VERTEX_CUR));
	}
	return hCur; 
}

void ExtrudeVECMode::EnterMode() {
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom_vertex);
	if (!hGeom) return;
	ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_EXTRUDE));
	but->SetCheck(TRUE);
	ReleaseICustButton(but);
}

void ExtrudeVECMode::ExitMode() {
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom_vertex);
	if (!hGeom) return;
	ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_EXTRUDE));
	but->SetCheck(FALSE);
	ReleaseICustButton(but);
}

// ------------------------------------------------------

int BevelMouseProc::proc (HWND hwnd, int msg, int point, int flags, IPoint2 m ) {	
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
			mpEditPoly->EpfnBeginBevel (mpEditPoly->GetMNSelLevel(), MN_SEL, true, ip->GetTime());
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
			mpEditPoly->EpfnDragBevel (0.0f, height, ip->GetTime());
			mpEditPoly->getParamBlock()->SetValue (ep_bevel_height, ip->GetTime(), height);
			break;
		case 2:
			ip->RedrawViews(ip->GetTime(),REDRAW_END);
			mpEditPoly->EpfnEndBevel (true, ip->GetTime());
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
			mpEditPoly->EpfnDragBevel (0.0f, height, ip->GetTime());
			mpEditPoly->getParamBlock()->SetValue (ep_bevel_height, ip->GetTime(), height);
		} else {
			p0 = vpt->MapScreenToView (m1, float(-200));
			m2.x = m1.x;
			p1 = vpt->MapScreenToView (m2, float(-200));
			float outline = Length (p1-p0);
			if (m.y > m1.y) outline *= -1.0f;
			mpEditPoly->EpfnDragBevel (outline, height, ip->GetTime());
			mpEditPoly->getParamBlock()->SetValue (ep_bevel_outline, ip->GetTime(), outline);
		}

		ip->RedrawViews(ip->GetTime(),REDRAW_INTERACTIVE);
		break;

	case MOUSE_ABORT:
		mpEditPoly->EpfnEndBevel (false, ip->GetTime());
		ip->RedrawViews(ip->GetTime(),REDRAW_END);
		m1set = m0set = FALSE;
		break;
	}

	if (vpt) ip->ReleaseViewport(vpt);
	return TRUE;
}

HCURSOR BevelSelectionProcessor::GetTransformCursor() { 
	static HCURSOR hCur = NULL;
	if ( !hCur ) hCur = LoadCursor(hInstance,MAKEINTRESOURCE(IDC_BEVELCUR));
	return hCur;
}

void BevelCMode::EnterMode() {
	mpEditPoly->SuspendContraints (true);
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom_face);
	if (!hGeom) return;
	ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_BEVEL));
	but->SetCheck(TRUE);
	ReleaseICustButton(but);
}

void BevelCMode::ExitMode() {
	mpEditPoly->SuspendContraints (false);
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom_face);
	if (!hGeom) return;
	ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_BEVEL));
	but->SetCheck(FALSE);
	ReleaseICustButton(but);
}

// ------------------------------------------------------

int InsetMouseProc::proc (HWND hwnd, int msg, int point, int flags, IPoint2 m ) {
	ViewExp *vpt=ip->GetViewport (hwnd);
	Point3 p0, p1;
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
			mpEditPoly->EpfnBeginInset (mpEditPoly->GetMNSelLevel(), MN_SEL, ip->GetTime());
			break;
		case 1:
			ip->RedrawViews(ip->GetTime(),REDRAW_END);
			mpEditPoly->EpfnEndInset (true, ip->GetTime());
			m0set = FALSE;
			break;
		}
		break;

	case MOUSE_MOVE:
		if (!m0set) break;
		p0 = vpt->MapScreenToView (m0, float(-200));
		p1 = vpt->MapScreenToView (m, float(-200));
		inset = Length (p1-p0);
		mpEditPoly->EpfnDragInset (inset, ip->GetTime());
		mpEditPoly->getParamBlock()->SetValue (ep_inset, ip->GetTime(), inset);
		ip->RedrawViews(ip->GetTime(),REDRAW_INTERACTIVE);
		break;

	case MOUSE_ABORT:
		mpEditPoly->EpfnEndInset (false, ip->GetTime());
		ip->RedrawViews(ip->GetTime(),REDRAW_END);
		m0set = FALSE;
		break;
	}

	if (vpt) ip->ReleaseViewport(vpt);
	return TRUE;
}

HCURSOR InsetSelectionProcessor::GetTransformCursor() { 
	static HCURSOR hCur = NULL;
	if ( !hCur ) hCur = LoadCursor(hInstance,MAKEINTRESOURCE(IDC_INSETCUR));
	return hCur;
}

void InsetCMode::EnterMode() {
	mpEditPoly->SuspendContraints (true);
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom_face);
	if (!hGeom) return;
	ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_INSET));
	but->SetCheck(TRUE);
	ReleaseICustButton(but);
}

void InsetCMode::ExitMode() {
	mpEditPoly->SuspendContraints (false);
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom_face);
	if (!hGeom) return;
	ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_INSET));
	but->SetCheck(FALSE);
	ReleaseICustButton(but);
}

// ------------------------------------------------------

int OutlineMouseProc::proc (HWND hwnd, int msg, int point, int flags, IPoint2 m ) {
	ViewExp *vpt=ip->GetViewport (hwnd);
	Point3 p0, p1;
	IPoint2 m1;
	ISpinnerControl *spin=NULL;
	float outline;

	switch (msg) {
	case MOUSE_PROPCLICK:
		if (mpEditPoly->EpPreviewOn()) mpEditPoly->EpPreviewCancel ();
		ip->PopCommandMode ();
		break;

	case MOUSE_POINT:
		switch (point) {
		case 0:
			m0 = m;
			m0set = TRUE;
			// Prevent flash of last outline value:
			mpEditPoly->getParamBlock()->SetValue (ep_outline, ip->GetTime(), 0.0f);
			mpEditPoly->EpPreviewBegin (epop_outline);
			break;
		case 1:
			mpEditPoly->EpPreviewAccept();
			m0set = FALSE;
			break;
		}
		break;

	case MOUSE_MOVE:
		if (!m0set) break;

		// Get signed right/left distance from original point:
		p0 = vpt->MapScreenToView (m0, float(-200));
		m1.y = m.y;
		m1.x = m0.x;
		p1 = vpt->MapScreenToView (m1, float(-200));
		outline = Length (p1-p0);
		if (m.y > m0.y) outline *= -1.0f;

		mpEditPoly->EpPreviewSetDragging (true);
		mpEditPoly->getParamBlock()->SetValue (ep_outline, ip->GetTime(), outline);
		mpEditPoly->EpPreviewSetDragging (false);
		mpEditPoly->RefreshScreen ();
		break;

	case MOUSE_ABORT:
		mpEditPoly->EpPreviewCancel ();
		m0set = FALSE;
		break;
	}

	if (vpt) ip->ReleaseViewport(vpt);
	return TRUE;
}

HCURSOR OutlineSelectionProcessor::GetTransformCursor() { 
	static HCURSOR hCur = NULL;
	if ( !hCur ) hCur = LoadCursor(hInstance,MAKEINTRESOURCE(IDC_INSETCUR));	// STEVE: need new cursor?
	return hCur;
}

void OutlineCMode::EnterMode() {
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom_face);
	if (!hGeom) return;
	ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_OUTLINE));
	but->SetCheck(TRUE);
	ReleaseICustButton(but);
}

void OutlineCMode::ExitMode() {
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom_face);
	if (!hGeom) return;
	ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_OUTLINE));
	but->SetCheck(FALSE);
	ReleaseICustButton(but);
}

// ------------------------------------------------------

int ChamferMouseProc::proc (HWND hwnd, int msg, int point, int flags, IPoint2 m ) {	
	ViewExp *vpt=ip->GetViewport (hwnd);
	Point3 p0, p1;
	static float chamfer;

	switch (msg) {
	case MOUSE_PROPCLICK:
		ip->PopCommandMode ();
		break;

	case MOUSE_POINT:
		if (!point) {
			mpEditPoly->EpfnBeginChamfer (mpEditPoly->GetMNSelLevel(), ip->GetTime());
			chamfer = 0;
			om = m;
		} else {
			ip->RedrawViews (ip->GetTime(),REDRAW_END);
			switch (mpEditPoly->GetMNSelLevel()) {
			case MNM_SL_VERTEX:
				mpEditPoly->getParamBlock()->SetValue (ep_vertex_chamfer, ip->GetTime(), chamfer);
				break;
			case MNM_SL_EDGE:
				mpEditPoly->getParamBlock()->SetValue (ep_edge_chamfer, ip->GetTime(), chamfer);
				break;
			}
			mpEditPoly->EpfnEndChamfer (true, ip->GetTime());
		}
		break;

	case MOUSE_MOVE:
		p0 = vpt->MapScreenToView(om,float(-200));
		p1 = vpt->MapScreenToView(m,float(-200));
		chamfer = Length(p1-p0);
		mpEditPoly->EpfnDragChamfer (chamfer, ip->GetTime());
		switch (mpEditPoly->GetMNSelLevel()) {
		case MNM_SL_VERTEX:
			mpEditPoly->getParamBlock()->SetValue (ep_vertex_chamfer, ip->GetTime(), chamfer);
			break;
		case MNM_SL_EDGE:
			mpEditPoly->getParamBlock()->SetValue (ep_edge_chamfer, ip->GetTime(), chamfer);
			break;
		}
		ip->RedrawViews(ip->GetTime(),REDRAW_INTERACTIVE);
		break;

	case MOUSE_ABORT:
		mpEditPoly->EpfnEndChamfer (false, ip->GetTime());			
		ip->RedrawViews (ip->GetTime(),REDRAW_END);
		break;
	}

	if (vpt) ip->ReleaseViewport(vpt);
	return TRUE;
}

HCURSOR ChamferSelectionProcessor::GetTransformCursor() { 
	static HCURSOR hECur = NULL;
	static HCURSOR hVCur = NULL;
	if (mpEditPoly->GetSelLevel() == EP_SL_VERTEX) {
		if ( !hVCur ) hVCur = LoadCursor(hInstance,MAKEINTRESOURCE(IDC_VCHAMFERCUR));
		return hVCur;
	}
	if ( !hECur ) hECur = LoadCursor(hInstance,MAKEINTRESOURCE(IDC_ECHAMFERCUR));
	return hECur;
}

void ChamferCMode::EnterMode() {
	mpEditPoly->SuspendContraints (true);
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom_vertex);
	if (!hGeom) return;
	ICustButton *but = GetICustButton(GetDlgItem(hGeom,IDC_CHAMFER));
	but->SetCheck(TRUE);
	ReleaseICustButton(but);
}

void ChamferCMode::ExitMode() {
	mpEditPoly->SuspendContraints (false);
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom_vertex);
	if (!hGeom) return;
	ICustButton *but = GetICustButton(GetDlgItem(hGeom,IDC_CHAMFER));
	but->SetCheck(FALSE);
	ReleaseICustButton(but);
}

// --- Slice: not really a command mode, just looks like it.--------- //

static bool sPopSliceCommandMode = false;

void EditPolyObject::EnterSliceMode () {
	sliceMode = TRUE;
	HWND hGeom = GetDlgHandle (ep_geom);
	if (hGeom) {
		ICustButton *but = GetICustButton (GetDlgItem(hGeom,IDC_SLICE));
		but->Enable ();
		ReleaseICustButton (but);
		but = GetICustButton (GetDlgItem (hGeom, IDC_RESET_PLANE));
		but->Enable ();
		ReleaseICustButton (but);
		but = GetICustButton (GetDlgItem (hGeom, IDC_SLICEPLANE));
		but->SetCheck (TRUE);
		ReleaseICustButton (but);
	}

	if (!sliceInitialized) EpResetSlicePlane ();

	EpPreviewBegin (epop_slice);

	// If we're already in a SO move or rotate mode, stay in it;
	// Otherwise, enter SO move.
	if ((ip->GetCommandMode() != moveMode) && (ip->GetCommandMode() != rotMode)) {
		ip->PushCommandMode (moveMode);
		sPopSliceCommandMode = true;
	} else sPopSliceCommandMode = false;

	NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);
	RefreshScreen ();
}

void EditPolyObject::ExitSliceMode () {
	sliceMode = FALSE;

	if (EpPreviewOn()) EpPreviewCancel ();
	HWND hGeom = GetDlgHandle (ep_geom);
	if (hGeom) {
		ICustButton *but = GetICustButton (GetDlgItem(hGeom,IDC_SLICE));
		but->Disable ();
		ReleaseICustButton (but);
		but = GetICustButton (GetDlgItem (hGeom, IDC_RESET_PLANE));
		but->Disable ();
		ReleaseICustButton (but);
		but = GetICustButton (GetDlgItem (hGeom, IDC_SLICEPLANE));
		but->SetCheck (FALSE);
		ReleaseICustButton (but);
	}

	if (sPopSliceCommandMode && (ip->GetCommandStackSize()>1)) ip->PopCommandMode ();

	NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);
	RefreshScreen ();
}

void EditPolyObject::GetSlicePlaneBoundingBox (Box3 & box, Matrix3 *tm) {
	Matrix3 rotMatrix;
	sliceRot.MakeMatrix (rotMatrix);
	rotMatrix.SetTrans (sliceCenter);
	if (tm) rotMatrix *= (*tm);
	box += Point3(-sliceSize,-sliceSize,0.0f)*rotMatrix;
	box += Point3(-sliceSize,sliceSize,0.0f)*rotMatrix;
	box += Point3(sliceSize,sliceSize,0.0f)*rotMatrix;
	box += Point3(sliceSize,-sliceSize,0.0f)*rotMatrix;
}

void EditPolyObject::DisplaySlicePlane (GraphicsWindow *gw) {
	// Draw rectangle representing slice plane.
	Point3 rp[5];
	Matrix3 rotMatrix;
	sliceRot.MakeMatrix (rotMatrix);
	rotMatrix.SetTrans (sliceCenter);
	rp[0] = Point3(-sliceSize,-sliceSize,0.0f)*rotMatrix;
	rp[1] = Point3(-sliceSize,sliceSize,0.0f)*rotMatrix;
	rp[2] = Point3(sliceSize,sliceSize,0.0f)*rotMatrix;
	rp[3] = Point3(sliceSize,-sliceSize,0.0f)*rotMatrix;
	gw->setColor(LINE_COLOR,GetUIColor(COLOR_SEL_GIZMOS));
	gw->polyline (4, rp, NULL, NULL, TRUE, NULL);
}

// -- Cut proc/mode -------------------------------------

HitRecord *CutProc::HitTestVerts (IPoint2 &m, ViewExp *vpt, bool completeAnalysis) {
	vpt->ClearSubObjHitList();

	mpEditPoly->SetHitLevelOverride (SUBHIT_MNVERTS);
	ip->SubObHitTest (ip->GetTime(), HITTYPE_POINT, 0, 0, &m, vpt);
	mpEditPoly->ClearHitLevelOverride ();
	if (!vpt->NumSubObjHits()) return NULL;
	HitLog& hitLog = vpt->GetSubObjHitList();

	// Ok, if we haven't yet started a cut, and we don't want to know the hit location,
	// we're done:
	if ((startIndex<0) && !completeAnalysis) return hitLog.First();

	// Ok, now we need to find the closest eligible point:
	// First we grab our node's transform from the first hit:
	mObj2world = hitLog.First()->nodeRef->GetObjectTM(ip->GetTime());

	int bestHitIndex = -1;
	int bestHitDistance = 0;
	Point3 bestHitPoint(0.0f,0.0f,0.0f);
	HitRecord *ret = NULL;

	for (HitRecord *hitRec=hitLog.First(); hitRec != NULL; hitRec=hitRec->Next()) {
		// Always want the closest hit pixelwise:
		if ((bestHitIndex>-1) && (bestHitDistance < hitRec->distance)) continue;

		if (startIndex >= 0)
		{
			// Check that our hit doesn't touch starting component:
			switch (startLevel) {
			case MNM_SL_VERTEX:
				if (startIndex == int(hitRec->hitInfo)) continue;
				break;
			case MNM_SL_EDGE:
				if (mpEPoly->GetMeshPtr()->e[startIndex].v1 == hitRec->hitInfo) continue;
				if (mpEPoly->GetMeshPtr()->e[startIndex].v2 == hitRec->hitInfo) continue;
				break;
			case MNM_SL_FACE:
				// Any face is suitable.
				break;
			}
		}

		Point3 & p = mpEPoly->GetMeshPtr()->P(hitRec->hitInfo);

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

HitRecord *CutProc::HitTestEdges (IPoint2 &m, ViewExp *vpt, bool completeAnalysis) {
	vpt->ClearSubObjHitList();

	mpEditPoly->SetHitLevelOverride (SUBHIT_MNEDGES);
	ip->SubObHitTest (ip->GetTime(), HITTYPE_POINT, 0, 0, &m, vpt);
	mpEditPoly->ClearHitLevelOverride ();
	int numHits = vpt->NumSubObjHits();
	if (numHits == 0) return NULL;

	HitLog& hitLog = vpt->GetSubObjHitList();

	// Ok, if we haven't yet started a cut, and we don't want to know the actual hit location,
	// we're done:
	if ((startIndex<0) && !completeAnalysis) return hitLog.First ();

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

		if (startIndex>-1) 	{
			// Check that component doesn't touch starting component:
			switch (startLevel) {
			case MNM_SL_VERTEX:
				if (mpEPoly->GetMeshPtr()->e[hitRec->hitInfo].v1 == startIndex) continue;
				if (mpEPoly->GetMeshPtr()->e[hitRec->hitInfo].v2 == startIndex) continue;
				break;
			case MNM_SL_EDGE:
				if (startIndex == int(hitRec->hitInfo)) continue;
				break;
				// (any face is acceptable)
			}
		}

		// Get endpoints in world space:
		Point3 Aobj = mpEPoly->GetMeshPtr()->P(mpEPoly->GetMeshPtr()->e[hitRec->hitInfo].v1);
		Point3 Bobj = mpEPoly->GetMeshPtr()->P(mpEPoly->GetMeshPtr()->e[hitRec->hitInfo].v2);
		Point3 A = mObj2world * Aobj;
		Point3 B = mObj2world * Bobj;

		// Find proportion of our nominal hit point along this edge:
		Point3 Xdir = B-A;
		Xdir -= DotProd(Xdir, mLastHitDirection)*mLastHitDirection;	// make orthogonal to mLastHitDirection.

		// OLP Avoid division by zero
		float prop = 0.0f;
		float XdirLength = LengthSquared (Xdir);
		if (XdirLength == 0.0f)
		{
			prop = 1.0f;
		}
		else
		{
			prop = DotProd (Xdir, mLastHitPoint-A) / XdirLength;
		if (prop<.0001f) prop=0;
		if (prop>.9999f) prop=1;
		}

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

HitRecord *CutProc::HitTestFaces (IPoint2 &m, ViewExp *vpt, bool completeAnalysis) {
	vpt->ClearSubObjHitList();

	mpEditPoly->SetHitLevelOverride (SUBHIT_MNFACES);
	ip->SubObHitTest (ip->GetTime(), HITTYPE_POINT, 0, 0, &m, vpt);
	mpEditPoly->ClearHitLevelOverride ();
	if (!vpt->NumSubObjHits()) return NULL;
	HitLog& hitLog = vpt->GetSubObjHitList();

	// We don't bother to choose the view-direction-closest face,
	// since generally that's the only one we expect to get.
	HitRecord *ret = hitLog.ClosestHit();
	if (!completeAnalysis) return ret;

	mLastHitIndex = ret->hitInfo;
	mObj2world = ret->nodeRef->GetObjectTM (ip->GetTime ());
	
	// Get the average normal for the face, thus the plane, and intersect.
	MNMesh & mm = *(mpEPoly->GetMeshPtr());
	Point3 planeNormal = mm.GetFaceNormal (mLastHitIndex, TRUE);
	planeNormal = Normalize (mObj2world.VectorTransform (planeNormal));
	float planeOffset=0.0f;
	for (int i=0; i<mm.f[mLastHitIndex].deg; i++)
		planeOffset += DotProd (planeNormal, mObj2world*mm.v[mm.f[mLastHitIndex].vtx[i]].p);
	planeOffset = planeOffset/float(mm.f[mLastHitIndex].deg);

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

HitRecord *CutProc::HitTestAll (IPoint2 & m, ViewExp *vpt, int flags, bool completeAnalysis) {
	HitRecord *hr = NULL;
	mLastHitLevel = MNM_SL_OBJECT;	// no hit.

	mpEditPoly->ForceIgnoreBackfacing (true);
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
	mpEditPoly->ForceIgnoreBackfacing (false);

	return hr;
}

void CutProc::DrawCutter (HWND hWnd, bool bErase, bool bPresent) {
	if (startIndex<0) return;

	IPoint2 pts[2];
	pts[0].x = mMouse1.x;
	pts[0].y = mMouse1.y;
	pts[1].x = mMouse2.x;
	pts[1].y = mMouse2.y;
	XORDottedPolyline(hWnd, 2, pts, 0, bErase, !bPresent);
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
		IN EditPolyObject* pEditPoly, 
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
	IN EditPolyObject* pEditPoly, 
	IN const Matrix3& obj2World)
{
	DbgAssert(NULL != pEditPoly);

	Box3 bbox;
	pEditPoly->mm.BBox(bbox);

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

int CutProc::proc (HWND hwnd, int msg, int point, int flags, IPoint2 m ) {
	ViewExp* vpt = NULL;
	IPoint2 betterM(0,0);
	Matrix3 world2obj;
	Ray r;
	static HCURSOR hCutVertCur = NULL, hCutEdgeCur=NULL, hCutFaceCur = NULL;
	bool bDrawNewCutter = false;

	if (!hCutVertCur) {
		hCutVertCur = LoadCursor (hInstance, MAKEINTRESOURCE(IDC_CUT_VERT_CUR));
		hCutEdgeCur = LoadCursor (hInstance, MAKEINTRESOURCE(IDC_CUT_EDGE_CUR));
		hCutFaceCur = LoadCursor (hInstance, MAKEINTRESOURCE(IDC_CUT_FACE_CUR));
	}

	switch (msg) {
	case MOUSE_ABORT:
		startIndex = -1;
		// Turn off Cut preview:
		mpEditPoly->EpPreviewCancel ();
		if (mpEditPoly->EpPreviewGetSuspend ()) {
			mpEditPoly->EpPreviewSetSuspend (false);
			ip->PopPrompt ();
		}
		return FALSE;

	case MOUSE_PROPCLICK:
		ip->PopCommandMode ();
		// Turn off Cut preview:
		mpEditPoly->EpPreviewCancel ();
		if (mpEditPoly->EpPreviewGetSuspend ()) {
			mpEditPoly->EpPreviewSetSuspend (false);
			ip->PopPrompt ();
		}
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
			if (NULL == HitTestAll (m, vpt, flags, true))
			{
				snapPlane.HitTestFarPlane(mLastHitPoint, mpEditPoly, mObj2world);
			}
		}

		ip->ReleaseViewport (vpt);

		if (mLastHitLevel == MNM_SL_OBJECT) return true;

		// Get hit directon in object space:
		world2obj = Inverse (mObj2world);
		mLastHitDirection = Normalize (VectorTransform (world2obj, mLastHitDirection));

		if (startIndex<0) {
			startIndex = mLastHitIndex;
			startLevel = mLastHitLevel;
			mpEPoly->getParamBlock()->SetValue (ep_cut_start_level, TimeValue(0), startLevel);
			mpEPoly->getParamBlock()->SetValue (ep_cut_start_index, TimeValue(0), startIndex);
			mpEPoly->getParamBlock()->SetValue (ep_cut_start_coords, TimeValue(0), mLastHitPoint);
			mpEPoly->getParamBlock()->SetValue (ep_cut_end_coords, TimeValue(0), mLastHitPoint);
			mpEPoly->getParamBlock()->SetValue (ep_cut_normal, TimeValue(0), -mLastHitDirection);
			if (ip->GetSnapState()) {
				// Must suspend "fully interactive" while snapping in Cut mode.
				mpEditPoly->EpPreviewSetSuspend (true);
				ip->PushPrompt (GetString (IDS_CUT_SNAP_PREVIEW_WARNING));
			}
			if (true) {
				MNMeshUtilities mmu(mpEditPoly->GetMeshPtr());
				mmu.CutPrepare();
			}
			mpEditPoly->EpPreviewBegin (epop_cut);
			mMouse1 = betterM;
			mMouse2 = betterM;
			DrawCutter (hwnd, false, true);
			break;
		}

		// Erase last cutter line:
		DrawCutter (hwnd, true, true);

		// Do the cut:
		mpEPoly->getParamBlock()->SetValue (ep_cut_end_coords, TimeValue(0), mLastHitPoint);
		mpEPoly->getParamBlock()->SetValue (ep_cut_normal, TimeValue(0), -mLastHitDirection);
		mpEditPoly->EpPreviewAccept ();

		mpEPoly->getParamBlock()->GetValue (ep_cut_start_index, TimeValue(0), mLastHitIndex, FOREVER);
		mpEPoly->getParamBlock()->GetValue (ep_cut_start_level, TimeValue(0), mLastHitLevel, FOREVER);
		if ((startLevel == mLastHitLevel) && (startIndex == mLastHitIndex)) {
			// no change - cut was unable to finish.
			startIndex = -1;
			if (mpEditPoly->EpPreviewGetSuspend ()) {
				mpEditPoly->EpPreviewSetSuspend (false);
				ip->PopPrompt ();
			}
			return false;	// end cut mode.
		}

		// Otherwise, start on next cut.
		startIndex = mLastHitIndex;
		startLevel = MNM_SL_VERTEX;
		mpEPoly->getParamBlock()->SetValue (ep_cut_start_coords, TimeValue(0),
			mpEPoly->GetMeshPtr()->P(startIndex));
		mpEditPoly->EpPreviewBegin (epop_cut);
		mMouse1 = betterM;
		mMouse2 = betterM;
		DrawCutter (hwnd, false, true);
		return true;

	case MOUSE_MOVE:
		vpt = ip->GetViewport (hwnd);

		// Show snap preview:
		vpt->SnapPreview (m, m, NULL, SNAP_FORCE_3D_RESULT);
		ip->RedrawViews (ip->GetTime(), REDRAW_INTERACTIVE);	// hey - why are we doing this?

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
				snapPlane.HitTestFarPlane(mLastHitPoint, mpEditPoly, mObj2world);
			}
		}
		
		if (startIndex>-1) {
			// Erase last dotted line
			DrawCutter (hwnd, true ,false);

			// Set the cut preview to use the point we just hit on:
			mpEditPoly->EpPreviewSetDragging (true);
			mpEPoly->getParamBlock()->SetValue (ep_cut_end_coords, TimeValue(0), mLastHitPoint);
			mpEditPoly->EpPreviewSetDragging (false);

			// Draw new dotted line
			mMouse2 = betterM;
			bDrawNewCutter = true;
		}

		// Set cursor based on best subobject match:
		switch (mLastHitLevel) {
		case MNM_SL_VERTEX: SetCursor (hCutVertCur); break;
		case MNM_SL_EDGE: SetCursor (hCutEdgeCur); break;
		case MNM_SL_FACE: SetCursor (hCutFaceCur); break;
		default: SetCursor (LoadCursor (NULL, IDC_ARROW));
		}

		ip->ReleaseViewport (vpt);
		// STEVE: why does this need preview protection when the same call in QuickSliceProc doesn't
		int fullyInteractive;
		mpEPoly->getParamBlock()->GetValue (ep_interactive_full, TimeValue(0), fullyInteractive, FOREVER);
		if (fullyInteractive)
			ip->RedrawViews (ip->GetTime(), REDRAW_INTERACTIVE);
		if (bDrawNewCutter)
			DrawCutter (hwnd, false, true);

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

void CutCMode::EnterMode() {
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom);
	if (hGeom) {
		ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_CUT));
		if (but) {
			but->SetCheck(TRUE);
			ReleaseICustButton(but);
		}
	}

	proc.startIndex = -1;
}

void CutCMode::ExitMode() {
	// Lets make double-extra-sure that Cut's suspension of the preview system doesn't leak out...
	// (This line is actually necessary in the case where the user uses a shortcut key to jump-start
	// another command mode.)
	if (mpEditPoly->EpPreviewGetSuspend ()) {
		mpEditPoly->EpPreviewSetSuspend (false);
		if (mpEditPoly->ip) mpEditPoly->ip->PopPrompt ();
	}

	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom);
	if (!hGeom) return;
	ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_CUT));
	but->SetCheck(FALSE);
	ReleaseICustButton(but);
}

//------- QuickSlice proc/mode------------------------

void QuickSliceProc::DrawSlicer (HWND hWnd, bool bErase, bool bPresent) {
	if (!mSlicing) return;

	IPoint2 pts[2];
	pts[0].x = mMouse1.x;
	pts[0].y = mMouse1.y;
	pts[1].x = mMouse2.x;
	pts[1].y = mMouse2.y;
	XORDottedPolyline(hWnd, 2, pts, 0, bErase, !bPresent);
}

// Given two points and a view direction (in obj space), find slice plane:
void QuickSliceProc::ComputeSliceParams (bool center) {
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
	float size;
	if (!mpEditPoly->EpGetSliceInitialized()) mpEPoly->EpResetSlicePlane ();	// initializes size if needed.
	Point3 foo1, foo2;
	mpEPoly->EpGetSlicePlane (foo1, foo2, &size);	// Don't want to modify size.
	if (center) {
		Box3 bbox;
		mpEPoly->GetMeshPtr()->BBox(bbox, false);
		Point3 planeCtr = bbox.Center();
		planeCtr = planeCtr - DotProd (planeNormal, planeCtr) * planeNormal;
		planeCtr += DotProd (planeNormal, mLocal1) * planeNormal;
		mpEPoly->EpSetSlicePlane (planeNormal, planeCtr, size);
	} else {
		mpEPoly->EpSetSlicePlane (planeNormal, (mLocal1+mLocal2)*.5f, size);
	}
}

static HCURSOR hCurQuickSlice = NULL;

int QuickSliceProc::proc (HWND hwnd, int msg, int point, int flags, IPoint2 m ) {
	if (!hCurQuickSlice)
		hCurQuickSlice = LoadCursor(hInstance,MAKEINTRESOURCE(IDC_QUICKSLICE_CURSOR));

	ViewExp* vpt = NULL;
	IPoint2 betterM(0,0);
	Point3 snapPoint(0.0f,0.0f,0.0f);
	Ray r;

	switch (msg) {
	case MOUSE_ABORT:
		if (mSlicing) DrawSlicer (hwnd, true, true);	// Erase last slice line.
		mSlicing = false;
		mpEditPoly->EpPreviewCancel ();
		if (mpEditPoly->EpPreviewGetSuspend()) {
			mpInterface->PopPrompt();
			mpEditPoly->EpPreviewSetSuspend (false);
		}
		return FALSE;

	case MOUSE_PROPCLICK:
		if (mSlicing) DrawSlicer (hwnd, true, true);	// Erase last slice line.
		mSlicing = false;
		mpEditPoly->EpPreviewCancel ();
		if (mpEditPoly->EpPreviewGetSuspend()) {
			mpInterface->PopPrompt();
			mpEditPoly->EpPreviewSetSuspend (false);
		}
		mpInterface->PopCommandMode ();
		return FALSE;

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
			// We need to get node of this polyobject, and obtain world->obj transform.
			ModContextList mcList;
			INodeTab nodes;
			mpInterface->GetModContexts(mcList,nodes);
			Matrix3 otm = nodes[0]->GetObjectTM(mpInterface->GetTime());
			nodes.DisposeTemporary();
			mWorld2obj = Inverse (otm);

			// first point:
			mLocal1 = mWorld2obj * snapPoint;
			mLocal2 = mLocal1;

			// We'll also need to find our Z direction:
			vpt->MapScreenToWorldRay ((float)betterM.x, (float)betterM.y, r);
			mZDir = Normalize (mWorld2obj.VectorTransform (r.dir));

			// Set the slice plane position:
			ComputeSliceParams (false);

			if (mpInterface->GetSnapState()) {
				// Must suspend "fully interactive" while snapping in Quickslice mode.
				mpEditPoly->EpPreviewSetSuspend (true);
				mpInterface->PushPrompt (GetString (IDS_QUICKSLICE_SNAP_PREVIEW_WARNING));
			}

			// Start the slice preview mode:
			mpEditPoly->EpPreviewBegin (epop_slice);

			mSlicing = true;
			mMouse1 = betterM;
			mMouse2 = betterM;
			DrawSlicer (hwnd, false, true);
		} else {
			DrawSlicer (hwnd, true, true);	// Erase last slice line.

			// second point:
			mLocal2 = mWorld2obj * snapPoint;
			ComputeSliceParams (true);
			mSlicing = false;	// do before PreviewAccept to make sure display is correct.
			mpEditPoly->EpPreviewAccept ();
			if (mpEditPoly->EpPreviewGetSuspend()) {
				mpInterface->PopPrompt();
				mpEditPoly->EpPreviewSetSuspend (false);
			}
		}
		mpInterface->ReleaseViewport (vpt);
		return true;

	case MOUSE_MOVE:
		if (!mSlicing) break;	// Nothing to do if we haven't clicked first point yet

		SetCursor (hCurQuickSlice);
		DrawSlicer (hwnd, true, false);	// Erase last slice line.

		// Find where we hit, in world space, on the construction plane or snap location:
		vpt = mpInterface->GetViewport (hwnd);
		snapPoint = vpt->SnapPoint (m, mMouse2, NULL);
		snapPoint = vpt->MapCPToWorld (snapPoint);
		mpInterface->ReleaseViewport (vpt);

		// Find object-space equivalent for mLocal2:
		mLocal2 = mWorld2obj * snapPoint;

		mpEditPoly->EpPreviewSetDragging (true);
		ComputeSliceParams ();
		mpEditPoly->EpPreviewSetDragging (false);

		mpInterface->RedrawViews (mpInterface->GetTime());

		DrawSlicer (hwnd, false, true);	// Draw this slice line.
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

void QuickSliceCMode::EnterMode() {
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom);
	if (!hGeom) return;
	ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_QUICKSLICE));
	but->SetCheck(TRUE);
	ReleaseICustButton(but);
	proc.mSlicing = false;
}

void QuickSliceCMode::ExitMode() {
	// Lets make double-extra-sure that QuickSlice's suspension of the preview system doesn't leak out...
	// (This line is actually necessary in the case where the user uses a shortcut key to jump-start
	// another command mode.)
	if (mpEditPoly->EpPreviewGetSuspend()) {
		if (mpEditPoly->ip) mpEditPoly->ip->PopPrompt();
		mpEditPoly->EpPreviewSetSuspend (false);
	}

	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom);
	if (!hGeom) return;
	ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_QUICKSLICE));
	but->SetCheck(FALSE);
	ReleaseICustButton(but);
}

//------- LiftFromEdge proc/mode------------------------

HitRecord *LiftFromEdgeProc::HitTestEdges (IPoint2 &m, ViewExp *vpt) {
	vpt->ClearSubObjHitList();
	EditPolyObject *pEditPoly = (EditPolyObject *) mpEPoly;	// STEVE: make go away someday.
	pEditPoly->SetHitLevelOverride (SUBHIT_MNEDGES);
	ip->SubObHitTest (ip->GetTime(), HITTYPE_POINT, 0, 0, &m, vpt);
	pEditPoly->ClearHitLevelOverride ();
	if (!vpt->NumSubObjHits()) return NULL;
	HitLog& hitLog = vpt->GetSubObjHitList();
	return hitLog.ClosestHit();	// (may be NULL.)
}

// Mouse interaction:
// - user clicks on hinge edge
// - user drags angle.

int LiftFromEdgeProc::proc (HWND hwnd, int msg, int point, int flags, IPoint2 m ) {
	ViewExp* vpt = NULL;
	HitRecord* hr = NULL;
	Point3 snapPoint;
	IPoint2 diff;
	float angle;
	EditPolyObject* pEditPoly = NULL;

	switch (msg) {
	case MOUSE_ABORT:
		pEditPoly = (EditPolyObject *) mpEPoly;
		pEditPoly->EpPreviewCancel ();
		return FALSE;

	case MOUSE_PROPCLICK:
		pEditPoly = (EditPolyObject *) mpEPoly;
		pEditPoly->EpPreviewCancel ();
		ip->PopCommandMode ();
		return FALSE;

	case MOUSE_DBLCLICK:
		return false;

	case MOUSE_POINT:
		ip->SetActiveViewport (hwnd);
		vpt = ip->GetViewport (hwnd);
		snapPoint = vpt->SnapPoint (m, m, NULL);
		snapPoint = vpt->MapCPToWorld (snapPoint);
		if (!edgeFound) {
			hr = HitTestEdges (m, vpt);
			if (!hr) return true;
			pEditPoly = (EditPolyObject *) mpEPoly;
			edgeFound = true;
			mpEPoly->getParamBlock()->SetValue (ep_lift_edge, ip->GetTime(), int(hr->hitInfo));
			mpEPoly->getParamBlock()->SetValue (ep_lift_angle, ip->GetTime(), 0.0f);	// prevent "flash"
			pEditPoly->EpPreviewBegin (epop_lift_from_edge);
			firstClick = m;
		} else {
			IPoint2 diff = m - firstClick;
			// (this arbirtrarily scales each pixel to one degree.)
			float angle = diff.y * PI / 180.0f;
			mpEPoly->getParamBlock()->SetValue (ep_lift_angle, ip->GetTime(), angle);
			pEditPoly = (EditPolyObject *) mpEPoly;
			pEditPoly->EpPreviewAccept ();
			ip->RedrawViews (ip->GetTime());
			edgeFound = false;
			return false;
		}
		if (vpt) ip->ReleaseViewport (vpt);
		return true;

	case MOUSE_MOVE:
		// Find where we hit, in world space, on the construction plane or snap location:
		vpt = ip->GetViewport (hwnd);
		snapPoint = vpt->SnapPoint (m, m, NULL);
		snapPoint = vpt->MapCPToWorld (snapPoint);

		if (!edgeFound) {
			// Just set cursor depending on presence of edge:
			if (HitTestEdges(m,vpt)) SetCursor(ip->GetSysCursor(SYSCUR_SELECT));
			else SetCursor(LoadCursor(NULL,IDC_ARROW));
			if (vpt) ip->ReleaseViewport(vpt);
			break;
		}
		ip->ReleaseViewport (vpt);
		diff = m - firstClick;
		// (this arbirtrarily scales each pixel to one degree.)
		angle = diff.y * PI / 180.0f;

		pEditPoly = (EditPolyObject *) mpEPoly;	// STEVE: Temporarily necessary.
		pEditPoly->EpPreviewSetDragging (true);
		mpEPoly->getParamBlock()->SetValue (ep_lift_angle, ip->GetTime(), angle);
		pEditPoly->EpPreviewSetDragging (false);

		// Even if we're not fully interactive, we need to update the dotted line display:
		pEditPoly->NotifyDependents (FOREVER, PART_DISPLAY, REFMSG_CHANGE);
		ip->RedrawViews (ip->GetTime());
		break;

	case MOUSE_FREEMOVE:
		vpt = ip->GetViewport (hwnd);
		snapPoint = vpt->SnapPoint (m, m, NULL);
		snapPoint = vpt->MapCPToWorld (snapPoint);

		if (!edgeFound) {
			// Just set cursor depending on presence of edge:
			if (HitTestEdges(m,vpt)) SetCursor(ip->GetSysCursor(SYSCUR_SELECT));
		else SetCursor(LoadCursor(NULL,IDC_ARROW));
		}
		ip->ReleaseViewport (vpt);
		break;
	}

	return TRUE;
}

void LiftFromEdgeCMode::EnterMode() {
	proc.Reset();
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom_face);
	if (!hGeom) return;
	ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_LIFT_FROM_EDG));
	but->SetCheck(TRUE);
	ReleaseICustButton(but);
}

void LiftFromEdgeCMode::ExitMode() {
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom_face);
	if (!hGeom) return;
	ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_LIFT_FROM_EDG));
	but->SetCheck(FALSE);
	ReleaseICustButton(but);
}

//-------------------------------------------------------

bool PickLiftEdgeProc::EdgePick (int edge, float prop) {
	mpEPoly->getParamBlock()->SetValue (ep_lift_edge, ip->GetTime(), edge);
	ip->RedrawViews (ip->GetTime());
	return false;	// false = exit mode when done; true = stay in mode.
}

void PickLiftEdgeCMode::EnterMode () {
	HWND hSettings = mpEditPoly->GetDlgHandle (ep_settings);
	if (!hSettings) return;
	ICustButton *but = GetICustButton (GetDlgItem (hSettings,IDC_LIFT_PICK_EDGE));
	but->SetCheck(true);
	ReleaseICustButton(but);
}

void PickLiftEdgeCMode::ExitMode () {
	HWND hSettings = mpEditPoly->GetDlgHandle (ep_settings);
	if (!hSettings) return;
	ICustButton *but = GetICustButton (GetDlgItem (hSettings,IDC_LIFT_PICK_EDGE));
	but->SetCheck(false);
	ReleaseICustButton(but);
}

//-------------------------------------------------------

void WeldCMode::EnterMode () {
	mproc.Reset();
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom_vertex);
	if (!hGeom) return;
	ICustButton *but = GetICustButton (GetDlgItem (hGeom, IDC_TARGET_WELD));
	but->SetCheck(TRUE);
	ReleaseICustButton(but);
}

void WeldCMode::ExitMode () {
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom_vertex);
	if (!hGeom) return;
	ICustButton *but = GetICustButton (GetDlgItem (hGeom, IDC_TARGET_WELD));
	but->SetCheck(FALSE);
	ReleaseICustButton(but);
}

void WeldMouseProc::DrawWeldLine (HWND hWnd,IPoint2 &m, bool bErase, bool bPresent) {
	if (targ1<0) return;
	
	IPoint2 pts[2];
	pts[0].x = m1.x;
	pts[0].y = m1.y;
	pts[1].x = m.x;
	pts[1].y = m.y;
	XORDottedPolyline(hWnd, 2, pts, 0, bErase, !bPresent);
}

bool WeldMouseProc::CanWeldVertices (int v1, int v2) {
	MNMesh & mm = mpEditPoly->GetMesh ();
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

bool WeldMouseProc::CanWeldEdges (int e1, int e2) {
	MNMesh & mm = mpEditPoly->GetMesh ();
	if (mm.e[e1].f2 > -1) return false;
	if (mm.e[e2].f2 > -1) return false;
	if (mm.e[e1].f1 == mm.e[e2].f1) return false;
	if (mm.e[e1].v1 == mm.e[e2].v1) return false;
	if (mm.e[e1].v2 == mm.e[e2].v2) return false;
	return true;
}

HitRecord *WeldMouseProc::HitTest (IPoint2 &m, ViewExp *vpt) {
	vpt->ClearSubObjHitList();
	// Use the default pixel value - no one wanted the old weld_pixels spinner.
	if (mpEditPoly->GetMNSelLevel()==MNM_SL_EDGE) {
		mpEditPoly->SetHitLevelOverride (SUBHIT_EDGES|SUBHIT_OPENONLY);
	}
	ip->SubObHitTest (ip->GetTime(), HITTYPE_POINT, 0, 0, &m, vpt);
	mpEditPoly->ClearHitLevelOverride ();
	if (!vpt->NumSubObjHits()) return NULL;
	HitLog& hitLog = vpt->GetSubObjHitList();
	HitRecord *hr = hitLog.ClosestHit();
	if (targ1>-1) {
		if (targ1 == hr->hitInfo) return NULL;
		if (mpEditPoly->GetMNSelLevel() == MNM_SL_EDGE) {
			if (!CanWeldEdges (targ1, hr->hitInfo)) return NULL;
		} else {
			if (!CanWeldVertices (targ1, hr->hitInfo)) return NULL;
		}
	}
	return hr;
}

int WeldMouseProc::proc (HWND hwnd, int msg, int point, int flags, IPoint2 m) {
	ViewExp *vpt = NULL;
	int res = TRUE, stringID = 0;
	HitRecord* hr = NULL;
	bool ret(false);

	switch (msg) {
	case MOUSE_ABORT:
		// Erase last weld line:
		if (targ1>-1) DrawWeldLine (hwnd, oldm2, true, true);
		targ1 = -1;
		return FALSE;

	case MOUSE_PROPCLICK:
		// Erase last weld line:
		if (targ1>-1) DrawWeldLine (hwnd, oldm2, true, true);
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
			m1 = m;
			DrawWeldLine (hwnd, m, false, true);
			oldm2 = m;
			break;
		}

		// Otherwise, we're completing the weld.
		// Erase the last weld-line:
		DrawWeldLine (hwnd, oldm2, true, true);

		// Do the weld:
		theHold.Begin();
		ret      = false;
		stringID = 0;
		if (mpEditPoly->GetSelLevel() == EP_SL_VERTEX) {
			Point3 destination = mpEditPoly->mm.P(hr->hitInfo);
			ret = mpEditPoly->EpfnWeldVerts (targ1, hr->hitInfo, destination)?true:false;
			stringID = IDS_WELD_VERTS;
		} else if (mpEditPoly->GetMNSelLevel() == MNM_SL_EDGE) {
			ret = mpEditPoly->EpfnWeldEdges (targ1, hr->hitInfo)?true:false;
			stringID = IDS_WELD_EDGES;
		}
		if (ret) {
			theHold.Accept (GetString(stringID));
			mpEditPoly->RefreshScreen ();
		} else {
			theHold.Cancel ();
		}
		targ1 = -1;
		ip->ReleaseViewport(vpt);
		return false;

	case MOUSE_MOVE:
	case MOUSE_FREEMOVE:
		vpt = ip->GetViewport(hwnd);
		if (targ1 > -1) {
			DrawWeldLine (hwnd, oldm2, true, false);
			oldm2 = m;
		}
		if (HitTest (m,vpt)) SetCursor(ip->GetSysCursor(SYSCUR_SELECT));
		else SetCursor (LoadCursor (NULL, IDC_ARROW));
		ip->RedrawViews (ip->GetTime());
		if (targ1 > -1) DrawWeldLine (hwnd, m, false, true);
		break;
	}
	
	if (vpt) ip->ReleaseViewport(vpt);
	return true;	
}

//-------------------------------------------------------

void EditTriCMode::EnterMode() {
	HWND hSurf = mpEditPoly->GetDlgHandle (ep_geom_face);
	if (hSurf) {
		ICustButton *but = GetICustButton (GetDlgItem (hSurf, IDC_FS_EDIT_TRI));
		but->SetCheck(TRUE);
		ReleaseICustButton(but);
	}
	mpEditPoly->SetDisplayLevelOverride (MNDISP_DIAGONALS);
	mpEditPoly->NotifyDependents (FOREVER, PART_DISPLAY, REFMSG_CHANGE);
	mpEditPoly->ip->RedrawViews(mpEditPoly->ip->GetTime());
}

void EditTriCMode::ExitMode() {
	HWND hSurf = mpEditPoly->GetDlgHandle (ep_geom_face);
	if (hSurf) {
		ICustButton *but = GetICustButton (GetDlgItem (hSurf, IDC_FS_EDIT_TRI));
		but->SetCheck (FALSE);
		ReleaseICustButton(but);
	}
	mpEditPoly->ClearDisplayLevelOverride();
	mpEditPoly->NotifyDependents (FOREVER, PART_DISPLAY, REFMSG_CHANGE);
	mpEditPoly->ip->RedrawViews(mpEditPoly->ip->GetTime());
}

void EditTriProc::VertConnect () {
	DbgAssert (v1>=0);
	DbgAssert (v2>=0);
	Tab<int> v1fac = mpEPoly->GetMeshPtr()->vfac[v1];
	int i, j, ff = 0, v1pos = 0, v2pos=-1;
	for (i=0; i<v1fac.Count(); i++) {
		MNFace & mf = mpEPoly->GetMeshPtr()->f[v1fac[i]];
		for (j=0; j<mf.deg; j++) {
			if (mf.vtx[j] == v2) v2pos = j;
			if (mf.vtx[j] == v1) v1pos = j;
		}
		if (v2pos<0) continue;
		ff = v1fac[i];
		break;
	}

	if (v2pos<0) return;

	theHold.Begin();
	mpEPoly->EpfnSetDiagonal (ff, v1pos, v2pos);
	theHold.Accept (GetString (IDS_EDIT_TRIANGULATION));
	mpEPoly->RefreshScreen ();
}

//-------------------------------------------------------

void TurnEdgeCMode::EnterMode() {
	HWND hSurf = mpEditPoly->GetDlgHandle (ep_geom_face);
	if (hSurf) {
		ICustButton *but = GetICustButton (GetDlgItem (hSurf, IDC_TURN_EDGE));
		if (but) {
			but->SetCheck(true);
			ReleaseICustButton(but);
		}
	}
	mpEditPoly->SetDisplayLevelOverride (MNDISP_DIAGONALS);
	mpEditPoly->LocalDataChanged (PART_DISPLAY);
	mpEditPoly->RefreshScreen ();
}

void TurnEdgeCMode::ExitMode() {
	HWND hSurf = mpEditPoly->GetDlgHandle (ep_geom_face);
	if (hSurf) {
		ICustButton *but = GetICustButton (GetDlgItem (hSurf, IDC_TURN_EDGE));
		if (but) {
			but->SetCheck (false);
			ReleaseICustButton(but);
		}
	}
	mpEditPoly->ClearDisplayLevelOverride();
	mpEditPoly->LocalDataChanged (PART_DISPLAY);
	mpEditPoly->RefreshScreen ();
}

HitRecord *TurnEdgeProc::HitTestEdges (IPoint2 & m, ViewExp *vpt) {
	vpt->ClearSubObjHitList();

	mpEPoly->ForceIgnoreBackfacing (true);
	ip->SubObHitTest (ip->GetTime(), HITTYPE_POINT, 0, 0, &m, vpt);
	mpEPoly->ForceIgnoreBackfacing (false);

	if (!vpt->NumSubObjHits()) return NULL;
	HitLog& hitLog = vpt->GetSubObjHitList();

	// Find the closest hit that is not a border edge:
	HitRecord *hr = NULL;
	for (HitRecord *hitRec=hitLog.First(); hitRec != NULL; hitRec=hitRec->Next()) {
		// Always want the closest hit pixelwise:
		if (hr && (hr->distance < hitRec->distance)) continue;
		MNMesh & mesh = mpEPoly->GetMesh();
		if (mesh.e[hitRec->hitInfo].f2<0) continue;
		hr = hitRec;
	}

	return hr;	// (may be NULL.)
}

HitRecord *TurnEdgeProc::HitTestDiagonals (IPoint2 & m, ViewExp *vpt) {
	vpt->ClearSubObjHitList();

	mpEPoly->SetHitLevelOverride (SUBHIT_MNDIAGONALS);
	mpEPoly->ForceIgnoreBackfacing (true);
	ip->SubObHitTest (ip->GetTime(), HITTYPE_POINT, 0, 0, &m, vpt);
	mpEPoly->ForceIgnoreBackfacing (false);
	mpEPoly->ClearHitLevelOverride ();

	if (!vpt->NumSubObjHits()) return NULL;
	HitLog& hitLog = vpt->GetSubObjHitList();
	return hitLog.ClosestHit ();
}

int TurnEdgeProc::proc (HWND hwnd, int msg, int point, int flags, IPoint2 m) {
	ViewExp* vpt = NULL;
	HitRecord* hr = NULL;
	Point3 snapPoint;
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
		if (hitDiag) mpEPoly->EpfnTurnDiagonal (hitDiag->mFace, hitDiag->mDiag);
		//else mpEPoly->EpModTurnEdge (hr->hitInfo, hr->nodeRef);
		theHold.Accept (GetString (IDS_TURN_EDGE));
		mpEPoly->RefreshScreen();
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

//-------------------------------------------------------

HitRecord *BridgeMouseProc::HitTest (IPoint2 & m, ViewExp *vpt) {
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
         int j;
			for (j=0; j<mDisallowedSecondHits.Count(); j++)
				if (hr->hitInfo == mDisallowedSecondHits[j]) break;
			if (j<mDisallowedSecondHits.Count()) continue;
		}
		if ((bestHit == NULL) || (bestHit->distance>hr->distance)) bestHit = hr;
	}
	return bestHit;
}

void BridgeMouseProc::DrawDiag (HWND hWnd, const IPoint2 & m, bool bErase, bool bPresent) {
	if (hit1<0) return;

	IPoint2 pts[2];
	pts[0].x = m1.x;
	pts[0].y = m1.y;
	pts[1].x = m.x;
	pts[1].y = m.y;
	XORDottedPolyline(hWnd, 2, pts, 0, bErase, !bPresent);
}

int BridgeMouseProc::proc (HWND hwnd, int msg, int point, int flags, IPoint2 m) {
	ViewExp *vpt = NULL;
	HitRecord *hr = NULL;
	Point3 snapPoint;

	switch (msg) {
	case MOUSE_INIT:
		hit1 = hit2 = -1;
		mDisallowedSecondHits.ZeroCount();
		break;

	case MOUSE_PROPCLICK:
		hit1 = hit2 = -1;
		mDisallowedSecondHits.ZeroCount();
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
			hit1 = hr->hitInfo;
			m1 = m;
			lastm = m;
			SetupDisallowedSecondHits ();
			DrawDiag (hwnd, m, false, true);
			break;
		}
		// Otherwise, we've found a connection.
		DrawDiag (hwnd, lastm, true, true);	// erase last dotted line.
		hit2 = hr->hitInfo;
		Bridge ();
		hit1 = -1;
		mDisallowedSecondHits.ZeroCount();
		return FALSE;	// Done with this connection.

	case MOUSE_MOVE:
	case MOUSE_FREEMOVE:
		vpt = ip->GetViewport(hwnd);
		vpt->SnapPreview (m, m, NULL, SNAP_FORCE_3D_RESULT);
		hr=HitTest(m,vpt);
		if (hr!=NULL) SetCursor(ip->GetSysCursor(SYSCUR_SELECT));
		else SetCursor(LoadCursor(NULL,IDC_ARROW));
		if (vpt) ip->ReleaseViewport(vpt);
		DrawDiag (hwnd, lastm, true, false);
		DrawDiag (hwnd, m, false, true);
		lastm = m;
		break;
	}

	return TRUE;
}

void BridgeBorderProc::Bridge () {
	theHold.Begin ();
	mpEPoly->getParamBlock()->SetValue (ep_bridge_selected, 0, false);
	mpEPoly->getParamBlock()->SetValue (ep_bridge_target_1, 0, hit1+1);
	mpEPoly->getParamBlock()->SetValue (ep_bridge_target_2, 0, hit2+1);
	mpEPoly->getParamBlock()->SetValue (ep_bridge_twist_1, 0, 0);
	mpEPoly->getParamBlock()->SetValue (ep_bridge_twist_2, 0, 0);

	if (!mpEPoly->EpfnBridge (EP_SL_BORDER, MN_SEL)) {
		theHold.Cancel ();
		return;
	}
	theHold.Accept (GetString (IDS_BRIDGE_BORDERS));
	mpEPoly->RefreshScreen ();
}

void BridgeBorderProc::SetupDisallowedSecondHits () {
	// Second hit can't be in same border loop as first hit.
	MNMeshUtilities mmu(mpEPoly->GetMeshPtr());
	mmu.GetBorderFromEdge (hit1, mDisallowedSecondHits);
}

void BridgeBorderCMode::EnterMode () {
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom_border);
	if (hGeom) {
		ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_BRIDGE));
		if (but) {
			but->SetCheck(TRUE);
			ReleaseICustButton(but);
		}
	}
}

void BridgeBorderCMode::ExitMode () {
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom_border);
	if (hGeom) {
		ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_BRIDGE));
		if (but) {
			but->SetCheck (false);
			ReleaseICustButton(but);
		}
	}
}

// ------------------------------------------------------

void BridgePolygonProc::Bridge () {
	theHold.Begin ();
	mpEPoly->getParamBlock()->SetValue (ep_bridge_selected, 0, false);
	mpEPoly->getParamBlock()->SetValue (ep_bridge_target_1, 0, hit1+1);
	mpEPoly->getParamBlock()->SetValue (ep_bridge_target_2, 0, hit2+1);
	mpEPoly->getParamBlock()->SetValue (ep_bridge_twist_1, 0, 0);
	mpEPoly->getParamBlock()->SetValue (ep_bridge_twist_2, 0, 0);

	int twist2(0);
	if ((hit1>=0) && (hit2>=0)) {
		MNMeshUtilities mmu(mpEPoly->GetMeshPtr());
		twist2 = mmu.FindDefaultBridgeTwist (hit1, hit2);
	}
	mpEPoly->getParamBlock()->SetValue (ep_bridge_twist_2, 0, twist2);

	if (!mpEPoly->EpfnBridge (EP_SL_FACE, MN_SEL)) {
		theHold.Cancel ();
		return;
	}

	theHold.Accept (GetString (IDS_BRIDGE_POLYGONS));
	mpEPoly->RefreshScreen ();
}

void BridgePolygonProc::SetupDisallowedSecondHits () {
	// Second hit can't be the same as, or a neighbor of, the first hit:
	MNMesh & mesh = mpEPoly->GetMesh();
	mDisallowedSecondHits.Append (1, &hit1, mesh.f[hit1].deg*3);
	for (int i=0; i<mesh.f[hit1].deg; i++) {
		Tab<int> & vf = mesh.vfac[mesh.f[hit1].vtx[i]];
		for (int j=0; j<vf.Count(); j++) {
			if (vf[j] == hit1) continue;
			mDisallowedSecondHits.Append (1, vf.Addr(j));	// counts many faces twice; that's fine, still a short list.
		}
	}
}

void BridgePolygonCMode::EnterMode () {
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom_face);
	if (hGeom) {
		ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_BRIDGE));
		if (but) {
			but->SetCheck(TRUE);
			ReleaseICustButton(but);
		}
	}
}

void BridgePolygonCMode::ExitMode () {
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom_face);
	if (hGeom) {
		ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_BRIDGE));
		if (but) {
			but->SetCheck (false);
			ReleaseICustButton(but);
		}
	}
}
// ------------------------------------------------------

void BridgeEdgeProc::Bridge () {
	float l_Angle = PI/4.0f;
	theHold.Begin ();
	mpEPoly->getParamBlock()->SetValue (ep_bridge_selected, 0, false);
	mpEPoly->getParamBlock()->SetValue (ep_bridge_target_1, 0, hit1+1);
	mpEPoly->getParamBlock()->SetValue (ep_bridge_target_2, 0, hit2+1);
	mpEPoly->getParamBlock()->SetValue (ep_bridge_adjacent, 0, l_Angle);

	if (!mpEPoly->EpfnBridge (EP_SL_EDGE, MN_SEL)) {
		theHold.Cancel ();
		return;
	}

	theHold.Accept (GetString (IDS_BRIDGE_POLYGONS));
	mpEPoly->RefreshScreen ();
}

void BridgeEdgeProc::SetupDisallowedSecondHits () {
	// Second hit can't be the same as, or a neighbor of, the first hit:
	MNMesh & mesh = mpEPoly->GetMesh();
	mDisallowedSecondHits.Append (1, &hit1, 20);

	int l_StartVertx = mesh.e[hit1].v1;

	Tab<int> & startEdge = mesh.vedg[mesh.e[hit1][0]];

	for (int j=0; j<startEdge.Count(); j++) 
	{
		if (startEdge[j] == hit1) continue;
		mDisallowedSecondHits.Append (1, startEdge.Addr(j));	// counts many edges twice; that's fine, still a short list.
	}

	Tab<int> & endEdge = mesh.vedg[mesh.e[hit1][1]];
	for (int j=0; j<endEdge.Count(); j++) 
	{
		if (endEdge[j] == hit1) continue;
		mDisallowedSecondHits.Append (1, endEdge.Addr(j));	// counts many edges twice; that's fine, still a short list.
	}


}

void BridgeEdgeCMode::EnterMode () {
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom_edge);
	if (hGeom) {
		ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_BRIDGE));
		if (but) {
			but->SetCheck(TRUE);
			ReleaseICustButton(but);
		}
	}
}

void BridgeEdgeCMode::ExitMode () {
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom_edge);
	if (hGeom) {
		ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_BRIDGE));
		if (but) {
			but->SetCheck (false);
			ReleaseICustButton(but);
		}
	}
}

// ------------------------------------------------------

PickBridgeTargetProc::PickBridgeTargetProc (EditPolyObject* e, IObjParam *i, int end) : mpEPoly(e), ip(i), mWhichEnd(end) 
{
	int otherEnd = e->getParamBlock()->GetInt (mWhichEnd ? ep_bridge_target_1 : ep_bridge_target_2);
	
	if (otherEnd <= 0 )
		return;

	otherEnd--;

	if (e->GetEPolySelLevel() == EP_SL_BORDER) 
	{
		// data validation BUG-CER FIX 904837 laurent 04/07
		if ( otherEnd >= mpEPoly->GetMesh().nume)
			return; 

		MNMeshUtilities mmu (e->GetMeshPtr());
		mmu.GetBorderFromEdge (otherEnd, mDisallowedHits);
	} 
	else if (e->GetEPolySelLevel() == EP_SL_FACE)
	{
		MNMesh &mesh = mpEPoly->GetMesh();

		// data validation  BUG FIX 904837
		if ( otherEnd >= mesh.numf)
			return; 

		// Second hit can't be the same as, or a neighbor of, the first hit:
		mDisallowedHits.Append (1, &otherEnd, 20);
		
		for (int i = 0; i< mesh.f[otherEnd].deg; i++) 
		{
			Tab<int> & vf = mesh.vfac[mesh.f[otherEnd].vtx[i]];

			for (int j=0; j<vf.Count(); j++) 
			{
				if (vf[j] == otherEnd) continue;
				mDisallowedHits.Append (1, vf.Addr(j));	// counts many faces twice; that's fine, still a short list.
			}
		}
	}
	else if (e->GetEPolySelLevel() == EP_SL_EDGE)
	{
		// Second hit can't be the same as, or a neighbor of, the first hit:
		MNMesh & mesh = mpEPoly->GetMesh();

		// data validation  BUG FIX 904837
		if ( otherEnd >= mesh.nume)
			return; 

		mDisallowedHits.Append (1, &otherEnd, 20);

		int l_StartVertx = mesh.e[otherEnd].v1;

		Tab<int> & startEdge = mesh.vedg[mesh.e[otherEnd][0]];

		for (int j=0; j<startEdge.Count(); j++) 
		{
			if (startEdge[j] == otherEnd) continue;
			mDisallowedHits.Append (1, startEdge.Addr(j));	// counts many faces twice; that's fine, still a short list.
		}

		Tab<int> & endEdge = mesh.vedg[mesh.e[otherEnd][1]];
		for (int j=0; j<endEdge.Count(); j++) 
		{
			if (endEdge[j] == otherEnd) continue;
			mDisallowedHits.Append (1, endEdge.Addr(j));	// counts many faces twice; that's fine, still a short list.
		}

	}
}

HitRecord *PickBridgeTargetProc::HitTest (IPoint2 &m, ViewExp *vpt) 
{
	vpt->ClearSubObjHitList();
	ip->SubObHitTest(ip->GetTime(),HITTYPE_POINT,0, 0, &m, vpt);
	if (!vpt->NumSubObjHits()) return NULL;

	HitLog& hitLog = vpt->GetSubObjHitList();

	HitRecord *ret = NULL;
	for (HitRecord *hitRec=hitLog.First(); hitRec != NULL; hitRec=hitRec->Next()) {
		// Filter out edges in the other border,
		// or polygons neighboring the other polygon.
      int j;
		for (j=0; j<mDisallowedHits.Count(); j++) {
			if (mDisallowedHits[j] == hitRec->hitInfo) break;
		}
		if (j<mDisallowedHits.Count()) continue;

		// Always want the closest hit pixelwise:
		if ((ret==NULL) || (ret->distance > hitRec->distance)) ret = hitRec;
	}
	return ret;
}

int PickBridgeTargetProc::proc (HWND hwnd, int msg, int point, int flags, IPoint2 m) {
	ViewExp* vpt = NULL;
	HitRecord* hr = NULL;
	Point3 snapPoint;

	switch (msg) {
	case MOUSE_PROPCLICK:
		ip->PopCommandMode ();
		break;

	case MOUSE_POINT:
		ip->SetActiveViewport(hwnd);
		vpt = ip->GetViewport(hwnd);
		snapPoint = vpt->SnapPoint (m, m, NULL);
		snapPoint = vpt->MapCPToWorld (snapPoint);
		hr = HitTest (m, vpt);
		ip->ReleaseViewport(vpt);
	
		if (!hr) break;

		theHold.Begin ();
		{
			mpEPoly->getParamBlock()->SetValue (mWhichEnd ? ep_bridge_target_2 : ep_bridge_target_1, ip->GetTime(),
				(int)hr->hitInfo+1);

			int targ1 = mpEPoly->getParamBlock()->GetInt (ep_bridge_target_1)-1;
			int targ2 = mpEPoly->getParamBlock()->GetInt (ep_bridge_target_2)-1;
			int twist2(0);
			if ((targ1>=0) && (targ2>=0)) {
				MNMeshUtilities mmu(mpEPoly->GetMeshPtr());
				twist2 = mmu.FindDefaultBridgeTwist (targ1, targ2);
			}
			mpEPoly->getParamBlock()->SetValue (ep_bridge_twist_1, ip->GetTime(), 0);
			mpEPoly->getParamBlock()->SetValue (ep_bridge_twist_2, 0, twist2);
		}

		theHold.Accept (GetString ((mpEPoly->GetEPolySelLevel()==EP_SL_BORDER ||mpEPoly->GetEPolySelLevel()==EP_SL_EDGE) ?
			(mWhichEnd ? IDS_BRIDGE_PICK_EDGE_2 : IDS_BRIDGE_PICK_EDGE_1) :
			(mWhichEnd ? IDS_BRIDGE_PICK_POLYGON_2 : IDS_BRIDGE_PICK_POLYGON_1)));
		mpEPoly->RefreshScreen();

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

void PickBridge1CMode::EnterMode () {
	HWND hSettings = mpEditPoly->GetDlgHandle (ep_settings);
	if (!hSettings) return;
	ICustButton *but = GetICustButton (GetDlgItem (hSettings,IDC_BRIDGE_PICK_TARG1));
	if (but) {
		but->SetCheck(true);
		ReleaseICustButton(but);
	}
}

void PickBridge1CMode::ExitMode () {
	HWND hSettings = mpEditPoly->GetDlgHandle (ep_settings);
	if (!hSettings) return;
	ICustButton *but = GetICustButton (GetDlgItem (hSettings,IDC_BRIDGE_PICK_TARG1));
	if (but) {
		but->SetCheck(false);
		ReleaseICustButton(but);
	}
}

void PickBridge2CMode::EnterMode () {
	HWND hSettings = mpEditPoly->GetDlgHandle (ep_settings);
	if (!hSettings) return;
	ICustButton *but = GetICustButton (GetDlgItem (hSettings,IDC_BRIDGE_PICK_TARG2));
	if (but) {
		but->SetCheck(true);
		ReleaseICustButton(but);
	}
}

void PickBridge2CMode::ExitMode () {
	HWND hSettings = mpEditPoly->GetDlgHandle (ep_settings);
	if (!hSettings) return;
	ICustButton *but = GetICustButton (GetDlgItem (hSettings,IDC_BRIDGE_PICK_TARG2));
	if (but) {
		but->SetCheck(false);
		ReleaseICustButton(but);
	}
}


//////////////////////////////////////////

void EditPolyObject::DoAccept(TimeValue t)
{
}

//for EditSSCB 
void EditPolyObject::SetPinch(TimeValue t, float pinch)
{
	pblock->SetValue (ep_ss_pinch, t, pinch);
}
void EditPolyObject::SetBubble(TimeValue t, float bubble)
{
	pblock->SetValue (ep_ss_bubble, t, bubble);
}
float EditPolyObject::GetPinch(TimeValue t)
{
	return pblock->GetFloat (ep_ss_pinch, t);

}
float EditPolyObject::GetBubble(TimeValue t)
{
	return pblock->GetFloat (ep_ss_bubble, t);
}
float EditPolyObject::GetFalloff(TimeValue t)
{
	return pblock->GetFloat (ep_ss_falloff, t);
}
void EditPolyObject::SetFalloff(TimeValue t, float falloff)
{
	pblock->SetValue (ep_ss_falloff, t, falloff);
}















