/******************************************************************************
DESCRIPTION:  
	Implementation of UI classes that are reaction controller-related:
	ReactionDlg, StateListItem, ReactionListItem

CREATED BY: 
	Adam Felt		

HISTORY: 

Copyright (c) 1998-2006, All Rights Reserved.
*******************************************************************************/
#include "reactionMgr.h"
#include "restoreObjs.h"
#include "maxicon.h"
#include <commctrl.h>

#include "3dsmaxport.h"

//////////////////////////////////////////////////////////////
//************************************************************

static INT_PTR CALLBACK ReactionDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static LRESULT CALLBACK SplitterControlProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

const int ReactionDlg::kMaxTextLength = 255;
const int ReactionDlg::kNumReactionColumns = 4;
const int ReactionDlg::kNumStateColumns = 5;

HIMAGELIST ReactionDlg::mIcons = NULL;

ReactionDlg* theReactionDlg = NULL;

ReactionDlg* GetReactionDlg()
{
	return theReactionDlg;
}

void SetReactionDlg(ReactionDlg* dlg)
{
	theReactionDlg = dlg;
}

Reactor* FindReactor(ICurve* curve)
{
	MyEnumProc dep(curve);
	curve->DoEnumDependents(&dep);

	for(int x=0; x<dep.anims.Count(); x++)
	{
		Animatable* anim = dep.anims[x];
		DbgAssert(anim != NULL);
		if (anim->SuperClassID() == REF_MAKER_CLASS_ID && anim->ClassID() == CURVE_CONTROL_CLASS_ID)
		{
			ReferenceTarget*cc = (ReferenceTarget*)anim;
			MyEnumProc dep2(cc);
			cc->DoEnumDependents(&dep2);

			for(int i=0; i<dep2.anims.Count(); i++)
			{
				Reactor* r = (Reactor*)GetIReactorInterface(dep2.anims[i]);
				if (r != NULL)
					return r;
			}
		}
	}
	return NULL;
}
ReactionListItem* GetReactionListItem(HWND hList, int row, int col)
{
	LVITEM item;
	item.mask = LVIF_PARAM;
	item.iItem = row;
	item.iSubItem = col;
	ListView_GetItem(hList, &item);

	return (ReactionListItem*)item.lParam;
}

StateListItem* GetStateListItem(HWND hList, int row, int col)
{
	LVITEM item;
	item.mask = LVIF_PARAM;
	item.iItem = row;
	item.iSubItem = col;
	ListView_GetItem(hList, &item);

	return (StateListItem*)item.lParam;
}

ReactionDlg::ReactionDlg(IObjParam *ip, HWND hParent)
{
	theReactionDlg = this;
	this->ip = ip;
	iCCtrl = NULL;
	valid = false;
	selectionValid = stateSelectionValid = reactionListValid = false;
	origPt = NULL;
	rListPos = sListPos = 1.0f/3.0f;
	blockInvalidation = false;
	mCurPickMode = NULL;
	reactionManager = NULL;
	
	selectedNodes.SetCount(0, TRUE);

	theHold.Suspend();
	ReplaceReference(0,GetReactionManager());
	theHold.Resume();

	mIcons = ImageList_Create(16, 16, ILC_COLOR24 | ILC_MASK, 14, 0);
	TSTR IconFile = TSTR(_T("ReactionManager"));

	hWnd = CreateDialogParam(
		hInstance,
		MAKEINTRESOURCE(IDD_REACTION_DIALOG),
		hParent,
		(DLGPROC)ReactionDlgProc,
		(LPARAM)this);	
	TSTR title = TSTR(GetString(IDS_REACTIONMGR_TITLE));
	SetWindowText(hWnd,title);
	ip->RegisterDlgWnd(hWnd);
	ip->RegisterTimeChangeCallback(this);

	LoadMAXFileIcon(IconFile, mIcons, kWindow, FALSE);
	SendMessage(hWnd, WM_SETICON, ICON_SMALL, DLGetClassLongPtr<LPARAM>(GetCOREInterface()->GetMAXHWnd(), GCLP_HICONSM));

	reactionDlgActionCB = new ReactionDlgActionCB<ReactionDlg>(this);
	ip->GetActionManager()->ActivateActionTable(reactionDlgActionCB, kReactionMgrActions);

	RegisterNotification(SelectionSetChanged, this, NOTIFY_SELECTIONSET_CHANGED);
	RegisterNotification(PreSceneNodeDeleted, this, NOTIFY_SCENE_PRE_DELETED_NODE);
	RegisterNotification(PreReset, this, NOTIFY_SYSTEM_PRE_RESET);
	RegisterNotification(PreOpen, this, NOTIFY_FILE_PRE_OPEN);
	RegisterNotification(PreOpen, this, NOTIFY_SYSTEM_PRE_NEW);
	RegisterNotification(PostOpen, this, NOTIFY_FILE_POST_OPEN);
	RegisterNotification(PostOpen, this, NOTIFY_SYSTEM_POST_NEW);
}

void ReactionDlg::PreSceneNodeDeleted(void *param, NotifyInfo *info)
{
	ReactionDlg* dlg = (ReactionDlg*)param;
	INode* node = (INode*)info->callParam;
	for(int i = dlg->selectedNodes.Count()-1; i >=0; i--)
	{
		if (dlg->selectedNodes[i] == node)
		{
			dlg->selectedNodes.Delete(i, 1);
			break;
		}
	}
}

void ReactionDlg::SelectionSetChanged(void *param, NotifyInfo *info)
{
	ReactionDlg* dlg = (ReactionDlg*)param;
	ICustButton* showBut = GetICustButton(GetDlgItem(dlg->hWnd, IDC_SHOW_SELECTED));
	if (showBut->IsChecked())
	{
		ICustButton* but = GetICustButton(GetDlgItem(dlg->hWnd, IDC_REFRESH));
		but->Enable();
		ReleaseICustButton(but);
	}
	ReleaseICustButton(showBut);
}

void ReactionDlg::PreReset(void *param, NotifyInfo *info)
{
	ReactionDlg* dlg = (ReactionDlg*)param;
	dlg->blockInvalidation = true;
	DestroyWindow(dlg->hWnd);
}

void ReactionDlg::PreOpen(void *param, NotifyInfo *info)
{
	DbgAssert(info);

	// Do not empty the reaction manager if we're just loading a render preset file
	if (info != NULL && info->intcode == NOTIFY_FILE_PRE_OPEN && info->callParam) {
		FileIOType type = *(static_cast<FileIOType*>(info->callParam));
		if (type == IOTYPE_RENDER_PRESETS)
			return;
	}

	ReactionDlg* dlg = (ReactionDlg*)param;
	dlg->blockInvalidation = true;
	dlg->InvalidateReactionList();
	ListView_DeleteAllItems(GetDlgItem(dlg->hWnd, IDC_STATE_LIST));
	ListView_DeleteAllItems(GetDlgItem(dlg->hWnd, IDC_REACTION_LIST));
	if (dlg->iCCtrl != NULL)
	{
		theHold.Suspend();
		dlg->ClearCurves();
		theHold.Resume();
	}
	//ListView_SetItemState(GetDlgItem(dlg->hWnd, IDC_REACTION_LIST), -1, 0, LVIS_SELECTED);
}

void ReactionDlg::PostOpen(void *param, NotifyInfo *info)
{
	ReactionDlg* dlg = (ReactionDlg*)param;
	dlg->blockInvalidation = false;
	dlg->Invalidate();
}

ReactionDlg::~ReactionDlg()
	{
	ICustButton* pickBut = GetICustButton(GetDlgItem(hWnd, IDC_ADD_MASTER));
	ICustButton* pickBut2 = GetICustButton(GetDlgItem(hWnd, IDC_ADD_SLAVES));
	if (pickBut->IsChecked() || pickBut2->IsChecked())
		GetCOREInterface()->ClearPickMode();
	ReleaseICustButton(pickBut);
	ReleaseICustButton(pickBut2);
		
	if (mCurPickMode != NULL)
		delete mCurPickMode;

	theReactionDlg = NULL;
	UnRegisterNotification(SelectionSetChanged, this, NOTIFY_SELECTIONSET_CHANGED);
	UnRegisterNotification(PreSceneNodeDeleted, this, NOTIFY_SCENE_PRE_DELETED_NODE);
	UnRegisterNotification(PreReset, this, NOTIFY_SYSTEM_PRE_RESET);
	UnRegisterNotification(PreOpen, this, NOTIFY_FILE_PRE_OPEN);
	UnRegisterNotification(PreOpen, this, NOTIFY_SYSTEM_PRE_NEW);
	UnRegisterNotification(PostOpen, this, NOTIFY_FILE_POST_OPEN);
	UnRegisterNotification(PostOpen, this, NOTIFY_SYSTEM_POST_NEW);

	ip->GetActionManager()->DeactivateActionTable(reactionDlgActionCB, kReactionMgrActions);
	delete reactionDlgActionCB;
	ReleaseICustEdit(floatWindow);

	ip->UnRegisterDlgWnd(hWnd);
	ip->UnRegisterTimeChangeCallback(this);

	ip = NULL;
	theHold.Suspend();
	DeleteAllRefsFromMe();
	if (iCCtrl)
	{
		ClearCurves();
		iCCtrl->MaybeAutoDelete();
	}
	theHold.Resume();

	ImageList_Destroy(mIcons);

	}

void ReactionDlg::Invalidate()
	{
	if (!blockInvalidation)
		{
		valid = FALSE;
		InvalidateRect(hWnd,NULL,FALSE);
		}
	}

void ReactionDlg::SetupUI(HWND hWnd)
{
	this->hWnd = hWnd;
	theHold.Suspend();
	BuildCurveCtrl();
	SetupReactionList();
	SetupStateList();
	theHold.Resume();
	valid = FALSE;

	FillButton(hWnd, IDC_ADD_MASTER, 0, 11, GetString(IDS_ADD_MASTER), CBT_CHECK);
	FillButton(hWnd, IDC_ADD_SLAVES, 1, 12, GetString(IDS_ADD_SLAVE), CBT_CHECK);
	FillButton(hWnd, IDC_ADD_SELECTED, 2, 13,  GetString(IDS_ADD_SELECTED), CBT_PUSH);
	FillButton(hWnd, IDC_DELETE_SELECTED, 3, 14, GetString(IDS_DELETE_SELECTED), CBT_PUSH);
	FillButton(hWnd, IDC_PREFERENCES, 4, 15, GetString(IDS_PREFERENCES), CBT_PUSH);
	FillButton(hWnd, IDC_REFRESH, 9, 20, GetString(IDS_REFRESH), CBT_PUSH);

	FillButton(hWnd, IDC_SET_STATE, 5, 16, GetString(IDS_SET_STATE), CBT_PUSH);
	FillButton(hWnd, IDC_NEW_STATE, 6, 17, GetString(IDS_NEW_STATE), CBT_PUSH);
	FillButton(hWnd, IDC_APPEND_STATE, 10, 21, GetString(IDS_APPEND_STATE), CBT_PUSH);
	FillButton(hWnd, IDC_DELETE_STATE, 3, 14, GetString(IDS_DELETE_STATE), CBT_PUSH);

	ICustButton* but = GetICustButton(GetDlgItem(hWnd, IDC_CREATE_STATES));
	but->SetType(CBT_CHECK);
	ReleaseICustButton(but);

	but = GetICustButton(GetDlgItem(hWnd, IDC_EDIT_STATE));
	but->SetType(CBT_CHECK);
	ReleaseICustButton(but);

	but = GetICustButton(GetDlgItem(hWnd, IDC_SHOW_SELECTED));
	but->SetType(CBT_CHECK);
	ReleaseICustButton(but);

	but = GetICustButton(GetDlgItem(hWnd, IDC_REFRESH));
	but->Disable();
	ReleaseICustButton(but);

	but = GetICustButton(GetDlgItem(hWnd, IDC_SPLITTER1));
	but->Disable();
	ReleaseICustButton(but);

	but = GetICustButton(GetDlgItem(hWnd, IDC_SPLITTER2));
	but->Disable();
	ReleaseICustButton(but);

	ShowWindow(GetDlgItem(hWnd, IDC_PREFERENCES), SW_HIDE);

	floatWindow = GetICustEdit(GetDlgItem(hWnd, IDC_EDIT_FLOAT));
	floatWindow->WantReturn(TRUE);
	ShowWindow(floatWindow->GetHwnd(), SW_HIDE);

	PerformLayout();
}

void ReactionDlg::BuildCurveCtrl()
{
	theHold.Suspend();
	iCCtrl = (ICurveCtl*)CreateInstance(REF_MAKER_CLASS_ID,CURVE_CONTROL_CLASS_ID);
	theHold.Resume();

	if(!iCCtrl)
		return;

	theHold.Suspend();
	iCCtrl->SetNumCurves(0, FALSE);
	iCCtrl->SetCCFlags(CC_SINGLESELECT|CC_DRAWBG|CC_DRAWGRID|CC_DRAWUTOOLBAR|CC_SHOWRESET|CC_DRAWLTOOLBAR|CC_DRAWSCROLLBARS|CC_AUTOSCROLL|
						CC_DRAWRULER|CC_RCMENU_MOVE_XY|CC_RCMENU_MOVE_X|CC_RCMENU_MOVE_Y|CC_RCMENU_SCALE|
						CC_RCMENU_INSERT_CORNER|CC_RCMENU_INSERT_BEZIER|CC_RCMENU_DELETE|CC_SHOW_CURRENTXVAL);
	iCCtrl->SetXRange(0.0f,0.01f);
	iCCtrl->SetYRange(-100.0f,100.0f);
	iCCtrl->RegisterResourceMaker(this);
	iCCtrl->ZoomExtents();
	iCCtrl->SetCustomParentWnd(hWnd);
	iCCtrl->SetMessageSink(hWnd);

	theHold.Resume();
	iCCtrl->SetActive(TRUE);
}

void ReactionDlg::FillButton(HWND hWnd, int butID, int iconIndex, int disabledIconIndex, TSTR toolText, CustButType type)
{
	ICustButton *iBut = GetICustButton(GetDlgItem(hWnd, butID));
	iBut->SetImage(mIcons, iconIndex, iconIndex, disabledIconIndex, disabledIconIndex, 16, 16);
	iBut->SetType(type);
	iBut->Execute(I_EXEC_CB_NO_BORDER);
	iBut->SetTooltip(TRUE, toolText);
	ReleaseICustButton(iBut);
}

void ReactionDlg::PerformLayout()
{
	const int buttonWidth = 24;
	const int bigButtonWidth = buttonWidth*4;
	const int controlSpacing = 2;
	const int buttonSpacing = buttonWidth+controlSpacing;
	const int bigButtonSpacing = bigButtonWidth+controlSpacing;
	const int buttonHeight = 24;
	const int borderWidth = 6;

	int nextX = borderWidth;
	int nextY = borderWidth;
	SetWindowPos(GetDlgItem(hWnd, IDC_ADD_MASTER), NULL, nextX, nextY, buttonWidth, buttonHeight, SWP_NOZORDER); nextX += buttonSpacing;
	SetWindowPos(GetDlgItem(hWnd, IDC_ADD_SLAVES), NULL, nextX, nextY, buttonWidth, buttonHeight, SWP_NOZORDER); nextX += buttonSpacing;
	SetWindowPos(GetDlgItem(hWnd, IDC_ADD_SELECTED), NULL, nextX, nextY, buttonWidth, buttonHeight, SWP_NOZORDER); nextX += buttonSpacing;
	SetWindowPos(GetDlgItem(hWnd, IDC_DELETE_SELECTED), NULL, nextX, nextY, buttonWidth, buttonHeight, SWP_NOZORDER); nextX += buttonSpacing;
	SetWindowPos(GetDlgItem(hWnd, IDC_PREFERENCES), NULL, nextX, nextY, buttonWidth, buttonHeight, SWP_NOZORDER); nextX += buttonSpacing;
	SetWindowPos(GetDlgItem(hWnd, IDC_SHOW_SELECTED), NULL, nextX, nextY, bigButtonWidth, buttonHeight, SWP_NOZORDER); nextX += bigButtonSpacing;
	SetWindowPos(GetDlgItem(hWnd, IDC_REFRESH), NULL, nextX, nextY, buttonWidth, buttonHeight, SWP_NOZORDER);
	
	nextX = borderWidth;
	nextY += buttonHeight + borderWidth;

	RECT rect;
	GetClientRect(hWnd, &rect);
	int height = ((rect.bottom-(4*borderWidth)) * rListPos) - (buttonHeight + borderWidth);
	int width = rect.right - 2*borderWidth;

	SetWindowPos(GetDlgItem(hWnd, IDC_REACTION_LIST), NULL, nextX, nextY, width, height, SWP_NOZORDER);
	nextY += height;

	nextX = borderWidth;
	SetWindowPos(GetDlgItem(hWnd, IDC_SPLITTER1), NULL, nextX, nextY, width, borderWidth-2, SWP_NOZORDER);
	nextY += borderWidth;

	SetWindowPos(GetDlgItem(hWnd, IDC_CREATE_STATES), NULL, nextX, nextY, bigButtonWidth, buttonHeight, SWP_NOZORDER); nextX += bigButtonSpacing;
	SetWindowPos(GetDlgItem(hWnd, IDC_NEW_STATE), NULL, nextX, nextY, buttonWidth, buttonHeight, SWP_NOZORDER); nextX += buttonSpacing;
	SetWindowPos(GetDlgItem(hWnd, IDC_APPEND_STATE), NULL, nextX, nextY, buttonWidth, buttonHeight, SWP_NOZORDER); nextX += buttonSpacing;
	SetWindowPos(GetDlgItem(hWnd, IDC_EDIT_STATE), NULL, nextX, nextY, bigButtonWidth, buttonHeight, SWP_NOZORDER); nextX += bigButtonSpacing;
	SetWindowPos(GetDlgItem(hWnd, IDC_SET_STATE), NULL, nextX, nextY, buttonWidth, buttonHeight, SWP_NOZORDER); nextX += buttonSpacing;
	SetWindowPos(GetDlgItem(hWnd, IDC_DELETE_STATE), NULL, nextX, nextY, buttonWidth, buttonHeight, SWP_NOZORDER);
	nextY += buttonHeight + borderWidth;
	
	nextX = borderWidth;
	height = ((rect.bottom-(4*borderWidth)) * sListPos) - (buttonHeight + borderWidth);
	SetWindowPos(GetDlgItem(hWnd, IDC_STATE_LIST), NULL, nextX, nextY, width, height, SWP_NOZORDER);

	nextY += height;
	nextX = borderWidth;
	SetWindowPos(GetDlgItem(hWnd, IDC_SPLITTER2), NULL, nextX, nextY, width, borderWidth-2, SWP_NOZORDER);
	
	nextY += borderWidth;
	if (iCCtrl != NULL)
		SetWindowPos(iCCtrl->GetHWND(), NULL, nextX, nextY, width, rect.bottom - borderWidth - nextY, SWP_NOZORDER);
}

