/*********



add tooltips


add soft selection

**********/

#include "mods.h"
#include "bonesdef.h"
#include "macrorec.h"

#include "3dsmaxport.h"

//Windows proc to control  the Weight Table Window
INT_PTR CALLBACK WeightToolDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{

	static ISpinnerControl *iCustom  = NULL;
	static ISpinnerControl *iScale  = NULL;
	static ISpinnerControl *iTolerance  = NULL;
	BonesDefMod *mod = DLGetWindowLongPtr<BonesDefMod*>(hWnd);
	switch (msg) {

		case WM_INITDIALOG:
			{
			//initialized custom control GLP_USERDATE to point to the owning skin mod for each custom window
			mod = (BonesDefMod*)lParam;
			DLSetWindowLongPtr(hWnd, mod);

			GetCOREInterface()->RegisterDlgWnd(hWnd);

			SendMessage(hWnd, WM_SETICON, ICON_SMALL, GetClassLongPtr(GetCOREInterface()->GetMAXHWnd(), GCLP_HICONSM)); 

			float scale,tolerance,weight;
			mod->pblock_param->GetValue(skin_weighttool_tolerance,0,tolerance,FOREVER);
			mod->pblock_param->GetValue(skin_weighttool_scale,0,scale,FOREVER);
			mod->pblock_param->GetValue(skin_weighttool_weight,0,weight,FOREVER);


			iCustom = SetupFloatSpinner(
				hWnd,IDC_CUSTOMSPIN,IDC_CUSTOM,
				0.0f,1.0f,weight);	
			iCustom->SetScale(0.05f);

			iScale = SetupFloatSpinner(
				hWnd,IDC_SCALESPIN,IDC_SCALE,
				0.001f,5.0f,scale);	
			iScale->SetScale(0.05f);
			

			iTolerance = SetupFloatSpinner(
				hWnd,IDC_TOLERANCESPIN,IDC_TOLERANCE,
				0.0f,10.0f,tolerance);	
			iTolerance->SetScale(0.1f);
			iTolerance->LinkToEdit(GetDlgItem(hWnd,IDC_TOLERANCE), EDITTYPE_POS_UNIVERSE);
			
			int ct = mod->vertexWeightCopyBuffer.copyBuffer.Count();

			if (ct == 0)
			{
				EnableWindow(GetDlgItem(hWnd,IDC_PASTE_BUTTON),FALSE);
				EnableWindow(GetDlgItem(hWnd,IDC_PASTEBYPOS_BUTTON),FALSE);
			}
			TSTR copyStatus;
			copyStatus.printf("%d %s",ct,GetString(IDS_PW_COPYSTATUS));
			SetWindowText(GetDlgItem(hWnd,IDC_COPYSTATUS), copyStatus  );

			mod->EnableWeightToolControls();

			break;
			}


		case WM_DESTROY:
			mod->SaveWeightToolDialogPos();
			mod->weightToolHWND = NULL;
			GetCOREInterface()->UnRegisterDlgWnd(hWnd);
			ReleaseISpinner(iCustom);
			iCustom = NULL;
			ReleaseISpinner(iScale);
			iScale = NULL;
			ReleaseISpinner(iTolerance);
			iTolerance = NULL;
			if (mod->iWeightTool)
				mod->iWeightTool->SetCheck(FALSE);
			EndDialog(hWnd, 0);
			break;
		case WM_CLOSE:
			DestroyWindow(hWnd);
			break;
		
		case WM_SIZE:
			{
			//anchor the scrollwindow
			WINDOWPLACEMENT mainWinPos;

			GetClientRect(hWnd , &mainWinPos.rcNormalPosition);
			int mainHeight = mainWinPos.rcNormalPosition.bottom - mainWinPos.rcNormalPosition.top;
			int mainWidth = mainWinPos.rcNormalPosition.right - mainWinPos.rcNormalPosition.left;

			WINDOWINFO pwiMain;
			GetWindowInfo(hWnd,&pwiMain);

			WINDOWINFO pwiStatus;
			GetWindowInfo(GetDlgItem(hWnd,IDC_WEIGHT_STATUS1),&pwiStatus);
			
			
			HWND listHWND = GetDlgItem(hWnd,IDC_BONELIST);
			WINDOWINFO pwi;
			GetWindowInfo(listHWND,&pwi);
			
			int x,y,w,h;
			x = 4;
			y = pwiStatus.rcWindow.bottom - pwiMain.rcClient.top +4;
			w = mainWinPos.rcNormalPosition.right - mainWinPos.rcNormalPosition.left - 8;
			h = mainWinPos.rcNormalPosition.bottom - y - 4;
			MoveWindow(listHWND,x,y,w,h, TRUE);

			break;
			}
		case WM_SIZING:
			{
 				RECT *rc = (RECT*)lParam;
				int width =  rc->right - rc->left; 
				int height = rc->bottom - rc->top;
				if (width < 181) rc->right = rc->left + 182;
				if (height < 300) rc->bottom = rc->top + 301;
				

			}
		
		case WM_CUSTEDIT_ENTER:
			{
			float scale,tolerance,weight;
			scale = iScale->GetFVal();
			tolerance = iTolerance->GetFVal();
			weight = iCustom->GetFVal();	
			mod->stopMessagePropogation = TRUE;
			mod->pblock_param->SetValue(skin_weighttool_tolerance,0,tolerance);
			mod->pblock_param->SetValue(skin_weighttool_scale,0,scale);
			mod->pblock_param->SetValue(skin_weighttool_weight,0,weight);
			mod->stopMessagePropogation = FALSE;

			break;
			}
		case CC_SPINNER_BUTTONUP:
			if (HIWORD(wParam))
			{
				float scale,tolerance,weight;
				scale = iScale->GetFVal();
				tolerance = iTolerance->GetFVal();
				weight = iCustom->GetFVal();	
				mod->stopMessagePropogation = TRUE;
				mod->pblock_param->SetValue(skin_weighttool_tolerance,0,tolerance);
				mod->pblock_param->SetValue(skin_weighttool_scale,0,scale);
				mod->pblock_param->SetValue(skin_weighttool_weight,0,weight);
				mod->stopMessagePropogation = FALSE;
			}
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam)) 
				{

				case IDC_RING:
					theHold.Begin();
					mod->EdgeSel(RING_SEL);
					theHold.Accept(GetString(IDS_PW_RING));
					macroRecorder->FunctionCall(_T("skinOps.ringSelection"), 1, 0, mr_reftarg, mod);
					break;
				case IDC_LOOP:
					theHold.Begin();
					mod->EdgeSel(LOOP_SEL);
					theHold.Accept(GetString(IDS_PW_LOOP));
					macroRecorder->FunctionCall(_T("skinOps.loopSelection"), 1, 0, mr_reftarg, mod);
					break;
				case IDC_SHRINK:
					theHold.Begin();
					mod->GrowVertSel(SHRINK_SEL);
					theHold.Accept(GetString(IDS_PW_SHRINK));
					macroRecorder->FunctionCall(_T("skinOps.shrinkSelection"), 1, 0, mr_reftarg, mod);
					break;
				case IDC_GROW:
					theHold.Begin();
					mod->GrowVertSel(GROW_SEL);
					macroRecorder->FunctionCall(_T("skinOps.growSelection"), 1, 0, mr_reftarg, mod);
					theHold.Accept(GetString(IDS_PW_GROW));
					break;

				case IDC_BLEND_BUTTON:
					{
						mod->BlurSelected();
						macroRecorder->FunctionCall(_T("skinOps.blendSelected"), 1, 0, mr_reftarg, mod );
						break;
					}

				case IDC_BONELIST:
					{
					if (HIWORD(wParam)==LBN_SELCHANGE)
						{
						int fsel;
						fsel = SendMessage(GetDlgItem(hWnd,IDC_BONELIST),LB_GETCURSEL,0,0);	
						if (fsel != -1)
						{
							if (fsel < mod->weightToolSelBoneList.Count())
							{
								mod->SelectBone(mod->weightToolSelBoneList[fsel]);
								macroRecorder->FunctionCall(_T("skinOps.selectBoneByNode"), 2,0, mr_reftarg, mod, mr_int,mod->weightToolSelBoneList[fsel] );
							}
						}

						

						}
					break;
					}
				case IDC_COPY_BUTTON:
					{
 					mod->CopyWeights();
					macroRecorder->FunctionCall(_T("skinOps.copyWeights"), 1, 0, mr_reftarg, mod );
					}
					break;
				case IDC_PASTE_BUTTON:
					{
					float tolerance = iTolerance->GetFVal();
					mod->PasteWeights(FALSE,tolerance);
					macroRecorder->FunctionCall(_T("skinOps.pasteWeights"), 1, 0, mr_reftarg, mod );

					break;
					}
				case IDC_PASTEBYPOS_BUTTON:
					{
					float tolerance = iTolerance->GetFVal();
					mod->PasteWeights(FALSE,tolerance);
					macroRecorder->FunctionCall(_T("skinOps.pasteWeightsByPos"), 2, 0, mr_reftarg, mod, mr_float, tolerance );

					break;
					}
				case IDC_ADDWEIGHT_BUTTON:
					{
 		 			mod->AddWeight(0.05f);
					macroRecorder->FunctionCall(_T("skinOps.addWeight"), 2, 0, mr_reftarg, mod, mr_float, 0.05f );
					break;
					}
				case IDC_SUBTRACTWEIGHT_BUTTON:
					{
					mod->AddWeight(-0.05f);
					macroRecorder->FunctionCall(_T("skinOps.addWeight"), 2, 0, mr_reftarg, mod, mr_float, -0.05f );
		 			break;
					}
				case IDC_SCALEWEIGHT_BUTTON:
					{
					float v = iScale->GetFVal();
					mod->ScaleWeight(v);
					macroRecorder->FunctionCall(_T("skinOps.scaleWeight"), 2, 0, mr_reftarg, mod, mr_float, v );
					break;
					}
				case IDC_ADDSCALEWEIGHT_BUTTON:
					{
 		 			mod->ScaleWeight(1.05f);
					macroRecorder->FunctionCall(_T("skinOps.scaleWeight"), 2, 0, mr_reftarg, mod, mr_float, 1.05f );
					break;
					}
				case IDC_SUBTRACTSCALEWEIGHT_BUTTON:
					{
					mod->ScaleWeight(0.95f);
					macroRecorder->FunctionCall(_T("skinOps.scaleWeight"), 2, 0, mr_reftarg, mod, mr_float, 0.95f );
		 			break;
					}


				case IDC_CUSTOMWEIGHT_BUTTON:
					{
					float v = iCustom->GetFVal();
					mod->SetWeight(v);				
					macroRecorder->FunctionCall(_T("skinOps.setWeight"), 2, 0, mr_reftarg, mod, mr_float, v );
					break;
					}
				case IDC_00_BUTTON:
					mod->SetWeight(0.0f);
					macroRecorder->FunctionCall(_T("skinOps.setWeight"), 2, 0, mr_reftarg, mod, mr_float, 0.0f );
					break;
				case IDC_01_BUTTON:
					mod->SetWeight(0.1f);
					macroRecorder->FunctionCall(_T("skinOps.setWeight"), 2, 0, mr_reftarg, mod, mr_float, 0.1f );
					break;
				case IDC_25_BUTTON:
					mod->SetWeight(0.25f);
					macroRecorder->FunctionCall(_T("skinOps.setWeight"), 2, 0, mr_reftarg, mod, mr_float, 0.25f );
					break;
				case IDC_50_BUTTON:
					mod->SetWeight(0.5f);
					macroRecorder->FunctionCall(_T("skinOps.setWeight"), 2, 0, mr_reftarg, mod, mr_float, 0.5f );
					break;
				case IDC_75_BUTTON:
					mod->SetWeight(0.75f);
					macroRecorder->FunctionCall(_T("skinOps.setWeight"), 2, 0, mr_reftarg, mod, mr_float, 0.75f );
					break;
				case IDC_09_BUTTON:
					mod->SetWeight(0.9f);
					macroRecorder->FunctionCall(_T("skinOps.setWeight"), 2, 0, mr_reftarg, mod, mr_float, 0.9f );
					break;
				case IDC_100_BUTTON:
					mod->SetWeight(1.f);
					macroRecorder->FunctionCall(_T("skinOps.setWeight"), 2, 0, mr_reftarg, mod, mr_float, 1.0f );
					break;

				}
			break;


		default:
			return FALSE;

		}
	return TRUE;
	}



