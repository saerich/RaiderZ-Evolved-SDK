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
#include "maxscrpt\MAXScrpt.h"
#include "maxscrpt\MAXObj.h"
#include "maxscrpt\3DMath.h"
#include "maxscrpt\msplugin.h"


void SmoothingGroupUIHandler::InitializeButtons (HWND hWnd)
{

	for (int i=IDC_SMOOTH_GRP1; i<IDC_SMOOTH_GRP1+32; i++)
		if(GetDlgItem(hWnd,i))
			SendMessage (GetDlgItem(hWnd,i),CC_COMMAND,CC_CMD_SET_TYPE,CBT_CHECK);
}

void SmoothingGroupUIHandler::UpdateSmoothButtons (HWND hWnd, DWORD set, DWORD indeterminate, DWORD unused) {
	for (int i=0; i<32; i++) {
		HWND hButton = GetDlgItem (hWnd, i+IDC_SMOOTH_GRP1);
		if (!hButton) continue;

		DWORD grp = DWORD(1)<<i;

		if (unused & grp) {
			ShowWindow (hButton, SW_HIDE);
			continue;
		}

		if (indeterminate & grp) {
			SetWindowText (hButton, NULL);
			SendMessage (hButton, CC_COMMAND, CC_CMD_SET_STATE, false);
		} else {
			TSTR buf;
			buf.printf(_T("%d"),i+1);
			SetWindowText (hButton, buf);
			SendMessage (hButton, CC_COMMAND, CC_CMD_SET_STATE, (set&grp)?true:false);
		}
		InvalidateRect (hButton, NULL, TRUE);
	}
}

void SmoothingGroupUIHandler::UpdateSmoothButtons (HWND hWnd, EPolyMod *pMod)
{
	if (!pMod || !pMod->EpModGetIP()) return;

	DWORD allFaces, anyFaces;
	mUIValid = GetSmoothingGroups (pMod, &anyFaces, &allFaces, true);

	DWORD indeterminate = anyFaces & ~allFaces;

	//if (pMod->GetPolyOperationID() == ep_op_smooth)
	//{
	//	int iset = pMod->getParamBlock()->GetInt (epm_smooth_group_set);
	//	int iclr = pMod->getParamBlock()->GetInt (epm_smooth_group_clear);
	//	DWORD *dset = (DWORD *) ((void*)&iset);
	//	DWORD *dclr = (DWORD *) ((void*)&iclr);

	//	indeterminate &= ~((*dset) | (*dclr));
	//	allFaces |= (*dset);
	//	allFaces &= ~(*dclr);
	//}

	UpdateSmoothButtons (hWnd, allFaces, indeterminate);
}

bool SmoothingGroupUIHandler::GetSmoothingGroups (EPolyMod *pMod, DWORD *anyFaces, DWORD *allFaces, bool useSel) {
	bool ret = true;
	// Initialization:
	*anyFaces = 0;
	if (allFaces) *allFaces = ~0;

	bool useStackSelection = pMod->getParamBlock()->GetInt (epm_stack_selection) != 0;

	ModContextList list;
	INodeTab nodes;
	pMod->EpModGetIP()->GetModContexts (list, nodes);
	for (int i=0; i<list.Count(); i++)
	{
		EditPolyData *pData = (EditPolyData *) list[i]->localData;
		if (!pData) continue;
		MNMesh *pMesh = pData->GetMeshOutput();
		if (!pMesh) {
			ret = false;
			continue;
		}

		DWORD l_selFlag = useStackSelection ? MN_EDITPOLY_STACK_SELECT:MN_SEL;

		for (int j=0; j<pMesh->numf; j++)
		{
			if (useSel && !pMesh->f[j].GetFlag (l_selFlag)) 
				continue;
			if (pMesh->f[j].GetFlag (MN_DEAD)) 
				continue;

			*anyFaces |= pMesh->f[j].smGroup;
			if (allFaces) *allFaces &= pMesh->f[j].smGroup;
		}
	}
	nodes.DisposeTemporary ();

	if (allFaces) *allFaces &= *anyFaces;
	return ret;
}