void ReactionDlg::SetupReactionList()
{
	HWND hList = GetDlgItem(hWnd, IDC_REACTION_LIST);
	int sel = ListView_GetNextItem(hList, -1, LVNI_SELECTED);
	
	ListView_SetExtendedListViewStyleEx(hList, LVS_EX_GRIDLINES, LVS_EX_GRIDLINES);

	ListView_SetImageList(hList, mIcons, LVSIL_STATE);

	LV_COLUMN column;
	column.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;

	column.fmt = LVCFMT_LEFT;
	column.pszText = GetString(IDS_AF_REACTIONS);
	column.cx = 340;
	ListView_InsertColumn(hList, kNameCol, &column);

	column.pszText = GetString(IDS_AF_FROM);
	column.cx = 60;
	ListView_InsertColumn(hList, kFromCol, &column);

	column.pszText = GetString(IDS_AF_TO);
	column.cx = 60;
	ListView_InsertColumn(hList, kToCol, &column);

	column.pszText = GetString(IDS_AF_CURVE);
	column.cx = 60;
	ListView_InsertColumn(hList, kCurveCol, &column);

	UpdateReactionList();
}

void ReactionDlg::SetupStateList()
{
	HWND hList = GetDlgItem(hWnd, IDC_STATE_LIST);
	int sel = ListView_GetNextItem(hList, -1, LVNI_SELECTED);
	
	ListView_SetExtendedListViewStyleEx(hList, LVS_EX_GRIDLINES, LVS_EX_GRIDLINES);
	ListView_SetImageList(hList, mIcons, LVSIL_STATE);

	LV_COLUMN column;
	column.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;

	column.fmt = LVCFMT_LEFT;
	column.pszText = GetString(IDS_AF_STATES); 
	column.cx = 280;
	ListView_InsertColumn(hList, kNameCol, &column);

	column.pszText = GetString(IDS_AF_VALUE); 
	column.cx = 60;
	ListView_InsertColumn(hList, kValueCol, &column);

	column.pszText = GetString(IDS_AF_STRENGTH); 
	column.cx = 60;
	ListView_InsertColumn(hList, kStrengthCol, &column);

	column.pszText =  GetString(IDS_AF_INFLUENCE);
	column.cx = 60;
	ListView_InsertColumn(hList, kInfluenceCol, &column);

	column.pszText =  GetString(IDS_AF_FALLOFF);
	column.cx = 60;
	ListView_InsertColumn(hList, kFalloffCol, &column);
	
	UpdateStateList();
}

bool ReactionDlg::AddSlaveRow(HWND hList, ReactionListItem* listItem, int itemID)
{
	TSTR buf = listItem->GetName();
	if (buf.Length() > 0)
	{
		LVITEM slaveItem;
		slaveItem.mask = LVIF_TEXT|LVIF_PARAM; //|LVIF_INDENT; (doesn't work...)
		slaveItem.iItem = itemID;
		slaveItem.iSubItem = 0;
		//slaveItem.iIndent = 2;
		TSTR buf2 = _T("");
		buf2.printf("     %s", buf);
		slaveItem.pszText = buf2;
		slaveItem.cchTextMax = static_cast<int>(_tcslen(buf2));
		slaveItem.lParam = (LPARAM)listItem;

		itemID = ListView_InsertItem(hList, &slaveItem)+1;
		for (int col = 1; col < kNumReactionColumns; col++)
		{
 			LV_ITEM lvitem;
			lvitem.mask = LVIF_PARAM;
			lvitem.iItem = itemID;
			lvitem.iSubItem = col;
			lvitem.lParam = (LPARAM)listItem;
			ListView_SetItem(hList, &lvitem);
		}
		return true;
	}
	return false;
}

bool ReactionDlg::AddSlaveStateRow(HWND hList, StateListItem* listItem, int itemID)
{
	TSTR buf = listItem->GetName();
	if (buf.Length() > 0)
	{
		LVITEM slaveItem;
		slaveItem.mask = LVIF_TEXT|LVIF_PARAM; // |LVIF_INDENT; (doesn't work...)
		slaveItem.iItem = itemID;
		slaveItem.iSubItem = 0;
		//slaveItem.iIndent = 2;
		TSTR buf2 = _T("");
		buf2.printf("     %s", buf);
		slaveItem.pszText = buf2;
		slaveItem.cchTextMax = static_cast<int>(_tcslen(buf2));
		slaveItem.lParam = (LPARAM)listItem;

		itemID = ListView_InsertItem(hList, &slaveItem)+1;
		for (int col = 1; col < kNumStateColumns; col++)
		{
 			LV_ITEM lvitem;
			lvitem.mask = LVIF_PARAM;
			lvitem.iItem = itemID;
			lvitem.iSubItem = col;
			lvitem.lParam = (LPARAM)listItem;
			ListView_SetItem(hList, &lvitem);
		}
		return true;
	}
	return false;
}

class GatherSelectedReactionComponents : public AnimEnum
{
	Tab<Reactor*> reactors;
	Tab<ReactionSet*> masters;
	BitArray selMasters;
	bool unselectedCleared;
public:
	GatherSelectedReactionComponents(Tab<ReactionSet*> m)
	{
		masters = m;
		reactors.SetCount(0);
		unselectedCleared = false;
		selMasters.SetSize(masters.Count());
		selMasters.ClearAll();
		SetScope(SCOPE_ALL);
	}

	int proc(Animatable *anim, Animatable *client, int subNum)
	{
		if (anim->GetInterface(REACTOR_INTERFACE))
		{
			Reactor* r = (Reactor*)anim;
			reactors.Append(1, &r);
		}
		for(int i = 0; i < masters.Count(); i++)
		{
			if (anim == masters[i]->GetReactionMaster()->GetMaster())
			{
				selMasters.Set(i, TRUE);
				break;
			}
		}
		return ANIM_ENUM_PROCEED;
	}

	Tab<Reactor*> SelectedReactors() { return reactors; }
	Tab<ReactionSet*> GetSelectedMasters()
	{
		if (!unselectedCleared)
		{
			for(int i = masters.Count()-1; i >= 0; i--)
			{
				if (!selMasters[i])
					masters.Delete(i, 1);
			}
			unselectedCleared = true;
			selMasters.SetSize(masters.Count());
			selMasters.SetAll();
		}
		return masters;
	}
	
};

void ReactionDlg::UpdateReactionList()
{
	selectionValid = false;

	ICustButton* but = GetICustButton(GetDlgItem(hWnd, IDC_SHOW_SELECTED));
	BOOL showSelected = but->IsChecked();
	ReleaseICustButton(but);

	HWND hList = GetDlgItem(hWnd, IDC_REACTION_LIST);

	Tab<Reactor*> reactors;
	Tab<ReactionSet*> masters;
	reactors.SetCount(0);
	masters.SetCount(0);

	if (showSelected)
	{
		// collect all non-null masters.
		for (int i=0; i<GetReactionManager()->ReactionCount(); i++) 
		{
			ReactionSet* set = GetReactionManager()->GetReactionSet(i);
			ReactionMaster* master = set->GetReactionMaster();
			if (master != NULL)
			{
				ReferenceTarget* owner = master->Owner();
				if (owner != NULL)
					masters.Append(1, &set);
			}
		}
		if (masters.Count() == 0)
		{
			int itemCt = ListView_GetItemCount(hList);	
			for (int x = itemCt-1; x >= 0; x--)
				ListView_DeleteItem(hList, x);

			reactionListValid = true;
			SelectionChanged();
			return;
		}

		// clear flags then enumerate nodes
		GatherSelectedReactionComponents* ae = new GatherSelectedReactionComponents(masters);
		for(int x = 0; x < selectedNodes.Count(); x++)
		{
            selectedNodes[x]->EnumAnimTree((AnimEnum*)ae, NULL, 0);
		}
		reactors = ae->SelectedReactors();
		masters = ae->GetSelectedMasters();
		delete ae;
	}

	int itemID = 0;
	for (int i=0; i<GetReactionManager()->ReactionCount(); i++) {
		ReactionSet* set = GetReactionManager()->GetReactionSet(i);	
		if (set == NULL)
			continue;
		
		bool gotMatch = false;
		if (showSelected)
		{
			int x;
			for(x = 0; x < masters.Count(); x++)
			{
				if (set == masters[x])
				{
					gotMatch = true;
					break;
				}
			}
			if (!gotMatch)
			{
				bool gotIt = false;
				for(x = 0;x < set->SlaveCount(); x++)
				{
					for(int y = 0; y < reactors.Count(); y++)
					{
						if (set->Slave(x) == reactors[y])
						{
							gotIt = true;
							break;
						}
					}
					if (gotIt)
						break;
				}
				if (!gotIt)
					continue;
			}
		}

		ReactionListItem* listItem = new ReactionListItem(ReactionListItem::kReactionSet, set, set, i);

		LV_ITEM mainItem;
		mainItem.mask = LVIF_TEXT|LVIF_PARAM|LVIF_STATE;
			
		mainItem.iItem = itemID;
		mainItem.iSubItem = 0;
		TSTR buf = listItem->GetName();
		mainItem.pszText = buf;
		mainItem.cchTextMax = static_cast<int>(_tcslen(buf));
		mainItem.stateMask = LVIS_SELECTED;
		mainItem.state = (i == GetReactionManager()->GetSelected())?LVIS_SELECTED:0;

		if (listItem->IsExpandable())
		{
			mainItem.stateMask |= LVIS_STATEIMAGEMASK;
			mainItem.state |= INDEXTOSTATEIMAGEMASK(9);
		}
		mainItem.lParam = (LPARAM)listItem;

		if (ListView_GetItemCount(hList) <= itemID)
			ListView_InsertItem(hList,&mainItem);
		else 
			ListView_SetItem(hList, &mainItem);

		LVITEM item;
		for (int i = 1; i < kNumReactionColumns; i++)
		{
			item.mask = LVIF_PARAM;
			item.iSubItem = i;
			item.lParam = (LPARAM)listItem;
			ListView_SetItem(hList, &item);
		}

		itemID++;


		for(int x = 0; x < set->SlaveCount(); x++)
		{
			Reactor* r = set->Slave(x);
			if (showSelected && !gotMatch)
			{
				bool gotIt = false;
				for(int y = 0; y < reactors.Count(); y++)
				{
					if (r == reactors[y])
					{
						gotIt = true;
						break;
					}
				}
				if (!gotIt)
					continue;
			}
			listItem = new ReactionListItem(ReactionListItem::kSlave, r, set, x);
			if (AddSlaveRow(hList, listItem, itemID))
				itemID++;
		}
	}
	int itemCt = ListView_GetItemCount(hList);	
	for (int x = itemCt; x > itemID; x--)
		ListView_DeleteItem(hList, x-1);

	reactionListValid = true;
	SelectionChanged();
}

ReactionListItem* ReactionDlg::GetSelectedMaster()
{
	HWND hList = GetDlgItem(hWnd, IDC_REACTION_LIST);
	int count = ListView_GetItemCount(hList);
	if (count)
	{
		for(int item = 0; item < count; item++)
		{
			LV_ITEM lvItem;
			lvItem.iItem = item;
			lvItem.iSubItem = 0;
			lvItem.mask = LVIF_PARAM|LVIF_STATE;
			lvItem.stateMask = LVIS_SELECTED;
			BOOL bRet = ListView_GetItem(hList, &lvItem);	
			if (lvItem.state & LVIS_SELECTED)
			{
				ReactionListItem* listItem = (ReactionListItem*)lvItem.lParam;
				if (listItem->GetReactionSet() != NULL)
					return listItem;
			}
		}
	}
	return NULL;
}

Tab<ReactionListItem*> ReactionDlg::GetSelectedSlaves()
{
	Tab<ReactionListItem*> items;
	HWND hList = GetDlgItem(hWnd, IDC_REACTION_LIST);
	int count = ListView_GetItemCount(hList);
	if (count)
	{
		for(int item = 0; item < count; item++)
		{
			LV_ITEM lvItem;
			lvItem.iItem = item;
			lvItem.iSubItem = 0;
			lvItem.mask = LVIF_PARAM|LVIF_STATE;
			lvItem.stateMask = LVIS_SELECTED;
			BOOL bRet = ListView_GetItem(hList, &lvItem);	
			if (lvItem.state & LVIS_SELECTED)
			{
				ReactionListItem* listItem = (ReactionListItem*)lvItem.lParam;
				if (listItem->Slave() != NULL)
					items.Append(1, &listItem);
			}
		}
	}
	return items;
}

Tab<StateListItem*> ReactionDlg::GetSelectedStates()
{
	Tab<StateListItem*> states;
	HWND hList = GetDlgItem(hWnd, IDC_STATE_LIST);
	int count = ListView_GetItemCount(hList);
	if (count)
	{
		LV_ITEM lvItem;
		lvItem.iSubItem = 0;
		lvItem.mask = LVIF_PARAM|LVIF_STATE;
		lvItem.stateMask = LVIS_SELECTED;

		for(int item = 0; item < count; item++)
		{
			lvItem.iItem = item;
			BOOL bRet = ListView_GetItem(hList, &lvItem);	
			if (lvItem.state & LVIS_SELECTED)
			{
				StateListItem* state = (StateListItem*)lvItem.lParam;
				states.Append(1, &state);
			}
				
		}
	}
	return states;
}

Tab<StateListItem*> ReactionDlg::GetSelectedMasterStates()
{
	Tab<StateListItem*> listItems = GetSelectedStates();
	Tab<StateListItem*> states;
	if (listItems.Count() > 0)
	{
		for(int i = 0; i < listItems.Count(); i++)
		{
			if (listItems[i]->GetMasterState() != NULL)
				states.Append(1, &listItems[i]);
		}
	}
	return states;
}

Tab<StateListItem*> ReactionDlg::GetSelectedSlaveStates()
{
	Tab<StateListItem*> listItems = GetSelectedStates();
	Tab<StateListItem*> states;
	if (listItems.Count() > 0)
	{
		for(int i = 0; i < listItems.Count(); i++)
		{
			if (listItems[i]->GetSlaveState() != NULL)
				states.Append(1, &listItems[i]);
		}
	}
	return states;
}

BOOL ReactionDlg::IsMasterStateSelected()
{
	HWND hList = GetDlgItem(hWnd, IDC_STATE_LIST);
	int count = ListView_GetItemCount(hList);
	if (count)
	{
		LV_ITEM lvItem;
		lvItem.iSubItem = 0;
		lvItem.mask = LVIF_PARAM|LVIF_STATE;
		lvItem.stateMask = LVIS_SELECTED;

		for(int item = 0; item < count; item++)
		{
			lvItem.iItem = item;
			BOOL bRet = ListView_GetItem(hList, &lvItem);	
			if (lvItem.state & LVIS_SELECTED)
			{
				StateListItem* state = (StateListItem*)lvItem.lParam;
				if (state->GetMasterState() != NULL)
					return TRUE;
			}
		}
	}
	return FALSE;
}

BOOL ReactionDlg::IsSlaveStateSelected()
{
	HWND hList = GetDlgItem(hWnd, IDC_STATE_LIST);
	int count = ListView_GetItemCount(hList);
	if (count)
	{
		LV_ITEM lvItem;
		lvItem.iSubItem = 0;
		lvItem.mask = LVIF_PARAM|LVIF_STATE;
		lvItem.stateMask = LVIS_SELECTED;

		for(int item = 0; item < count; item++)
		{
			lvItem.iItem = item;
			BOOL bRet = ListView_GetItem(hList, &lvItem);	
			if (lvItem.state & LVIS_SELECTED)
			{
				StateListItem* state = (StateListItem*)lvItem.lParam;
				if (state->GetSlaveState() != NULL)
					return TRUE;
			}
		}
	}
	return FALSE;
}


void ReactionDlg::SelectionChanged() 
{ 
	blockInvalidation = true;
	selectionValid = true;
	UpdateStateList();
	UpdateButtons();
	UpdateStateButtons();
	UpdateCurveControl();
	UpdateCreateStates();
	blockInvalidation = false;
}

void ReactionDlg::StateSelectionChanged()
{
	blockInvalidation = true;
	stateSelectionValid = true;
	UpdateStateButtons();
	UpdateEditStates();
	blockInvalidation = false;
}