void BonesDefMod::BringUpWeightTool()
{
	if (weightToolHWND == NULL)
	{
		HWND hWnd;
		hWnd = GetCOREInterface()->GetMAXHWnd();
		weightToolHWND = CreateDialogParam(hInstance,MAKEINTRESOURCE(IDD_WEIGHTTOOL_DIALOG),
							hWnd,WeightToolDlgProc,(LPARAM)this);
		SetWeightToolDialogPos();
		ShowWindow(weightToolHWND,SW_SHOW);
		UpdateWeightToolVertexStatus();
		if (iWeightTool)
			iWeightTool->SetCheck(TRUE);
		
	}
	else
	{
		DestroyWindow(weightToolHWND);
		if (iWeightTool)
			iWeightTool->SetCheck(FALSE);
	}
	GetCUIFrameMgr()->SetMacroButtonStates(false);
	

}


void BonesDefMod::SetWeight(float weight)
{
	if (ip == NULL) return;

	updateDialogs = FALSE;
	//get the current bone
	//get the vertex selection
	//weight them
	ModContextList mcList;		
	INodeTab nodes;

	ip->GetModContexts(mcList,nodes);
	int objects = mcList.Count();

	if (!theHold.IsSuspended())
		theHold.Begin();
	
	for (int i =0; i < objects; i++)
	{
		
		BoneModData *bmd = (BoneModData*)mcList[i]->localData;
		if (!theHold.IsSuspended())
			theHold.Put(new WeightRestore(this,bmd));
		bmd->validVerts.ClearAll();
		SetSelectedVertices(bmd, ModeBoneIndex, weight);		
	}
	if (!theHold.IsSuspended())
		theHold.Accept(GetString(IDS_PW_WEIGHTCHANGE));
//	Reevaluate(TRUE);
	NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
	
	ip->RedrawViews(ip->GetTime());	
	updateDialogs = TRUE;
	PaintAttribList();
}




