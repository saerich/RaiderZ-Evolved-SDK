#include "MorphByBone.h"


static PickControlNode thePickMode;
/*
void MorphByBone::AddItemToTree(HWND hwndTV, int nodeID)
{
//add node
	//loop through owning bones morph targets
}
*/
HTREEITEM AddItemToTree(HWND hwndTV, TVITEM &tvi, LPSTR lpszItem, int nLevel, HTREEITEM parent)
{ 
//    TVITEM tvi; 
    TVINSERTSTRUCT tvins; 
    static HTREEITEM hPrev = (HTREEITEM) TVI_FIRST; 
    static HTREEITEM hPrevRootItem = NULL; 
    static HTREEITEM hPrevLev2Item = NULL; 
 
    tvi.mask = TVIF_TEXT  | TVIF_PARAM | TVIF_STATE; 
 
    // Set the text of the item. 
    tvi.pszText = lpszItem; //LPSTR_TEXTCALLBACK;//
    tvi.cchTextMax = lstrlen(lpszItem); 

	
 
    // Assume the item is not a parent item, so give it a 
    // document image. 
 
    // Save the heading level in the item's application-defined 
    // data area. 
    tvi.lParam = (LPARAM) nLevel; 
 
    tvins.item = tvi; 
    tvins.hInsertAfter = TVI_LAST; 
 
    // Set the parent item based on the specified level. 
//    if (nLevel == 1) 
        tvins.hParent = parent; 
  //  else if (nLevel == 2) 
    //    tvins.hParent = hPrevRootItem; 
  //  else 
    //    tvins.hParent = hPrevLev2Item; 
 
    // Add the item to the tree view control. 
    hPrev = (HTREEITEM) SendMessage(hwndTV, TVM_INSERTITEM, 0, 
         (LPARAM) (LPTVINSERTSTRUCT) &tvins); 
 
    // Save the handle to the item. 
    if (nLevel == 1) 
        hPrevRootItem = hPrev; 
    else if (nLevel == 2) 
        hPrevLev2Item = hPrev; 
 
    // The new item is a child item. Give the parent item a 
    // closed folder bitmap to indicate it now has child items. 
 
    return hPrev; 
} 


void MorphByBone::BuildTreeList()
{
//get our handle

	suspendUI = TRUE;
	HWND treeHWND = GetDlgItem(rollupHWND,IDC_TREE1);
	int tempBone, tempMorph;
	tempBone = currentBone;
	tempMorph = currentMorph;

	
	COLORREF clrBk = ColorMan()->GetColor(kWindow);
	TreeView_SetBkColor( treeHWND, clrBk); 



//	TVITEM tvi;

	BitArray expanded;
	expanded.SetSize(boneData.Count());
	expanded.ClearAll();
	for (int i = 0; i < boneData.Count(); i++)
		{
	//if !null
		INode *node = GetNode(i);
		if (node != NULL)
			{
	//add to parent
			TSTR name = node->GetName();
	//add the node as a root entry
			BOOL expand = TreeView_GetItemState(treeHWND,boneData[i]->treeHWND,TVIS_EXPANDED);
			if (expand)
				{
				expanded.Set(i);
				}
			}
		}

	TreeView_DeleteAllItems(treeHWND);	
	currentBone = tempBone;
	currentMorph = tempMorph;

	//DebugPrint(_T("Morph name creatiuon\n"));
//loop through our nodes
	for (int i = 0; i < boneData.Count(); i++)
		{
	//if !null
		INode *node = GetNode(i);
		if (node != NULL)
			{
	//add to parent
			TSTR name = node->GetName();
	//add the node as a root entry
			boneData[i]->tvi.state = 0;

			HTREEITEM parent = AddItemToTree(treeHWND, boneData[i]->tvi, name,i,TVI_ROOT);
			
			boneData[i]->treeHWND = parent;
			if (expanded[i])
			{
				boneData[i]->tvi.state = TVIS_EXPANDED;
				TreeView_SetItemState(treeHWND,boneData[i]->treeHWND,TVIS_EXPANDED,TVIS_EXPANDED);
			}
			if ((currentMorph == -1) && (i==currentBone))
			{
				if (i==currentBone)
				{
 				TreeView_SetItemState(treeHWND,boneData[currentBone]->treeHWND,TVIS_SELECTED,TVIS_SELECTED);
				TreeView_SetItemState(treeHWND,boneData[currentBone]->treeHWND,TVIS_BOLD,TVIS_BOLD);
				}
				else
				{
				TreeView_SetItemState(treeHWND,boneData[currentBone]->treeHWND,0,TVIS_SELECTED);
				TreeView_SetItemState(treeHWND,boneData[currentBone]->treeHWND,0,TVIS_BOLD);

				}
			}
	//loop through targets and add those as children
			for (int j = 0 ; j < boneData[i]->morphTargets.Count(); j++)
				{
				if (!boneData[i]->morphTargets[j]->IsDead())
					{
//add morph target to list
					TSTR morphName;
					if (boneData[i]->morphTargets[j]->IsDisabled())
						morphName.printf("%s(%s)",boneData[i]->morphTargets[j]->name,GetString(IDS_DISABLED));
					else morphName.printf("%s(%0.1f)",boneData[i]->morphTargets[j]->name,boneData[i]->morphTargets[j]->weight*100.f);

//					boneData[i]->morphTargets[j]->tvi.state = TVIS_BOLD ;
//					boneData[i]->morphTargets[j]->tvi.stateMask |= TVIS_BOLD ;
//DebugPrint(_T("Morph name %s\n"),morphName);
					boneData[i]->morphTargets[j]->tvi.state = 0;
					boneData[i]->morphTargets[j]->treeHWND = AddItemToTree(treeHWND ,boneData[i]->morphTargets[j]->tvi, morphName, (i+1)*1000+j,parent);

					if ((i== currentBone) && (j == currentMorph))
					{
						TreeView_SetItemState(treeHWND,boneData[i]->morphTargets[j]->treeHWND,TVIS_SELECTED ,TVIS_SELECTED );
						TreeView_SetItemState(treeHWND,boneData[i]->morphTargets[j]->treeHWND,TVIS_BOLD ,TVIS_BOLD );
					}
					else
					{
						TreeView_SetItemState(treeHWND,boneData[i]->morphTargets[j]->treeHWND,0 ,TVIS_SELECTED );
						TreeView_SetItemState(treeHWND,boneData[i]->morphTargets[j]->treeHWND,0 ,TVIS_BOLD );
					}





					
					}
				}
			}
		}
	suspendUI = FALSE;

}