void ReactionDlg::UpdateEditStates()
{
	bool changed = false;

	ICustButton* but = GetICustButton(GetDlgItem(hWnd, IDC_EDIT_STATE));
	BOOL isChecked = but->IsChecked();
	ReleaseICustButton(but);

	if (isChecked)
	{
		for(int i = 0; i < GetReactionManager()->ReactionCount(); i++)
		{
			ReactionSet* set = GetReactionManager()->GetReactionSet(i);
			for(int x = 0; x < set->SlaveCount(); x++)
			{
				Reactor* r = set->Slave(x);
				if (r) {
					r->setEditReactionMode(FALSE);
					changed = true;
				}
			}
		}
		Tab<StateListItem*> listItems = GetSelectedSlaveStates();
		if (listItems.Count() > 0)
		{
			for(int i = 0; i < listItems.Count(); i++)
			{
				Reactor* r = (Reactor*)listItems[i]->GetOwner();
				r->setEditReactionMode(TRUE);
				changed = true;
			}
		}		
		else
		{
			listItems = GetSelectedMasterStates();
			if (listItems.Count() > 0)
			{
				ReactionSet* set = (ReactionSet*)listItems[0]->GetOwner();
				for(int i = 0; i < set->SlaveCount(); i++)
				{
					Reactor* r = set->Slave(i);
					if (r) {
						for(int x = 0; x < r->slaveStates.Count(); x++)
						{
							if (r->slaveStates[x].masterID == listItems[0]->GetIndex())
							{
								blockInvalidation = true;
								r->setSelected(x);
								r->setEditReactionMode(TRUE);
								blockInvalidation = false;
								changed = true;
							}
						}
					}
				}
			}
		}
	}

	if (changed)
		ip->RedrawViews(ip->GetTime());
}

void ReactionDlg::UpdateCreateStates()
{
	bool changed = false;
	ICustButton* but = GetICustButton(GetDlgItem(hWnd, IDC_CREATE_STATES));
	BOOL isChecked = but->IsChecked();
	ReleaseICustButton(but);

	if (isChecked)
	{
		for(int i = 0; i < GetReactionManager()->ReactionCount(); i++)
		{
			ReactionSet* set = GetReactionManager()->GetReactionSet(i);
			for(int x = 0; x < set->SlaveCount(); x++)
			{
				Reactor* r = set->Slave(x);
				if (r) {
					r->setCreateReactionMode(FALSE);
					changed = true;
				}
			}
		}
		Tab<ReactionListItem*> listItems = GetSelectedSlaves();
		if (listItems.Count() > 0)
		{
			for(int i = 0; i < listItems.Count(); i++)
			{
				Reactor* r = listItems[i]->Slave();
				if (r) {
					r->setCreateReactionMode(TRUE);
					changed = true;
				}
			}
		}
		else
		{
			ReactionListItem* rListItem = GetSelectedMaster();
			if (rListItem != NULL)
			{
				ReactionSet* set = rListItem->GetReactionSet();
				for(int i = 0; i < set->SlaveCount(); i++)
				{
					Reactor* r = set->Slave(i);
					if (r) {
						r->setCreateReactionMode(TRUE);
						changed = true;
					}
				}
			}
		}
	}
	if (changed)
		ip->RedrawViews(ip->GetTime());
}

void ReactionDlg::UpdateStateButtons()
{
	Tab<StateListItem*> listItems = GetSelectedStates();
	bool masterStateSelected = false;
	bool slaveStateSelected = false;

	ICustButton* setState = GetICustButton(GetDlgItem(hWnd, IDC_SET_STATE));
	ICustButton* deleteState = GetICustButton(GetDlgItem(hWnd, IDC_DELETE_STATE));
	ICustButton* editState = GetICustButton(GetDlgItem(hWnd, IDC_EDIT_STATE));
	ICustButton* appendState = GetICustButton(GetDlgItem(hWnd, IDC_APPEND_STATE));

	if (listItems.Count() > 0)
	{
		for(int i = 0; i < listItems.Count(); i++)
		{
			if (listItems[i]->GetMasterState() != NULL)
				masterStateSelected = true;
			else
				slaveStateSelected = true;
		}
		deleteState->Enable(TRUE);
	}
	else
		deleteState->Enable(FALSE);


	if (masterStateSelected)
	{
		appendState->Enable(TRUE);
		setState->Enable(TRUE);
	}
	else
	{
		appendState->Enable(FALSE);
		setState->Enable(FALSE);
	}
	
	if (slaveStateSelected || masterStateSelected)
		editState->Enable(TRUE);
	else if (!editState->IsChecked())
		editState->Enable(FALSE);

	ReleaseICustButton(appendState);
	ReleaseICustButton(deleteState);
	ReleaseICustButton(setState);
	ReleaseICustButton(editState);
}

void ReactionDlg::UpdateStateList()
{
	HWND hList = GetDlgItem(hWnd, IDC_STATE_LIST);
	int itemID = 0;
	ReactionSet* set = NULL;
	ReactionListItem* rListItem = GetSelectedMaster();
	if (rListItem != NULL)
	{
		set = rListItem->GetReactionSet();
	}
	else
	{
		Tab<ReactionListItem*> listItems = GetSelectedSlaves();
		if (listItems.Count() > 0)
		{
			set = (ReactionSet*)listItems[0]->GetOwner();
		}
	}
	if (set != NULL)
	{
		ReactionMaster* master = set->GetReactionMaster();
		if (master != NULL)
		{
			for (int i=0; i<master->states.Count(); i++) 
			{
				MasterState* state = master->states[i];				
				StateListItem* listItem = new StateListItem(StateListItem::kMasterState, state, set, i);

				LV_ITEM item;
				if (listItem->IsExpandable())
					item.mask = LVIF_TEXT|LVIF_PARAM|LVIF_STATE;
				else
					item.mask = LVIF_TEXT|LVIF_PARAM;

				item.iItem = itemID;
				item.iSubItem = 0;
				item.pszText = state->name;
				item.cchTextMax = static_cast<int>(_tcslen(state->name));
				if (listItem->IsExpandable())
				{
					item.stateMask = LVIS_STATEIMAGEMASK;
					item.state = INDEXTOSTATEIMAGEMASK(9);
				}
				item.lParam = (LPARAM)listItem;

				if (ListView_GetItemCount(hList) <= itemID)
					ListView_InsertItem(hList,&item);
				else 
					ListView_SetItem(hList, &item);

				//LVITEM item;
				for (int k = 1; k < kNumStateColumns; k++)
				{
					item.mask = LVIF_PARAM;
					item.iSubItem = k;
					item.lParam = (LPARAM)listItem;
					ListView_SetItem(hList, &item);
				}

				itemID++;
				ReactionSet* set = (ReactionSet*)listItem->GetOwner();
				for(int k = 0; k < set->SlaveCount(); k++)
				{
					Reactor* r = set->Slave(k);
					if (r) {
						for(int x = 0; x < r->slaveStates.Count(); x++)
						{
							if (r->slaveStates[x].masterID == i )
							{
								listItem = new StateListItem(StateListItem::kSlaveState, &r->slaveStates[x], r, x);
								if (AddSlaveStateRow(hList, listItem, itemID))
								{
									itemID++;
									break;
								}
							}
						}
					}
				}
			}
		}
	}
	int itemCt = ListView_GetItemCount(hList);	
	for (int x = itemCt; x > itemID; x--)
		ListView_DeleteItem(hList, x-1);
}

void ReactionDlg::ClearCurves()
{
	for (int i = 0; i < iCCtrl->GetNumCurves(); i++ )
	{
		bool found = false;
		ICurve* curve = iCCtrl->GetControlCurve(i);
		DependentIterator di(curve);
		ReferenceMaker* maker = NULL;
		while ((maker = di.Next()) != NULL)
		{
			if (maker->SuperClassID() == REF_MAKER_CLASS_ID &&
					maker->ClassID() == CURVE_CONTROL_CLASS_ID)
			{
				ICurveCtl* cc = (ICurveCtl*)maker;
				if (cc != iCCtrl)
				{
					for (int x = 0; x < cc->GetNumCurves(); x++ )
					{
						if (cc->GetControlCurve(x) == curve)
						{
							cc->SetReference(x, curve);
							found = true;
							break;
						}
					}
				}
			}
			if (found)
				break;
		}
	}

	iCCtrl->SetNumCurves(0, FALSE);
}

void ReactionDlg::UpdateCurveControl()
{
	Tab<ReactionListItem*> slaveItems = GetSelectedSlaves();
	float min = 0.0f;
	float max = 0.0f;
	bool initialized = false;
	Tab<ICurve*> curves;
	Tab<Reactor*> reactors;

	if (slaveItems.Count() > 0)
	{
		for (int x = 0; x < slaveItems.Count(); x++)
		{
			Reactor* r = slaveItems[x]->Slave();
			if (r)
				reactors.Append(1, &r);
		}
	}
	else
	{
		ReactionListItem* rListItem = GetSelectedMaster();
		if (rListItem != NULL)
		{
			ReactionSet* set = rListItem->GetReactionSet();
			for(int x = 0; x < set->SlaveCount(); x++)
			{
				Reactor* r = set->Slave(x);
				if (r)
					reactors.Append(1, &r);
			}
		}
	}
	
	theHold.Suspend();

	for (int x = 0; x < reactors.Count(); x++)
	{
		Reactor* r = reactors[x];
		if (r && (r->iCCtrl != NULL && r->isCurveControlled))
		{
			r->UpdateCurves(false);
			int numCurves = r->iCCtrl->GetNumCurves();
			for(int i = 0; i < numCurves; i++)
			{
				ICurve* curve = r->iCCtrl->GetControlCurve(i);
				if (!initialized)
				{
					min = curve->GetPoint(0, 0).p.x;
					max = curve->GetPoint(0, curve->GetNumPts()-1).p.x;
					initialized = true;
				}
				else
				{
					min = min(curve->GetPoint(0, 0).p.x, min);
					max = max(curve->GetPoint(0, curve->GetNumPts()-1).p.x, max);
				}
				curves.Append(1, &curve);
			}
		}
	}
	
	int curveCount = curves.Count();
	ClearCurves();
	iCCtrl->SetNumCurves(curveCount);

	BitArray ba;
	ba.SetSize(curveCount);
	ba.SetAll();
	iCCtrl->SetDisplayMode(ba);	


	for(int i = 0; i < curveCount; i++)
	{
		iCCtrl->ReplaceReference(i, curves[i]);
	}

	if (min == max)
		max = min + 1.0f;
	iCCtrl->SetXRange(min, max, FALSE);
	iCCtrl->ZoomExtents();
	theHold.Resume();
}

void ReactionDlg::UpdateButtons()
{
	ICustButton* addSlave = GetICustButton(GetDlgItem(hWnd, IDC_ADD_SLAVES));
	ICustButton* addSel = GetICustButton(GetDlgItem(hWnd, IDC_ADD_SELECTED));
	ICustButton* deleteSel = GetICustButton(GetDlgItem(hWnd, IDC_DELETE_SELECTED));
	ICustButton* newState = GetICustButton(GetDlgItem(hWnd, IDC_NEW_STATE));
	ICustButton* createStates = GetICustButton(GetDlgItem(hWnd, IDC_CREATE_STATES));
	
	ReactionListItem *masterItem = GetSelectedMaster();
	if (masterItem != NULL)
	{
		if (((ReactionSet*)masterItem->GetOwner())->GetReactionMaster() != NULL)
		{
			addSlave->Enable(TRUE);
			addSel->Enable(TRUE);
			newState->Enable(TRUE);
			createStates->Enable(TRUE);
		}
		else
		{
			addSlave->Enable(FALSE);
			addSel->Enable(FALSE);
			newState->Enable(FALSE);
			createStates->Enable(FALSE);
		}

		deleteSel->Enable(TRUE);
	}
	else
	{
		addSlave->Enable(FALSE);
		addSel->Enable(FALSE);
		Tab<ReactionListItem*> slaveItems = GetSelectedSlaves();
		if (slaveItems.Count() > 0)
		{
			deleteSel->Enable(TRUE);
			bool aborted = false;
			for(int i = 0; i < slaveItems.Count(); i++)
			{
				if (slaveItems[i]->Slave() && slaveItems[i]->Slave()->GetMaster() == NULL)
				{
					createStates->Enable(FALSE);
					newState->Enable(FALSE);
					aborted = true;
					break;
				}
			}
			if (!aborted)
			{
				createStates->Enable(TRUE);
				newState->Enable(TRUE);
			}
		}
		else
		{
			deleteSel->Enable(FALSE);
			newState->Enable(FALSE);
			if (!createStates->IsChecked())
			{
				createStates->Disable();
			}
		}
	}

	ReleaseICustButton(addSlave);
	ReleaseICustButton(addSel);
	ReleaseICustButton(deleteSel);
	ReleaseICustButton(newState);
	ReleaseICustButton(createStates);
}

// provides hit testing into the grid system of the list control
POINT ReactionDlg::HitTest(POINT &p, UINT id)
{
	HWND hList = GetDlgItem(hWnd, id);

    // Check which cell was hit
    LV_HITTESTINFO hittestinfo;
    POINT ptGrid;
	ptGrid.x = -1;
	ptGrid.y = -1;
    hittestinfo.pt.x = p.x;
    hittestinfo.pt.y = p.y;
    int nRow = ListView_SubItemHitTest(hList, &hittestinfo);
    int nCol = 0;
    bool found = false;

    if ( hittestinfo.flags & LVHT_ONITEMSTATEICON)
    {
	    nCol = -1;
		nRow = hittestinfo.iItem;
        found = true;
    }
	else if ( hittestinfo.flags & LVHT_ONITEM)
	{
	    nCol = hittestinfo.iSubItem;
		nRow = hittestinfo.iItem;
		found = true;
	}

	if (found)
	{
	    ptGrid.x = nRow;
		ptGrid.y = nCol;
	}

    return ptGrid;
}

ReactionListItem* ReactionDlg::GetReactionDataAt(int where)
{
 	HWND hList = GetDlgItem(hWnd, IDC_REACTION_LIST);

	LV_ITEM lvItem;
    lvItem.iItem = where;
    lvItem.cchTextMax = kMaxTextLength;
    lvItem.iSubItem = 0;
    lvItem.mask = LVIF_PARAM;
    BOOL bRet = ListView_GetItem(hList, &lvItem);
	if(bRet)
		return (ReactionListItem *)(lvItem.lParam);
	else
		return NULL;
}

StateListItem* ReactionDlg::GetStateDataAt(int where)
{
 	HWND hList = GetDlgItem(hWnd, IDC_STATE_LIST);

	LV_ITEM lvItem;
    lvItem.iItem = where;
    lvItem.cchTextMax = kMaxTextLength;
    lvItem.iSubItem = 0;
    lvItem.mask = LVIF_PARAM;
    BOOL bRet = ListView_GetItem(hList, &lvItem);
	if(bRet)
		return (StateListItem *)(lvItem.lParam);
	else
		return NULL;
}

bool ReactionDlg::InsertListItem(ReactionListItem *pItem, int after)
{
  	HWND hList = GetDlgItem(hWnd, IDC_REACTION_LIST);

   // Get the next row number
	if (after == -1)
		after = ListView_GetItemCount(hList);

	return AddSlaveRow(hList, pItem, after);		
}

void ReactionDlg::InsertSlavesInList(ReactionSet *set, int after)
{
	ReactionListItem *pItemInfo = NULL;
	for (int i = 0; i < set->SlaveCount(); i++)
	{
		pItemInfo = new ReactionListItem(ReactionListItem::kSlave, set->Slave(i), set, i);
		if (InsertListItem(pItemInfo, after))
			after++;
	}
}

void ReactionDlg::RemoveSlavesFromList(ReactionSet *master, int after)
{
  	HWND hList = GetDlgItem(hWnd, IDC_REACTION_LIST);

	int mark = -1;
	int count = ListView_GetItemCount(hList);
    LV_ITEM lvItem;

	if (after >= count)
		return;

	lvItem.iItem = after+1;
	lvItem.cchTextMax = kMaxTextLength;
	lvItem.iSubItem = 0;
	lvItem.mask = LVIF_PARAM;
	BOOL bRet = ListView_GetItem(hList, &lvItem);
	assert(bRet);
	if(!bRet)
		return;
	ReactionListItem *pItemData = (ReactionListItem *) lvItem.lParam;
	while(pItemData->Slave() != NULL)
	{
		ListView_DeleteItem(hList, after+1);
		bRet = ListView_GetItem(hList, &lvItem);
		if(bRet)
			pItemData = (ReactionListItem *) lvItem.lParam;
		else
			break;
	}
}

bool ReactionDlg::ToggleReactionExpansionState(int nRow)
{
 	HWND hList = GetDlgItem(hWnd, IDC_REACTION_LIST);

	RECT rectItem;
	LV_ITEM lvItem;
	lvItem.iItem = nRow;
	lvItem.iSubItem = 0;
	lvItem.mask = LVIF_PARAM;
	BOOL bRet = ListView_GetItem(hList, &lvItem);
	ReactionListItem *pItemInfo = (ReactionListItem*)lvItem.lParam;
	if(!pItemInfo->IsExpandable())
		return false;
	
	ReactionSet* master = pItemInfo->GetReactionSet();

	if(master != NULL)
	{
		if(!pItemInfo->IsExpanded())
		{
			InsertSlavesInList(master, nRow+1);
			pItemInfo->Expand(true);
		}
		else
		{
			RemoveSlavesFromList(master, nRow);
			pItemInfo->Expand(false);
		}
	}

	ListView_SetItemState(hList, nRow, INDEXTOSTATEIMAGEMASK(pItemInfo->IsExpanded()?9:8), LVIS_STATEIMAGEMASK);
	ListView_GetItemRect(hList, nRow, &rectItem, LVIR_SELECTBOUNDS);
	InvalidateRect(hList, &rectItem, TRUE);
	//UpdateWindow(hList);
	return true;
}

bool ReactionDlg::InsertListItem(StateListItem *pItem, int after)
{
  	HWND hList = GetDlgItem(hWnd, IDC_STATE_LIST);

   // Get the next row number
	if (after == -1)
		after = ListView_GetItemCount(hList);

	return AddSlaveStateRow(hList, pItem, after);		
}

