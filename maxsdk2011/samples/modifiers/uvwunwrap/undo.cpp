#include "unwrap.h"

//*********************************************************
// Undo record for TV posiitons and flags
//*********************************************************


TVertRestore::TVertRestore(UnwrapMod *m, MeshTopoData *d, BOOL update) 
{
	mod = m;
	ld = d;

	ld->CopyTVData(undo);
	uvsel = ld->GetTVVertSelection();


	updateView = update;
}

void TVertRestore::Restore(int isUndo) 
{
	if (isUndo) 
	{
		ld->CopyTVData(redo);
		rvsel = ld->GetTVVertSelection();

	}

	ld->PasteTVData(undo);
	ld->SetTVVertSelection(uvsel);

	if (updateView)
	{

		if (mod->fnGetSyncSelectionMode()) 
		{
			theHold.Suspend();
			mod->fnSyncGeomSelection();
			theHold.Resume();
		}

		mod->NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);
		if (mod->editMod==mod && mod->hView) mod->InvalidateView();
	}
}

void TVertRestore::Redo() 
{
	ld->PasteTVData(redo);
	ld->SetTVVertSelection(rvsel);

	if (mod->fnGetSyncSelectionMode()) 
	{
		theHold.Suspend();
		mod->fnSyncGeomSelection();
		theHold.Resume();
	}

	if (updateView)
	{
		mod->NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);
		if (mod->editMod==mod && mod->hView) mod->InvalidateView();
	}
}
void TVertRestore::EndHold() 
{
	updateView = TRUE;
	mod->ClearAFlag(A_HELD);
}
TSTR TVertRestore::Description() 
{
	return TSTR(_T(GetString(IDS_PW_UVW_VERT_EDIT)));
}		


//*********************************************************
// Undo record for TV posiitons and face topology
//*********************************************************


TVertAndTFaceRestore::TVertAndTFaceRestore(UnwrapMod *m,MeshTopoData *d,bool hide) 
	{
	mHidePeltDialog = hide;
	mod = m;

	mod = m;
	ld = d;

	ld->CopyTVData(undo);
	ld->CopyFaceData(fundo);

	uvsel   = d->GetTVVertSelection();
	ufsel   = d->GetFaceSelection();
	uesel   = d->GetTVEdgeSelection();
	update = FALSE;
	}
TVertAndTFaceRestore::~TVertAndTFaceRestore() 
	{
	int ct = fundo.Count();
	for (int i =0; i < ct; i++)
		{
		if (fundo[i]->vecs) delete fundo[i]->vecs;
		fundo[i]->vecs = NULL;

		if (fundo[i]) delete fundo[i];
		fundo[i] = NULL;
		}
			
	ct = fredo.Count();
	for (int i =0; i < ct; i++)
		{
		if (fredo[i]->vecs) delete fredo[i]->vecs;
		fredo[i]->vecs = NULL;

		if (fredo[i]) delete fredo[i];
		fredo[i] = NULL;
		}


	}	

void TVertAndTFaceRestore::Restore(int isUndo) 
	{
	if (isUndo) 
		{
		ld->CopyTVData(redo);

		ld->CopyFaceData(fredo);

		rvsel   = ld->GetTVVertSelection();
		rfsel   = ld->GetFaceSelection();
		resel   = ld->GetTVEdgeSelection();

		if (mHidePeltDialog && (mod->mapMapMode == PELTMAP))
			{
			mod->peltData.SetPeltMapMode(mod,FALSE);
			mod->mapMapMode = NOMAP;
			}
		}

	ld->PasteTVData(undo);
	ld->PasteFaceData(fundo);
	ld->SetTVVertSelection(uvsel);
	ld->SetFaceSelection(ufsel);
	ld->SetTVEdgeSelection(uesel);

	if (mod->fnGetSyncSelectionMode()) 
		{
		theHold.Suspend();
		mod->fnSyncGeomSelection();
		theHold.Resume();
		}

	if (update)
		{
		mod->NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);
		if (mod->editMod==mod && mod->hView) mod->InvalidateView();
		}
	}
void TVertAndTFaceRestore::Redo() 
	{
	ld->PasteTVData(redo);
	ld->PasteFaceData(fredo);
	ld->SetTVVertSelection(rvsel);
	ld->SetFaceSelection(rfsel);
	ld->SetTVEdgeSelection(resel);

	if (mod->fnGetSyncSelectionMode()) 
		{
		theHold.Suspend();
		mod->fnSyncGeomSelection();
		theHold.Resume();
		}

	if (update)
		{
		mod->NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);
		if (mod->editMod==mod && mod->hView) mod->InvalidateView();
		}
	}