/*
BOOL InitTreeViewItems(HWND hwndTV) 
{ 
 
    // Open the file to parse. 
	TSTR name("testA");
    AddItemToTree(hwndTV, name, 0); 
	TSTR nameB("testB");
    AddItemToTree(hwndTV, nameB, 0); 
 


    return TRUE; 
} 

*/



INT_PTR MorphByBoneParamsMapDlgProc::DlgProc(TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)		


			
	{


	switch (msg) 
		{
		case WM_INITDIALOG:
			{
			mod->rollupHWND = hWnd;

			mod->iPickBoneButton = GetICustButton(GetDlgItem(hWnd, IDC_PICKBONE));
			mod->iPickBoneButton->SetType(CBT_CHECK);
			mod->iPickBoneButton->SetHighlightColor(GREEN_WASH);

//			mod->hWnd = hWnd;
			mod->BuildTreeList();
//			mod->currentBone = -1;
//			mod->currentMorph = -1;

//			InitTreeViewItems(GetDlgItem(hWnd,IDC_TREE1));
//			CreateATreeView(hWnd);

			break;
			}
		case WM_NOTIFY :
			{
			switch (((LPNMHDR)lParam)->code) 
				{ 

				case TVN_GETDISPINFO:
					{
						break;
					}
				case TVN_SELCHANGED:
					{
					if (!mod->suspendUI)
						{
						NM_TREEVIEW *pnmtv = (NM_TREEVIEW FAR *)lParam;									
						int whichSelected = pnmtv->itemNew.lParam;
						mod->HoldBoneSelections();
						mod->SetSelection(whichSelected,TRUE);
						macroRecorder->FunctionCall(_T("$.modifiers[#Skin_Morph].skinMorphOps.selectBone"), 2, 0,
												mr_reftarg,mod->GetNode(mod->currentBone),
												mr_string,mod->GetMorphName(mod->GetNode(mod->currentBone),mod->currentMorph));						

						}
					break;
					}
				case TVN_ENDLABELEDIT:
					{
					break;
					}

 				}
			
			break;
			}
		case WM_COMMAND:
			{
			switch (LOWORD(wParam)) 
				{

				case IDC_PICKBONE:
					{
					thePickMode.mod = mod;
					thePickMode.pickExternalNode = FALSE;
					GetCOREInterface()->SetPickMode(&thePickMode);
					break;
					}

				case IDC_ADD_BUTTON:
					{
					theHold.Begin();
					theHold.Put(new MorphUIRestore(mod));  //stupid Hack to handle UI update after redo
					mod->AddNodes();
					theHold.Put(new MorphUIRestore(mod)); //stupid Hack to handle UI update after redo
					theHold.Accept(GetString(IDS_ADDBONE));
					break;
					}
				case IDC_REMOVE_BUTTON:
					{
					macroRecorder->FunctionCall(_T("$.modifiers[#Skin_Morph].skinMorphOps.removeBone"), 1, 0,mr_reftarg,mod->GetNode(mod->currentBone));
					theHold.Begin();
					theHold.Put(new MorphUIRestore(mod));  //stupid Hack to handle UI update after redo
					if ((mod->currentBone >= 0) && (mod->currentBone < mod->boneData.Count()))
					{

						for (int i = 0; i < mod->boneData[mod->currentBone]->morphTargets.Count(); i++)
						{
							if (!mod->boneData[mod->currentBone]->morphTargets[i]->IsDead())
							{
								theHold.Put(new RemoveMorphRestore(mod,mod->boneData[mod->currentBone]->morphTargets[i]));
							}
						}
					}
					mod->DeleteBone(mod->currentBone);
					theHold.Put(new MorphUIRestore(mod)); //stupid Hack to handle UI update after redo
					theHold.Accept(GetString(IDS_REMOVEBONE));
					
					
					break;
					}

				}

			break;
			}



		}
	return FALSE;
	}