void BonesDefMod::AddWeight(float weight)
{
	if (ip == NULL) return;

	//get the current bone
	//get the vertex selection
	//weight them

	updateDialogs = FALSE;
	ModContextList mcList;		
	INodeTab nodes;
	theHold.Begin();

	ip->GetModContexts(mcList,nodes);
	int objects = mcList.Count();
	
	for (int i =0; i < objects; i++)
	{
		
		BoneModData *bmd = (BoneModData*)mcList[i]->localData;
		theHold.Put(new WeightRestore(this,bmd));
		bmd->validVerts.ClearAll();

		int selcount = bmd->selected.GetSize();

		Tab<int> vsel;
		for (int j = 0 ; j <bmd->VertexData.Count();j++)
			{
			if (bmd->selected[j]) vsel.Append(1,&j,1);
			}

		//float effect,originaleffect;
		for ( int j = 0; j < vsel.Count();j++)
		{
			int found = 0;

			int k = vsel[j];
			float amount = 0.0f;
			int ct = bmd->VertexData[k]->WeightCount();
			BOOL hit = FALSE;
			for (int j = 0; j < ct; j++)
			{
				if (bmd->VertexData[k]->GetBoneIndex(j) == ModeBoneIndex)
				{
					amount = bmd->VertexData[k]->GetNormalizedWeight(j)+weight;
					if (amount < 0.0f) amount = 0.0f;
					if (amount > 1.0f) amount = 1.0f;

					SetVertex(bmd,k, ModeBoneIndex, amount);
					hit = TRUE;
				}
				}
			if (!hit)
			{
				if (weight >= 0.0f)
					SetVertex(bmd,k, ModeBoneIndex, weight);
			}
		}

	
	}


  	theHold.Accept(GetString(IDS_PW_WEIGHTCHANGE));
//	Reevaluate(TRUE);
	
	NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);

	ip->RedrawViews(ip->GetTime());	
	
	updateDialogs = TRUE;
	PaintAttribList();
	
	
}