void SmoothingGroupUIHandler::SetSmoothing (HWND hWnd, int buttonID, EPolyMod *pMod)
{
	ICustButton *pBut = GetICustButton (GetDlgItem (hWnd, buttonID));
	if (pBut == NULL) return;
	bool set = pBut->IsChecked()!=0;

	IParamBlock2 *pBlock = pMod->getParamBlock ();
	int i = buttonID - IDC_SMOOTH_GRP1;
	DWORD bit = DWORD(1)<<i;

	theHold.Begin ();
	bool startFresh = (pMod->GetPolyOperationID() != ep_op_smooth);
	if (startFresh) pMod->EpModSetOperation (ep_op_smooth);
	int acceptID;
	if (set) {
		int smoothers = startFresh ? 0 : pBlock->GetInt (epm_smooth_group_set);
		DWORD *realSmoothers = (DWORD *)((void *)&smoothers);
		(*realSmoothers) |= bit;
		pBlock->SetValue (epm_smooth_group_set, 0, smoothers);
		acceptID = IDS_SET_SMOOTHING;
	} else {
		int smoothers = startFresh ? 0 : pBlock->GetInt (epm_smooth_group_clear);
		DWORD *realSmoothers = (DWORD *)((void *)&smoothers);
		(*realSmoothers) |= bit;
		pBlock->SetValue (epm_smooth_group_clear, 0, smoothers);

		// Make sure to remove this from the ones being "set" as well, if needed:
		if (!startFresh && (smoothers = pBlock->GetInt (epm_smooth_group_set)) != 0) {
			if ((*realSmoothers) & bit) {
				(*realSmoothers) -= bit;
				pBlock->SetValue (epm_smooth_group_set, 0, smoothers);
			}
		}

		acceptID = IDS_CLEAR_SMOOTHING;
	}

	theHold.Accept (GetString (acceptID));
}

void SmoothingGroupUIHandler::ClearAll (EPolyMod *pMod)
{
	theHold.Begin ();
	pMod->EpModSetOperation (ep_op_smooth);
	pMod->getParamBlock()->SetValue (epm_smooth_group_set, 0, 0);
	pMod->getParamBlock()->SetValue (epm_smooth_group_clear, 0, -1);
	theHold.Accept (GetString (IDS_CLEAR_SMOOTHING));
}

static DWORD selectBySmoothGroups = 0;
static bool selectBySmoothClear = true;

static INT_PTR CALLBACK SelectBySmoothDlgProc (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	static DWORD *param;
	int i(0);
	ICustButton* iBut = NULL;

	static SmoothingGroupUIHandler smoothingGroupUIHandler;

	switch (msg) {
	case WM_INITDIALOG:
		ThePopupPosition()->Set (hWnd);
		param = (DWORD*)lParam;
		for (i=IDC_SMOOTH_GRP1; i<IDC_SMOOTH_GRP1+32; i++)
			if(GetDlgItem(hWnd,i))
				SendMessage(GetDlgItem(hWnd,i),CC_COMMAND,CC_CMD_SET_TYPE,CBT_CHECK);
		selectBySmoothGroups = 0;
		smoothingGroupUIHandler.UpdateSmoothButtons (hWnd, selectBySmoothGroups, 0, param[0]);
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
			ThePopupPosition()->Scan (hWnd);
			if(GetDlgItem(hWnd,IDC_CLEARSELECTION))
				selectBySmoothClear = IsDlgButtonChecked(hWnd,IDC_CLEARSELECTION) ? true : false;
			EndDialog(hWnd,1);
			break;

		case IDCANCEL:
			ThePopupPosition()->Scan (hWnd);
			EndDialog(hWnd,0);
			break;
		}
		break;			

	default:
		return FALSE;
	}
	return TRUE;
}

bool SmoothingGroupUIHandler::GetSelectBySmoothParams (EPolyMod *pEPoly) {
	// Find all the smoothing groups used in all our meshes:
	DWORD usedBits;
	GetSmoothingGroups (pEPoly, &usedBits, NULL, false);
	DWORD unused = ~usedBits;
	if (DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_EP_SELECTBYSMOOTH),
		pEPoly->EpModGetIP()->GetMAXHWnd(), SelectBySmoothDlgProc, (LPARAM)&unused)) {
		int *selby = (int *)((void*)&selectBySmoothGroups);
		pEPoly->getParamBlock()->SetValue (epm_smoother_selby, 0, *selby);
		pEPoly->getParamBlock()->SetValue (epm_smoother_selby_clear, 0, selectBySmoothClear);
		return true;
	}
	return false;
}