void ReactionDlg::InsertStatesInList(MasterState* state, ReactionSet* set, int after)
{
	StateListItem *pItemInfo = NULL;
	
	for(int k = 0; k < set->SlaveCount(); k++)
	{
		Reactor* r = set->Slave(k);
		if (r) {
			for(int x = 0; x < r->slaveStates.Count(); x++)
			{
				if (r->getMasterState(x) == state )
				{
					pItemInfo = new StateListItem(StateListItem::kSlaveState, &r->slaveStates[x], r, x);
					if (InsertListItem(pItemInfo, after))
						after++;
				}
			}
		}
	}
}

void ReactionDlg::RemoveStatesFromList(MasterState *state, int after)
{
  	HWND hList = GetDlgItem(hWnd, IDC_STATE_LIST);

	int mark = -1;
	int count = ListView_GetItemCount(hList);
    LV_ITEM lvItem;

	if (after >= count)
		return;

	lvItem.iItem = after+1;
	lvItem.cchTextMax = kMaxTextLength;
	lvItem.iSubItem = 0;
	lvItem.mask = LVIF_PARAM;
	BOOL bRet = ListView_GetItem(hList, &lvItem);
	assert(bRet);
	if(!bRet)
		return;
	StateListItem *pItemData = (StateListItem *) lvItem.lParam;
	while(pItemData->GetSlaveState() != NULL)
	{
		ListView_DeleteItem(hList, after+1);
		bRet = ListView_GetItem(hList, &lvItem);
		if(bRet)
			pItemData = (StateListItem *) lvItem.lParam;
		else
			break;
	}
}

bool ReactionDlg::ToggleStateExpansionState(int nRow)
{
 	HWND hList = GetDlgItem(hWnd, IDC_STATE_LIST);

	RECT rectItem;
	LV_ITEM lvItem;
	lvItem.iItem = nRow;
	lvItem.iSubItem = 0;
	lvItem.mask = LVIF_PARAM;
	BOOL bRet = ListView_GetItem(hList, &lvItem);
	StateListItem *pItemInfo = (StateListItem*)lvItem.lParam;
	if(!pItemInfo->IsExpandable())
		return false;
	
	MasterState* state = pItemInfo->GetMasterState();

	if(state != NULL)
	{
		if(!pItemInfo->IsExpanded())
		{
			ReactionSet* owner = (ReactionSet*)pItemInfo->GetOwner();
			InsertStatesInList(state, owner, nRow+1);
			pItemInfo->Expand(true);
		}
		else
		{
			RemoveStatesFromList(state, nRow);
			pItemInfo->Expand(false);
		}
	}

	ListView_SetItemState(hList, nRow, INDEXTOSTATEIMAGEMASK(pItemInfo->IsExpanded()?9:8), LVIS_STATEIMAGEMASK);
	ListView_GetItemRect(hList, nRow, &rectItem, LVIR_SELECTBOUNDS);
	InvalidateRect(hList, &rectItem, TRUE);
	//UpdateWindow(hList);
	return true;
}



void ReactionDlg::Update()
{
	valid = TRUE;
	//iCCtrl->Redraw();
}

bool ReactionListItem::GetRange(TimeValue &t, bool start)
{
	Interval iv;
	bool match = false;

	switch(mType)
	{
		case kReactionSet:
			{
			ReactionSet* set = GetReactionSet();
			if (set != NULL)
			{
				for(int i = 0; i < set->SlaveCount(); i++)
				{
					Reactor* reactor = set->Slave(i);
					if (reactor) {
						if (i == 0) 
						{
							match = true;
							iv = reactor->GetTimeRange(TIMERANGE_ALL);
						}
						else 
						{
							if (iv.Start() != reactor->GetTimeRange(TIMERANGE_ALL).Start()) 
							{
								match = false;
								break;
							}
						}
					}
				}
			}
			}
			break;
		case kSlave:
			{
			Reactor* reactor = Slave();
			if (reactor != NULL)
			{
				match = true;
				iv = reactor->GetTimeRange(TIMERANGE_ALL);
			}
			}
			break;
		default:
			assert(0);
	}
	if (match)
	{
		if (start)
			t = iv.Start();
		else
			t = iv.End();

		return true;
	}
	return false;

}

void ReactionListItem::SetRange(TimeValue t, bool start)
{
	Interval iv;
	bool match = false;

	switch(mType)
	{
		case kReactionSet:
			{
			//ReactionSet* set = GetReactionSet();
			//if (set != NULL)
			//{
			//	for(int i = 0; i < set->SlaveCount(); i++)
			//	{
			//		Reactor* reactor = set->Slave(i);
			//		if (i == 0) 
			//		{
			//			match = true;
			//			iv = reactor->GetTimeRange(TIMERANGE_ALL);
			//		}
			//		else 
			//		{
			//			if (iv.Start() != reactor->GetTimeRange(TIMERANGE_ALL).Start()) 
			//			{
			//				match = false;
			//				break;
			//			}
			//		}
			//	}
			//}
			}
			break;
		case kSlave:
			{
			Reactor* reactor = Slave();
			if (reactor != NULL)
			{
				iv = reactor->GetTimeRange(TIMERANGE_ALL);
				if (start)
					iv.SetStart(t);
				else
					iv.SetEnd(t);
				reactor->EditTimeRange(iv, TIMERANGE_ALL);
			}
			}
			break;
		default:
			assert(0);
	}
}

void ReactionListItem::GetValue(void* val)
{
	if (mType == kReactionSet)
	{
		ReactionMaster* master = GetReactionSet()->GetReactionMaster();
		if ( master == NULL) return;

		ReferenceTarget* owner = master->Owner();
		int subnum = master->SubIndex();
		if ( subnum < 0 )
		{
			if (subnum == IKTRACK)	
			{
				Quat q;
				GetAbsoluteControlValue((INode*)owner, subnum, GetCOREInterface()->GetTime(), &q, FOREVER);
				*(Quat*)val = q;
			}
			else {
				Point3 p;
				GetAbsoluteControlValue((INode*)owner, subnum, GetCOREInterface()->GetTime(), &p, FOREVER);
				*(Point3*)val = p;
			}
		}
		else {
			if (Control* c = GetControlInterface(owner->SubAnim(subnum))) {
				switch (master->GetType())
				{
					case FLOAT_VAR:
						{
							float f;
							c->GetValue(GetCOREInterface()->GetTime(), &f, FOREVER);
							*(float*)val = f;
						}
						break;
					case VECTOR_VAR:
						{
							Point3 p;
							c->GetValue(GetCOREInterface()->GetTime(), &p, FOREVER);
							*(Point3*)val = p;
						}
						break;
					case SCALE_VAR:
						{
							ScaleValue s;
							c->GetValue(GetCOREInterface()->GetTime(), &s, FOREVER);
							*(Point3*)val = s.s;
						}
						break;
					case QUAT_VAR:
						{
							Quat q;
							c->GetValue(GetCOREInterface()->GetTime(), &q, FOREVER);
							*(Quat*)val = q;
						}
						break;
					default: 
						val = NULL;
						break;
				}
			}
		}
	}
	else
	{
		Reactor* r = this->Slave();
		if (r) {
			switch (r->type)
			{
				case REACTORFLOAT:
					*(float*)val = r->curfval;
					break;
				case REACTORPOS:
				case REACTORP3:
				case REACTORSCALE:
					*(Point3*)val = r->curpval;
					break;
				case REACTORROT:
					*(Quat*)val = r->curqval;
					break;
				default:
					val = NULL;
					break;
			}
		}
	}
}

void RemoveSlaveFromScene(Reactor* r)
{
	if (theHold.Holding())
		theHold.Put (new DlgUpdateRestore());

	MyEnumProc dep(r);             
	((ReferenceTarget*)r)->DoEnumDependents(&dep);

	for(int x=0; x<dep.anims.Count(); x++)
	{
		for(int i=0; i<dep.anims[x]->NumSubs(); i++)
		{
			Animatable* n = dep.anims[x]->SubAnim(i);
			if (n == r)
			{
				Animatable* parent = dep.anims[x];
				int subAnim = i;

				Control* cont = NULL;
				switch (r->SuperClassID())
				{
					case CTRL_POSITION_CLASS_ID:
						cont = NewDefaultPositionController();
						break;
					case CTRL_POINT3_CLASS_ID:
						cont = NewDefaultPoint3Controller();
						break;
					case CTRL_ROTATION_CLASS_ID:
						cont = NewDefaultRotationController();
						break;
					case CTRL_SCALE_CLASS_ID:
						cont = NewDefaultScaleController();
						break;
					case CTRL_FLOAT_CLASS_ID:
						cont = NewDefaultFloatController();
						break;
				}

				if (cont != NULL)
				{
					cont->Copy(r);
					parent->AssignController(cont, subAnim);
				}
			}
		}
	}
}

struct StateIndexPair
{
	MasterState* state;
	int index;
};

void ReactionDlg::WMCommand(int id, int notify, HWND hCtrl)
	{
		switch (id) {
			case IDC_ADD_MASTER:
				{
				ICustButton* but = GetICustButton(hCtrl);
				if (mCurPickMode)
				{
					GetCOREInterface()->ClearPickMode();
					delete mCurPickMode;
					mCurPickMode = NULL;
				}
				if (but->IsChecked())
				{
					mCurPickMode = new ReactionMasterPickMode(but);
					GetCOREInterface()->SetPickMode(mCurPickMode);
					// button is released by the pick mode
				}
				}
				break;
			case IDC_REPLACE_MASTER:
				{
				if (mCurPickMode)
				{
					GetCOREInterface()->ClearPickMode();
					delete mCurPickMode;
					mCurPickMode = NULL;
				}
				ReactionListItem* listItem = GetSelectedMaster();
				if ( listItem != NULL)
				{
					ICustButton* but = GetICustButton(GetDlgItem(hWnd, IDC_ADD_MASTER));
					mCurPickMode = new ReplaceMasterPickMode(listItem->GetReactionSet(), but);
					GetCOREInterface()->SetPickMode(mCurPickMode);
					// button is released by the pick mode
				}
				}
				break;
			case IDC_ADD_SLAVES:
				{
				if (mCurPickMode)
				{
					GetCOREInterface()->ClearPickMode();
					delete mCurPickMode;
					mCurPickMode = NULL;
				}
				ICustButton* but = GetICustButton(hCtrl);
				if (but->IsChecked())
				{
					ReactionListItem* listItem = GetSelectedMaster();
					if (listItem != NULL)
					{
						Tab<int> indices;
						Tab<StateListItem*> sListItems = GetSelectedStates();
						for(int i = 0; i < sListItems.Count(); i++)
						{
							if (sListItems[i]->GetMasterState() != NULL)
							{
								int index = sListItems[i]->GetIndex();
								indices.Append(1, &index);
							}
						}
						mCurPickMode = new ReactionSlavePickMode(listItem->GetReactionSet(), indices, but);
						GetCOREInterface()->SetPickMode(mCurPickMode);
					}
					else
						but->SetCheck(FALSE);
				}
				}
				break;
			case IDC_ADD_SELECTED:
				{
				Tab<ReferenceTarget*> selectedNodes;
				for (int x = 0; x < GetCOREInterface()->GetSelNodeCount(); x++)
				{
					ReferenceTarget* targ = GetCOREInterface()->GetSelNode(x);
					selectedNodes.Append(1, &targ);
				}
				if (selectedNodes.Count() > 0)
				{
					ReactionListItem* listItem = GetSelectedMaster();
					if (listItem != NULL)
					{
						Tab<int> indices;
						Tab<StateListItem*> sListItems = GetSelectedStates();
						for(int i = 0; i < sListItems.Count(); i++)
						{
							if (sListItems[i]->GetMasterState() != NULL)
							{
								int index = sListItems[i]->GetIndex();
								indices.Append(1, &index);
							}
						}

						ReactionSlavePickMode cb = ReactionSlavePickMode(listItem->GetReactionSet(), indices, NULL);
						theAnimPopupMenu.DoPopupMenu(&cb, selectedNodes); 
					}
				}
				}
				break;
			case IDC_DELETE_SELECTED:
				{
				ReactionListItem* listItem = GetSelectedMaster();
				Tab<ReactionListItem*> slaveItems = GetSelectedSlaves();

				if (slaveItems.Count() > 0 || listItem != NULL)
				{
					theHold.Begin();

					for(int i = 0; i < slaveItems.Count(); i++)
					{
						RemoveSlaveFromScene(slaveItems[i]->Slave());
					}
					if (listItem != NULL)
					{
						ReactionSet* set = listItem->GetReactionSet();
						if(set != NULL)
						{
							for (int i = set->SlaveCount()-1; i >=0; i--)
							{
								RemoveSlaveFromScene(set->Slave(i));
							}
							GetReactionManager()->RemoveTarget(set);
						}
					}
					theHold.Accept(GetString(IDS_DELETE_SELECTED));
					InvalidateReactionList();
				}
				}
				break;
			case IDC_SHOW_SELECTED:
			case IDC_REFRESH:
				{
					ICustButton* but = GetICustButton(GetDlgItem(hWnd, IDC_SHOW_SELECTED));
					BOOL isChecked = but->IsChecked();
					ReleaseICustButton(but);

					selectedNodes.SetCount(0);
					if (isChecked)
					{
						for (int x = 0; x < GetCOREInterface()->GetSelNodeCount(); x++)
						{
							INode* targ = GetCOREInterface()->GetSelNode(x);
							selectedNodes.Append(1, &targ);
						}
					}

					InvalidateReactionList();

					but = GetICustButton(GetDlgItem(hWnd, IDC_REFRESH));
					but->Disable();
					ReleaseICustButton(but);
				}
				break;
			case IDC_NEW_STATE:
				{
					ReactionListItem* listItem = GetSelectedMaster();
					Tab<ReactionListItem*> slaves = GetSelectedSlaves();
					theHold.Begin();
					ReactionSet* set = NULL;
					if (listItem)
						set = listItem->GetReactionSet();
					else if (slaves.Count() > 0)
						set = (ReactionSet*)slaves[0]->GetOwner();

					if (set != NULL)
					{
						ReactionMaster* master = set->GetReactionMaster();
						if (master != NULL)
						{
							int index;
							MasterState* state = master->AddState(NULL, index, true, ip->GetTime());

							
							if (slaves.Count() > 0)
							{
								for(int i = 0; i < slaves.Count(); i++)
								{
									Reactor* r = (Reactor*)slaves[i]->Slave();
									if (r)
										r->CreateReactionAndReturn(TRUE, NULL, ip->GetTime(), index);
								}
							}
							else
							{
								for(int i = 0; i < set->SlaveCount(); i++)
								{
									Reactor* r = set->Slave(i);
									if (r)
										r->CreateReactionAndReturn(TRUE, NULL, ip->GetTime(), index);
								}
							}
							InvalidateSelection();
						}
					}
					theHold.Accept(GetString(IDS_NEW_STATE));
				}
				break;
 			case IDC_APPEND_STATE:
				{
					Tab<StateListItem*> sListItems = GetSelectedStates();
					Tab<StateIndexPair> states;

					for(int k = 0; k < sListItems.Count(); k++)
					{
						MasterState* state = sListItems[k]->GetMasterState();
						if (state != NULL)
						{
							StateIndexPair pair;
							pair.state = state;
							pair.index = sListItems[k]->GetIndex();
							states.Append(1, &pair);
						}
					}

					if (states.Count() > 0)
					{
						theHold.Begin();
						ReactionListItem* listItem = GetSelectedMaster();
						Tab<ReactionListItem*> slaves = GetSelectedSlaves();

						ReactionSet* set = NULL;
						if (listItem)
							set = listItem->GetReactionSet();
						else if (slaves.Count() > 0)
							set = (ReactionSet*)slaves[0]->GetOwner();

						if (set != NULL)
						{
							ReactionMaster* master = set->GetReactionMaster();
							if (master != NULL)
							{							
								if (slaves.Count() > 0)
								{
									for(int i = 0; i < slaves.Count(); i++)
									{
										Reactor* r = (Reactor*)slaves[i]->Slave();
										if (r) {
											for(int j = 0; j < states.Count(); j++)
											{
												int x;
												for(x = 0; x < r->slaveStates.Count(); x++)
												{
													if (r->getMasterState(x) == states[j].state)
													{
														break;
													}
												}
												if (x == r->slaveStates.Count())
												{
													this->selectionValid = false;
													r->CreateReactionAndReturn(TRUE, NULL, ip->GetTime(), states[j].index);
												}
											}
										}
									}
								}
								else
								{
									for(int i = 0; i < set->SlaveCount(); i++)
									{
										Reactor* r = set->Slave(i);
										if (r) {
											for(int j = 0; j < states.Count(); j++)
											{
												int x;
												for(x = 0; x < r->slaveStates.Count(); x++)
												{
													if (r->getMasterState(x) == states[j].state)
													{
														break;
													}
												}
												if (x == r->slaveStates.Count())
												{
													this->selectionValid = false;
													r->CreateReactionAndReturn(TRUE, NULL, ip->GetTime(), states[j].index);
												}
											}
										}
									}
								}
								InvalidateSelection();
							}
						}
						theHold.Accept(GetString(IDS_NEW_STATE));
					}
				}
				break;
 			case IDC_SET_STATE:
				{
					Tab<StateListItem*> sListItems = GetSelectedMasterStates();
					Tab<StateIndexPair> states;

					theHold.Begin();
					
					for(int k = 0; k < sListItems.Count(); k++)
					{
						MasterState* state = sListItems[k]->GetMasterState();
						StateIndexPair pair;
						pair.state = state;
						pair.index = sListItems[k]->GetIndex();
						states.Append(1, &pair);
					}

					if (states.Count() > 0)
					{
						ReactionListItem* listItem = GetSelectedMaster();
						if(listItem)
						{
							MasterState* state = states[0].state;
							ReactionSet* set = listItem->GetReactionSet();
							ReactionMaster* master = set->GetReactionMaster();
							if (master != NULL)
							{
								switch (master->GetType())
								{
									case FLOAT_VAR:
										{
											float f;
											listItem->GetValue(&f);
											master->SetState(states[0].index, f);
										}
										break;
									case SCALE_VAR:
									case VECTOR_VAR:
										{
											Point3 p;
											listItem->GetValue(&p);
											master->SetState(states[0].index, p);
										}
										break;
									case QUAT_VAR:
										{
											Quat q;
											listItem->GetValue(&q);
											master->SetState(states[0].index, q);
										}
										break;
								}
								master->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
							}
						}
						Tab<ReactionListItem*> slaves = GetSelectedSlaves();
						for(int j = 0; j < states.Count(); j++)
						{
							for(int i = 0; i < slaves.Count(); i++)
							{
								Reactor* r = (Reactor*)slaves[i]->Slave();
								if (r) {
									int x;
									for(x = 0; x < r->slaveStates.Count(); x++)
									{
										if (r->getMasterState(x) == states[j].state)
										{
											break;
										}
									}
									if (x == r->slaveStates.Count())
									{
										this->selectionValid = false;
										r->CreateReactionAndReturn(TRUE, NULL, ip->GetTime(), states[j].index);
									}
								}
							}
						}
					}
					theHold.Accept(GetString(IDS_SET_STATE));
				}
				break;
			case IDC_DELETE_STATE:
				{
					theHold.Begin();
					Tab<StateListItem*> sListItems = GetSelectedMasterStates();
					for(int k = sListItems.Count()-1; k >= 0; k--)
					{
						MasterState* state = sListItems[k]->GetMasterState();
						ReactionSet* set = (ReactionSet*)sListItems[k]->GetOwner();
						set->GetReactionMaster()->DeleteState(sListItems[k]->GetIndex());
					}
					
					sListItems = GetSelectedSlaveStates();
					for(int k = sListItems.Count()-1; k >= 0; k--)
					{
						int id = sListItems[k]->GetSlaveState()->masterID;
						Reactor* r = (Reactor*)sListItems[k]->GetOwner();
						for(int x = r->slaveStates.Count()-1; x >=0 ; x--)
						{
							if (r->slaveStates[x].masterID == id)
							{
								r->DeleteReaction(x);
								break;
							}
						}
					}

					theHold.Accept(GetString(IDS_DELETE_STATE));
				}
				break;
			case IDC_EDIT_STATE:
				{
				ICustButton* thisBut = GetICustButton(GetDlgItem(hWnd, IDC_EDIT_STATE));
				ICustButton* but = GetICustButton(GetDlgItem(hWnd, IDC_CREATE_STATES));
				if (thisBut->IsChecked() && but->IsChecked())
				{
					but->SetCheck(FALSE);
					WMCommand(IDC_CREATE_STATES, 0, but->GetHwnd());
				}
				ReleaseICustButton(but);
				ReleaseICustButton(thisBut);

				but = GetICustButton(GetDlgItem(hWnd, IDC_EDIT_STATE));
				Tab<StateListItem*> listItems = GetSelectedSlaveStates();
				BOOL editing = but->IsChecked();
				bool changed = false;
				if (listItems.Count() > 0)
				{
					for(int i = 0; i < listItems.Count(); i++)
					{
						Reactor* r = (Reactor*)listItems[i]->GetOwner();
						r->setEditReactionMode(editing);
						changed = true;
						if (editing && i == 0)
						{
							switch (r->type)
							{
								case REACTORPOS:
								case REACTORP3:
									ip->SetStdCommandMode(CID_OBJMOVE);
									break;
								case REACTORROT:
									ip->SetStdCommandMode(CID_OBJROTATE);
									break;
								case REACTORSCALE:
									ip->SetStdCommandMode(CID_OBJSCALE);
									break;
								default:
									break;
							}
						}
					}
				}
				else
				{
					listItems = GetSelectedMasterStates();
					if (listItems.Count() > 0)
					{
						ReactionSet* set = (ReactionSet*)listItems[0]->GetOwner();
						for(int i = 0; i < set->SlaveCount(); i++)
						{
							Reactor* r = set->Slave(i);
							if (r) {
								for(int x = 0; x < r->slaveStates.Count(); x++)
								{
									if (r->slaveStates[x].masterID == listItems[0]->GetIndex())
									{
										blockInvalidation = true;
										r->setSelected(x);
										r->setEditReactionMode(editing);
										blockInvalidation = false;
										if (editing && !changed)
										{
											switch (r->type)
											{
												case REACTORPOS:
												case REACTORP3:
													ip->SetStdCommandMode(CID_OBJMOVE);
													break;
												case REACTORROT:
													ip->SetStdCommandMode(CID_OBJROTATE);
													break;
												case REACTORSCALE:
													ip->SetStdCommandMode(CID_OBJSCALE);
													break;
												default:
													break;
											}
										}
										changed = true;
									}
								}
							}
						}
					}
				}

				if (changed)
					ip->RedrawViews(ip->GetTime());
				else if (!editing)
					but->Disable();

				ReleaseICustButton(but);
				}
				break;
			case IDC_CREATE_STATES:
				{
				ICustButton* but = GetICustButton(GetDlgItem(hWnd, IDC_CREATE_STATES));
				Tab<ReactionListItem*> slaveItems = GetSelectedSlaves();
				BOOL creating = but->IsChecked();
				if (!creating && slaveItems.Count() == 0)
					but->Disable();
				ReleaseICustButton(but);

				if (creating)
				{
					but = GetICustButton(GetDlgItem(hWnd, IDC_EDIT_STATE));
					if(but->IsChecked())
					{
						WMCommand(IDC_EDIT_STATE, 0, but->GetHwnd());
						but->SetCheck(FALSE);
					}
					ReleaseICustButton(but);
				}

				bool changed = false;
				if (slaveItems.Count() > 0)
				{
					for(int i = 0; i < slaveItems.Count(); i++)
					{
						Reactor* r = slaveItems[i]->Slave();
						if (r) {
							r->setCreateReactionMode(creating);
							if (creating && i == 0)
							{
								switch (r->type)
								{
									case REACTORPOS:
									case REACTORP3:
										ip->SetStdCommandMode(CID_OBJMOVE);
										break;
									case REACTORROT:
										ip->SetStdCommandMode(CID_OBJROTATE);
										break;
									case REACTORSCALE:
										ip->SetStdCommandMode(CID_OBJSCALE);
										break;
									default:
										break;
								}
							}
						}
					}
				}
				else
				{
					ReactionListItem* rListItem = GetSelectedMaster();
					if (rListItem != NULL)
					{
						ReactionSet* set = rListItem->GetReactionSet();
						for(int i = 0; i < set->SlaveCount(); i++)
						{
							Reactor* r = set->Slave(i);
							if (r) {
								r->setCreateReactionMode(creating);
								if (creating && i == 0)
								{
									switch (r->type)
									{
										case REACTORPOS:
										case REACTORP3:
											ip->SetStdCommandMode(CID_OBJMOVE);
											break;
										case REACTORROT:
											ip->SetStdCommandMode(CID_OBJROTATE);
											break;
										case REACTORSCALE:
											ip->SetStdCommandMode(CID_OBJSCALE);
											break;
										default:
											break;
									}
								}
							}
						}
					}
				}
				if(!creating) ip->RedrawViews(ip->GetTime());
				}
				break;
			default: break;
		}
	}