void BonesDefMod::ScaleWeight(float scale)
{
	if (ip == NULL) return;

	//get the current bone
	//get the vertex selection
	//weight them
	ModContextList mcList;		
	INodeTab nodes;

	updateDialogs = FALSE;

	ip->GetModContexts(mcList,nodes);
	int objects = mcList.Count();
	theHold.Begin();
	
	for (int i =0; i < objects; i++)
	{
		
		BoneModData *bmd = (BoneModData*)mcList[i]->localData;
		theHold.Put(new WeightRestore(this,bmd));
		bmd->validVerts.ClearAll();

		int selcount = bmd->selected.GetSize();

		Tab<int> vsel;
		for (int j = 0 ; j <bmd->VertexData.Count();j++)
			{
			if (bmd->selected[j]) vsel.Append(1,&j,1);
			}

		//float effect,originaleffect;
		for ( int j = 0; j < vsel.Count();j++)
		{
			int found = 0;

			int k = vsel[j];
			float amount = 0.0f;
			int ct = bmd->VertexData[k]->WeightCount();
			for (int l = 0; l < ct; l++)
			{
				if (bmd->VertexData[k]->GetBoneIndex(l) == ModeBoneIndex)
				{
					amount = bmd->VertexData[k]->GetNormalizedWeight(l);
					amount *= scale;	
					if (amount < 0.0f) amount = 0.0f;
					if (amount > 1.0f) amount = 1.0f;
					SetVertex(bmd,k, ModeBoneIndex, amount);
				}
			}
		}

	
	}

	theHold.Accept(GetString(IDS_PW_WEIGHTCHANGE));
//	Reevaluate(TRUE);
	NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
	
	ip->RedrawViews(ip->GetTime());	

	updateDialogs = TRUE;
	PaintAttribList();
}