INT_PTR MorphByBoneParamsMapDlgProcProp::DlgProc(TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)		


			
	{


	switch (msg) 
		{
		case WM_INITDIALOG:
			{

			mod->iEditButton = GetICustButton(GetDlgItem(hWnd, IDC_EDITMORPH));
			mod->iEditButton->SetType(CBT_CHECK);
			mod->iEditButton->SetHighlightColor(GREEN_WASH);

			mod->iNodeButton = GetICustButton(GetDlgItem(hWnd, IDC_EXTERNALNODE));
			mod->iNodeButton->SetType(CBT_CHECK);
			mod->iNodeButton->SetHighlightColor(GREEN_WASH);



			mod->iDeleteMorphButton = GetICustButton(GetDlgItem(hWnd, IDC_DELETEMORPH_BUTTON));
			mod->iCreateMorphButton = GetICustButton(GetDlgItem(hWnd, IDC_CREATEMORPH_BUTTON));

			mod->iResetMorphButton = GetICustButton(GetDlgItem(hWnd, IDC_RESET_BUTTON));
			mod->iClearVertsButton = GetICustButton(GetDlgItem(hWnd, IDC_CLEARVERTICES));
			mod->iRemoveVertsButton = GetICustButton(GetDlgItem(hWnd, IDC_REMOVEVERTICES));
			
			mod->iReloadButton = GetICustButton(GetDlgItem(hWnd, IDC_RELOAD_BUTTON));
			mod->iGraphButton = GetICustButton(GetDlgItem(hWnd, IDC_EDITGRAPH));

			mod->iNameField = GetICustEdit(GetDlgItem(hWnd, IDC_NAME));
			mod->iInfluenceAngleSpinner = GetISpinner(GetDlgItem(hWnd, IDC_ANGLE_SPIN));
			mod->hwndFalloffDropList = GetDlgItem(hWnd, IDC_FALLOFF_COMBO);
			mod->hwndJointTypeDropList = GetDlgItem(hWnd, IDC_JOINTTYPE_COMBO);

			mod->hwndSelectedVertexCheckBox = GetDlgItem(hWnd, IDC_RELOADSELECTED_CHECK);
			mod->hwndEnabledCheckBox = GetDlgItem(hWnd, IDC_ENABLED_CHECK);

			mod->UpdateLocalUI();

			mod->rollupPropHWND = hWnd;
			break;
			}
		case WM_COMMAND:
			{
			switch (LOWORD(wParam)) 
				{

				case IDC_CLEARVERTICES:
					{
					macroRecorder->FunctionCall(_T("$.modifiers[#Skin_Morph].skinMorphOps.clearSelectedVertices"), 0, 0);
					mod->ClearSelectedVertices(FALSE);
					mod->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
					GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());

					break;
					}
				case IDC_REMOVEVERTICES:
					{
					macroRecorder->FunctionCall(_T("$.modifiers[#Skin_Morph].skinMorphOps.deleteSelectedVertices"), 0, 0);
					mod->ClearSelectedVertices(TRUE);
					mod->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
					GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());

					break;
					}


				case IDC_CREATEMORPH_BUTTON:
					{
					macroRecorder->FunctionCall(_T("$.modifiers[#Skin_Morph].skinMorphOps.createMorph"), 1, 0,mr_reftarg,mod->GetNode(mod->currentBone));
					mod->CreateMorph(mod->GetNode(mod->currentBone),TRUE);
					break;
					}
				case IDC_DELETEMORPH_BUTTON:
					{
					macroRecorder->FunctionCall(_T("$.modifiers[#Skin_Morph].skinMorphOps.removeMorph"), 2, 0,
								mr_reftarg,mod->GetNode(mod->currentBone),
								mr_string,mod->GetMorphName(mod->GetNode(mod->currentBone),mod->currentMorph)
								);
					mod->DeleteMorph(mod->currentBone, mod->currentMorph);
					mod->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
					GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());

					break;
					}

				case IDC_EDITMORPH:
					{
					if (mod->iEditButton->IsChecked())
						{
						macroRecorder->FunctionCall(_T("$.modifiers[#Skin_Morph].skinMorphOps.editMorph"), 1, 0,
												mr_bool,TRUE);
						mod->editMorph = TRUE;
						GetCOREInterface()->SetSubObjectLevel(1);
						mod->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
						GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());

						}
					else
						{
						macroRecorder->FunctionCall(_T("$.modifiers[#Skin_Morph].skinMorphOps.editMorph"), 1, 0,
												mr_bool,FALSE);

						mod->editMorph = FALSE;
						mod->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
						GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
						}
					break;
					}
				case IDC_EXTERNALNODE:
					{
					thePickMode.mod = mod;
					thePickMode.pickExternalNode = TRUE;
					GetCOREInterface()->SetPickMode(&thePickMode);
					break;
					}
				case IDC_RELOAD_BUTTON:
					{
					macroRecorder->FunctionCall(_T("$.modifiers[#Skin_Morph].skinMorphOps.reloadTarget"), 2, 0,
								mr_reftarg,mod->GetNode(mod->currentBone),
								mr_string,mod->GetMorphName(mod->GetNode(mod->currentBone),mod->currentMorph)
								);
					mod->ReloadMorph(mod->currentBone, mod->currentMorph);
					mod->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
					GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
					break;
					}
				case IDC_EDITGRAPH:
					{
					macroRecorder->FunctionCall(_T("$.modifiers[#Skin_Morph].skinMorphOps.editFalloffGraph"), 2, 0,
								mr_reftarg,mod->GetNode(mod->currentBone),
								mr_string,mod->GetMorphName(mod->GetNode(mod->currentBone),mod->currentMorph)
								);
					mod->BringUpGraph(mod->currentBone, mod->currentMorph);
					break;
					}
				case IDC_RESET_BUTTON:
					{
					macroRecorder->FunctionCall(_T("$.modifiers[#Skin_Morph].skinMorphOps.resetOrientation"), 2, 0,
								mr_reftarg,mod->GetNode(mod->currentBone),
								mr_string,mod->GetMorphName(mod->GetNode(mod->currentBone),mod->currentMorph)
								);
					mod->ResetMorph(mod->currentBone, mod->currentMorph);
					mod->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
					GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());

					break;
					}
				}

			break;
			}



		}
	return FALSE;
	}