void TVertAndTFaceRestore::EndHold() 
	{
	update = TRUE;
	mod->ClearAFlag(A_HELD);
	}
TSTR TVertAndTFaceRestore::Description() {return TSTR(_T(GetString(IDS_PW_UVW_EDIT)));}




//*********************************************************
// Undo record for selection of point in the dialog window
//*********************************************************

TSelRestore::TSelRestore(UnwrapMod *m,MeshTopoData *ld) 
{
	mod = m;
	this->ld = ld;

	undo = ld->GetTVVertSelection();
	eundo = ld->GetTVEdgeSelection();
	fundo = ld->GetFaceSelection();
	gvundo = ld->GetGeomVertSelection();

	geundo = ld->GetGeomEdgeSelection();
	
}

void TSelRestore::Restore(int isUndo) 
{
	if (isUndo) 
	{
		redo = ld->GetTVVertSelection();
		eredo = ld->GetTVEdgeSelection();
		fredo = ld->GetFaceSelection();

		gvredo = ld->GetGeomVertSelection();
		geredo = ld->GetGeomEdgeSelection();
	

	}

	ld->SetTVVertSelection(undo);
	ld->SetTVEdgeSelection(eundo);
	ld->SetFaceSelection(fundo);
	ld->SetGeomVertSelection(gvundo);
	ld->SetGeomEdgeSelection(geundo);

	mod->RebuildDistCache();

	if (mod->fnGetSyncSelectionMode()) 
		{
		theHold.Suspend();
		mod->fnSyncGeomSelection();
		theHold.Resume();
		}

	if (mod->editMod==mod && mod->hView) 
		mod->InvalidateView();
	}
void TSelRestore::Redo() 
	{
	ld->SetTVVertSelection(redo);
	ld->SetTVEdgeSelection(eredo);
	ld->SetFaceSelection(fredo);
	ld->SetGeomVertSelection(gvredo);
	ld->SetGeomEdgeSelection(geredo);

	mod->RebuildDistCache();

	if (mod->fnGetSyncSelectionMode()) 
		{
		theHold.Suspend();
		mod->fnSyncGeomSelection();
		theHold.Resume();
		}

	if (mod->editMod==mod && mod->hView) mod->InvalidateView();
	}
void TSelRestore::EndHold() {mod->ClearAFlag(A_HELD);}
TSTR TSelRestore::Description() {return TSTR(_T(GetString(IDS_PW_SELECT_UVW)));}


//*********************************************************
// Undo record for a reset operation
//*********************************************************


ResetRestore::ResetRestore(UnwrapMod *m) 
	{
			
	mod = m;

	uchan   = mod->channel;
	}
ResetRestore::~ResetRestore() 
	{



	}	

void ResetRestore::Restore(int isUndo) 
	{
	if (isUndo) 
		{

		rchan   = mod->channel;
		}
	mod->channel = uchan;
	if (mod->ip)
	{
		if (mod->iMapID) 
			mod->iMapID->SetValue(mod->channel,FALSE);

		if (mod->channel == 1)
		{
			CheckRadioButton(  mod->hParams, IDC_MAP_CHAN1, IDC_MAP_CHAN2,IDC_MAP_CHAN2);
		}
		else 
		{
			CheckRadioButton(  mod->hParams, IDC_MAP_CHAN1, IDC_MAP_CHAN2,IDC_MAP_CHAN1);
		}	
	}

	mod->RebuildEdges();
	mod->NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);
	if (mod->editMod==mod && mod->hView) mod->InvalidateView();
	}
void ResetRestore::Redo() 
	{
	mod->channel = rchan;
	if (mod->ip)
	{

		if (mod->iMapID) 
			mod->iMapID->SetValue(mod->channel,FALSE);
		if (mod->channel == 1)
		{
			CheckRadioButton(  mod->hParams, IDC_MAP_CHAN1, IDC_MAP_CHAN2,IDC_MAP_CHAN2);
		}
		else 
		{
			CheckRadioButton(  mod->hParams, IDC_MAP_CHAN1, IDC_MAP_CHAN2,IDC_MAP_CHAN1);
		}	
	}

	mod->RebuildEdges();
	mod->NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);
	if (mod->editMod==mod && mod->hView) mod->InvalidateView();
	}

void ResetRestore::EndHold() {mod->ClearAFlag(A_HELD);}
TSTR ResetRestore::Description() {return TSTR(_T(GetString(IDS_PW_RESET_UNWRAP)));}




UnwrapPivotRestore::UnwrapPivotRestore(UnwrapMod *m) 
{
	mod     = m;
	upivot = mod->freeFormPivotOffset;
	
}