BOOL ReactionDlg::OnLMouseButtonDown(HWND hDlg, WPARAM wParam, POINT lParam, UINT id)
{
	int nRow;
	int nCol;
	POINT hit;

	POINT pt;
	pt.x = lParam.x;  // horizontal position of cursor 
	pt.y = lParam.y;  // vertical position of cursor  
	
	// Map mouse click to the controls grid
	hit = HitTest(pt, id);
	nRow = hit.x;
	nCol = hit.y;

	if (nRow == -1 ) 
		return FALSE;
	
	BOOL ret = FALSE;
	if (id == IDC_REACTION_LIST)
	{
		switch (nCol)
		{
		case kExpandCol:
			if(ToggleReactionExpansionState(nRow))
			{
				ret = TRUE;
			}
			nCol = hit.y = kNameCol;
			//fall through 
		case kNameCol:	// name
			{
			}
			break;
		case kFromCol:
			{
			ReactionListItem* listItem = GetReactionListItem(GetDlgItem(hWnd, IDC_REACTION_LIST), nRow, nCol);
			if (listItem->GetRange(origTime, true))
			{
				ret = TRUE;
			}
			}
			break;
		case kToCol:
			{
			ReactionListItem* listItem = GetReactionListItem(GetDlgItem(hWnd, IDC_REACTION_LIST), nRow, nCol);
			if (listItem->GetRange(origTime, false))
			{
				ret = TRUE;
			}
			break;
			}
		case kCurveCol:
			{
			ReactionListItem* listItem = GetReactionListItem(GetDlgItem(hWnd, IDC_REACTION_LIST), nRow, nCol);
			listItem->ToggleUseCurve();
			ip->RedrawViews(ip->GetTime());
			ret = TRUE;
			}
			break;
		default:
			ret = TRUE;
		}
	}
	else if (id == IDC_STATE_LIST)
	{
		switch (nCol)
		{
		case kExpandCol:
			if(ToggleStateExpansionState(nRow))
				{
				ret = TRUE;
			}
			nCol = hit.y = kNameCol;
			break;
		case kNameCol:
			{
			StateListItem* listItem = GetStateListItem(GetDlgItem(hWnd, IDC_STATE_LIST), nRow, nCol);
			if (listItem->GetSlaveState() != NULL)
			{
				blockInvalidation = true;
				Reactor* r = (Reactor*)listItem->GetOwner();
				if (r) {
					r->setSelected(listItem->GetIndex());
					blockInvalidation = false;
				}
			}
			}
			break;
		case kValueCol:
			{
			StateListItem* listItem = GetStateListItem(GetDlgItem(hWnd, IDC_STATE_LIST), nRow, nCol);
			if (listItem->GetType() == FLOAT_VAR)
			{
				listItem->GetValue(&origValue);
				ret = TRUE;
			}
			}
			break;
		case kStrengthCol:
			{
			StateListItem* listItem = GetStateListItem(GetDlgItem(hWnd, IDC_STATE_LIST), nRow, nCol);
			SlaveState* slaveState = listItem->GetSlaveState();
			if (slaveState)
			{
				Reactor* owner  = (Reactor*)listItem->GetOwner();
				if (!(owner->useCurve() && owner->getReactionType() == FLOAT_VAR))
				{
					origValue = slaveState->strength;
					ret = TRUE;
				}
			}
			}
			break;
		case kInfluenceCol:
			{
			StateListItem* listItem = GetStateListItem(GetDlgItem(hWnd, IDC_STATE_LIST), nRow, nCol);
			SlaveState* slaveState = listItem->GetSlaveState();
			if (slaveState)
			{
				Reactor* owner  = (Reactor*)listItem->GetOwner();
				if (!(owner->useCurve() && owner->getReactionType() == FLOAT_VAR))
				{
					origValue = listItem->GetInfluence();
					ret = TRUE;
				}
			}
			}
			break;
		case kFalloffCol:
			{
			StateListItem* listItem = GetStateListItem(GetDlgItem(hWnd, IDC_STATE_LIST), nRow, nCol);
			SlaveState* slaveState = listItem->GetSlaveState();
			if (slaveState && !((Reactor*)listItem->GetOwner())->useCurve())
			{
				origValue = slaveState->falloff;
				ret = TRUE;
			}
			}
			break;
		default: 
			ret = TRUE;
		}
	}
	NMHDR nh;
	nh.code = LVN_ITEMCHANGED;
	SendMessage(hWnd, WM_NOTIFY, id, (LPARAM)&nh);
	return ret;
}

void ReactionDlg::SpinnerChange(int id, POINT pt, int nRow, int nCol)
{
	if (nRow == -1 ) 
		return;
	
	BOOL ret = FALSE;
	if (id == IDC_REACTION_LIST)
	{
		switch (nCol)
		{
		case kExpandCol:
			nCol = kNameCol;
			//fall through 
		case kNameCol:	// name
			{
			}
			break;
		case kFromCol:
			{
			ReactionListItem* listItem = GetReactionListItem(GetDlgItem(hWnd, IDC_REACTION_LIST), nRow, nCol);
			int frame = origTime / GetTicksPerFrame();
			frame += (origPt->y - pt.y)/2;
			listItem->SetRange(TimeValue(frame * GetTicksPerFrame()), true);
			ret = TRUE;
			break;
			}
		case kToCol:
			{
			ReactionListItem* listItem = GetReactionListItem(GetDlgItem(hWnd, IDC_REACTION_LIST), nRow, nCol);
			int frame = origTime / GetTicksPerFrame();
			frame += (origPt->y - pt.y)/2;
			listItem->SetRange(TimeValue(frame * GetTicksPerFrame()), false);
			ret = TRUE;
			break;
			}
		default:
			ret = TRUE;
		}
	}
	else if (id == IDC_STATE_LIST)
	{
		switch (nCol)
		{
		case kExpandCol:
			ret = TRUE;
			nCol = kNameCol;
			//fall through 
		case kNameCol:	// name
			{
			}
			break;
		case kValueCol:
			{
			StateListItem* listItem = GetStateListItem(GetDlgItem(hWnd, IDC_STATE_LIST), nRow, nCol);
			if (listItem->GetType() == FLOAT_VAR)
			{
				float multiplier = 10.0f;
				if((GetKeyState(VK_CONTROL) & 0x8000))
					multiplier = 1.0f;
				float val = (origValue + (float)(origPt->y - pt.y)/multiplier);
				listItem->SetValue(val);
				iCCtrl->Redraw();
				ret = TRUE;
			}
			break;
			}
		case kStrengthCol:
			{
			StateListItem* listItem = GetStateListItem(GetDlgItem(hWnd, IDC_STATE_LIST), nRow, nCol);
			SlaveState* slaveState = listItem->GetSlaveState();
			if (slaveState)
			{
				Reactor* owner  = (Reactor*)listItem->GetOwner();
				if (!(owner->useCurve() && owner->getReactionType() == FLOAT_VAR))
			{
				float multiplier = 1000.0f;
				if((GetKeyState(VK_CONTROL) & 0x8000))
					multiplier = 100.0f;
				float strength = (origValue + (float)(origPt->y - pt.y)/multiplier);
					owner->setStrength(listItem->GetIndex(), strength);
				}
			}
			}
			break;
		case kInfluenceCol:
			{
			StateListItem* listItem = GetStateListItem(GetDlgItem(hWnd, IDC_STATE_LIST), nRow, nCol);
			SlaveState* slaveState = listItem->GetSlaveState();
			if (slaveState)
			{
				Reactor* owner  = (Reactor*)listItem->GetOwner();
				if (!(owner->useCurve() && owner->getReactionType() == FLOAT_VAR))
			{
				float multiplier = 10.0f;
				if((GetKeyState(VK_CONTROL) & 0x8000))
					multiplier = 1.0f;
				float influence = (origValue + (float)(origPt->y - pt.y)/multiplier);
					listItem->SetInfluence(influence);
				}
			}
			}
			break;
		case kFalloffCol:
			{
			StateListItem* listItem = GetStateListItem(GetDlgItem(hWnd, IDC_STATE_LIST), nRow, nCol);
			SlaveState* slaveState = listItem->GetSlaveState();
			if (slaveState)
			{
				Reactor* owner = (Reactor*)listItem->GetOwner();
				if(!owner->useCurve())
			{
				float multiplier = 100.0f;
				if((GetKeyState(VK_CONTROL) & 0x8000))
					multiplier = 10.0f;
				float falloff = (origValue + (float)(origPt->y - pt.y)/multiplier);
					owner->setFalloff(listItem->GetIndex(), falloff);
				}
			}
			}
			break;
		default: 
			ret = TRUE;
		}
	}
}

	//if (!drag)
	//	if (!theHold.Holding()) {
	//	SpinnerStart(id);
	//	}
	//
	//switch (id) {
	//	default:
	//		Change(FALSE);
	//		break;
	//	}
	//	
	//}

void ReactionDlg::SpinnerEnd(int id, BOOL cancel, int nRow, int nCol)
{
	if (cancel) {
		theHold.Cancel();
	} else if (id == IDC_REACTION_LIST)
	{
		switch (nCol) {
			case kToCol:
				{
				theHold.Accept(GetString(IDS_AF_RANGE_CHANGE));
				break;
				}
			case kFromCol:
				{
				theHold.Accept(GetString(IDS_AF_RANGE_CHANGE));
				break;
				}
			default:
				theHold.Accept(_T(""));
				break;
		}
	}else if (id == IDC_STATE_LIST)
	{
		switch (nCol) {
			case kValueCol:
				{
				theHold.Accept(GetString(IDS_AF_CHANGESTATE));
				break;
				}
			case kStrengthCol:
				{
				StateListItem* listItem = GetStateListItem(GetDlgItem(hWnd, IDC_STATE_LIST), nRow, nCol);
				SlaveState* slaveState = listItem->GetSlaveState();
				if (slaveState)
				{
					Reactor* owner  = (Reactor*)listItem->GetOwner();
					if (!(owner->useCurve() && owner->getReactionType() == FLOAT_VAR))
					{
					theHold.Accept(GetString(IDS_AF_CHANGESTRENGTH));
					}
				}
				break;
				}
			case kInfluenceCol:
				{
				StateListItem* listItem = GetStateListItem(GetDlgItem(hWnd, IDC_STATE_LIST), nRow, nCol);
				SlaveState* slaveState = listItem->GetSlaveState();
				if (slaveState)
				{
					Reactor* owner  = (Reactor*)listItem->GetOwner();
					if (!(owner->useCurve() && owner->getReactionType() == FLOAT_VAR))
					{
					theHold.Accept(GetString(IDS_AF_CHANGEINFLUENCE));
					}
				}
				break;
				}
			case kFalloffCol:
				{
				StateListItem* listItem = GetStateListItem(GetDlgItem(hWnd, IDC_STATE_LIST), nRow, nCol);
				SlaveState* slaveState = listItem->GetSlaveState();
				if (slaveState && !((Reactor*)listItem->GetOwner())->useCurve())
				{
					theHold.Accept(GetString(IDS_AF_CHANGEFALLOFF));
				}
				break;
				}
			default:
				theHold.Accept(_T(""));
				break;
		}
	}
	ip->RedrawViews(ip->GetTime());
}