void MaterialUIHandler::GetMtlIDList (Mtl *mtl, NumList& mtlIDList)
{
	if (mtl != NULL && mtl->ClassID() == Class_ID(MULTI_CLASS_ID, 0)) {
		int subs = mtl->NumSubMtls();
		if (subs <= 0) subs = 1;

		for (int i=0; i<subs;i++){
			if(mtl->GetSubMtl(i)) mtlIDList.Add(i, TRUE);
		}
	}
}

void MaterialUIHandler::GetMtlIDList (EPolyMod *pMod, INode *node, NumList& mtlIDList)
{
	if (!pMod) return;
	if (!node) return;

	MNMesh *pMesh = pMod->EpModGetMesh (node);
	if (!pMesh) return;
	for (int i=0; i<pMesh->numf; i++)
	{
		int mid = pMesh->f[i].material;
		if (mid != -1) mtlIDList.Add (mid, true);
	}
}

INode* MaterialUIHandler::GetNode (EPolyMod *pMod){
	if (!pMod || !pMod->EpModGetIP()) return NULL;

	ModContextList mcList;
	INodeTab nodes;
	pMod->EpModGetIP()->GetModContexts (mcList, nodes);
	INode* objnode = nodes.Count() == 1 ? nodes[0]->GetActualINode(): NULL;
	nodes.DisposeTemporary();
	return objnode;
}