void UnwrapPivotRestore::Restore(int isUndo) {
	if (isUndo) {
		rpivot = mod->freeFormPivotOffset;
		}
	
	mod->freeFormPivotOffset = upivot; 
	mod->NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	mod->InvalidateView();
//	mod->SetNumSelLabel();
}

void UnwrapPivotRestore::Redo() {
	mod->freeFormPivotOffset = rpivot; 
	
	mod->NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	mod->InvalidateView();
	}

UnwrapSeamAttributesRestore::UnwrapSeamAttributesRestore(UnwrapMod *m) 
{
	mod     = m;

	uReflatten = mod->fnGetPreventFlattening();
	uThick = mod->fnGetThickOpenEdges();
	uShowMapSeams = mod->fnGetViewportOpenEdges();
	uShowPeltSeams = mod->fnGetAlwayShowPeltSeams();

}

void UnwrapSeamAttributesRestore::Restore(int isUndo) 
{
	if (isUndo) 
	{
		rReflatten = mod->fnGetPreventFlattening();
		rThick = mod->fnGetThickOpenEdges();
		rShowMapSeams = mod->fnGetViewportOpenEdges();
		rShowPeltSeams = mod->fnGetAlwayShowPeltSeams();
	}

	 mod->fnSetPreventFlattening(uReflatten);
	 mod->fnSetThickOpenEdges(uThick);
	 mod->fnSetViewportOpenEdges(uShowMapSeams);
	 mod->fnSetAlwayShowPeltSeams(uShowPeltSeams);


	mod->NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	mod->InvalidateView();
}

void UnwrapSeamAttributesRestore::Redo() 
{

	 mod->fnSetPreventFlattening(rReflatten);
	 mod->fnSetThickOpenEdges(rThick);
	 mod->fnSetViewportOpenEdges(rShowMapSeams);
	 mod->fnSetAlwayShowPeltSeams(rShowPeltSeams);

	mod->NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	mod->InvalidateView();
}


UnwrapMapAttributesRestore::UnwrapMapAttributesRestore(UnwrapMod *m) 
{
	mod     = m;

	uPreview = mod->fnGetQMapPreview();
	uNormalize = mod->fnGetNormalizeMap();
	uAlign = mod->GetQMapAlign();

}

void UnwrapMapAttributesRestore::Restore(int isUndo) 
{
	if (isUndo) 
	{
		rPreview = mod->fnGetQMapPreview();
		rNormalize = mod->fnGetNormalizeMap();
		rAlign = mod->GetQMapAlign();
	}

	mod->fnSetNormalizeMap(uNormalize);
	int align = uAlign;
	HWND hWnd = mod->hMapParams;
	CheckDlgButton(hWnd,IDC_RADIO1,FALSE);
	CheckDlgButton(hWnd,IDC_RADIO2,FALSE);
	CheckDlgButton(hWnd,IDC_RADIO3,FALSE);
	CheckDlgButton(hWnd,IDC_RADIO4,FALSE);
	if (align == 0)
		CheckDlgButton(hWnd,IDC_RADIO1,TRUE);
	else if (align == 1)
		CheckDlgButton(hWnd,IDC_RADIO2,TRUE);
	else if (align == 2)
		CheckDlgButton(hWnd,IDC_RADIO3,TRUE);
	else if (align == 3)
		CheckDlgButton(hWnd,IDC_RADIO4,TRUE);
	CheckDlgButton(hWnd,IDC_PREVIEW_CHECK,uPreview);

	mod->NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	mod->InvalidateView();
}

void UnwrapMapAttributesRestore::Redo() 
{

	mod->fnSetNormalizeMap(rNormalize);
	int align = rAlign;
	HWND hWnd = mod->hMapParams;
	CheckDlgButton(hWnd,IDC_RADIO1,FALSE);
	CheckDlgButton(hWnd,IDC_RADIO2,FALSE);
	CheckDlgButton(hWnd,IDC_RADIO3,FALSE);
	CheckDlgButton(hWnd,IDC_RADIO4,FALSE);
	if (align == 0)
		CheckDlgButton(hWnd,IDC_RADIO1,TRUE);
	else if (align == 1)
		CheckDlgButton(hWnd,IDC_RADIO2,TRUE);
	else if (align == 2)
		CheckDlgButton(hWnd,IDC_RADIO3,TRUE);
	else if (align == 3)
		CheckDlgButton(hWnd,IDC_RADIO4,TRUE);
	CheckDlgButton(hWnd,IDC_PREVIEW_CHECK,rPreview);


	mod->NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	mod->InvalidateView();
}