INT_PTR MorphByBoneParamsMapDlgProcSoftSelection::DlgProc(TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)		


			
	{


	switch (msg) 
		{
		case WM_INITDIALOG:
			{
			ICurveCtl *curveCtl = mod->GetCurveCtl();
			curveCtl->SetCustomParentWnd(GetDlgItem(hWnd, IDC_CURVE));
			curveCtl->SetMessageSink(hWnd);
			curveCtl->SetActive(TRUE);

			break;
			}
		case WM_COMMAND:
			{
			switch (LOWORD(wParam)) 
				{

				case IDC_LOOP_BUTTON:
				{
					macroRecorder->FunctionCall(_T("$.modifiers[#Skin_Morph].skinMorphOps.loop"), 0, 0);
					mod->EdgeSel(TRUE);
					break;
				}
				case IDC_RING_BUTTON:
				{
					macroRecorder->FunctionCall(_T("$.modifiers[#Skin_Morph].skinMorphOps.ring"), 0, 0);
					mod->EdgeSel(FALSE);
					break;
				}


				case IDC_SHRINK_BUTTON:
				{
					macroRecorder->FunctionCall(_T("$.modifiers[#Skin_Morph].skinMorphOps.shrink"), 0, 0);
					mod->GrowVerts(FALSE);
					break;
				}
				case IDC_GROW_BUTTON:
				{
					macroRecorder->FunctionCall(_T("$.modifiers[#Skin_Morph].skinMorphOps.grow"), 0, 0);
					mod->GrowVerts(TRUE);
					break;
				}

				case IDC_RESETGRAPH_BUTTON:
					{
					macroRecorder->FunctionCall(_T("$.modifiers[#Skin_Morph].skinMorphOps.resetSelectionGraph"), 0, 0);
					mod->ResetSelectionGraph();
					break;
					}

				}

			break;
			}



		}
	return FALSE;
	}




