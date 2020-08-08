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

// ------------------- Paint stuff --------------

void EditPolyMod::GetSoftSelData( GenSoftSelData& softSelData, TimeValue t ) {
	softSelData.useSoftSel =	mpParams->GetInt (epm_ss_use, t);
	softSelData.useEdgeDist =	mpParams->GetInt (epm_ss_edist_use, t);
	softSelData.edgeIts =		mpParams->GetInt (epm_ss_edist, t);
	softSelData.ignoreBack =	(mpParams->GetInt (epm_ss_affect_back, t)? FALSE:TRUE);
	softSelData.falloff =		mpParams->GetFloat (epm_ss_falloff, t);
	softSelData.pinch =			mpParams->GetFloat (epm_ss_pinch, t);
	softSelData.bubble =		mpParams->GetFloat (epm_ss_bubble, t);
}

static EditPolyPaintDeformDlgProc thePaintDeformDlgProc;
EditPolyPaintDeformDlgProc *ThePaintDeformDlgProc () { return &thePaintDeformDlgProc; }

void EditPolyPaintDeformDlgProc::SetEnables (HWND hWnd) {
	PaintModeID paintMode = mpEPoly->GetPaintMode();

	bool isModelingMode = (mpEPoly->getParamBlock()->GetInt(epm_animation_mode) == 0);
	BOOL isPaintPushPull =	(paintMode==PAINTMODE_PAINTPUSHPULL? TRUE:FALSE);
	BOOL isPaintRelax =		(paintMode==PAINTMODE_PAINTRELAX? TRUE:FALSE);
	BOOL isErasing  =		(paintMode==PAINTMODE_ERASEDEFORM? TRUE:FALSE);
	BOOL isActive =			(mpEPoly->IsPaintDataActive(PAINTMODE_DEFORM)? TRUE:FALSE);

	ICustButton* but = GetICustButton (GetDlgItem (hWnd, IDC_PAINTDEFORM_PUSHPULL));
	but->Enable (isModelingMode);
	but->SetCheck( isPaintPushPull );
	ReleaseICustButton (but);

	but = GetICustButton (GetDlgItem (hWnd, IDC_PAINTDEFORM_RELAX));
	but->Enable (isModelingMode);
	but->SetCheck( isPaintRelax );
	ReleaseICustButton (but);

	but = GetICustButton (GetDlgItem (hWnd, IDC_PAINTDEFORM_REVERT));
	but->Enable( isActive && isModelingMode );
	but->SetCheck( isErasing );
	ReleaseICustButton (but);

	ISpinnerControl *spin = GetISpinner (GetDlgItem (hWnd, IDC_PAINTDEFORM_VALUE_SPIN));
	spin->Enable ((!isPaintRelax) && (!isErasing) && isModelingMode);
	ReleaseISpinner (spin);
	BOOL enablePushPull = (((!isPaintRelax) && (!isErasing) && isModelingMode)? TRUE:FALSE);
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

	EnableWindow (GetDlgItem (hWnd, IDC_PAINTDEFORM_SIZE_LABEL), isModelingMode);
	EnableWindow (GetDlgItem (hWnd, IDC_PAINTDEFORM_STRENGTH_LABEL), isModelingMode);

	spin = GetISpinner (GetDlgItem (hWnd, IDC_PAINTDEFORM_SIZE_SPIN));
	spin->Enable (isModelingMode);
	ReleaseISpinner (spin);

	spin = GetISpinner (GetDlgItem (hWnd, IDC_PAINTDEFORM_STRENGTH_SPIN));
	spin->Enable (isModelingMode);
	ReleaseISpinner (spin);

	but = GetICustButton (GetDlgItem (hWnd, IDC_PAINTDEFORM_OPTIONS));
	but->Enable (isModelingMode);
	ReleaseICustButton (but);

	but = GetICustButton (GetDlgItem (hWnd, IDC_PAINTDEFORM_APPLY));
	but->Enable( isActive && isModelingMode );
	ReleaseICustButton (but);

	but = GetICustButton (GetDlgItem (hWnd, IDC_PAINTDEFORM_CANCEL));
	but->Enable( isActive && isModelingMode );
	ReleaseICustButton (but);
}