void BonesDefMod::CopyWeights()
{
	vertexWeightCopyBuffer.FreeCopyBuffer();

	if (ip == NULL) return;

	//get the current bone
	//get the vertex selection
	//weight them
	ModContextList mcList;		
	INodeTab nodes;

	ip->GetModContexts(mcList,nodes);
	int objects = mcList.Count();
	
	int s = 0;
	for (int i =0; i < objects; i++)
	{
		
		BoneModData *bmd = (BoneModData*)mcList[i]->localData;
		
		s += bmd->selected.NumberSet();
	}
	vertexWeightCopyBuffer.copyBuffer.SetCount(s);

	s = 0;
	TimeValue t = ip->GetTime();
	for (int i =0; i < objects; i++)
	{
		Matrix3 tm = nodes[i]->GetObjectTM(t);
		BoneModData *bmd = (BoneModData*)mcList[i]->localData;
		for (int j = 0; j < bmd->VertexData.Count(); j++)
		{
			if (bmd->selected[j])
			{
				vertexWeightCopyBuffer.copyBuffer[s] = new VertexListClass();
				vertexWeightCopyBuffer.copyBuffer[s]->flags =  bmd->VertexData[j]->flags;
				vertexWeightCopyBuffer.copyBuffer[s]->PasteWeights(bmd->VertexData[j]->CopyWeights());
				vertexWeightCopyBuffer.copyBuffer[s]->LocalPos =  bmd->VertexData[j]->LocalPos * tm;
				vertexWeightCopyBuffer.copyBuffer[s]->LocalPosPostDeform =  bmd->VertexData[j]->LocalPosPostDeform;							
				s++;
			}
		}
		
		
	}
	vertexWeightCopyBuffer.bones.SetCount(BoneData.Count());
	for (int i =0; i < BoneData.Count(); i++)
	{
		vertexWeightCopyBuffer.bones[i] = BoneData[i].Node;
	}

	if (weightToolHWND)
	{
		int ct = vertexWeightCopyBuffer.copyBuffer.Count();
		BOOL enable = TRUE;
		if (ct == 0) enable = FALSE;

		EnableWindow(GetDlgItem(weightToolHWND,IDC_PASTE_BUTTON),enable);
		EnableWindow(GetDlgItem(weightToolHWND,IDC_PASTEBYPOS_BUTTON),enable);

		TSTR copyStatus;
		copyStatus.printf("%d %s",ct,GetString(IDS_PW_COPYSTATUS));
		SetWindowText(GetDlgItem(weightToolHWND,IDC_COPYSTATUS), copyStatus  );
	}


}
void BonesDefMod::PasteWeights(BOOL byPosition, float tolerance)
{
	if (ip == NULL) return;

	if (vertexWeightCopyBuffer.copyBuffer.Count() == 0) return;

	//get the current bone
	//get the vertex selection
	//weight them
	ModContextList mcList;		
	INodeTab nodes;

	ip->GetModContexts(mcList,nodes);
	int objects = mcList.Count();
	theHold.Begin();
	
	int currentPasteBufferIndex = 0;

	Tab<int> lookupTable;
	lookupTable.SetCount(vertexWeightCopyBuffer.bones.Count());
	for (int i = 0; i < vertexWeightCopyBuffer.bones.Count(); i++)
	{
		lookupTable[i] = -1;
		for (int j = 0; j < BoneData.Count(); j++)
		{
			if (BoneData[j].Node == vertexWeightCopyBuffer.bones[i])
			{
				lookupTable[i] = j;
				j = BoneData.Count();
			}
		}
	}

	float toleranceSquared = tolerance * tolerance;

	TimeValue t = ip->GetTime();
	for (int i =0; i < objects; i++)
	{
		
 		BoneModData *bmd = (BoneModData*)mcList[i]->localData;
		bmd->rebuildWeights = TRUE;
		theHold.Put(new WeightRestore(this,bmd));

		if (byPosition)
		{
			Box3 bounds;
			bounds.Init();

			Matrix3 tm = nodes[i]->GetObjectTM(t);

			for (int k = 0; k < vertexWeightCopyBuffer.copyBuffer.Count(); k++)
			{
				bounds += vertexWeightCopyBuffer.copyBuffer[k]->LocalPos;
			}
			bounds.EnlargeBy(tolerance);

			Tab<int> potentialMatches;
			for (int j = 0; j < bmd->VertexData.Count(); j++)
			{
				if (bmd->selected[j])
				{
					Point3 p = bmd->VertexData[j]->LocalPos * tm;
					if (bounds.Contains(p))
						potentialMatches.Append(1,&j,300);
				}
			}
			for (int i = 0; i < potentialMatches.Count(); i++)
			{
				int index = potentialMatches[i];
				int hitIndex = -1;
				float closestDist = 0.0f;
				Point3 p = bmd->VertexData[index]->LocalPos * tm;
				for (int j = 0; j < vertexWeightCopyBuffer.copyBuffer.Count(); j++)
				{
					Point3 cp = vertexWeightCopyBuffer.copyBuffer[j]->LocalPos;
					float d = LengthSquared(cp-p);
					if ((d < closestDist) || (hitIndex == -1))
					{
						hitIndex = j;
						closestDist = d;
					}
				}
				if ((hitIndex != -1) && (closestDist <= toleranceSquared))
				{
					int j = hitIndex;
					bmd->VertexData[j]->flags = vertexWeightCopyBuffer.copyBuffer[currentPasteBufferIndex]->flags;
					bmd->VertexData[j]->Modified(TRUE);
					bmd->VertexData[j]->ZeroWeights();
					for (int k = 0; k < vertexWeightCopyBuffer.copyBuffer[currentPasteBufferIndex]->WeightCount(); k++)
					{
						int copyBoneIndex = vertexWeightCopyBuffer.copyBuffer[currentPasteBufferIndex]->GetBoneIndex(k);
						int newBoneIndex = lookupTable[copyBoneIndex];
						if (newBoneIndex != -1)
						{
							VertexListClass *source = vertexWeightCopyBuffer.copyBuffer[currentPasteBufferIndex];
							VertexListClass *target = bmd->VertexData[j];
							bmd->VertexData[j]->AppendWeight(vertexWeightCopyBuffer.copyBuffer[currentPasteBufferIndex]->CopySingleWeight(k));
							int ct = bmd->VertexData[j]->WeightCount() -1;
							bmd->VertexData[j]->SetBoneIndex(ct, newBoneIndex);
						}
					}
					bmd->VertexData[j]->NormalizeWeights();
					currentPasteBufferIndex++;
					if (currentPasteBufferIndex >= vertexWeightCopyBuffer.copyBuffer.Count())
						currentPasteBufferIndex = 0;

				}
			}

		}
		else
		{
			for (int j = 0; j < bmd->VertexData.Count(); j++)
			{
				if (bmd->selected[j])
				{
					bmd->VertexData[j]->flags = vertexWeightCopyBuffer.copyBuffer[currentPasteBufferIndex]->flags;
					bmd->VertexData[j]->Modified(TRUE);
					bmd->VertexData[j]->ZeroWeights();
					for (int k = 0; k < vertexWeightCopyBuffer.copyBuffer[currentPasteBufferIndex]->WeightCount(); k++)
					{
						int copyBoneIndex = vertexWeightCopyBuffer.copyBuffer[currentPasteBufferIndex]->GetBoneIndex(k);
						int newBoneIndex = lookupTable[copyBoneIndex];
						if (newBoneIndex != -1)
						{
							VertexListClass *source = vertexWeightCopyBuffer.copyBuffer[currentPasteBufferIndex];
							VertexListClass *target = bmd->VertexData[j];
							bmd->VertexData[j]->AppendWeight(vertexWeightCopyBuffer.copyBuffer[currentPasteBufferIndex]->CopySingleWeight(k));
							int ct = bmd->VertexData[j]->WeightCount() -1;
							bmd->VertexData[j]->SetBoneIndex(ct, newBoneIndex);
						}
					}
					bmd->VertexData[j]->NormalizeWeights();
					currentPasteBufferIndex++;
					if (currentPasteBufferIndex >= vertexWeightCopyBuffer.copyBuffer.Count())
						currentPasteBufferIndex = 0;

				}
			}
		}
	}
	theHold.Accept(GetString(IDS_PW_WEIGHTCHANGE));
	Reevaluate(TRUE);
	NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
	
	ip->RedrawViews(ip->GetTime());	
}