void ReactionDlg::SetMinInfluence()
{
	HWND list = GetDlgItem(hWnd, IDC_STATE_LIST);
	Tab<StateListItem*> listItems = GetSelectedMasterStates();
	for(int i = 0; i < listItems.Count(); i++)
	{
		ReactionSet* set = (ReactionSet*)listItems[i]->GetOwner();
		for(int x = 0; x < set->SlaveCount(); x++)
		{
			Reactor* r = set->Slave(x);
			if (r) {
				for (int y = 0; y < r->slaveStates.Count(); y++)
				{
					if(r->slaveStates[y].masterID == listItems[i]->GetIndex())
					{
						r->setMinInfluence(y);
						break;
					}
				}
			}
		}
	}
	listItems = GetSelectedSlaveStates();
	for(int i = 0; i < listItems.Count(); i++)
	{
		Reactor* r = (Reactor*)listItems[i]->GetOwner();
		r->setMinInfluence(listItems[i]->GetIndex());
	}
}

void ReactionDlg::SetMaxInfluence()
{
	HWND list = GetDlgItem(hWnd, IDC_STATE_LIST);
	Tab<StateListItem*> listItems = GetSelectedMasterStates();
	for(int i = 0; i < listItems.Count(); i++)
	{
		ReactionSet* set = (ReactionSet*)listItems[i]->GetOwner();
		for(int x = 0; x < set->SlaveCount(); x++)
		{
			Reactor* r = set->Slave(x);
			if (r) {
				for (int y = 0; y < r->slaveStates.Count(); y++)
				{
					if(r->slaveStates[y].masterID == listItems[i]->GetIndex())
					{
						r->setMaxInfluence(y);
						break;
					}
				}
			}
		}
	}
	listItems = GetSelectedSlaveStates();
	for(int i = 0; i < listItems.Count(); i++)
	{
		Reactor* r = (Reactor*)listItems[i]->GetOwner();
		r->setMaxInfluence(listItems[i]->GetIndex());
	}
}

void ReactionDlg::Change(BOOL redraw)
	{
	GetReactionManager()->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	UpdateWindow(GetParent(hWnd));	
	if (redraw) ip->RedrawViews(ip->GetTime());
	}

RefResult ReactionDlg::NotifyRefChanged(
		Interval changeInt, 
		RefTargetHandle hTarget, 
     	PartID& partID,  
     	RefMessage message)
	{
	switch (message) {
		case REFMSG_SUBANIM_STRUCTURE_CHANGED:
			InvalidateReactionList();
			break;
		case REFMSG_CHANGE:
			Invalidate();			
			break;
		case REFMSG_REACTION_COUNT_CHANGED:
			InvalidateSelection();
			break;
		case REFMSG_REACTOR_SELECTION_CHANGED:
			Invalidate();
			break;
		case REFMSG_NODE_NAMECHANGE:
		case REFMSG_REACTTO_OBJ_NAME_CHANGED:
			//UpdateNodeName();
			break;
		case REFMSG_USE_CURVE_CHANGED:
			UpdateCurveControl();
			//SetupFalloffUI();
			break;
		case REFMSG_REACTION_NAME_CHANGED:
			//updateFlags |= REACTORDLG_LIST_CHANGED_SIZE;
			Invalidate();
			break;
		case REFMSG_REF_DELETED:
			//MaybeCloseWindow();
			break;
		}
	return REF_SUCCEED;
	}
static int sRow = -1;
static int sCol = -1;
static int sID = -1;
static BOOL sHandled = FALSE;

static LRESULT CALLBACK ReactionListControlProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	ReactionDlg *dlg = DLGetWindowLongPtr<ReactionDlg*>(hWnd);
	BOOL bHandled = FALSE;
	BOOL result = FALSE;

	switch (message)
	{
		case WM_LBUTTONDBLCLK:
			{
				SetFocus(hWnd);
				ShowWindow(dlg->floatWindow->GetHwnd(), SW_HIDE);
				POINT pt = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
				POINT hit = dlg->HitTest(pt, IDC_REACTION_LIST);
				sRow = hit.x;
				sCol = hit.y;
				sID = IDC_REACTION_LIST;
				if (sCol > 0)
				{
					RECT rect, pRect;
					GetWindowRect(hWnd, &pRect);
					POINT pt = {pRect.left, pRect.top};
					ScreenToClient(GetParent(hWnd), &pt);

					ListView_GetSubItemRect(hWnd, sRow, sCol, LVIR_BOUNDS, &rect);

					int left = rect.left+pt.x+2;
					int top = rect.top+pt.y+2;
					SetWindowPos(dlg->floatWindow->GetHwnd(), hWnd, left, top, rect.right-rect.left, rect.bottom-rect.top, 0);
					ShowWindow(dlg->floatWindow->GetHwnd(), SW_SHOW);
					dlg->floatWindow->GiveFocus();
					ReactionListItem* listItem = dlg->GetReactionDataAt(sRow);
					switch (sCol)
					{
					case kFromCol:
						{
							TimeValue t;
							listItem->GetRange(t, true);
							dlg->floatWindow->SetText(t/GetTicksPerFrame(), 0);
						}
						break;
					case kToCol:
						{
							TimeValue t;
							listItem->GetRange(t, false);
							dlg->floatWindow->SetText(t/GetTicksPerFrame(), 0);
						}
						break;
					}
				}
			}
			break;

		case WM_LBUTTONDOWN:
			{
			if (GetFocus() != hWnd)
				SetFocus(hWnd);
			dlg->origPt = new POINT();
			dlg->origPt->x = GET_X_LPARAM(lParam);
			dlg->origPt->y = GET_Y_LPARAM(lParam);
			POINT hit = dlg->HitTest(*(dlg->origPt), IDC_REACTION_LIST);
			sRow = hit.x;
			sCol = hit.y;

			sHandled = bHandled = dlg->OnLMouseButtonDown(hWnd, wParam, *dlg->origPt, IDC_REACTION_LIST);
			if (bHandled)
			{
				SetCapture(hWnd);
				if (sCol > 0)
					theHold.Begin();
			}
			else
			{
				delete dlg->origPt;
				dlg->origPt = NULL;
			}
			//dlg->selectionValid = false;
			}
			break;
		case WM_MOUSEMOVE:
			{
			POINT p = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
			if (dlg->origPt != NULL && sHandled && p.y != dlg->origPt->y)
			{
				dlg->SpinnerChange(IDC_REACTION_LIST, p, sRow, sCol);
				bHandled = sHandled;
			}
			}
			break;

		case WM_LBUTTONUP: 
		case WM_RBUTTONDOWN:
			if (dlg->origPt != NULL)
			{
				delete dlg->origPt;
				dlg->origPt = NULL;
				bHandled = sHandled;
				if (bHandled)
				{
					dlg->SpinnerEnd(IDC_REACTION_LIST, (message == WM_RBUTTONDOWN), sRow, sCol);
					ReleaseCapture();
				}
				dlg->selectionValid = false;
			}
			else if (message == WM_RBUTTONDOWN)
			{
				// pop up a right-click menu.
				dlg->InvokeRightClickMenu();
				bHandled = TRUE;
				result = TRUE;
			}
			break;
		case WM_PAINT:
			if (!dlg->blockInvalidation)
			{
				if (!dlg->reactionListValid)
					dlg->UpdateReactionList();
				else 
					if (!dlg->selectionValid)
					dlg->SelectionChanged();
			}
			break;
	}

	if (bHandled)
		return result;

	return CallWindowProc(dlg->reactionListWndProc, hWnd, message, wParam, lParam);
}

void ReactionDlg::InvokeRightClickMenu()
{
	HMENU menu;
	if ((menu = CreatePopupMenu()) != NULL)
	{
		ReactionListItem* selMaster = GetSelectedMaster();
		int anyMasterSelectionValid = (selMaster!=NULL)?0:MF_GRAYED;
		int masterSelectionValid = ((selMaster!=NULL)&&((ReactionSet*)selMaster->GetOwner())->GetReactionMaster()!= NULL)?0:MF_GRAYED;
		int slaveSelectionValid = (GetSelectedSlaves().Count() > 0)?0:MF_GRAYED;
		int masterStateSelectionValid = IsMasterStateSelected()?0:MF_GRAYED;
		int slaveStateSelectionValid = IsSlaveStateSelected()?0:MF_GRAYED;

		AppendMenu(menu, MF_STRING, IDC_ADD_MASTER, GetString(IDS_ADD_MASTER));
		AppendMenu(menu, (MF_STRING|anyMasterSelectionValid), IDC_REPLACE_MASTER, GetString(IDS_REPLACE_MASTER));
		AppendMenu(menu, (MF_STRING|masterSelectionValid), IDC_ADD_SLAVES, GetString(IDS_ADD_SLAVE));
		AppendMenu(menu, (MF_STRING|masterSelectionValid), IDC_ADD_SELECTED, GetString(IDS_ADD_SELECTED));
		AppendMenu(menu, (MF_STRING|(anyMasterSelectionValid&slaveSelectionValid)), IDC_DELETE_SELECTED, GetString(IDS_DELETE_SELECTED));

		AppendMenu(menu, MF_SEPARATOR, 0, 0);

		AppendMenu(menu, (MF_STRING|(masterSelectionValid&slaveSelectionValid)), IDC_CREATE_STATES, GetString(IDS_CREATE_STATES));
		AppendMenu(menu, (MF_STRING|(masterSelectionValid&slaveSelectionValid)), IDC_NEW_STATE, GetString(IDS_NEW_STATE));
		AppendMenu(menu, (MF_STRING|masterStateSelectionValid), IDC_APPEND_STATE, GetString(IDS_APPEND_STATE));
		AppendMenu(menu, (MF_STRING|(masterStateSelectionValid|masterSelectionValid)), IDC_SET_STATE, GetString(IDS_SET_STATE));
		AppendMenu(menu, (MF_STRING|(masterStateSelectionValid&slaveStateSelectionValid)), IDC_DELETE_STATE, GetString(IDS_DELETE_STATE));
		AppendMenu(menu, (MF_STRING|(masterStateSelectionValid&slaveStateSelectionValid)), IDC_EDIT_STATE, GetString(IDS_EDIT_STATE));

		// pop the menu
		POINT mp;
		GetCursorPos(&mp);
		int id = TrackPopupMenu(menu, TPM_CENTERALIGN + TPM_NONOTIFY + TPM_RETURNCMD, mp.x, mp.y, 0, hWnd, NULL);
		DestroyMenu(menu);

		if (id != 0)
			ip->GetActionManager()->FindTable(kReactionMgrActions)->GetAction(id)->ExecuteAction();
	}
}

static LRESULT CALLBACK StateListControlProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	ReactionDlg *dlg = DLGetWindowLongPtr<ReactionDlg*>(hWnd);
	BOOL bHandled = FALSE;
	BOOL result = FALSE;

	switch (message)
	{
		case WM_LBUTTONDBLCLK:
			{
				SetFocus(hWnd);
				ShowWindow(dlg->floatWindow->GetHwnd(), SW_HIDE);
				POINT pt = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
				POINT hit = dlg->HitTest(pt, IDC_STATE_LIST);
				sRow = hit.x;
				sCol = hit.y;
				sID = IDC_STATE_LIST;
				if (sCol > 0)
				{
					RECT rect, pRect;
					GetWindowRect(hWnd, &pRect);
					POINT pt = {pRect.left, pRect.top};
					ScreenToClient(GetParent(hWnd), &pt);

					ListView_GetSubItemRect(hWnd, sRow, sCol, LVIR_BOUNDS, &rect);

					int left = rect.left+pt.x+2;
					int top = rect.top+pt.y+2;
					SetWindowPos(dlg->floatWindow->GetHwnd(), hWnd, left, top, rect.right-rect.left, rect.bottom-rect.top, 0);
					StateListItem* listItem = dlg->GetStateDataAt(sRow);
					switch (sCol)
					{
					case kValueCol:
						switch(listItem->GetType())
						{
						case FLOAT_VAR:
							{
							float f;
							listItem->GetValue(&f);
							dlg->floatWindow->SetText(f);
							ShowWindow(dlg->floatWindow->GetHwnd(), SW_SHOW);
							dlg->floatWindow->GiveFocus();
							}
							break;
						}
						break;
					case kStrengthCol:
						{
							SlaveState* state = listItem->GetSlaveState();
							if (state != NULL)
							{
								Reactor* owner  = (Reactor*)listItem->GetOwner();
								if (!(owner->useCurve() && owner->getReactionType() == FLOAT_VAR))
								{
									dlg->floatWindow->SetText(state->strength*100.0f);
									ShowWindow(dlg->floatWindow->GetHwnd(), SW_SHOW);
									dlg->floatWindow->GiveFocus();
								}
							}
						}
						break;
					case kInfluenceCol:
						{
							SlaveState* state = listItem->GetSlaveState();
							if (state != NULL)
							{
								Reactor* owner  = (Reactor*)listItem->GetOwner();
								if (!(owner->useCurve() && owner->getReactionType() == FLOAT_VAR))
								{
									dlg->floatWindow->SetText(listItem->GetInfluence());
									ShowWindow(dlg->floatWindow->GetHwnd(), SW_SHOW);
									dlg->floatWindow->GiveFocus();
								}
							}
						}
						break;
					case kFalloffCol:
						{
							SlaveState* state = listItem->GetSlaveState();
							if (state != NULL && !((Reactor*)listItem->GetOwner())->useCurve())
							{
								dlg->floatWindow->SetText(state->falloff);
								ShowWindow(dlg->floatWindow->GetHwnd(), SW_SHOW);
								dlg->floatWindow->GiveFocus();
							}
						}
						break;
					default:
						break;
					}
				}
			}
			break;
		case WM_LBUTTONDOWN:
			{
			if (GetFocus() != hWnd)
				SetFocus(hWnd);
			dlg->origPt = new POINT();
			dlg->origPt->x = GET_X_LPARAM(lParam);
			dlg->origPt->y = GET_Y_LPARAM(lParam);
			POINT hit = dlg->HitTest(*(dlg->origPt), IDC_STATE_LIST);
			sRow = hit.x;
			sCol = hit.y;

			sHandled = bHandled = dlg->OnLMouseButtonDown(hWnd, wParam, *dlg->origPt, IDC_STATE_LIST);
			if (bHandled)
			{
				SetCapture(hWnd);
				if (sCol > 0)
					theHold.Begin();
			}
			else
			{
				delete dlg->origPt;
				dlg->origPt = NULL;
			}
			//dlg->stateSelectionValid = false;
			}
			break;
		case WM_MOUSEMOVE:
			{
			POINT p = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
			if (dlg->origPt != NULL && sHandled && p.y != dlg->origPt->y)
			{
				dlg->SpinnerChange(IDC_STATE_LIST, p, sRow, sCol);
				bHandled = sHandled;
			}
			}
			break;

		case WM_LBUTTONUP: 
		case WM_RBUTTONDOWN:
			if (dlg->origPt != NULL)
			{
				delete dlg->origPt;
				dlg->origPt = NULL;
				bHandled = sHandled;
				if (bHandled)
				{
					dlg->SpinnerEnd(IDC_STATE_LIST, (message == WM_RBUTTONDOWN), sRow, sCol);
					ReleaseCapture();
				}
				dlg->stateSelectionValid = false;
			}
			else if (message == WM_RBUTTONDOWN)
			{
				// pop up a right-click menu.
				dlg->InvokeRightClickMenu();
				bHandled = TRUE;
				result = TRUE;
			}
			break;
		case WM_PAINT:
			if (!dlg->blockInvalidation)
			{
				if (!dlg->reactionListValid)
					dlg->UpdateReactionList();
				else if (!dlg->selectionValid)
					dlg->SelectionChanged();
				else 
				if (!dlg->stateSelectionValid)
					dlg->StateSelectionChanged();
			}
			break;

	}

	if (bHandled)
		return result;

	return CallWindowProc(dlg->stateListWndProc, hWnd, message, wParam, lParam);
}

static LRESULT CALLBACK SplitterControlProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	ReactionDlg *dlg = DLGetWindowLongPtr<ReactionDlg*>(GetParent(hWnd));
	BOOL bHandled = FALSE;
	BOOL result = FALSE;
	static int dragging = -1;
	static float total = 0.0f;

	switch (message)
	{
		case WM_LBUTTONDOWN:
			{
			total = dlg->rListPos + dlg->sListPos;

			RECT rect;
			GetWindowRect(hWnd, &rect);

			POINT curPos;
			curPos.x = (short)LOWORD(lParam);
			curPos.y = (short)HIWORD(lParam);
			ClientToScreen(hWnd, &curPos);
			dragging = curPos.y - rect.top;

			SetCapture(hWnd);
			}
			break;
		case WM_MOUSEMOVE:
			SetCursor(LoadCursor(NULL, IDC_SIZENS));
			if (dragging >= 0)
			{
				RECT pRect;
				HWND parent = GetParent(hWnd);
				GetClientRect(parent, &pRect);

				POINT curPos;
				curPos.x = (short)LOWORD(lParam);
				curPos.y = (short)HIWORD(lParam) + dragging;
				ClientToScreen(hWnd, &curPos);
				ScreenToClient(parent, &curPos);

				float curY = ((float)curPos.y)/(float)pRect.bottom;
				if (hWnd == GetDlgItem(dlg->hWnd, IDC_SPLITTER1))
				{
					dlg->rListPos = min(max(curY, 0.05f), 0.95f);
					dlg->sListPos = total - dlg->rListPos;
				}
				else
				{
					dlg->sListPos = min(max(curY - dlg->rListPos, 0.05f), 0.95f);
				}
				dlg->PerformLayout();
				UpdateWindow(dlg->hWnd);
			}
			break;

		case WM_LBUTTONUP: 
			dragging = -1;
			ReleaseCapture();
			break;
		case WM_PAINT:
			break;

	}

	if (bHandled)
		return result;

	return CallWindowProc(dlg->splitterWndProc, hWnd, message, wParam, lParam);
}