INT_PTR MorphByBoneParamsMapDlgProcCopy::DlgProc(TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)		


			
	{


	switch (msg) 
		{
		case WM_INITDIALOG:
			{

			mod->iMultiSelectButton = GetICustButton(GetDlgItem(hWnd, IDC_ALLOWMULTIPLESELECTIONS));
			mod->iMultiSelectButton->SetType(CBT_CHECK);
			mod->iMultiSelectButton->SetHighlightColor(GREEN_WASH);


			break;
			}
		case WM_COMMAND:
			{
			switch (LOWORD(wParam)) 
				{
				case IDC_ALLOWMULTIPLESELECTIONS:
					{
/*
					if (mod->iMultiSelectButton->IsChecked())
						{
						mod->multiSelectMode = TRUE;
						mod->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
						GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());

						}
					else
						{
						mod->multiSelectMode = FALSE;
						for (int i = 0; i < mod->boneData.Count(); i++)
							mod->boneData[i]->SetMultiSelected(FALSE);

						mod->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
						GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());


						}
					break;
*/
					}

//				case IDC_COPY:
					
//					break;
				case IDC_PASTEMIRROR:
					
					macroRecorder->FunctionCall(_T("$.modifiers[#Skin_Morph].skinMorphOps.mirrorPaste"), 1, 0,
								mr_reftarg,mod->GetNode(mod->currentBone)
								);
					mod->PasteToMirror();
					mod->UpdateLocalUI();
					mod->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
					GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());

					break;

				}

			break;
			}



		}
	return FALSE;
	}




void MorphByBone::AddNodes()

{
GetCOREInterface()->DoHitByNameDialog(new DumpHitDialog(this));
void BuiltTreeList();
}



void DumpHitDialog::proc(INodeTab &nodeTab)

{


int nodeCount = nodeTab.Count(); 

if (nodeCount == 0) return;

TimeValue t = GetCOREInterface()->GetTime();

for (int i=0;i<nodeTab.Count();i++)
{
	macroRecorder->FunctionCall(_T("$.modifiers[#Skin_Morph].skinMorphOps.addBone"), 1, 0,mr_reftarg,nodeTab[i]);
	eo->AddBone(nodeTab[i]);
}
eo->BuildTreeList();
}



int DumpHitDialog::filter(INode *node)

{

	TCHAR name1[200];
	_tcscpy(name1,node->GetName());

	node->BeginDependencyTest();
	eo->NotifyDependents(FOREVER,0,REFMSG_TEST_DEPENDENCY);
	if (node->EndDependencyTest()) 
		{		
		return FALSE;
		} 
	else 
		{

		for (int i = 0;i < eo->boneData.Count(); i++)
			{
			INode *tnode = eo->GetNode(i);
			
			if  (node == tnode)
				return FALSE;


			}


		}

 	return TRUE;

}


BOOL PickControlNode::Filter(INode *node)
	{
	node->BeginDependencyTest();
	mod->NotifyDependents(FOREVER,0,REFMSG_TEST_DEPENDENCY);
	if (node->EndDependencyTest()) 
		{		
		return FALSE;
		} 

	if (pickExternalNode == FALSE)
	{
		for (int i = 0;i < mod->boneData.Count(); i++)
			{
			INode *tnode = mod->GetNode(i);
			
			if  (node == tnode)
				return FALSE;
			}
	}
	else
	{
		if (mod->localDataList.Count() && mod->localDataList[0])
		{           
			TimeValue t = GetCOREInterface()->GetTime();
			ObjectState os = node->EvalWorldState(t);
			Object* obj = os.obj;
			DbgAssert(obj != NULL);
			int ct = obj->NumPoints();
			int modCount = mod->localDataList[0]->originalPoints.Count();
			if (ct != modCount) 
			{
				return FALSE;
			}
		}
	}
	return TRUE;
	}