INT_PTR EditPolyPaintDeformDlgProc::DlgProc (TimeValue t, IParamMap2 *pmap, HWND hWnd,
						   UINT msg, WPARAM wParam, LPARAM lParam)
{
	//BOOL ret;
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

	case WM_COMMAND:
		//uiValid = false;
		switch (LOWORD(wParam)) {
		case IDC_PAINTDEFORM_PUSHPULL:
			mpEPoly->OnPaintBtn( PAINTMODE_PAINTPUSHPULL, t );
			SetEnables (hWnd);
			break;
		case IDC_PAINTDEFORM_RELAX:
			mpEPoly->OnPaintBtn( PAINTMODE_PAINTRELAX, t );
			SetEnables (hWnd);
			break;
		case IDC_PAINTDEFORM_REVERT:
			mpEPoly->OnPaintBtn( PAINTMODE_ERASEDEFORM, t );
			SetEnables (hWnd);
			break;
		case IDC_PAINTDEFORM_APPLY:
			theHold.Begin ();
			mpEPoly->EpModCommit (t, false, true);
			theHold.Accept (GetString (IDS_PAINTDEFORM));
			break;
		case IDC_PAINTDEFORM_CANCEL:
			mpEPoly->EpModCancel ();
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

void EditPolyMod::PrePaintStroke (PaintModeID paintMode)
{
	if (IsPaintDeformMode(paintMode))
	{
		SetOperation (ep_op_paint_deform, false, true);
	}
}

void EditPolyMod::GetPaintHosts( Tab<MeshPaintHost*>& hosts, Tab<INode*>& paintNodes ) {
	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);
	EditPolyData* modData = NULL;
	for (int i=0; i<mcList.Count(); i++ ) {
		if( (modData=(EditPolyData*)mcList[i]->localData)== NULL) continue;
		MeshPaintHost* host = modData;
		hosts.Append( 1, &(host) );
		INode *pNode = nodes[i]->GetActualINode();
		paintNodes.Append( 1, &pNode );
	}
	nodes.DisposeTemporary();
}

float EditPolyMod::GetPaintValue(  PaintModeID paintMode  ) {
	if( (mpParams!=NULL) && IsPaintSelMode(paintMode) )		return mpParams->GetFloat(epm_paintsel_value);
	if( (mpParams!=NULL) && IsPaintDeformMode(paintMode) )	return mpParams->GetFloat(epm_paintdeform_value);
	return 0;
}
void EditPolyMod::SetPaintValue(  PaintModeID paintMode, float value  ) {
	if( (mpParams!=NULL) && IsPaintSelMode(paintMode) )		mpParams->SetValue(epm_paintsel_value, 0, value);
	if( (mpParams!=NULL) && IsPaintDeformMode(paintMode) )	mpParams->SetValue(epm_paintdeform_value, 0, value);
}

PaintAxisID EditPolyMod::GetPaintAxis( PaintModeID paintMode ) {
	if( (mpParams!=NULL) && IsPaintDeformMode(paintMode) )
		return (PaintAxisID)mpParams->GetInt(epm_paintdeform_axis);
	return PAINTAXIS_NONE;
}
void EditPolyMod::SetPaintAxis( PaintModeID paintMode, PaintAxisID axis ) {
	if( (mpParams!=NULL) && IsPaintDeformMode(paintMode) )
		mpParams->SetValue(epm_paintdeform_axis, 0, (int)axis);
}

MNMesh* EditPolyMod::GetPaintObject( MeshPaintHost* host )
{
	EditPolyData* modData = static_cast<EditPolyData*>(host);
	return modData->GetPaintMesh();
}

void EditPolyMod::GetPaintInputSel( MeshPaintHost* host, FloatTab& selValues ) {
	EditPolyData* modData = static_cast<EditPolyData*>(host);

	GenSoftSelData softSelData;
	GetSoftSelData( softSelData, ip->GetTime() );
	FloatTab* weights = modData->TempData()->VSWeight(
		softSelData.useEdgeDist, softSelData.edgeIts, softSelData.ignoreBack,
		softSelData.falloff, softSelData.pinch, softSelData.bubble );

	if (!weights) {
		// can happen if num vertices == 0.
		selValues.ZeroCount();
		return;
	}
	selValues = *weights;
}

void EditPolyMod::GetPaintInputVertPos( MeshPaintHost* host, Point3Tab& vertPos ) {
	EditPolyData* modData = static_cast<EditPolyData*>(host);
	MNMesh *pMesh = modData->GetMesh();
	DbgAssert (pMesh);
	if (!pMesh) return;	// avoid crashing.
	MNMesh& mesh = *(pMesh);
	int count = mesh.VNum();
	vertPos.SetCount( count );
	for( int i=0; i<count; i++ ) vertPos[i] = mesh.v[i].p;
}

void EditPolyMod::GetPaintMask( MeshPaintHost* host, BitArray& sel, FloatTab& softSel ) {
	EditPolyData* modData = static_cast<EditPolyData*>(host);
	if (!modData->GetMesh()) return;
	MNMesh& mesh = *(modData->GetMesh());
	sel = mesh.VertexTempSel ();
	float* weights = mesh.getVSelectionWeights ();
	int count = (weights==NULL? 0:mesh.VNum());
	softSel.SetCount(count);
	if( count>0 ) memcpy( softSel.Addr(0), weights, count*sizeof(float) );
}

void EditPolyMod::ApplyPaintSel( MeshPaintHost* host ) {
	EditPolyData* modData = static_cast<EditPolyData*>(host);
	if (!modData->GetMesh()) return;
	MNMesh& mesh = *(modData->GetMesh());

	//arValid = NEVER;
	// TODO: Where do we keep this soft selection?
	float* destWeights = mesh.getVSelectionWeights();
	float* srcWeights = modData->GetPaintSelValues();
	int count = modData->GetPaintSelCount();

	if (mesh.numv < count) {
		DbgAssert (false);
		count = mesh.numv;
	}

	memcpy( destWeights, srcWeights, count * sizeof(float) );

	if (mesh.numv > count) {
		DbgAssert (false);
		for (int i=count; i<mesh.numv; i++) destWeights[i] = 0.0f;
	}
}

void LocalPaintDeformData::SetOffsets (int count, Point3 *pOffsets, MNMesh *origMesh)
{
	if (!origMesh) return;
	mVertexMesh.setNumVerts (origMesh->numv);
	memcpy (mVertexMesh.v, origMesh->v, origMesh->numv*sizeof(MNVert));
	if (mVertexMesh.numv<count) count = mVertexMesh.numv;
	for (int i=0; i<count; i++) mVertexMesh.v[i].p += pOffsets[i];
}

const USHORT kChunkLocalPaintDeformMesh = 0x1234;	// obselete - used poor chunk design - kept for backward compat.
const USHORT kChunkLocalPaintDeformMeshNumVertices = 0x1235;
const USHORT kChunkLocalPaintDeformMeshVertices = 0x1236;

IOResult LocalPaintDeformData::Save (ISave *isave) {
	ULONG nb;

	int num = mVertexMesh.numv;
	if (num>0) {
		//isave->BeginChunk (kChunkLocalPaintDeformMesh);
		//isave->Write (&num, sizeof(int), &nb);
		//isave->Write (mVertexMesh.v, num*sizeof(MNVert), &nb);
		//isave->EndChunk();

		isave->BeginChunk (kChunkLocalPaintDeformMeshNumVertices);
		isave->Write (&num, sizeof(int), &nb);
		isave->EndChunk ();

		isave->BeginChunk (kChunkLocalPaintDeformMeshVertices);
		isave->Write (mVertexMesh.v, num*sizeof(MNVert), &nb);
		isave->EndChunk ();
	}

	return IO_OK;
}

IOResult LocalPaintDeformData::Load (ILoad *iload) {
	IOResult res;
	int num;
	ULONG nb;

	while (IO_OK==(res=iload->OpenChunk())) {
		switch (iload->CurChunkID ())
		{
		case kChunkLocalPaintDeformMesh:
			if ((res = iload->Read (&num, sizeof(int), &nb)) != IO_OK) break;
			mVertexMesh.setNumVerts (num);
			res = iload->Read (mVertexMesh.v, num*sizeof(MNVert), &nb);
			break;

		case kChunkLocalPaintDeformMeshNumVertices:
			res = iload->Read (&num, sizeof(int), &nb);
			if (res == IO_OK) mVertexMesh.setNumVerts (num);
			break;

		case kChunkLocalPaintDeformMeshVertices:
			res = iload->Read (mVertexMesh.v, mVertexMesh.numv*sizeof(MNVert), &nb);
			break;
		}
		iload->CloseChunk();
		if (res!=IO_OK) return res;
	}
	return IO_OK;
}

void LocalPaintDeformData::RescaleWorldUnits (float f) {
	for (int i=0; i<mVertexMesh.numv; i++) mVertexMesh.v[i].p *= f;
}

void EditPolyMod::ApplyPaintDeform( MeshPaintHost* host ,BitArray *invalidVerts) {
	EditPolyData* modData = static_cast<EditPolyData*>(host);

	// Make sure we're set to the right operation:
	EpModSetOperation (ep_op_paint_deform);
	modData->SetPolyOpData (ep_op_paint_deform);

	LocalPaintDeformData *pDeform = (LocalPaintDeformData *) modData->GetPolyOpData();
	pDeform->SetOffsets (host->GetPaintDeformCount(), host->GetPaintDeformOffsets(), modData->GetMesh());

	EpModLocalDataChanged (PART_DISPLAY);
}

void EditPolyMod::RevertPaintDeform( MeshPaintHost* host,BitArray *invalidVerts ) {
	EditPolyData* modData = static_cast<EditPolyData*>(host);

	// Make sure we're set to the right operation:
	//EpModSetOperation (ep_op_paint_deform);
	//modData->SetPolyOpData (ep_op_paint_deform);

	if (modData->GetPolyOpData() == NULL) return;
	if (modData->GetPolyOpData()->OpID() != ep_op_paint_deform) return;
	LocalPaintDeformData *pDeform = (LocalPaintDeformData *) modData->GetPolyOpData();
	pDeform->ClearVertices ();

	EpModLocalDataChanged (PART_DISPLAY);
}

void EditPolyMod::EpModPaintDeformCommit() {
	if( GetPolyOperationID()==ep_op_paint_deform ) {
		theHold.Begin ();
		EpModCommit (ip->GetTime(), false, true);
		theHold.Accept (GetString (IDS_PAINTDEFORM));
	}
}

void EditPolyMod::EpModPaintDeformCancel() {
	if( GetPolyOperationID()==ep_op_paint_deform )
		EpModCancel ();
}

void EditPolyMod::OnPaintDataRedraw( PaintModeID paintMode ) {
	if( IsPaintSelMode(paintMode) )		NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	if( IsPaintDeformMode(paintMode) )	NotifyDependents(FOREVER, PART_GEOM, REFMSG_CHANGE);
	if (ip) ip->RedrawViews( ip->GetTime() );
}

void EditPolyMod::OnPaintModeChanged( PaintModeID prevMode, PaintModeID curMode ) {
	// Call SetEnables() on appropriate dialog
	if( IsPaintSelMode(prevMode) || IsPaintSelMode(curMode) ) {
		HWND hWnd = GetDlgHandle (ep_softsel);
		if (hWnd) TheSoftselDlgProc()->SetEnables (hWnd, ip ? ip->GetTime() : TimeValue(0));
	}
	if( IsPaintDeformMode(prevMode) || IsPaintDeformMode(curMode) ) {
		HWND hWnd = GetDlgHandle (ep_paintdeform);
		if (hWnd) thePaintDeformDlgProc.SetEnables (hWnd);
	}
}

void EditPolyMod::OnPaintDataActivate( PaintModeID paintMode, BOOL onOff ) {
	// Call SetEnables() on appropriate dialog
	if( IsPaintSelMode(paintMode) ) {
		HWND hWnd = GetDlgHandle (ep_softsel);
		if (hWnd) TheSoftselDlgProc()->SetEnables (hWnd, ip ? ip->GetTime() : TimeValue(0));
	}
	if( IsPaintDeformMode(paintMode) ) {
		HWND hWnd = GetDlgHandle (ep_paintdeform);
		if (hWnd) thePaintDeformDlgProc.SetEnables (hWnd);
	}
}

void EditPolyMod::OnPaintBrushChanged() {
	InvalidateDialogElement(epm_paintsel_size);
	InvalidateDialogElement(epm_paintsel_strength);
	InvalidateDialogElement(epm_paintdeform_size);
	InvalidateDialogElement(epm_paintdeform_strength);
}

// Helper method
static BOOL IsButtonEnabled(HWND hParent, DWORD id) {
	if( hParent==NULL ) return FALSE;
	ICustButton* but = GetICustButton(GetDlgItem(hParent,id));
	BOOL retval = but->IsEnabled();
	ReleaseICustButton(but);
	return retval;
}

void EditPolyMod::OnParamSet(PB2Value& v, ParamID id, int tabIndex, TimeValue t) {
	switch( id ) {
	case epm_constrain_type:
		SetConstraintType(v.i);
		break;
	case epm_select_mode:
		SetSelectMode(v.i);
		break;
	case epm_ss_use:
		//When the soft selection is turned off, end any active paint session
		if( (!v.i) && InPaintMode() ) EndPaintMode();
		break;
	case epm_ss_lock:	{
			BOOL selActive = IsPaintDataActive(PAINTMODE_SEL);
			if( v.i && !selActive ) //Enable the selection lock; Activate the paintmesh selection
				ActivatePaintData(PAINTMODE_SEL);
			else if( (!v.i) && selActive ) { //Disable the selection lock; De-activate the paintmesh selection
				if( InPaintMode() ) EndPaintMode (); //end any active paint mode
				DeactivatePaintData(PAINTMODE_SEL);
			}
			//NOTE: SetEnables() is called by OnPaintDataActive() when activating or deactivating
			InvalidateDistanceCache ();
			if (mpDialogSoftsel) mpDialogSoftsel->RedrawViews (t);
			break;
		}
	case epm_paintsel_size:
	case epm_paintdeform_size:
		SetPaintBrushSize(v.f);
		UpdatePaintBrush();
		break;
	case epm_paintsel_strength:
	case epm_paintdeform_strength:
		SetPaintBrushStrength(v.f);
		UpdatePaintBrush();
		break;
	case epm_paintsel_value:
	case epm_paintdeform_value:
	case epm_paintdeform_axis:
		UpdatePaintBrush(); break;
	case epm_paintsel_mode:
		if( v.i!=GetPaintMode() ) {
			BOOL valid = FALSE;
			HWND hwnd = (mpDialogSoftsel==NULL?  NULL : mpDialogSoftsel->GetHWnd());
			if( v.i==PAINTMODE_PAINTSEL )	valid = IsButtonEnabled( hwnd, IDC_PAINTSEL_PAINT );
			if( v.i==PAINTMODE_BLURSEL )	valid = IsButtonEnabled( hwnd, IDC_PAINTSEL_BLUR );
			if( v.i==PAINTMODE_ERASESEL )	valid = IsButtonEnabled( hwnd, IDC_PAINTSEL_REVERT );
			if( v.i==PAINTMODE_OFF )		valid = TRUE;
			if( valid ) OnPaintBtn(v.i, t);
		}
		break;
	case epm_paintdeform_mode:
		if( v.i!=GetPaintMode() ) {
			BOOL valid = FALSE;
			HWND hwnd = (mpDialogPaintDeform==NULL?  NULL : mpDialogPaintDeform->GetHWnd());
			if( v.i==PAINTMODE_PAINTPUSHPULL )	valid = IsButtonEnabled( hwnd, IDC_PAINTDEFORM_PUSHPULL );
			if( v.i==PAINTMODE_PAINTRELAX )		valid = IsButtonEnabled( hwnd, IDC_PAINTDEFORM_RELAX );
			if( v.i==PAINTMODE_ERASEDEFORM )	valid = IsButtonEnabled( hwnd, IDC_PAINTDEFORM_REVERT );
			if( v.i==PAINTMODE_OFF )			valid = TRUE;
			if( valid ) OnPaintBtn(v.i, t);
		}
		break;
	}
}

void EditPolyMod::OnParamGet(PB2Value& v, ParamID id, int tabIndex, TimeValue t, Interval &valid) {
	switch( id ) {
	//The selection lock is defined as being "on" anytime the paintmesh selection is active
	case epm_ss_lock:
		v.i = IsPaintDataActive(PAINTMODE_SEL);
		break;
	case epm_paintsel_size:
	case epm_paintdeform_size:
		v.f = GetPaintBrushSize(); break;
	case epm_paintsel_strength:
	case epm_paintdeform_strength:
		v.f = GetPaintBrushStrength(); break;
	case epm_paintsel_mode:
		v.i = GetPaintMode();
		if( !IsPaintSelMode(v.i) ) v.i=PAINTMODE_OFF;
		break;
	case epm_paintdeform_mode:
		v.i = GetPaintMode();
		if( !IsPaintDeformMode(v.i) ) v.i=PAINTMODE_OFF;
		break;
	}
}

void EditPolyMod::OnPaintBtn( int mode, TimeValue t ) {
	PaintModeID paintMode = (PaintModeID)mode;
	if( (mode==GetPaintMode()) || (mode==PAINTMODE_OFF) ) {
		EndPaintMode ();
	}
	else {
		// Defect 682348 moved this above the Activate paint data
		// since if we are not commited the exit will commit and then see that we are a paint mode
		// deactivate the paint data
		// Exit any current command mode, if we're in one:
		ExitAllCommandModes (true, false);

		if( IsPaintSelMode( paintMode ) ) {
			if( !mpParams->GetInt(epm_ss_use) )	mpParams->SetValue( epm_ss_use, t, TRUE );	//enable soft selection
			if( !mpParams->GetInt(epm_ss_lock) ) mpParams->SetValue( epm_ss_lock, t, TRUE );	//enable the soft selection lock
		} else if( IsPaintDeformMode( paintMode ) ) {
			if( !IsPaintDataActive(paintMode) ) ActivatePaintData(paintMode);
		}

		BeginPaintMode( paintMode );
	}
}

TCHAR *EditPolyMod::GetPaintUndoString (PaintModeID mode) {
	if (IsPaintSelMode(mode)) return GetString (IDS_RESTOREOBJ_MESHPAINTSEL);
	return GetString (IDS_RESTOREOBJ_MESHPAINTDEFORM);
}