static INT_PTR CALLBACK ReactionDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	ReactionDlg *dlg = DLGetWindowLongPtr<ReactionDlg*>(hWnd);

	switch (msg) {
		case WM_INITDIALOG:
			{
			dlg = (ReactionDlg*)lParam;
			DLSetWindowLongPtr(hWnd, lParam);

			HWND hList = GetDlgItem(hWnd, IDC_REACTION_LIST);
			DLSetWindowLongPtr(hList, lParam);
			dlg->reactionListWndProc = DLSetWindowProc(hList, ReactionListControlProc);
			
			hList = GetDlgItem(hWnd, IDC_STATE_LIST);
			DLSetWindowLongPtr(hList, lParam);
			dlg->stateListWndProc = DLSetWindowProc(hList, StateListControlProc);

			HWND hSplitter = GetDlgItem(hWnd, IDC_SPLITTER1);
			dlg->splitterWndProc = DLSetWindowLongPtr(hSplitter, SplitterControlProc);

			hSplitter = GetDlgItem(hWnd, IDC_SPLITTER2);
			DLSetWindowLongPtr(hSplitter, SplitterControlProc);

			dlg->SetupUI(hWnd);
			}
			break;
		case WM_NOTIFY:
			{
			LPNMHDR pNMHDR = (LPNMHDR)lParam;
			const UINT code = pNMHDR->code;

			switch (code){
				case LVN_GETDISPINFO:
					{
						NMLVDISPINFO *pDispInfo = (NMLVDISPINFO*)lParam;
						if (pDispInfo->item.mask |= LVIF_TEXT)
						{
							TSTR pSrcString = _T("");
							if (pDispInfo->hdr.idFrom == IDC_REACTION_LIST)
							{
								ReactionListItem *listItem = (ReactionListItem *)pDispInfo->item.lParam;
								switch (pDispInfo->item.iSubItem)
								{
									case kFromCol:
									{
										TimeValue t;
										if (listItem->GetRange(t, true))
											pSrcString.printf(_T("%i"), t/GetTicksPerFrame());
										break;
									}
									case kToCol:
									{
										TimeValue t;
										if (listItem->GetRange(t, false))
											pSrcString.printf(_T("%i"), t/GetTicksPerFrame());
										break;
									}
									case kCurveCol:
									{
										if (listItem->IsUsingCurve())
											pSrcString.printf(_T("X"));
										break;
									}
								}
							}
							else if (pDispInfo->hdr.idFrom == IDC_STATE_LIST)
							{
								StateListItem *listItem = (StateListItem *)pDispInfo->item.lParam;
								switch (pDispInfo->item.iSubItem)
								{
									case kValueCol:
										{
										int type = listItem->GetType();
										switch (type)
										{
											case FLOAT_VAR:
												{
													float f;
													listItem->GetValue(&f);
													pSrcString.printf( _T("%.3f"), f);
												}
												break;
											case SCALE_VAR:
											case VECTOR_VAR:
												{
													Point3 p;
													listItem->GetValue(&p);
													pSrcString.printf( _T("( %.3f; %.3f; %.3f )"), p[0], p[1], p[2]);
												}
												break;
											case QUAT_VAR:
												{
													Quat q;
													listItem->GetValue(&q);
													float x, y, z;
													q.GetEuler(&x, &y, &z);
													x = RadToDeg(x);
													y = RadToDeg(y);
													z = RadToDeg(z);
													pSrcString.printf(_T("( %.3f; %.3f; %.3f )"), x, y, z);
												}
												break;
										}
										}
										break;
									case kStrengthCol:
									{
										SlaveState* state = listItem->GetSlaveState();
										if (state != NULL)
										{
											Reactor* owner  = (Reactor*)listItem->GetOwner();
											if (!(owner->useCurve() && owner->getReactionType() == FLOAT_VAR))
											pSrcString.printf(_T("%.1f"), state->strength*100.0f);
										}
										break;
									}
									case kInfluenceCol:
									{
										SlaveState* state = listItem->GetSlaveState();
										if (state != NULL)
										{
											Reactor* owner  = (Reactor*)listItem->GetOwner();
											if (!(owner->useCurve() && owner->getReactionType() == FLOAT_VAR))
												pSrcString.printf(_T("%.1f"), listItem->GetInfluence());
										}
										break;
									}
									case kFalloffCol:
									{
										SlaveState* state = listItem->GetSlaveState();
										if (state != NULL && !((Reactor*)listItem->GetOwner())->useCurve())
											pSrcString.printf(_T("%.001f"), state->falloff);
										break;
									}
								}
							}
							_tcscpy(pDispInfo->item.pszText, pSrcString);
						}
					}

				//case LVN_ODSTATECHANGED:
				case LVN_ITEMCHANGED:
					{
						NMLISTVIEW *pListView = (NMLISTVIEW *)lParam;
						//NMLVODSTATECHANGE *pListView = (NMLVODSTATECHANGE *)lParam;
						// only handle change in selection state
						if ( !(pListView->uChanged & LVIF_STATE) )
							return TRUE;

						bool isSelected = (pListView->uNewState & LVIS_SELECTED) > 0;
						bool wasSelected = (pListView->uOldState & LVIS_SELECTED) > 0;
						//bool isSelected = (pListView->uNewState == LVIS_SELECTED);
						//bool wasSelected = (pListView->uOldState == LVIS_SELECTED);
						// I can't explain why we get messages with uOldState having
						// strange values (0x77d4xxxx) but if we ignore them we cut
						// down on the number of unnecessary reselections (#672926) 
						// @todo: find out why
						if (isSelected != wasSelected && pListView->uOldState <= 0x77d40000)
						{
							if (pListView->hdr.idFrom == IDC_REACTION_LIST)
							{
								dlg->selectionValid = false;
							}
							else if (pListView->hdr.idFrom == IDC_STATE_LIST)
							{
								dlg->stateSelectionValid = false;
							}

						}
					}
					break;
				case LVN_BEGINLABELEDIT:
					if (((NMLVDISPINFO*)lParam)->hdr.idFrom == IDC_STATE_LIST)
					{
						LVITEM item = ((NMLVDISPINFO*)lParam)->item;
						StateListItem* listItem = (StateListItem*)item.lParam;
						if (listItem == NULL || listItem->GetSlaveState() != NULL)
							return TRUE;
						DisableAccelerators();
					}
					return FALSE;
				case LVN_ENDLABELEDIT:
					if (((NMLVDISPINFO*)lParam)->hdr.idFrom == IDC_STATE_LIST)
					{
						BOOL ret = FALSE;
						LVITEM item = ((NMLVDISPINFO*)lParam)->item;
						StateListItem* listItem = (StateListItem*)item.lParam;

						MasterState* state = listItem->GetMasterState();
						if (state != NULL && item.pszText != NULL)
						{
							state->name.printf(_T("%s"), item.pszText);
							dlg->selectionValid = false;
							((ReactionMaster*)listItem->GetOwner())->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
							ret = TRUE;
						}
						EnableAccelerators();
						return ret;

					}
					return FALSE;

				//case NM_CUSTOMDRAW:
				//	{

				//	LPNMLVCUSTOMDRAW  lplvcd = (LPNMLVCUSTOMDRAW)lParam;

				//	switch(lplvcd->nmcd.dwDrawStage) {

				//		case CDDS_PREPAINT :
				//			return CDRF_NOTIFYITEMDRAW;

				//		case CDDS_ITEMPREPAINT:
				//	        return CDRF_NEWFONT;
				//			//return CDRF_NOTIFYSUBITEMREDRAW;

				//		case CDDS_SUBITEM | CDDS_ITEMPREPAINT:
				//			return CDRF_NEWFONT;    
				//		}
				//	}
				}
				return MAKELRESULT(1,0);
			}
			break;
		case CC_SPINNER_BUTTONDOWN:
			break;

		case CC_SPINNER_CHANGE:
			break;

		case WM_CUSTEDIT_ENTER:
			switch (LOWORD(wParam)) 
			{
			case IDC_EDIT_FLOAT:
				SetFocus(GetDlgItem(hWnd, sID));
				ShowWindow(dlg->floatWindow->GetHwnd(), SW_HIDE);
				switch (sID)
				{
				case IDC_REACTION_LIST:
					{
					ReactionListItem* listItem = dlg->GetReactionDataAt(sRow);
					switch (sCol)
					{
					case kFromCol:
						theHold.Begin();
						listItem->SetRange(dlg->floatWindow->GetInt()*GetTicksPerFrame(), true);
						theHold.Accept(GetString(IDS_AF_RANGE_CHANGE));
						break;
					case kToCol:
						theHold.Begin();
						listItem->SetRange(dlg->floatWindow->GetInt()*GetTicksPerFrame(), false);
						theHold.Accept(GetString(IDS_AF_RANGE_CHANGE));
						break;
					}
					sID = -1;
					}
					break;
				case IDC_STATE_LIST:
					{
						StateListItem* listItem = dlg->GetStateDataAt(sRow);
						switch (sCol)
						{
						case kValueCol:
							if (listItem->GetType() == FLOAT_VAR)
							{
								theHold.Begin();
								listItem->SetValue(dlg->floatWindow->GetFloat());
								dlg->iCCtrl->Redraw();
								theHold.Accept(GetString(IDS_AF_CHANGESTATE));
							}
							break;
						case kStrengthCol:
							{
								if (listItem->GetSlaveState() != NULL)
								{
									Reactor* owner  = (Reactor*)listItem->GetOwner();
									if (!(owner->useCurve() && owner->getReactionType() == FLOAT_VAR))
									{
										theHold.Begin();
										owner->setStrength(listItem->GetIndex(), dlg->floatWindow->GetFloat()/100.0f);
										theHold.Accept(GetString(IDS_AF_CHANGESTRENGTH));
									}
								}
							}
							break;
						case kInfluenceCol:
							{
								StateListItem* listItem = dlg->GetStateDataAt(sRow);
								if (listItem->GetSlaveState() != NULL)
								{
									Reactor* owner  = (Reactor*)listItem->GetOwner();
									if (!(owner->useCurve() && owner->getReactionType() == FLOAT_VAR))
									{
										theHold.Begin();
										listItem->SetInfluence(dlg->floatWindow->GetFloat());
										theHold.Accept(GetString(IDS_AF_CHANGEINFLUENCE));
									}
								}
							}
							break;
						case kFalloffCol:
							{
								StateListItem* listItem = dlg->GetStateDataAt(sRow);
								if (listItem->GetSlaveState() != NULL && !((Reactor*)listItem->GetOwner())->useCurve())
								{
									theHold.Begin();
									Reactor* r = (Reactor*)listItem->GetOwner();
									r->setFalloff(listItem->GetIndex(), dlg->floatWindow->GetFloat());
									theHold.Accept(GetString(IDS_AF_CHANGEFALLOFF));
								}
								break;
							}
						}
					}
					sID = -1;
					break;
				}
				break;
			default: 
					break;
			}
			break;

		case CC_SPINNER_BUTTONUP:
			break;
		
		case WM_MOUSEMOVE:
			SetCursor(LoadCursor(NULL,IDC_ARROW));	
			break;
		case WM_RBUTTONDOWN:
			// pop up a right-click menu.
			dlg->InvokeRightClickMenu();	
			break;

		case WM_COMMAND:
			//dlg->ip->GetActionManager()->FindTable(kReactionMgrActions)->GetAction(LOWORD(wParam))->ExecuteAction();
			dlg->WMCommand(LOWORD(wParam),HIWORD(wParam),(HWND)lParam);						
			break;

		case WM_CC_CHANGE_CURVEPT:
		case WM_CC_CHANGE_CURVETANGENT:
		{
			ICurve* curve = ((ICurve*)lParam);
			Reactor *reactor = FindReactor(curve);
			if (reactor->flags&REACTOR_BLOCK_CURVE_UPDATE) break;
			reactor->flags |= REACTOR_BLOCK_CURVE_UPDATE;

			dlg->iCCtrl->EnableDraw(FALSE);
			int ptNum = LOWORD(wParam);
			CurvePoint pt = curve->GetPoint(0,ptNum, FOREVER);
			
			theHold.Suspend();

			if ( reactor->getReactionType() == FLOAT_VAR) 
			{
				reactor->getMasterState(ptNum)->fvalue = pt.p.x;
				
				int curveNum;
				for (curveNum=0;curveNum<reactor->iCCtrl->GetNumCurves();curveNum++)
				{
					if (curve == reactor->iCCtrl->GetControlCurve(curveNum))
						break;
				}

				switch (reactor->type)
				{
					case REACTORFLOAT:
						reactor->slaveStates[ptNum].fstate = pt.p.y;
						reactor->UpdateCurves(false);
						break;
					case REACTORPOS:
					case REACTORP3:
					case REACTORSCALE:
						reactor->slaveStates[ptNum].pstate[curveNum] = pt.p.y;
						reactor->UpdateCurves(false);
						break;
					case REACTORROT:
					{
						float eulAng[3];
						QuatToEuler(reactor->slaveStates[ptNum].qstate, eulAng);
						eulAng[curveNum] = DegToRad(pt.p.y);
						EulerToQuat(eulAng, reactor->slaveStates[ptNum].qstate);
						reactor->UpdateCurves(false);
						break;
					}
					default:
						break;
				}
				
				float xMax = dlg->iCCtrl->GetXRange().y;
				float xMin = dlg->iCCtrl->GetXRange().x;
				float oldWidth = xMax - xMin;

				float oldMax = xMax;
				float oldMin = xMin;
				Point2 p,pin,pout;

				if (curve->GetNumPts() > ptNum+1 && pt.p.x >= curve->GetPoint(0,ptNum+1, FOREVER).p.x - pt.out.x - curve->GetPoint(0,ptNum+1, FOREVER).in.x)//{}
				{
					pt.p.x = curve->GetPoint(0,ptNum+1, FOREVER).p.x - pt.out.x - curve->GetPoint(0,ptNum+1, FOREVER).in.x - 0.001f;
				}
				else 
					if (ptNum && pt.p.x <= curve->GetPoint(0,ptNum-1, FOREVER).p.x + pt.in.x + curve->GetPoint(0,ptNum+1, FOREVER).out.x)//{}
					{
						pt.p.x = curve->GetPoint(0,ptNum-1, FOREVER).p.x + pt.in.x + curve->GetPoint(0,ptNum+1, FOREVER).out.x + 0.001f;
					}
				
				if (ptNum == 0) 
				{
					if (pt.p.x > xMax) 
						xMax = pt.p.x;
					xMin = pt.p.x;
					dlg->iCCtrl->SetXRange(pt.p.x, xMax, FALSE);
				}
				else if ( ptNum == curve->GetNumPts()-1 )
				{	
					if (pt.p.x < xMin) 
						xMin = pt.p.x;
					dlg->iCCtrl->SetXRange(xMin, pt.p.x, FALSE);
					xMax = pt.p.x;
				}

				//keep the rest of them in sync
				for (int i=0;i<reactor->iCCtrl->GetNumCurves();i++)
				{
					ICurve* otherCurve = reactor->iCCtrl->GetControlCurve(i);
					if (curve != otherCurve)
					{
						pt = otherCurve->GetPoint(0,ptNum, FOREVER);
						pt.p.x = reactor->getMasterState(ptNum)->fvalue;
						otherCurve->SetPoint(0,ptNum,&pt,FALSE, FALSE);
					}
				}
			}
			theHold.Resume();
			dlg->iCCtrl->EnableDraw(TRUE);
			reactor->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
			dlg->ip->RedrawViews(dlg->ip->GetTime());
			
			reactor->flags &= ~REACTOR_BLOCK_CURVE_UPDATE;
		
			break;
		}
		case WM_CC_SEL_CURVEPT:
		{
			ICurve* curve = ((ICurve*)lParam);
			Reactor *reactor = FindReactor(curve);
			if (reactor->getReactionType() == FLOAT_VAR)
			{
				int selCount = (LOWORD(wParam));
				if (selCount == 1 )
				{
					BitArray selPts = curve->GetSelectedPts();
					for (int i=0;i<selPts.GetSize();i++)
					{
						if (selPts[i])
						{
							//SendDlgItemMessage(hWnd, IDC_REACTION_LIST, LB_SETCURSEL, i, 0);
							//ListView_SetItemState(GetDlgItem(hWnd, IDC_STATE_LIST), i
							reactor->setSelected(i);
							break;
						}
					}
				}
			}
			break;
		}
		case WM_CC_DEL_CURVEPT:
			{
				ICurve* curve = ((ICurve*)lParam);
				Reactor *reactor = FindReactor(curve);
				if (reactor->getReactionType() == FLOAT_VAR)
				{
					int ptNum = LOWORD(wParam);
					reactor->flags |= REACTOR_BLOCK_CURVE_UPDATE;

					//delete the points on the other curves
					int curveNum;
					for (curveNum=0;curveNum<reactor->iCCtrl->GetNumCurves();curveNum++)
					{
						ICurve* otherCurve = reactor->iCCtrl->GetControlCurve(curveNum);
						if (curve != otherCurve)
						{
							otherCurve->Delete(ptNum);
						}
					}

					reactor->DeleteReaction(ptNum);
					reactor->flags &= ~REACTOR_BLOCK_CURVE_UPDATE;
					dlg->iCCtrl->Redraw();
					dlg->selectionValid = false;
				}
			}
			break;
		case WM_CC_INSERT_CURVEPT:
			{
				ICurve* curve = ((ICurve*)lParam);
				Reactor *reactor = FindReactor(curve);
				if (reactor->getReactionType() == FLOAT_VAR && !theHold.RestoreOrRedoing())
				{
					if (curve->GetNumPts() <= reactor->slaveStates.Count()) return 0;
					int ptNum = LOWORD(wParam);
					CurvePoint pt = curve->GetPoint(0,ptNum,FOREVER);
					//update the flags for the newly added point
					pt.flags |= CURVEP_NO_X_CONSTRAINT;
					curve->SetPoint(0,ptNum,&pt,FALSE,FALSE);

					int curNum;
					ICurve* tempCurve;

					reactor->flags |= REACTOR_BLOCK_CURVE_UPDATE;
					SlaveState* reaction = reactor->CreateReactionAndReturn(FALSE);
					MasterState* masterState = reactor->GetMaster()->GetState(reaction->masterID);
					reactor->flags &= ~REACTOR_BLOCK_CURVE_UPDATE;

					if (reaction)
					{
						masterState->fvalue = pt.p.x;

						switch ( reactor->type )
						{
							case REACTORFLOAT:
								reaction->fstate = pt.p.y;
								break;
							case REACTORROT:
								float ang[3];
								for (curNum=0;curNum<reactor->iCCtrl->GetNumCurves();curNum++)
								{
									tempCurve = reactor->iCCtrl->GetControlCurve(curNum);
									ang[curNum] = DegToRad(tempCurve->GetValue(0, masterState->fvalue, FOREVER, FALSE));
								}
								EulerToQuat(ang, reaction->qstate);
								break;
							case REACTORSCALE:
							case REACTORPOS:
							case REACTORP3:
								for (curNum=0;curNum<reactor->iCCtrl->GetNumCurves();curNum++)
								{
									tempCurve = reactor->iCCtrl->GetControlCurve(curNum);
									reaction->pstate[curNum] = tempCurve->GetValue(0, masterState->fvalue, FOREVER, FALSE);
								}
								break;
						}

						for (curNum=0;curNum<reactor->iCCtrl->GetNumCurves();curNum++)
						{
							tempCurve = reactor->iCCtrl->GetControlCurve(curNum);
							if (tempCurve != curve)
							{
								CurvePoint *newPt = new CurvePoint();
								newPt->p.x = masterState->fvalue;
								newPt->p.y = tempCurve->GetValue(0, masterState->fvalue);
								newPt->in = newPt->out = Point2(0.0f,0.0f);// newPt->p;
								newPt->flags = pt.flags;
								tempCurve->Insert(ptNum, *newPt);
							}
						}					

						if (reactor->slaveStates.Count() > 1)		//if its not the first reaction
						{
							reactor->setMinInfluence(ptNum);
						}
						reactor->SortReactions();
					}
					dlg->selectionValid = false;
				}
			}
			break;
		case WM_PAINT:
			if (!dlg->valid && !dlg->blockInvalidation)
				dlg->Update();
			return 0;
		case WM_SIZE:
			dlg->PerformLayout();
			break;
		case WM_CLOSE:
			DestroyWindow(hWnd);
			dlg = NULL;
			break;

		case WM_DESTROY:
			{
			HWND hList = GetDlgItem(hWnd, IDC_REACTION_LIST);
			DLSetWindowLongPtr(hList, NULL);
			DLSetWindowLongPtr(hList, dlg->reactionListWndProc);
			
			hList = GetDlgItem(hWnd, IDC_STATE_LIST);
			DLSetWindowLongPtr(hList, NULL);
			DLSetWindowLongPtr(hList, dlg->stateListWndProc);

			hList = GetDlgItem(hWnd, IDC_SPLITTER1);
			DLSetWindowLongPtr(hList, dlg->splitterWndProc);

			hList = GetDlgItem(hWnd, IDC_SPLITTER2);
			DLSetWindowLongPtr(hList, dlg->splitterWndProc);

			delete dlg;
			}
			break;
		
		default:
			return FALSE;
		}
	return TRUE;
	}