void BonesDefMod::UpdateWeightToolVertexStatus()
{
	if ((weightToolHWND == NULL) || (ip == NULL)) return;

	if (ip == NULL) return;

	EnableWeightToolControls();
	//get the current bone
	//get the vertex selection
	//weight them
	ModContextList mcList;		
	INodeTab nodes;

	ip->GetModContexts(mcList,nodes);
	int objects = mcList.Count();
	
	int numberSelected = 0;
	BOOL first = TRUE;

	TSTR blank;
	blank.printf("   ");
	SetWindowText(GetDlgItem(weightToolHWND,IDC_WEIGHT_STATUS1), blank  );
/*	SetWindowText(GetDlgItem(weightToolHWND,IDC_WEIGHT_STATUS2), blank  );
	SetWindowText(GetDlgItem(weightToolHWND,IDC_WEIGHT_STATUS3), blank  );
	SetWindowText(GetDlgItem(weightToolHWND,IDC_WEIGHT_STATUS4), blank  );
	SetWindowText(GetDlgItem(weightToolHWND,IDC_WEIGHT_STATUS5), blank  );
	SetWindowText(GetDlgItem(weightToolHWND,IDC_WEIGHT_STATUS6), blank  );
	SetWindowText(GetDlgItem(weightToolHWND,IDC_WEIGHT_STATUS7), blank  );


	SetWindowText(GetDlgItem(weightToolHWND,IDC_WEIGHTA_STATUS1), blank  );
	SetWindowText(GetDlgItem(weightToolHWND,IDC_WEIGHTA_STATUS2), blank  );
	SetWindowText(GetDlgItem(weightToolHWND,IDC_WEIGHTA_STATUS3), blank  );
	SetWindowText(GetDlgItem(weightToolHWND,IDC_WEIGHTA_STATUS4), blank  );
	SetWindowText(GetDlgItem(weightToolHWND,IDC_WEIGHTA_STATUS5), blank  );
	SetWindowText(GetDlgItem(weightToolHWND,IDC_WEIGHTA_STATUS6), blank  );
	SetWindowText(GetDlgItem(weightToolHWND,IDC_WEIGHTA_STATUS7), blank  );

*/

	SendMessage(GetDlgItem(weightToolHWND,IDC_BONELIST),LB_RESETCONTENT ,0,0);

	int selIndex = -1;
	weightToolSelBoneList.ZeroCount();

	HDC hdc = GetDC(GetDlgItem(weightToolHWND,IDC_BONELIST));
	HFONT hOldFont = (HFONT)SelectObject(hdc, GetCOREInterface()->GetAppHFont());
	int width = 0;

	for (int i =0; i < objects; i++)
	{
		
		BoneModData *bmd = (BoneModData*)mcList[i]->localData;
//		theHold.Put(new WeightRestore(this,bmd));
		if (bmd == NULL) continue;
		bmd->validVerts.ClearAll();

		numberSelected += bmd->selected.NumberSet();

		if (first)
		{
			for (int i = 0 ; i < bmd->VertexData.Count();i++)
			{	
				if (bmd->selected[i] && (first)) 
				{
					int ct = bmd->VertexData[i]->WeightCount();
					int total = ct;
//					if (ct > 6) ct = 6;
					int statusID = 0;
					for (int j = 0; j < ct; j++)
					{
						int boneID =  bmd->VertexData[i]->GetBoneIndex(j);
						float v = bmd->VertexData[i]->GetNormalizedWeight(j);
						INode *node = NULL;
						if ((boneID >= 0) && (boneID < BoneData.Count()))
							node = BoneData[boneID].Node;
						if (node )
						{
							TSTR status;
							if (boneID == ModeBoneIndex)
								selIndex = j;
							weightToolSelBoneList.Append(1,&boneID,100);
							status.printf("%1.3f : %s",v,node->GetName());

							SIZE size;		
							DLGetTextExtent(hdc,  (LPCTSTR)status,&size);

							if (size.cx > width)
								width = size.cx;

//							else status.printf("%1.3f | %s\n",v,node->GetName());
//							TSTR statusA;
//							statusA.printf("%1.3f\n",v);
							SendMessage(GetDlgItem(weightToolHWND,IDC_BONELIST),LB_ADDSTRING,0,(LPARAM)(TCHAR*)status);							
 //
//							SendMessage(GetDlgItem(weightToolHWND,IDC_BONELIST),LB_SETCURSEL,FALSE,j);
						}
					}
					first = FALSE;
				}
			}
		}
	}

 	SendDlgItemMessage(weightToolHWND, IDC_BONELIST, LB_SETHORIZONTALEXTENT, (width+8), 0);

	SelectObject(hdc, hOldFont);
	ReleaseDC(GetDlgItem(weightToolHWND,IDC_BONELIST),hdc);

 //DebugPrint("index %d\n",selIndex);
//	if (selIndex != -1)
	SendMessage(GetDlgItem(weightToolHWND,IDC_BONELIST),LB_SETCURSEL,selIndex,0);							
	
	TSTR status;
	status.printf("%d %s\n",numberSelected,GetString(IDS_VERT));	
	SetWindowText(GetDlgItem(weightToolHWND,IDC_WEIGHT_STATUS1), status  );
}