BOOL PickControlNode::HitTest(
		IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags)
	{	
	if (ip->PickNode(hWnd,m,this)) {
		return TRUE;
	} else {
		return FALSE;
		}
	}

BOOL PickControlNode::Pick(IObjParam *ip,ViewExp *vpt)
	{
	INode *node = vpt->GetClosestHit();
	if (node) 
		{

			if (pickExternalNode)
			{
				macroRecorder->FunctionCall(_T("$.modifiers[#Skin_Morph].skinMorphOps.setExternalNode"), 3, 0,
								mr_reftarg,mod->GetNode(mod->currentBone),
								mr_string,mod->GetMorphName(mod->GetNode(mod->currentBone),mod->currentMorph),
								mr_reftarg,node
								);
				mod->SetNode(node,mod->currentBone,mod->currentMorph);
				mod->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
				GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());

				mod->UpdateLocalUI();
				return TRUE;
			}
			else
			{

				theHold.Begin();
				theHold.Put(new MorphUIRestore(mod));  //stupid Hack to handle UI update after redo
				macroRecorder->FunctionCall(_T("$.modifiers[#Skin_Morph].skinMorphOps.addBone"), 1, 0,mr_reftarg,node);
				mod->AddBone(node);
				theHold.Put(new MorphUIRestore(mod)); //stupid Hack to handle UI update after redo
				theHold.Accept(GetString(IDS_ADDBONE));
				mod->BuildTreeList();
				return FALSE;
			}
		
		}
	return FALSE;
	}

void PickControlNode::EnterMode(IObjParam *ip)
	{

		if (pickExternalNode)
			mod->iNodeButton->SetCheck(TRUE);
		else mod->iPickBoneButton->SetCheck(TRUE);

	}
HCURSOR PickControlNode::GetDefCursor(IObjParam *ip)
	{
    static HCURSOR hCur = NULL;

    if ( !hCur ) 
		{
        hCur = LoadCursor(hInstance,MAKEINTRESOURCE(IDC_ADDBONECUR)); 
        }
	return hCur;
	}

HCURSOR PickControlNode::GetHitCursor(IObjParam *ip)
	{
    static HCURSOR hCur = NULL;

    if ( !hCur ) 
		{
        hCur = LoadCursor(hInstance,MAKEINTRESOURCE(IDC_ADDBONECUR1)); 
        }
	return hCur;
	}


void PickControlNode::ExitMode(IObjParam *ip)
	{
		if (pickExternalNode)
			mod->iNodeButton->SetCheck(FALSE);
		else mod->iPickBoneButton->SetCheck(FALSE);
	}



void MorphByBone::BringUpGraph(int whichBone, int whichMorph)
{
	if ((whichBone < 0) || (whichBone >= boneData.Count()) ||
		(whichMorph < 0) || (whichMorph >= boneData[whichBone]->morphTargets.Count()))
			{
			return;
			}	

	//see if we need to create a graph
	if (boneData[whichBone]->morphTargets[whichMorph]->externalFalloffID==-1)
		boneData[currentBone]->morphTargets[currentMorph]->CreateGraph(this,pblock);

	//get the ref
	ReferenceTarget *ref = NULL;
	int refID = boneData[whichBone]->morphTargets[whichMorph]->externalFalloffID;
	if ((refID < 0) || (refID >= pblock->Count(pb_falloffgraphlist))) return;
	pblock->GetValue(pb_falloffgraphlist,0,ref,FOREVER,refID);

	if (ref == NULL) return;

	ICurveCtl *graph = (ICurveCtl *) ref;
	//rename the window to the morph name
	graph->SetTitle(boneData[currentBone]->morphTargets[currentMorph]->name);

	graph->SetCustomParentWnd(GetDlgItem(rollupPropHWND, IDC_EDITGRAPH));
	graph->SetMessageSink(rollupPropHWND);

	//pop it up
	graph->SetCCFlags(CC_ASPOPUP|CC_DRAWBG|CC_DRAWSCROLLBARS|CC_AUTOSCROLL|CC_DRAWUTOOLBAR|
					  CC_DRAWRULER|CC_DRAWGRID|CC_DRAWLTOOLBAR|CC_SHOW_CURRENTXVAL|CC_ALL_RCMENU|CC_SINGLESELECT|CC_NOFILTERBUTTONS);
	if (graph->IsActive())
		graph->SetActive(FALSE);
	else 
		{
		graph->SetActive(TRUE);
		graph->ZoomExtents();
		}


}