int StateListItem::GetType()
{
	if (mType == kMasterState)
	{
		MasterState* state = GetMasterState();
		return state->type;
	}
	else 
	{
		Reactor* r = (Reactor*)GetOwner();
		switch(r->type)
		{
			case REACTORFLOAT:
				return FLOAT_VAR;
			case REACTORPOS:
			case REACTORP3:
			case REACTORSCALE:
				return VECTOR_VAR;
			case REACTORROT:
				return QUAT_VAR;
			default:
				return 0;
		}
	}
	return 0;
}

float StateListItem::GetInfluence()
{
	float infl = GetSlaveState()->influence;

	Reactor* r = (Reactor*)GetOwner();
	ReactionMaster* master = r->GetMaster();
	if (master && master->Owner()) {
		ParamDimension* dim = master->Owner()->GetParamDimension(master->SubIndex());
		infl = dim->Convert(infl);
	}
	return infl;
}

void StateListItem::SetInfluence(float infl)
{
	Reactor* r = (Reactor*)GetOwner();
	ReactionMaster* master = r->GetMaster();
	if (master && master->Owner()) {
		ParamDimension* dim = master->Owner()->GetParamDimension(master->SubIndex());
		r->setInfluence(GetIndex(), dim->UnConvert(infl));
	}
	else
		r->setInfluence(GetIndex(), infl);
}

void StateListItem::SetValue(float val)
{
	if (mType == kMasterState)
	{
		MasterState* state = GetMasterState();
		if (state->type == FLOAT_VAR)
		{
			ReactionMaster* master = ((ReactionSet*)this->GetOwner())->GetReactionMaster();
			if (master && master->Owner()) {
				ParamDimension* dim = master->Owner()->GetParamDimension(master->SubIndex());
				master->SetState(this->GetIndex(), dim->UnConvert(val));
			}
			else
				master->SetState(this->GetIndex(), val);
			GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
		}
	}
	else 
	{
		Reactor* r = (Reactor*)GetOwner();
		if (r->type == REACTORFLOAT)
		{
			ParamDimension* dim = NULL;
			DependentIterator di(r);
			ReferenceMaker* maker = NULL;
			while ((maker = di.Next()) != NULL)
			{
				for(int i=0; i<maker->NumSubs(); i++)
				{
					Animatable* n = maker->SubAnim(i);
					if (n == r)
					{
						dim = maker->GetParamDimension(i);
						val = dim->UnConvert(val);
						break;
					}
				}
				if (dim != NULL)
					break;
			}
			
			r->setState(GetIndex(), val);
			GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
		}
	}
}

void StateListItem::GetValue(void* val)
{
	if (mType == kMasterState)
	{
		MasterState* state = GetMasterState();
		switch (state->type)
		{
			case FLOAT_VAR:
				{
					ReactionMaster* master = ((ReactionSet*)this->GetOwner())->GetReactionMaster();
					if (master && master->Owner())
					{
						ParamDimension* dim = master->Owner()->GetParamDimension(master->SubIndex());
						*(float*)val = dim->Convert(state->fvalue);
					}
					else
						*(float*)val = state->fvalue;
				}
				break;
			case SCALE_VAR:
			case VECTOR_VAR:
				*(Point3*)val = state->pvalue;
				break;
			case QUAT_VAR:
				*(Quat*)val = state->qvalue;
				break;
		}
	}
	else 
	{
		SlaveState* state = GetSlaveState();
		Reactor* r = (Reactor*)this->GetOwner();
		if (r) {
			switch (r->type)
			{
				case REACTORFLOAT:
					{
					MyEnumProc dep(r); 
					r->DoEnumDependents(&dep);
					ParamDimension* dim = NULL;

					for(int x=0; x<dep.anims.Count(); x++)
					{
						for(int i=0; i<dep.anims[x]->NumSubs(); i++)
						{
							Animatable* n = dep.anims[x]->SubAnim(i);
							if (n == r)
							{
								dim = dep.anims[x]->GetParamDimension(i);
								*(float*)val = dim->Convert(state->fstate);
								break;
							}
						}
						if (dim != NULL)
							break;
					}

					if (dim == NULL)
						*(float*)val = state->fstate;
					}
					break;
				case REACTORPOS:
				case REACTORP3:
				case REACTORSCALE:
					*(Point3*)val = state->pstate;
					break;
				case REACTORROT:
					*(Quat*)val = state->qstate;
					break;
				default:
					val = NULL;
					break;
			}
		}
	}
}

TSTR StateListItem::GetName() 
{ 
	if (mType == kMasterState)
	{
		return GetMasterState()->name;
	}
	else if (owner != NULL)
	{
		TSTR buf = _T("");
		MyEnumProc dep(owner); 
		owner->DoEnumDependents(&dep);

		for(int x=0; x<dep.anims.Count(); x++)
		{
			for(int i=0; i<dep.anims[x]->NumSubs(); i++)
			{
				ReferenceMaker* parent = dep.anims[x];
				Animatable* n = parent->SubAnim(i);
				if (n == owner)
				{
					TSTR name = _T("");
					TSTR subAnimName = parent->SubAnimName(i);
					TSTR modName = _T("");

					if (parent->SuperClassID() == PARAMETER_BLOCK2_CLASS_ID)
						parent = ((IParamBlock2*)parent)->GetOwner();
					if (parent != NULL)
					{
						if (parent->SuperClassID() == OSM_CLASS_ID || parent->SuperClassID() == WSM_CLASS_ID)
							modName = ((BaseObject*)parent)->GetObjectName();

						if (modName.Length() > 0)
							subAnimName.printf(_T("%s / %s"), modName, subAnimName);

						parent->NotifyDependents(FOREVER,(PartID)&name,REFMSG_GET_NODE_NAME);
					}
					if (name.Length() > 0)
						buf.printf(_T("%s / %s"), name, subAnimName);
					else
						buf.printf(_T("%s"), subAnimName);

					//added (Locked) to the end of the name, like is done in trackview
					ILockedTracksMan *iltman = static_cast<ILockedTracksMan*>(GetCOREInterface(ILOCKEDTRACKSMAN_INTERFACE ));
					ReferenceTarget *ranim = dynamic_cast<ReferenceTarget *>(n);
					ReferenceTarget *rparent= dynamic_cast<ReferenceTarget *>(parent);
					if(iltman&&ranim &&iltman->GetLocked(ranim,rparent,i))
					{
						buf += GetString(IDS_LOCKED);
					}

					return buf;
				}
			}
		}
	return buf;
	}
	return _T("");
}

TSTR ReactionListItem::GetName() 
{ 
	TSTR buf = _T("");
	TSTR name;
	TSTR subAnimName;

	if (mType == kReactionSet)
	{
		ReactionMaster* master = GetReactionSet()->GetReactionMaster();
		if (master && master->Owner())
		{
			ReferenceMaker* owner = master->Owner();
			int subNum = master->SubIndex();
			if (subNum < 0)
			{
				if (subNum == IKTRACK)
					subAnimName = GetString(IDS_AF_ROTATION);
				else
					subAnimName = GetString(IDS_AF_WS_POSITION);
			}
			else 
				subAnimName = owner->SubAnimName(subNum);

			TSTR modName = _T("");

			if (owner->SuperClassID() == PARAMETER_BLOCK2_CLASS_ID)
				owner = ((IParamBlock2*)owner)->GetOwner();
			if (owner != NULL)
			{
				if (owner->SuperClassID() == OSM_CLASS_ID || owner->SuperClassID() == WSM_CLASS_ID)
					modName = ((BaseObject*)owner)->GetObjectName();

				if (modName.Length() > 0)
					subAnimName.printf(_T("%s / %s"), modName, subAnimName);

				if (owner->SuperClassID() == BASENODE_CLASS_ID)
					name = ((INode*)owner)->GetName();
				else
					owner->NotifyDependents(FOREVER,(PartID)&name,REFMSG_GET_NODE_NAME);
			}
			if (name.Length() > 0)
				buf.printf(_T("%s / %s"), name, subAnimName);
			else
				buf.printf(_T("%s"), subAnimName);
		}
		else
			buf = GetString(IDS_UNASSIGNED);

		return buf;
	}
	else
	{
		Reactor* r = Slave();
		if (r) {
			MyEnumProc dep(r); 
			r->DoEnumDependents(&dep);

			for(int x=0; x<dep.anims.Count(); x++)
			{
				for(int i=0; i<dep.anims[x]->NumSubs(); i++)
				{
					Animatable* parent = dep.anims[x];
					Animatable* n = parent->SubAnim(i);
					if (n == r)
					{
						subAnimName = parent->SubAnimName(i);
						TSTR modName = _T("");

						if (parent->SuperClassID() == PARAMETER_BLOCK2_CLASS_ID)
							parent = ((IParamBlock2*)parent)->GetOwner();
						if (parent != NULL)
						{
							if (parent->SuperClassID() == OSM_CLASS_ID || parent->SuperClassID() == WSM_CLASS_ID)
								modName = ((BaseObject*)parent)->GetObjectName();
						}

						if (modName.Length() > 0)
							subAnimName.printf(_T("%s / %s"), modName, subAnimName);

						r->NotifyDependents(FOREVER,(PartID)&name,REFMSG_GET_NODE_NAME);
						if (name.Length() > 0)
							buf.printf(_T("%s / %s"), name, subAnimName);
						else
							buf.printf(_T("%s"), subAnimName);
						//added (Locked) to the end of the name, like is done in trackview
						if(r->GetLocked()==true)
						{
							buf += GetString(IDS_LOCKED);
						}
						return buf;
					}
				}
			}
		}
		else
			buf = GetString(IDS_UNASSIGNED);
	}
	return buf;
}


//-----------------------------------------------------------------------

// action table
static ActionDescription spActions[] = {

	ID_MIN_INFLUENCE,
    IDS_MIN_INFLUENCE,
    IDS_MIN_INFLUENCE,
    IDS_AF_REACTION_MANAGER,

    ID_MAX_INFLUENCE,
    IDS_MAX_INFLUENCE,
    IDS_MAX_INFLUENCE,
    IDS_AF_REACTION_MANAGER,

	IDC_ADD_MASTER,
    IDS_ADD_MASTER,
    IDS_ADD_MASTER,
    IDS_AF_REACTION_MANAGER,

	IDC_REPLACE_MASTER,
    IDS_REPLACE_MASTER,
    IDS_REPLACE_MASTER,
    IDS_AF_REACTION_MANAGER,

	IDC_ADD_SLAVES,
    IDS_ADD_SLAVE,
    IDS_ADD_SLAVE,
    IDS_AF_REACTION_MANAGER,

	IDC_EDIT_STATE,
    IDS_EDIT_STATE,
    IDS_EDIT_STATE,
    IDS_AF_REACTION_MANAGER,

	IDC_ADD_SELECTED,
    IDS_ADD_SELECTED,
    IDS_ADD_SELECTED,
    IDS_AF_REACTION_MANAGER,

	IDC_DELETE_SELECTED,
    IDS_DELETE_SELECTED,
    IDS_DELETE_SELECTED,
    IDS_AF_REACTION_MANAGER,

	IDC_CREATE_STATES,
    IDS_CREATE_STATES,
    IDS_CREATE_STATES,
    IDS_AF_REACTION_MANAGER,

	IDC_NEW_STATE,
    IDS_NEW_STATE,
    IDS_NEW_STATE,
    IDS_AF_REACTION_MANAGER,

	IDC_APPEND_STATE,
    IDS_APPEND_STATE,
    IDS_APPEND_STATE,
    IDS_AF_REACTION_MANAGER,

	IDC_SET_STATE,
    IDS_SET_STATE,
    IDS_SET_STATE,
    IDS_AF_REACTION_MANAGER,

	IDC_DELETE_STATE,
    IDS_DELETE_STATE,
    IDS_DELETE_STATE,
    IDS_AF_REACTION_MANAGER
};

template <class T>
BOOL ReactionDlgActionCB<T>::ExecuteAction(int id)
{
	switch (id)
	{
	case ID_MIN_INFLUENCE:
		theHold.Begin();
		dlg->SetMinInfluence();
		theHold.Accept(GetString(IDS_MIN_INFLUENCE));
		break;
	case ID_MAX_INFLUENCE:
		theHold.Begin();
		dlg->SetMaxInfluence();
		theHold.Accept(GetString(IDS_MAX_INFLUENCE));
		break;
	case IDC_ADD_MASTER:
	case IDC_REPLACE_MASTER:
	case IDC_ADD_SLAVES:
	case IDC_CREATE_STATES:
	case IDC_EDIT_STATE:
		{
			ICustButton* but = GetICustButton(GetDlgItem(dlg->hWnd, id));
			if (but)
				but->SetCheck(TRUE);
			ReleaseICustButton(but);
		}
		// fall through
	case IDC_ADD_SELECTED:
	case IDC_DELETE_SELECTED:
	case IDC_NEW_STATE:
	case IDC_APPEND_STATE:
	case IDC_SET_STATE:
	case IDC_DELETE_STATE:
		WPARAM wParam = MAKEWPARAM(id, 0);
		PostMessage(dlg->hWnd, WM_COMMAND, wParam, (LPARAM)GetDlgItem(dlg->hWnd, id));

		//dlg->WMCommand(id, 0, GetDlgItem(dlg->hWnd, id));
		break;
	}
	return TRUE;
}

ActionTable* GetActions()
{
    TSTR name = GetString(IDS_AF_REACTION_MANAGER);
    HACCEL hAccel = LoadAccelerators(hInstance,
                                     MAKEINTRESOURCE(IDR_REACTIONMGR_SHORTCUTS));
    int numOps = NumElements(spActions);
    ActionTable* pTab;
    pTab = new ActionTable(kReactionMgrActions, kReactionMgrContext, name, hAccel, numOps,
                             spActions, hInstance);
#ifndef REACTOR_CONTROLLERS_PRIVATE
    GetCOREInterface()->GetActionManager()->RegisterActionContext(kReactionMgrContext, name.data());
#endif //REACTOR_CONTROLLERS_PRIVATE
    return pTab;
}