void BonesDefMod::BlurSelected()
{
	if (ip == NULL) return;

	ModContextList mcList;		
	INodeTab nodes;


	ip->GetModContexts(mcList,nodes);
	int objects = mcList.Count();

	theHold.Begin();
	for ( int i = 0; i < objects; i++ ) 
	{
		BoneModData *bmd = (BoneModData*)mcList[i]->localData;
		theHold.Put(new WeightRestore(this,bmd));
	}
	theHold.Accept(GetString(IDS_PW_WEIGHTCHANGE));
	
	for (int i = 0; i < objects; i++)
	{
		BoneModData *bmd = (BoneModData*)mcList[i]->localData;
		if (bmd)
		{

			bmd->BuildEdgeList();
			bmd->BuildBlurData(ModeBoneIndex);
			bmd->rebuildWeights = TRUE;

			for (int i = 0; i < bmd->VertexData.Count(); i++)
			{
				if (bmd->selected[i])
				{
					int numberOfBones = bmd->VertexData[i]->WeightCount();
						
					BOOL hit = FALSE;
					int whichIndex = -1;
					
					for (int k = 0; k <numberOfBones; k++)
					{
						int boneID = bmd->VertexData[i]->GetBoneIndex(k);
						if (boneID == ModeBoneIndex)
						{
							hit = TRUE;
							whichIndex = k;
						}
					}
					

					{
						//get the remainder
						float newWeight;
						if (whichIndex != -1)
							newWeight =  bmd->VertexData[i]->GetNormalizedWeight(whichIndex);
						else
						{
							newWeight = 0.0f;
						}
						float initialRemainder = 1.0f - newWeight;
						if (i >= bmd->blurredWeights.Count()) continue;
						newWeight = bmd->blurredWeights[i];
						if (newWeight == -1.0f) continue;
						float remainder = 1.0f-newWeight;

						int id = -1;
						VertexListClass *vi = bmd->VertexData[i];
						for (int k = 0; k <numberOfBones; k++)
						{
							int boneID = bmd->VertexData[i]->GetBoneIndex(k);
							if (boneID != ModeBoneIndex)
							{
								float w = bmd->VertexData[i]->GetNormalizedWeight(k);
								if (initialRemainder != 0.0f)
								{
									w = w/initialRemainder;
									w = w * remainder;
									bmd->VertexData[i]->SetWeight(k, w);
								}
								else bmd->VertexData[i]->SetWeight(k, 0.0f);
							}
							else id = k;
						}
			
						if (id != -1)
						{							
							bmd->VertexData[i]->SetWeight(id, newWeight);
							bmd->VertexData[i]->Modified(TRUE);
							bmd->VertexData[i]->NormalizeWeights();
						}
						else 
						{
							if (newWeight != 0.0f)
							{
								VertexInfluenceListClass t;
								t.Bones = ModeBoneIndex;
								t.Influences = newWeight;
								t.normalizedInfluences = newWeight;
								bmd->VertexData[i]->AppendWeight(t);
								bmd->VertexData[i]->Modified(TRUE);
								bmd->VertexData[i]->NormalizeWeights();
							}


						}
						


					}
				}

			}			
		}
	NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
	GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());

	}
}


void BonesDefMod::SetWeightToolDialogPos()

{
	if (weightToolWindowPos.length != 0) 
	{
		HWND maxHWND = GetCOREInterface()->GetMAXHWnd();
		SetWindowPlacement(weightToolHWND,&weightToolWindowPos);
	}
}