bool MaterialUIHandler::SetupMtlSubNameCombo (HWND hWnd, EPolyMod *pMod) {
	INode *singleNode = GetNode(pMod);
	Mtl *nodeMtl = (singleNode) ? singleNode->GetMtl() : NULL;
	// check for scripted material
	if(nodeMtl){
		MSPlugin* plugin = (MSPlugin*)((ReferenceTarget*)nodeMtl)->GetInterface(I_MAXSCRIPTPLUGIN);
		if(plugin)
			nodeMtl = (Mtl*)plugin->get_delegate();
	} 


	if (nodeMtl == NULL || nodeMtl->ClassID() != Class_ID(MULTI_CLASS_ID, 0)) {    //no UI for cloned nodes, and not MultiMtl 
		SendMessage(GetDlgItem(hWnd, IDC_MTLID_NAMES_COMBO), CB_RESETCONTENT, 0, 0);
		EnableWindow(GetDlgItem(hWnd, IDC_MTLID_NAMES_COMBO), false);
		return false;
	}

	NumList mtlIDList;
	NumList mtlIDMeshList;
	GetMtlIDList (nodeMtl, mtlIDList);
	GetMtlIDList (pMod, singleNode, mtlIDMeshList);
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

void MaterialUIHandler::UpdateNameCombo (HWND hWnd, ISpinnerControl *spin) {
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

void MaterialUIHandler::UpdateCurrentMaterial (HWND hWnd, EPolyMod *pMod, TimeValue t, Interval & validity)
{
	MtlID mat = 0;
	bool determined = false;

	if (pMod->GetPolyOperationID() == ep_op_set_material)
	{
		int materialHolder;
		pMod->getParamBlock()->GetValue (epm_material, t, materialHolder, validity);
		mat = materialHolder;
		determined = true;
	}
	else
	{
		bool useStackSelection = pMod->getParamBlock()->GetInt (epm_stack_selection) != 0;

		bool init = false;
		ModContextList list;
		INodeTab nodes;
		pMod->EpModGetIP()->GetModContexts (list, nodes);
      int i;
		for (i=0; i<list.Count(); i++)
		{
			EditPolyData *pData = (EditPolyData *) list[i]->localData;
			if (!pData) continue;
			MNMesh *pMesh = pData->GetMesh();
			if (!pMesh) {
				validity = NEVER;
				continue;
			}

			DWORD l_selFlag = useStackSelection ? MN_EDITPOLY_STACK_SELECT:MN_SEL;

         int j;
			for (j=0; j<pMesh->numf; j++)
			{
				if (!pMesh->f[j].GetFlag (l_selFlag)) 
					continue;
				if (pMesh->f[j].GetFlag (MN_DEAD))
					continue;

				if (!init)
				{
					mat = pMesh->f[j].material;
					init = true;
				}
				else if (mat != pMesh->f[j].material) break;
			}

			if (j<pMesh->numf) break;
		}
		nodes.DisposeTemporary ();

		determined = (i>=list.Count()) && init;
	}

	ISpinnerControl *spin = GetISpinner(GetDlgItem(hWnd,IDC_MAT_IDSPIN));
	spin->SetIndeterminate(!determined);
	if (determined) spin->SetValue (int(mat+1), FALSE);
	ReleaseISpinner(spin);

	spin = GetISpinner(GetDlgItem(hWnd,IDC_MAT_IDSPIN_SEL)); 
	spin->SetIndeterminate(!determined);
	if (determined) spin->SetValue (int(mat+1), FALSE);
	ReleaseISpinner(spin);

	if (GetDlgItem (hWnd, IDC_MTLID_NAMES_COMBO)) { 
		ValidateUINameCombo(hWnd, pMod);   
	}
}

void MaterialUIHandler::ValidateUINameCombo (HWND hWnd, EPolyMod *pMod) {
	SetupMtlSubNameCombo (hWnd, pMod);
	ISpinnerControl* spin = NULL;
	spin = GetISpinner(GetDlgItem(hWnd,IDC_MAT_IDSPIN));
	if (spin) UpdateNameCombo (hWnd, spin);
	ReleaseISpinner(spin);
}

void MaterialUIHandler::SelectByName (HWND hWnd, EPolyMod *pMod) {
	int index, val;
	index = SendMessage(GetDlgItem(hWnd,IDC_MTLID_NAMES_COMBO), CB_GETCURSEL, 0, 0);
	val = SendMessage(GetDlgItem(hWnd, IDC_MTLID_NAMES_COMBO), CB_GETITEMDATA, (WPARAM)index, 0);
	if (index != CB_ERR){
		int selectByMatClear = IsDlgButtonChecked(hWnd,IDC_CLEARSELECTION);
		theHold.Begin();
		pMod->getParamBlock()->SetValue (epm_material_selby, 0, val);
		pMod->getParamBlock()->SetValue (epm_material_selby_clear, 0, selectByMatClear);
		pMod->EpModButtonOp (ep_op_select_by_material);
		theHold.Accept(GetString(IDS_SELECT_BY_MATID));
		pMod->EpModRefreshScreen();
	}
}

static SurfaceDlgProc theSurfaceDlgProc;
static SurfaceDlgProc theSurfaceDlgProcFloater;
static FaceSmoothDlgProc theFaceSmoothDlgProc;
static FaceSmoothDlgProc theFaceSmoothDlgProcFloater;

SurfaceDlgProc *TheSurfaceDlgProc()
{
	return &theSurfaceDlgProc;
}

SurfaceDlgProc *TheSurfaceDlgProcFloater()
{
	return &theSurfaceDlgProc;
}


FaceSmoothDlgProc *TheFaceSmoothDlgProc()
{
	return &theFaceSmoothDlgProc;
}

FaceSmoothDlgProc *TheFaceSmoothDlgProcFloater()
{
	return &theFaceSmoothDlgProcFloater;
}

void SurfaceDlgProc::AddTimeChangeInvalidate (HWND hWnd) {
	if (!mTimeChangeHandle) {
		Interface *ip = GetCOREInterface();
		ip->RegisterTimeChangeCallback (this);
	}
	mTimeChangeHandle = hWnd;
}

void SurfaceDlgProc::RemoveTimeChangeInvalidate () {
	if (!mTimeChangeHandle) return;

	Interface *ip = GetCOREInterface ();
	ip->UnRegisterTimeChangeCallback (this);

	mTimeChangeHandle = NULL;
}

INT_PTR SurfaceDlgProc::DlgProc (TimeValue t, IParamMap2 *map, HWND hWnd,
							  UINT msg, WPARAM wParam, LPARAM lParam) {
	ISpinnerControl* spin = NULL;
	int buttonID(0);

	switch (msg) {
	case WM_INITDIALOG:
		SetupIntSpinner (hWnd, IDC_MAT_IDSPIN, IDC_MAT_ID, 1, MAX_MATID, 0);
		SetupIntSpinner (hWnd, IDC_MAT_IDSPIN_SEL, IDC_MAT_ID_SEL, 1, MAX_MATID, 0);     
		CheckDlgButton(hWnd, IDC_CLEARSELECTION, 1);                                 
		mMaterialUIHandler.SetupMtlSubNameCombo (hWnd, mpMod);           
		// Cue an update based on the current face selection.
		uiValid.SetEmpty();
		klugeToFixWM_CUSTEDIT_ENTEROnEnterFaceLevel = true;
		mSpinningMaterial = false;
		mTimeChangeHandle = NULL;
		break;

	case WM_PAINT:

		
		uiValid = FOREVER;
		// Display the correct material index:
		mMaterialUIHandler.UpdateCurrentMaterial (hWnd, mpMod, t, uiValid);

		if (!(uiValid == FOREVER) && !(uiValid == NEVER)) AddTimeChangeInvalidate (hWnd);
		else RemoveTimeChangeInvalidate ();

		klugeToFixWM_CUSTEDIT_ENTEROnEnterFaceLevel = false;
		return FALSE;

	case CC_SPINNER_BUTTONDOWN:
		switch (LOWORD(wParam)) {
		case IDC_MAT_IDSPIN:
			if (!mSpinningMaterial) {
				theHold.Begin ();
				mpMod->EpModSetOperation (ep_op_set_material);
				mSpinningMaterial = true;
			}
			break;
		}
		break;

	case WM_CUSTEDIT_ENTER:
	case CC_SPINNER_BUTTONUP:
		switch (LOWORD(wParam)) {
		case IDC_MAT_ID:
		case IDC_MAT_IDSPIN:
			if (!mSpinningMaterial) break;

			// For some reason, there's a WM_CUSTEDIT_ENTER sent on IDC_MAT_ID
			// when we start this dialog up.  Use this variable to suppress its activity.
			if (klugeToFixWM_CUSTEDIT_ENTEROnEnterFaceLevel) break;
			if (HIWORD(wParam) || msg==WM_CUSTEDIT_ENTER)
			{
				mpMod->EpModCommitUnlessAnimating (t);
				theHold.Accept(GetString(IDS_ASSIGN_MATID));
			}
			else theHold.Cancel();
			mpMod->EpModRefreshScreen ();
			mSpinningMaterial = false;
			break;
		}
		break;

	case CC_SPINNER_CHANGE:
		spin = (ISpinnerControl*)lParam;
		switch (LOWORD(wParam)) {
		case IDC_MAT_IDSPIN:
			if (!mSpinningMaterial)
			{
				theHold.Begin();
				mpMod->EpModSetOperation (ep_op_set_material);
				mSpinningMaterial = true;
			}
			mpMod->getParamBlock()->SetValue (epm_material, t, spin->GetIVal()-1);
			break;
		}
		break;

	case WM_COMMAND:
		if (HIWORD(wParam) == 1) return FALSE;	// not handling keyboard shortcuts here.
		buttonID = LOWORD(wParam);

		switch (buttonID) {
		case IDC_SELECT_BYID:
			int selBy;
			spin = GetISpinner (GetDlgItem (hWnd, IDC_MAT_IDSPIN_SEL));
			if (!spin) break;
			selBy = spin->GetIVal()-1;
			ReleaseISpinner (spin);
			mpMod->getParamBlock()->SetValue (epm_material_selby, t, selBy);
			mpMod->EpModButtonOp (ep_op_select_by_material);
			break;

		case IDC_MTLID_NAMES_COMBO:
			switch(HIWORD(wParam)){
			case CBN_SELENDOK:
				mMaterialUIHandler.SelectByName (hWnd, mpMod);
				break;                                                    
			}
			break;

		}
		break;
	case WM_CLOSE:
		{
			if (hWnd == mpMod->MatIDFloaterHWND())
			{
				EndDialog(hWnd,1);
				mpMod->CloseMatIDFloater();
				return TRUE;
			}
			return FALSE;
			break;
		}
	case WM_DESTROY:
		RemoveTimeChangeInvalidate();
		return FALSE;

	default:
		return FALSE;
	}

	return TRUE;
}



INT_PTR FaceSmoothDlgProc::DlgProc (TimeValue t, IParamMap2 *map, HWND hWnd,
							  UINT msg, WPARAM wParam, LPARAM lParam) {
	int buttonID;

	switch (msg) {
	case WM_INITDIALOG:
		mSmoothingGroupUIHandler.InitializeButtons (hWnd);

		// Cue an update based on the current face selection.
		uiValid.SetEmpty();
		klugeToFixWM_CUSTEDIT_ENTEROnEnterFaceLevel = true;
		mSpinningMaterial = false;
		mTimeChangeHandle = NULL;
		break;

	case WM_PAINT:
		if (uiValid.InInterval (t)) return FALSE;

		uiValid = FOREVER;
		// Display the correct smoothing groups for the current selection:
		mSmoothingGroupUIHandler.UpdateSmoothButtons (hWnd, mpMod);
		if (!mSmoothingGroupUIHandler.UIValid()) uiValid = NEVER;
		if (!(uiValid == FOREVER) && !(uiValid == NEVER)) AddTimeChangeInvalidate (hWnd);
		else RemoveTimeChangeInvalidate ();

		klugeToFixWM_CUSTEDIT_ENTEROnEnterFaceLevel = false;
		return FALSE;


	case WM_COMMAND:
		if (HIWORD(wParam) == 1) return FALSE;	// not handling keyboard shortcuts here.
		buttonID = LOWORD(wParam);

		if (buttonID>=IDC_SMOOTH_GRP1 && buttonID<=IDC_SMOOTH_GRP32) 
		{
			mSmoothingGroupUIHandler.SetSmoothing (hWnd, buttonID, mpMod);
			if (hWnd == mpMod->SmGrpFloaterHWND())
			{
				mpMod->GetModifier()->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
				GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
			}			
			break;
		}

		switch (buttonID) {

		case IDC_SELECTBYSMOOTH:
			if (mSmoothingGroupUIHandler.GetSelectBySmoothParams (mpMod))
				mpMod->EpModButtonOp (ep_op_select_by_smooth);
			break;

		case IDC_SMOOTH_CLEAR:
			mSmoothingGroupUIHandler.ClearAll (mpMod);
			break;

		case IDC_SMOOTH_AUTO:
			mpMod->EpModButtonOp (ep_op_autosmooth);
			break;
		}
		break;

	case WM_CLOSE:
		{
			if (hWnd == mpMod->SmGrpFloaterHWND())
			{
				EndDialog(hWnd,1);
				mpMod->CloseSmGrpFloater(); 
				return TRUE;
				
			}
			return FALSE;
			break;
		}
	case WM_DESTROY:
		RemoveTimeChangeInvalidate();
		return FALSE;
		break;

	default:
		return FALSE;
	}

	return TRUE;
}

void EditPolyMod::InvalidateSurfaceUI() {	

	
	

	if (mpDialogSurface)
	{
		theSurfaceDlgProc.Invalidate ();
		HWND hWnd = mpDialogSurface->GetHWnd();
		if (hWnd) InvalidateRect (hWnd, NULL, FALSE);
	}

	if (mpDialogSurfaceFloater)
	{
		theSurfaceDlgProcFloater.Invalidate ();
		HWND hWnd = mpDialogSurfaceFloater->GetHWnd();
		if (hWnd) InvalidateRect (hWnd, NULL, FALSE);
	}

}


void EditPolyMod::InvalidateFaceSmoothUI() {
	if (mpDialogFaceSmoothFloater)
	{
		theFaceSmoothDlgProcFloater.Invalidate ();
		HWND hWnd = mpDialogFaceSmoothFloater->GetHWnd();
		if (hWnd) InvalidateRect (hWnd, NULL, FALSE);

	}
	if (!mpDialogFaceSmooth) return;
	theFaceSmoothDlgProc.Invalidate ();
	HWND hWnd = mpDialogFaceSmooth->GetHWnd();
	if (hWnd) InvalidateRect (hWnd, NULL, FALSE);
}