void BonesDefMod::SaveWeightToolDialogPos()
{
	weightToolWindowPos.length = sizeof(WINDOWPLACEMENT); 
	GetWindowPlacement(weightToolHWND,&weightToolWindowPos);
}


void BonesDefMod::EnableWeightToolControls()
{
	BOOL enable = TRUE;

	ModContextList mcList;		
	INodeTab nodes;

	ip->GetModContexts(mcList,nodes);
	int objects = mcList.Count();
	
	
	int numSelected = 0;
	for (int i =0; i < objects; i++)
	{
		
		BoneModData *bmd = (BoneModData*)mcList[i]->localData;
		numSelected += bmd->selected.NumberSet();
	}
	if (numSelected == 0)
		enable = FALSE;

	if ((ip) && (ip->GetSubObjectLevel() == 0))
		enable = FALSE;

	if (weightToolHWND)
	{
		EnableWindow(GetDlgItem(weightToolHWND,IDC_00_BUTTON),enable);
		EnableWindow(GetDlgItem(weightToolHWND,IDC_01_BUTTON),enable);
		EnableWindow(GetDlgItem(weightToolHWND,IDC_25_BUTTON),enable);
		EnableWindow(GetDlgItem(weightToolHWND,IDC_50_BUTTON),enable);
		EnableWindow(GetDlgItem(weightToolHWND,IDC_75_BUTTON),enable);
		EnableWindow(GetDlgItem(weightToolHWND,IDC_90_BUTTON),enable);
		EnableWindow(GetDlgItem(weightToolHWND,IDC_100_BUTTON),enable);

		
		EnableWindow(GetDlgItem(weightToolHWND,IDC_CUSTOMWEIGHT_BUTTON),enable);
		EnableWindow(GetDlgItem(weightToolHWND,IDC_ADDWEIGHT_BUTTON),enable);
		EnableWindow(GetDlgItem(weightToolHWND,IDC_SUBTRACTWEIGHT_BUTTON),enable);					

		EnableWindow(GetDlgItem(weightToolHWND,IDC_SCALEWEIGHT_BUTTON),enable);					
		EnableWindow(GetDlgItem(weightToolHWND,IDC_ADDSCALEWEIGHT_BUTTON),enable);					
		EnableWindow(GetDlgItem(weightToolHWND,IDC_SUBTRACTSCALEWEIGHT_BUTTON),enable);					

		EnableWindow(GetDlgItem(weightToolHWND,IDC_COPY_BUTTON),enable);					
		EnableWindow(GetDlgItem(weightToolHWND,IDC_PASTE_BUTTON),enable);					
		EnableWindow(GetDlgItem(weightToolHWND,IDC_PASTEBYPOS_BUTTON),enable);					
		EnableWindow(GetDlgItem(weightToolHWND,IDC_BLEND_BUTTON),enable);	

		BOOL FilterVertices;
		pblock_param->GetValue(skin_filter_vertices,0,FilterVertices,FOREVER);
		if (!FilterVertices)
		{
			EnableWindow(GetDlgItem(weightToolHWND,IDC_SHRINK),FALSE);
			EnableWindow(GetDlgItem(weightToolHWND,IDC_GROW),FALSE);
			EnableWindow(GetDlgItem(weightToolHWND,IDC_LOOP),FALSE);
			EnableWindow(GetDlgItem(weightToolHWND,IDC_RING),FALSE);

			EnableWindow(GetDlgItem(hParam,IDC_RING),FALSE);
			EnableWindow(GetDlgItem(hParam,IDC_LOOP),FALSE);
			EnableWindow(GetDlgItem(hParam,IDC_GROW),FALSE);
			EnableWindow(GetDlgItem(hParam,IDC_SHRINK),FALSE);


		}
		else
		{
			EnableWindow(GetDlgItem(weightToolHWND,IDC_SHRINK),enable);
			EnableWindow(GetDlgItem(weightToolHWND,IDC_GROW),enable);
			EnableWindow(GetDlgItem(weightToolHWND,IDC_LOOP),enable);
			EnableWindow(GetDlgItem(weightToolHWND,IDC_RING),enable);

			EnableWindow(GetDlgItem(hParam,IDC_RING),enable);
			EnableWindow(GetDlgItem(hParam,IDC_LOOP),enable);
			EnableWindow(GetDlgItem(hParam,IDC_GROW),enable);
			EnableWindow(GetDlgItem(hParam,IDC_SHRINK),enable);


		}

		ISpinnerControl *iSpin = GetISpinner(GetDlgItem(weightToolHWND,IDC_CUSTOMSPIN));
		if (iSpin)
		{
			iSpin->Enable(enable);
			ReleaseISpinner(iSpin);
		}

		iSpin = GetISpinner(GetDlgItem(weightToolHWND,IDC_SCALESPIN));
		if (iSpin)
		{
			iSpin->Enable(enable);
			ReleaseISpinner(iSpin);
		}

		iSpin = GetISpinner(GetDlgItem(weightToolHWND,IDC_TOLERANCESPIN));
		if (iSpin)
		{
			iSpin->Enable(enable);
			ReleaseISpinner(iSpin);
		}

	}

}
