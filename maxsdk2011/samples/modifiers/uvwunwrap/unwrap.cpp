/********************************************************************** *<
FILE: unwrap.cpp

DESCRIPTION: A UVW map modifier unwraps the UVWs onto the image

HISTORY: 12/31/96
CREATED BY: Rolf Berteig
UPDATED Sept. 16, 1998 Peter Watje



*>	Copyright (c) 1998, All Rights Reserved.
**********************************************************************/




#include "unwrap.h"
#include "buildver.h"
#include "modstack.h"
#include "IDxMaterial.h"

#include "3dsmaxport.h"


PeltPointToPointMode* UnwrapMod::peltPointToPointMode   = NULL;
TweakMode* UnwrapMod::tweakMode = NULL;

class MyEnumProc : public DependentEnumProc 
{
public :
	virtual int proc(ReferenceMaker *rmaker); 
	INodeTab Nodes;              
};

int MyEnumProc::proc(ReferenceMaker *rmaker) 
{ 
	if (rmaker->SuperClassID()==BASENODE_CLASS_ID)    
	{
		Nodes.Append(1, (INode **)&rmaker);  
		return DEP_ENUM_SKIP;
	}

	return DEP_ENUM_CONTINUE;
}


static void PreSave (void *param, NotifyInfo *info)
{
	UnwrapMod *mod = (UnwrapMod*) param;
	if (mod->CurrentMap == 0)
		mod->ShowCheckerMaterial(FALSE);
}

static void PostSave (void *param, NotifyInfo *info)
{
	UnwrapMod *mod = (UnwrapMod*) param;
	if ((mod->CurrentMap == 0) && (mod->checkerWasShowing))
		mod->ShowCheckerMaterial(TRUE);
}





static GenSubObjType SOT_SelFace(19);
static GenSubObjType SOT_SelVerts(1);
static GenSubObjType SOT_SelEdges(2);
static GenSubObjType SOT_SelGizmo(14);

void UnwrapMatrixFromNormal(Point3& normal, Matrix3& mat);









HCURSOR PaintSelectMode::GetXFormCur() 
{
	return mod->zoomRegionCur;
}


HCURSOR ZoomRegMode::GetXFormCur() 
{
	return mod->zoomRegionCur;
}


HCURSOR ZoomMode::GetXFormCur() 
{
	return mod->zoomCur;
}


HCURSOR PanMode::GetXFormCur() 
{
	return mod->panCur;
}


HCURSOR RotateMode::GetXFormCur() 
{
	return mod->rotCur;
}


HCURSOR WeldMode::GetXFormCur() 
{
	return mod->weldCur;
}

HCURSOR MoveMode::GetXFormCur()
{		
	if (mod->move==1)
		return mod->moveXCur;
	else if (mod->move==2) return mod->moveYCur;
	return mod->moveCur;
}

HCURSOR ScaleMode::GetXFormCur()
{		
	if (mod->scale==1)
		return mod->scaleXCur;
	else if (mod->scale==2) return mod->scaleYCur;
	return mod->scaleCur;

}

HCURSOR FreeFormMode::GetXFormCur()	
{
	if (mod->freeFormSubMode == ID_TOOL_MOVE)
		return mod->moveCur;
	else if (mod->freeFormSubMode == ID_TOOL_SCALE)
		return mod->scaleCur;
	else if (mod->freeFormSubMode == ID_TOOL_ROTATE)
		return mod->rotCur;
	else if (mod->freeFormSubMode == ID_TOOL_MOVEPIVOT)
		return LoadCursor(NULL, IDC_SIZEALL);

	else return mod->selCur;
}




class UVWUnwrapDeleteEvent : public EventUser {
public:
	UnwrapMod *m;

	void Notify() {if (m) 
	{
		m->DeleteSelected();
		m->InvalidateView();

	}
	}
	void SetEditMeshMod(UnwrapMod *im) {m=im;}
};

UVWUnwrapDeleteEvent delEvent;







BOOL			UnwrapMod::executedStartUIScript = FALSE;

HWND            UnwrapMod::hOptionshWnd = NULL;
HWND            UnwrapMod::hSelRollup = NULL;
HWND            UnwrapMod::hParams = NULL;
HWND            UnwrapMod::hWnd = NULL;
HWND            UnwrapMod::hView = NULL;
IObjParam      *UnwrapMod::ip = NULL;
ICustToolbar   *UnwrapMod::iTool = NULL;
ICustToolbar   *UnwrapMod::iVertex = NULL;
ICustToolbar   *UnwrapMod::iView = NULL;
ICustToolbar   *UnwrapMod::iOption = NULL;
ICustToolbar   *UnwrapMod::iFilter = NULL;
ICustButton    *UnwrapMod::iMove = NULL;
ICustButton    *UnwrapMod::iRot = NULL;
ICustButton    *UnwrapMod::iScale = NULL;
ICustButton    *UnwrapMod::iFalloff = NULL;
ICustButton    *UnwrapMod::iFalloffSpace = NULL;
ICustButton    *UnwrapMod::iFreeForm = NULL;


ICustButton    *UnwrapMod::iMirror = NULL;
ICustButton    *UnwrapMod::iWeld = NULL;
ICustButton    *UnwrapMod::iPan = NULL;
ICustButton    *UnwrapMod::iZoom = NULL;
ICustButton    *UnwrapMod::iUpdate = NULL;
ISpinnerControl *UnwrapMod::iU = NULL;
ISpinnerControl *UnwrapMod::iV = NULL;
ISpinnerControl *UnwrapMod::iW = NULL;
ISpinnerControl *UnwrapMod::iStr = NULL;
ISpinnerControl *UnwrapMod::iMapID = NULL;
ISpinnerControl *UnwrapMod::iPlanarThreshold = NULL;
ISpinnerControl *UnwrapMod::iMatID = NULL;
ISpinnerControl *UnwrapMod::iSG = NULL;

ICustToolbar   *UnwrapMod::iUVWSpinBar = NULL;
ICustButton    *UnwrapMod::iUVWSpinAbsoluteButton = NULL;

MouseManager    UnwrapMod::mouseMan;
CopyPasteBuffer UnwrapMod::copyPasteBuffer;
IOffScreenBuf  *UnwrapMod::iBuf = NULL;
int             UnwrapMod::mode = ID_FREEFORMMODE;
int             UnwrapMod::oldMode = ID_FREEFORMMODE;

MoveMode       *UnwrapMod::moveMode = NULL;
RotateMode     *UnwrapMod::rotMode = NULL;
ScaleMode      *UnwrapMod::scaleMode = NULL;
PanMode        *UnwrapMod::panMode = NULL;
ZoomMode       *UnwrapMod::zoomMode = NULL;
ZoomRegMode    *UnwrapMod::zoomRegMode = NULL;
WeldMode       *UnwrapMod::weldMode = NULL;
//PELT
PeltStraightenMode       *UnwrapMod::peltStraightenMode = NULL;
RightMouseMode *UnwrapMod::rightMode = NULL;
MiddleMouseMode *UnwrapMod::middleMode = NULL;
FreeFormMode   *UnwrapMod::freeFormMode = NULL;
SketchMode		*UnwrapMod::sketchMode = NULL;

BOOL            UnwrapMod::viewValid = FALSE;
BOOL            UnwrapMod::typeInsValid = FALSE;
UnwrapMod      *UnwrapMod::editMod = NULL;
ICustButton    *UnwrapMod::iZoomReg = NULL;
ICustButton    *UnwrapMod::iZoomExt = NULL;
ICustButton    *UnwrapMod::iUVW = NULL;
ICustButton    *UnwrapMod::iProp = NULL;
ICustButton    *UnwrapMod::iShowMap = NULL;
ICustButton    *UnwrapMod::iLockSelected = NULL;
ICustButton    *UnwrapMod::iFilterSelected = NULL;
ICustButton    *UnwrapMod::iHide = NULL;
ICustButton    *UnwrapMod::iFreeze = NULL;
ICustButton    *UnwrapMod::iIncSelected = NULL;
ICustButton    *UnwrapMod::iDecSelected = NULL;
ICustButton    *UnwrapMod::iSnap = NULL;

ICustButton    *UnwrapMod::iBreak = NULL;
ICustButton    *UnwrapMod::iWeldSelected = NULL;



SelectModBoxCMode *UnwrapMod::selectMode      = NULL;
MoveModBoxCMode   *UnwrapMod::moveGizmoMode      = NULL;
RotateModBoxCMode *UnwrapMod::rotGizmoMode       = NULL;
UScaleModBoxCMode *UnwrapMod::uscaleGizmoMode    = NULL;
NUScaleModBoxCMode *UnwrapMod::nuscaleGizmoMode   = NULL;
SquashModBoxCMode *UnwrapMod::squashGizmoMode    = NULL;


PaintSelectMode *UnwrapMod::paintSelectMode      = NULL;



COLORREF UnwrapMod::lineColor = RGB(255,255,255);
COLORREF UnwrapMod::selColor  = RGB(255,0,0);
COLORREF UnwrapMod::openEdgeColor = RGB(0,255,0);
COLORREF UnwrapMod::handleColor = RGB(255,255,0);
COLORREF UnwrapMod::freeFormColor = RGB(255,100,50);
COLORREF UnwrapMod::sharedColor = RGB(0,0,255);

COLORREF UnwrapMod::backgroundColor = RGB(60,60,60);

float UnwrapMod::weldThreshold = 0.01f;
BOOL UnwrapMod::update = FALSE;
int UnwrapMod::showVerts = 0;
int UnwrapMod::midPixelSnap = 0;

//watje tile
IOffScreenBuf  *UnwrapMod::iTileBuf = NULL;
BOOL		   UnwrapMod::tileValid = FALSE;


HCURSOR UnwrapMod::selCur   = NULL;
HCURSOR UnwrapMod::moveCur  = NULL;
HCURSOR UnwrapMod::moveXCur  = NULL;
HCURSOR UnwrapMod::moveYCur  = NULL;
HCURSOR UnwrapMod::rotCur   = NULL;
HCURSOR UnwrapMod::scaleCur = NULL;
HCURSOR UnwrapMod::scaleXCur = NULL;
HCURSOR UnwrapMod::scaleYCur = NULL;

HCURSOR UnwrapMod::zoomCur = NULL;
HCURSOR UnwrapMod::zoomRegionCur = NULL;
HCURSOR UnwrapMod::panCur = NULL;
HCURSOR UnwrapMod::weldCur = NULL;
HCURSOR UnwrapMod::weldCurHit = NULL;
HCURSOR UnwrapMod::sketchCur = NULL;
HCURSOR UnwrapMod::sketchPickCur = NULL;
HCURSOR UnwrapMod::sketchPickHitCur = NULL;
HWND UnwrapMod::hRelaxDialog = NULL;

/*
HCURSOR UnwrapMod::circleCur[0] = NULL;
HCURSOR UnwrapMod::circleCur[1] = NULL;
HCURSOR UnwrapMod::circleCur[2] = NULL;
HCURSOR UnwrapMod::circleCur[3] = NULL;
HCURSOR UnwrapMod::circleCur[4] = NULL;
HCURSOR UnwrapMod::circleCur[5] = NULL;
HCURSOR UnwrapMod::circleCur[6] = NULL;
HCURSOR UnwrapMod::circleCur[7] = NULL;
HCURSOR UnwrapMod::circleCur[8] = NULL;
HCURSOR UnwrapMod::circleCur[9] = NULL;
HCURSOR UnwrapMod::circleCur[10] = NULL;
HCURSOR UnwrapMod::circleCur[11] = NULL;
HCURSOR UnwrapMod::circleCur[12] = NULL;
HCURSOR UnwrapMod::circleCur[13] = NULL;
HCURSOR UnwrapMod::circleCur[14] = NULL;
HCURSOR UnwrapMod::circleCur[15] = NULL;

*/


//--- UnwrapMod methods -----------------------------------------------




UnwrapMod::~UnwrapMod()
{
	DeleteAllRefsFromMe();
	FreeCancelBuffer();

}

static INT_PTR CALLBACK UnwrapSelRollupWndProc(
	HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	UnwrapMod *mod = DLGetWindowLongPtr<UnwrapMod*>(hWnd);

	static BOOL inEnter = FALSE;

	switch (msg) {
case WM_INITDIALOG:
	{
		mod = (UnwrapMod*)lParam;
		DLSetWindowLongPtr(hWnd, lParam);
		mod->hSelRollup = hWnd;
		//setup element check box
		CheckDlgButton(hWnd,IDC_SELECTELEMENT_CHECK,mod->fnGetGeomElemMode());



		//setup element check box
		CheckDlgButton(hWnd,IDC_PLANARANGLE_CHECK,mod->fnGetGeomPlanarMode());

		mod->iPlanarThreshold = GetISpinner(GetDlgItem(hWnd,IDC_PLANARANGLE_SPIN));
		mod->iPlanarThreshold->LinkToEdit(GetDlgItem(hWnd,IDC_PLANARANGLE),EDITTYPE_FLOAT);
		mod->iPlanarThreshold->SetLimits(0.0f, 180.0f, FALSE);
		mod->iPlanarThreshold->SetAutoScale();	
		mod->iPlanarThreshold->SetValue(mod->fnGetGeomPlanarModeThreshold(),TRUE);

		mod->iMatID = GetISpinner(GetDlgItem(hWnd,IDC_MATID_SPIN));
		mod->iMatID->LinkToEdit(GetDlgItem(hWnd,IDC_MATID),EDITTYPE_INT);
		mod->iMatID->SetLimits(1, 255, FALSE);
		mod->iMatID->SetScale(0.1f);	
		mod->iMatID->SetValue(0,TRUE);

		mod->iSG = GetISpinner(GetDlgItem(hWnd,IDC_SG_SPIN2));
		mod->iSG->LinkToEdit(GetDlgItem(hWnd,IDC_SG),EDITTYPE_INT);
		mod->iSG->SetLimits(1, 32, FALSE);
		mod->iSG->SetScale(0.1f);	
		mod->iSG->SetValue(0,TRUE);

		ICustButton *iEditSeamsByPointButton = GetICustButton(GetDlgItem(hWnd, IDC_UNWRAP_SEAMPOINTTOPOINT));
		iEditSeamsByPointButton->SetType(CBT_CHECK);
		iEditSeamsByPointButton->SetHighlightColor(GREEN_WASH);
		ReleaseICustButton(iEditSeamsByPointButton);

		mod->EnableFaceSelectionUI(FALSE);
		mod->EnableEdgeSelectionUI(FALSE);
		mod->EnableSubSelectionUI(FALSE);



		//setup back face cull check box
		CheckDlgButton(hWnd,IDC_IGNOREBACKFACING_CHECK,mod->fnGetBackFaceCull());			
		break;
	}
case WM_CUSTEDIT_ENTER:	
case CC_SPINNER_BUTTONUP:
	{
		if ( (LOWORD(wParam) == IDC_PLANARANGLE_SPIN) ||
			(LOWORD(wParam) == IDC_PLANARANGLE) )
		{
			float angle = mod->iPlanarThreshold->GetFVal();
			mod->fnSetGeomPlanarModeThreshold(angle);
			//send macro message
			TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap2.setGeomPlanarThreshold"));
			macroRecorder->FunctionCall(mstr, 1, 0,
				mr_float,mod->fnGetGeomPlanarModeThreshold());
			macroRecorder->EmitScript();
		}
		else if ( (LOWORD(wParam) == IDC_MATID_SPIN) ||
			      (LOWORD(wParam) == IDC_MATID) )
		{
			int id = mod->iMatID->GetIVal();
			mod->fnSetMatID(id);
			//send macro message
			TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap6.setSelectMatID"));
			macroRecorder->FunctionCall(mstr, 1, 0,
				mr_int,id);
			macroRecorder->EmitScript();
		}
		else if ( (LOWORD(wParam) == IDC_SG_SPIN2) ||
			      (LOWORD(wParam) == IDC_SG) )
		{
			int id = mod->iSG->GetIVal();
			//send macro message
			TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap6.setSG"));
			macroRecorder->FunctionCall(mstr, 1, 0,
				mr_int,id);
			macroRecorder->EmitScript();
		}

		break;
	}

case WM_COMMAND:
	switch (LOWORD(wParam)) 
	{

	case IDC_UNWRAP_LOOP:
		{
			mod->fnGeomLoopSelect();
			TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap5.geomEdgeLoopSelection"));
			macroRecorder->FunctionCall(mstr, 0, 0);
			break;
		}
	case IDC_UNWRAP_RING:
		{
			mod->fnGeomRingSelect();
			TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap5.geomEdgeRingSelection"));
			macroRecorder->FunctionCall(mstr, 0, 0);
			break;
		}


	case IDC_UNWRAP_SELECTEXPAND:
		{
			if (mod->ip && mod->ip->GetSubObjectLevel() == 1)
			{
				mod->fnGeomExpandVertexSel();
				//send macro message
				TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap5.expandGeomVertexSelection"));
				macroRecorder->FunctionCall(mstr, 0, 0);
				macroRecorder->EmitScript();
			}
			else if (mod->ip && mod->ip->GetSubObjectLevel() == 2)
			{
				mod->fnGeomExpandEdgeSel();
				//send macro message
				TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap5.expandGeomEdgeSelection"));
				macroRecorder->FunctionCall(mstr, 0, 0);
				macroRecorder->EmitScript();
			}

			else if (mod->ip && mod->ip->GetSubObjectLevel() == 3)
			{
				mod->fnGeomExpandFaceSel();
				//send macro message
				TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap2.expandGeomFaceSelection"));
				macroRecorder->FunctionCall(mstr, 0, 0);
				macroRecorder->EmitScript();
			}
			break;
		}
	case IDC_UNWRAP_SELECTCONTRACT:
		{
			if (mod->ip && mod->ip->GetSubObjectLevel() == 1)
			{
				mod->fnGeomContractVertexSel();
				//send macro message
				TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap5.contractGeomVertexSelection"));
				macroRecorder->FunctionCall(mstr, 0, 0);
				macroRecorder->EmitScript();
			}
			else if (mod->ip && mod->ip->GetSubObjectLevel() == 2)
			{
				mod->fnGeomContractEdgeSel();
				//send macro message
				TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap5.contractGeomEdgeSelection"));
				macroRecorder->FunctionCall(mstr, 0, 0);
				macroRecorder->EmitScript();
			}

			else if (mod->ip && mod->ip->GetSubObjectLevel() == 3)
			{
				mod->fnGeomContractFaceSel();
				//send macro message
				TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap2.contractGeomFaceSelection"));
				macroRecorder->FunctionCall(mstr, 0, 0);
				macroRecorder->EmitScript();
			}
			break;
		}
	case IDC_UNWRAP_EXPANDTOSEAMS:
		{
			mod->WtExecute(ID_PELT_EXPANDSELTOSEAM);
			TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap5.peltExpandSelectionToSeams"));
			macroRecorder->FunctionCall(mstr, 0, 0);						
			break;
		}
	case IDC_UNWRAP_SEAMPOINTTOPOINT:
		{			
			mod->WtExecute(ID_PELT_POINTTOPOINTSEAMS);
			TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap5.setPeltPointToPointSeamsMode"));
			macroRecorder->FunctionCall(mstr, 1, 0,	mr_bool,mod->fnGetPeltPointToPointSeamsMode());						
			break;
		}
	case IDC_UNWRAP_EDGESELTOSEAMS:
		{
			SHORT iret = GetAsyncKeyState (VK_CONTROL);
			if (iret==-32767)
			{
				mod->WtExecute(ID_PW_SELTOSEAM2);
				TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap5.peltEdgeSelToSeam"));
				macroRecorder->FunctionCall(mstr, 1, 0,	mr_bool,FALSE);												
			}
			else
			{
				mod->WtExecute(ID_PW_SELTOSEAM);
				TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap5.peltEdgeSelToSeam"));
				macroRecorder->FunctionCall(mstr, 1, 0,	mr_bool,TRUE);						
			}

			break;
		}
	case IDC_UNWRAP_SEAMSTOEDGESEL:
		{
			SHORT iret = GetAsyncKeyState (VK_CONTROL);
			if (iret==-32767)
			{
				mod->WtExecute(ID_PW_SEAMTOSEL2);
				TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap5.peltSeamToEdgeSel"));
				macroRecorder->FunctionCall(mstr, 1, 0,	mr_bool,FALSE);						

			}
			else
			{
				mod->WtExecute(ID_PW_SEAMTOSEL);
				TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap5.peltSeamToEdgeSel"));
				macroRecorder->FunctionCall(mstr, 1, 0,	mr_bool,TRUE);						
			}

			break;
		}


	case IDC_UNWRAP_SELECTSG:
		{
			int id = mod->iSG->GetIVal();
			mod->fnSelectBySG(id);
			//send macro message
			TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap2.selectBySG"));
			macroRecorder->FunctionCall(mstr, 1, 0,
				mr_int,id);
			macroRecorder->EmitScript();
			break;
		}
	case IDC_UNWRAP_SELECTMATID:
		{
			int id = mod->iMatID->GetIVal();
			mod->fnSelectByMatID(id);
			//send macro message
			TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap2.selectByMatID"));
			macroRecorder->FunctionCall(mstr, 1, 0,
				mr_int,id);
			macroRecorder->EmitScript();
			break;
		}




	case IDC_SELECTELEMENT_CHECK:
		{
			//set element mode swtich 
			//					CheckDlgButton(hWnd,IDC_SELECTELEMENT_CHECK,mod->fnGetGeomElemMode());
			mod->fnSetGeomElemMode(IsDlgButtonChecked(hWnd,IDC_SELECTELEMENT_CHECK));
			//send macro message
			TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap2.setGeomSelectElementMode"));
			macroRecorder->FunctionCall(mstr, 1, 0,
				mr_bool,mod->fnGetGeomElemMode());
			macroRecorder->EmitScript();

			if (mod->hWnd)
			{
				IMenuBarContext* pContext = (IMenuBarContext*) GetCOREInterface()->GetMenuManager()->GetContext(kUnwrapMenuBar);
				if (pContext)
					pContext->UpdateWindowsMenu();
			}


			break;

		}
	case IDC_PLANARANGLE_CHECK:
		{
			//set element mode swtich 
			//					CheckDlgButton(hWnd,IDC_SELECTELEMENT_CHECK,mod->fnGetGeomElemMode());
			mod->fnSetGeomPlanarMode(IsDlgButtonChecked(hWnd,IDC_PLANARANGLE_CHECK));
			//send macro message
			TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap2.setGeomPlanarThresholdMode"));
			macroRecorder->FunctionCall(mstr, 1, 0,
				mr_bool,mod->fnGetGeomPlanarMode());
			macroRecorder->EmitScript();

			if (mod->hWnd)
			{
				IMenuBarContext* pContext = (IMenuBarContext*) GetCOREInterface()->GetMenuManager()->GetContext(kUnwrapMenuBar);
				if (pContext)
					pContext->UpdateWindowsMenu();
			}


			break;
		}
	case IDC_IGNOREBACKFACING_CHECK:
		{
			//set element mode swtich 
			//					CheckDlgButton(hWnd,IDC_SELECTELEMENT_CHECK,mod->fnGetGeomElemMode());
			mod->fnSetBackFaceCull(IsDlgButtonChecked(hWnd,IDC_IGNOREBACKFACING_CHECK));
			//send macro message
			TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap2.setIgnoreBackFaceCull"));
			macroRecorder->FunctionCall(mstr, 1, 0,
				mr_bool,mod->fnGetBackFaceCull());
			macroRecorder->EmitScript();

			if (mod->hWnd)
			{
				IMenuBarContext* pContext = (IMenuBarContext*) GetCOREInterface()->GetMenuManager()->GetContext(kUnwrapMenuBar);
				if (pContext)
					pContext->UpdateWindowsMenu();
			}


			break;
		}


	}


default:
	return FALSE;
	}
	return TRUE;
}


static INT_PTR CALLBACK UnwrapRollupWndProc(
	HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	UnwrapMod *mod = DLGetWindowLongPtr<UnwrapMod*>(hWnd);

	static BOOL inEnter = FALSE;
	static int resetOnMapChange = 0;

	switch (msg) {
case WM_INITDIALOG:
	{

		mod = (UnwrapMod*)lParam;
		DLSetWindowLongPtr(hWnd, lParam);
		mod->hParams = hWnd;
		/*
		mod->iApplyButton = GetICustButton(GetDlgItem(hWnd, IDC_UNWRAP_APPLY));
		mod->iApplyButton->SetType(CBT_PUSH);
		mod->iApplyButton->SetHighlightColor(GREEN_WASH);
		mod->iApplyButton->Enable(TRUE);
		*/
		mod->iMapID = GetISpinner(GetDlgItem(hWnd,IDC_MAP_CHAN_SPIN));
		mod->iMapID->LinkToEdit(GetDlgItem(hWnd,IDC_MAP_CHAN),EDITTYPE_INT);
		mod->iMapID->SetLimits(1, 99, FALSE);
		mod->iMapID->SetScale(0.1f);	

		mod->SetupChannelButtons();
		//mod->ip->GetRightClickMenuManager()->Register(&rMenu);


		CheckDlgButton(hWnd,IDC_RADIO4,TRUE);



		CheckDlgButton(hWnd,IDC_DONOTREFLATTEN_CHECK,mod->fnGetPreventFlattening());

		BOOL thickSeams = mod->fnGetThickOpenEdges();
		BOOL showSeams = mod->fnGetViewportOpenEdges();

		CheckDlgButton(hWnd,IDC_SHOWMAPSEAMS_CHECK,FALSE);
		CheckDlgButton(hWnd,IDC_THINSEAM,FALSE);
		CheckDlgButton(hWnd,IDC_THICKSEAM,FALSE);


		if (thickSeams)
			CheckDlgButton(hWnd,IDC_THICKSEAM,TRUE);
		else CheckDlgButton(hWnd,IDC_THINSEAM,TRUE);

		if (showSeams)
		{
			CheckDlgButton(hWnd,IDC_SHOWMAPSEAMS_CHECK,TRUE);
		}



		CheckDlgButton(hWnd,IDC_ALWAYSSHOWPELTSEAMS_CHECK,mod->fnGetAlwayShowPeltSeams());

		//		mod->alignDir = 3;

		TCHAR filename[ MAX_PATH ];
		mod->GetCfgFilename( filename );
		TCHAR str[ MAX_PATH ];
		TSTR def( "DISCARD" );
		int res = GetPrivateProfileString( _T("Options"), _T("ResetOnMapChange"), def, str, MAX_PATH, filename );
		if ( res && _tcscmp( str, def ) )
			sscanf( str, "%d", &resetOnMapChange );

		break;
	}


case WM_CUSTEDIT_ENTER:
	{
		if (!inEnter)
		{
			inEnter = TRUE;
			TSTR buf1 = GetString(IDS_RB_SHOULDRESET);
			TSTR buf2 = GetString(IDS_RB_UNWRAPMOD);
			int tempChannel = mod->iMapID->GetIVal();
			if (tempChannel == 1) tempChannel = 0;
			if (tempChannel != mod->channel)
			{
				int res = IDYES;
				if ( resetOnMapChange )
					res = MessageBox(mod->ip->GetMAXHWnd(),buf1,buf2,MB_YESNO|MB_ICONQUESTION|MB_TASKMODAL);
				if (res==IDYES)
				{
					theHold.Begin();
					if ( resetOnMapChange )
						mod->Reset();
					mod->channel = mod->iMapID->GetIVal();
					if (mod->channel == 1) mod->channel = 0;
					theHold.Accept(GetString(IDS_RB_SETCHANNEL));					

					TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap.setMapChannel"));
					macroRecorder->FunctionCall(mstr, 1, 0,
						mr_int,mod->channel);
					macroRecorder->EmitScript();

					mod->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
					if (mod->ip) mod->ip->RedrawViews(mod->ip->GetTime());
					mod->InvalidateView();
				}
				else mod->SetupChannelButtons();
			}
			inEnter = FALSE;
		}

	}

	break;
case CC_SPINNER_BUTTONUP:
	{
		TSTR buf1 = GetString(IDS_RB_SHOULDRESET);
		TSTR buf2 = GetString(IDS_RB_UNWRAPMOD);
		int tempChannel = mod->iMapID->GetIVal();
		if (tempChannel == 1) tempChannel = 0;
		if (tempChannel != mod->channel)
		{
			int res = IDYES;
			if ( resetOnMapChange )
				res = MessageBox(mod->ip->GetMAXHWnd(),buf1,buf2,MB_YESNO|MB_ICONQUESTION|MB_TASKMODAL);
			if (res==IDYES)
			{
				theHold.Begin();
				if ( resetOnMapChange )
					mod->Reset();
				mod->channel = mod->iMapID->GetIVal();
				if (mod->channel == 1) mod->channel = 0;
				theHold.Accept(GetString(IDS_RB_SETCHANNEL));					
				mod->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);

				TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap.setMapChannel"));
				macroRecorder->FunctionCall(mstr, 1, 0,
					mr_int,mod->channel);
				macroRecorder->EmitScript();

				mod->SetCheckerMapChannel();

				if (mod->ip) mod->ip->RedrawViews(mod->ip->GetTime());
				mod->InvalidateView();
			}
			else mod->SetupChannelButtons();
		}

	}

	break;


case WM_COMMAND:
	switch (LOWORD(wParam)) {
case IDC_UNWRAP_SAVE:
	{
	mod->fnSave();
	TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap.save"));
	macroRecorder->FunctionCall(mstr, 0, 0 );
	macroRecorder->EmitScript();

	break;
	}
case IDC_UNWRAP_LOAD:
	{
	mod->fnLoad();
	TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap.load"));
	macroRecorder->FunctionCall(mstr, 0, 0 );
	macroRecorder->EmitScript();
	break;
	}
case IDC_MAP_CHAN1:
case IDC_MAP_CHAN2: 
	{

	TSTR buf1 = GetString(IDS_RB_SHOULDRESET);
	TSTR buf2 = GetString(IDS_RB_UNWRAPMOD);
	int res = IDYES;
	if ( resetOnMapChange && !mod->suppressWarning )
		res = MessageBox(mod->ip->GetMAXHWnd(),buf1,buf2,MB_YESNO|MB_ICONQUESTION|MB_TASKMODAL);
	if (res==IDYES)
	{
		theHold.Begin();
		if ( resetOnMapChange )
			mod->Reset();
		mod->channel = IsDlgButtonChecked(hWnd,IDC_MAP_CHAN2);
		if (mod->channel == 1)
			mod->iMapID->Enable(FALSE);
		else 
		{
			int ival = mod->iMapID->GetIVal();
			if (ival == 1) mod->channel = 0;
			else mod->channel = ival;
			mod->iMapID->Enable(TRUE);
		}
		mod->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
		if (mod->ip) mod->ip->RedrawViews(mod->ip->GetTime());
		mod->InvalidateView();
		TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap.setVC"));
		if (mod->channel == 1)
			macroRecorder->FunctionCall(mstr, 1, 0,
			mr_bool,TRUE);
		else macroRecorder->FunctionCall(mstr, 1, 0,
			mr_bool,FALSE);
		macroRecorder->EmitScript();

		theHold.Accept(GetString(IDS_RB_SETCHANNEL));					
	}
	else 
	{
		if ( resetOnMapChange )
			mod->SetupChannelButtons();
	}

	break;
					}

case IDC_UNWRAP_RESET: {
	mod->fnReset();
	TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap.reset"));
	macroRecorder->FunctionCall(mstr, 0, 0 );
	macroRecorder->EmitScript();
	break;
					   }

case IDC_UNWRAP_APPLY:
	{
		mod->fnPlanarMap();
		TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap.planarmap"));
		macroRecorder->FunctionCall(mstr, 0, 0 );
		macroRecorder->EmitScript();
		break;
	}
case IDC_UNWRAP_EDIT:
	{
		mod->fnEdit();
		TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap.edit"));
		macroRecorder->FunctionCall(mstr, 0, 0 );
		macroRecorder->EmitScript();
		break;
	}
case IDC_ALWAYSSHOWPELTSEAMS_CHECK:
	{
		theHold.Begin();
		{
			mod->fnSetAlwayShowPeltSeams(IsDlgButtonChecked(hWnd,IDC_ALWAYSSHOWPELTSEAMS_CHECK));
		}
		theHold.Accept(GetString (IDS_PELT_ALWAYSSHOWSEAMS));
		
		TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap5.setPeltAlwaysShowSeams"));
		macroRecorder->FunctionCall(mstr, 1, 0,
			mr_bool,mod->fnGetAlwayShowPeltSeams());
		macroRecorder->EmitScript();

		break;
	}
case IDC_DONOTREFLATTEN_CHECK:
	{
		//set element mode swtich 
		//					CheckDlgButton(hWnd,IDC_SELECTELEMENT_CHECK,mod->fnGetGeomElemMode());
		theHold.Begin();
		{
			mod->fnSetPreventFlattening(IsDlgButtonChecked(hWnd,IDC_DONOTREFLATTEN_CHECK));
		}
		theHold.Accept(GetString (IDS_PW_PREVENTREFLATTENING));


		//send macro message
		TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap2.setPreventFlattening"));
		macroRecorder->FunctionCall(mstr, 1, 0,
			mr_bool,mod->fnGetPreventFlattening());
		macroRecorder->EmitScript();

		if (mod->hWnd)
		{
			IMenuBarContext* pContext = (IMenuBarContext*) GetCOREInterface()->GetMenuManager()->GetContext(kUnwrapMenuBar);
			if (pContext)
				pContext->UpdateWindowsMenu();
		}


		break;
	}
case IDC_SHOWMAPSEAMS_CHECK:
	{
		BOOL showMapSeams = IsDlgButtonChecked(hWnd,IDC_SHOWMAPSEAMS_CHECK);
		theHold.Begin();
		{

			if (showMapSeams)
			{
				mod->fnSetViewportOpenEdges(TRUE);
			}
			else
			{
				mod->fnSetViewportOpenEdges(FALSE);
			}
		}
		theHold.Accept(GetString (IDS_PELT_ALWAYSSHOWSEAMS));

		TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap5.setShowMapSeams"));
		macroRecorder->FunctionCall(mstr, 1, 0,
			mr_bool,mod->fnGetViewportOpenEdges());
		mod->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
		if (mod->ip) mod->ip->RedrawViews(mod->ip->GetTime());

		break;
	}

case IDC_THINSEAM:
case IDC_THICKSEAM:
	{
		BOOL thickSeams = IsDlgButtonChecked(hWnd,IDC_THICKSEAM);

		theHold.Begin();
		{
			if (thickSeams)
				mod->fnSetThickOpenEdges(TRUE);
			else mod->fnSetThickOpenEdges(FALSE);
		}
		theHold.Accept(GetString (IDS_THICKSEAMS));

		mod->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
		if (mod->ip) mod->ip->RedrawViews(mod->ip->GetTime());

		TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap4.setThickOpenEdges"));
		macroRecorder->FunctionCall(mstr, 1, 0,
			mr_bool,mod->fnGetThickOpenEdges());



		break;
	}
	}
	break;

default:
	return FALSE;
	}
	return TRUE;
}

void UnwrapMod::SetupChannelButtons()
{
	if (hParams && editMod==this) {		
		if (channel == 0)
		{
			iMapID->Enable(TRUE);
			iMapID->SetValue(1,TRUE);
			CheckDlgButton(hParams,IDC_MAP_CHAN1,TRUE);
			CheckDlgButton(hParams,IDC_MAP_CHAN2,FALSE);

		}
		else if (channel == 1)
		{
			CheckDlgButton(hParams,IDC_MAP_CHAN1,FALSE);
			CheckDlgButton(hParams,IDC_MAP_CHAN2,TRUE);
			iMapID->Enable(FALSE);
			//			iMapID->SetValue(0,TRUE);
		}
		else
		{
			CheckDlgButton(hParams,IDC_MAP_CHAN1,TRUE);
			CheckDlgButton(hParams,IDC_MAP_CHAN2,FALSE);
			iMapID->Enable(TRUE);
			iMapID->SetValue(channel,TRUE);
		}
	}
}

void UnwrapMod::Reset()
{
	if (theHold.Holding()) 
	{
		HoldPointsAndFaces();
		theHold.Put(new ResetRestore(this));
	}

	for (int i=0; i<mUVWControls.cont.Count(); i++) 
		if (mUVWControls.cont[i]) DeleteReference(i+11+100);

	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		mMeshTopoData[ldID]->FreeCache();
	}

	updateCache = TRUE;

	NotifyDependents(FOREVER,0,REFMSG_CONTROLREF_CHANGE);  //fix for 894778 MZ	
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
}

BOOL UnwrapMod::AssignController(Animatable *control,int subAnim)
{
	ReplaceReference(subAnim+11+100,(RefTargetHandle)control);	
	NotifyDependents(FOREVER,0,REFMSG_CONTROLREF_CHANGE);  //fix for 894778 MZ	
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	return TRUE;
}


Object *GetBaseObject(Object *obj)
{
	if (obj->SuperClassID() == GEN_DERIVOB_CLASS_ID)
	{
		IDerivedObject* dobj;
		dobj = (IDerivedObject*)obj;
		while (dobj->SuperClassID() == GEN_DERIVOB_CLASS_ID)
		{
			dobj = (IDerivedObject*)dobj->GetObjRef();
		}

		return (Object *) dobj;

	}
	return obj;

}


class IsOnStack : public GeomPipelineEnumProc
	{
public:  
   IsOnStack(ReferenceTarget *me) : mRef(me),mOnStack(FALSE),mModData(NULL) {}
   PipeEnumResult proc(ReferenceTarget *object, IDerivedObject *derObj, int index);
   ReferenceTarget *mRef;
   BOOL mOnStack;
   MeshTopoData *mModData;
protected:
   IsOnStack(); // disallowed
   IsOnStack(IsOnStack& rhs); // disallowed
   IsOnStack& operator=(const IsOnStack& rhs); // disallowed
};

PipeEnumResult IsOnStack::proc(
   ReferenceTarget *object, 
   IDerivedObject *derObj, 
   int index)
{
   if ((derObj != NULL) && object == mRef) //found it!
   {
		mOnStack = TRUE;
		ModContext *mc = derObj->GetModContext(index);
		mModData = (MeshTopoData*)mc->localData;  
		return PIPE_ENUM_STOP;
   }
   return PIPE_ENUM_CONTINUE;
}





BOOL UnwrapMod::IsInStack(INode *node)
{
	int				i;
	SClass_ID		sc;
	IDerivedObject* dobj;


	// then osm stack
	Object* obj = node->GetObjectRef();

	if ((dobj = node->GetWSMDerivedObject()) != NULL)
	{
		IsOnStack pipeEnumProc(this);
        EnumGeomPipeline(&pipeEnumProc, dobj);
		if(pipeEnumProc.mOnStack ==TRUE)
		{
			BOOL found = FALSE;
			MeshTopoData *localData = pipeEnumProc.mModData;
			for (int k = 0; k < mMeshTopoData.Count(); k++)
			{
				if (mMeshTopoData[k] == localData)
					found = TRUE;
			}
			if (!found && localData)
				mMeshTopoData.Append(1,localData,node,10);
			return TRUE;
		}
	}

	if ((sc = obj->SuperClassID()) == GEN_DERIVOB_CLASS_ID)
	{
		IsOnStack pipeEnumProc(this);
        EnumGeomPipeline(&pipeEnumProc, obj);
		if(pipeEnumProc.mOnStack ==TRUE)
		{
			BOOL found = FALSE;
			MeshTopoData *localData = pipeEnumProc.mModData;
			for (int k = 0; k < mMeshTopoData.Count(); k++)
			{
				if (mMeshTopoData[k] == localData)
					found = TRUE;
			}
			if (!found && localData)
				mMeshTopoData.Append(1,localData,node,10);
			return TRUE;
		}
	}
	return FALSE;



}

void UnwrapMod::BeginEditParams(
								IObjParam  *ip, ULONG flags,Animatable *prev)
{
	if (loadDefaults)
	{
		fnLoadDefaults();
		loadDefaults = FALSE;
	}

	selCur   = ip->GetSysCursor(SYSCUR_SELECT);
	moveCur	 = ip->GetSysCursor(SYSCUR_MOVE);

	if (moveXCur == NULL)
		moveXCur	 = LoadCursor(hInstance, MAKEINTRESOURCE(IDC_MOVEX));
	if (moveYCur == NULL)
		moveYCur	 = LoadCursor(hInstance, MAKEINTRESOURCE(IDC_MOVEY));

	if (scaleXCur == NULL)
		scaleXCur	 = LoadCursor(hInstance, MAKEINTRESOURCE(IDC_SCALEX));
	if (scaleYCur == NULL)
		scaleYCur	 = LoadCursor(hInstance, MAKEINTRESOURCE(IDC_SCALEY));

	if (zoomCur == NULL)
		zoomCur	 = LoadCursor(hInstance, MAKEINTRESOURCE(IDC_ZOOM));
	if (zoomRegionCur == NULL)
		zoomRegionCur	 = LoadCursor(hInstance, MAKEINTRESOURCE(IDC_ZOOMREG));
	if (panCur == NULL)
		panCur	 = LoadCursor(hInstance, MAKEINTRESOURCE(IDC_PANHAND));
	if (weldCur == NULL)
		weldCur	 = LoadCursor(hInstance, MAKEINTRESOURCE(IDC_WELDCUR));

	if (weldCurHit == NULL)
		weldCurHit	 = LoadCursor(hInstance, MAKEINTRESOURCE(IDC_WELDCUR1));

	if (sketchCur == NULL)
		sketchCur	 = LoadCursor(hInstance, MAKEINTRESOURCE(IDC_SKETCHCUR));

	if (sketchPickCur == NULL)
		sketchPickCur	 = LoadCursor(hInstance, MAKEINTRESOURCE(IDC_SKETCHPICKCUR));

	if (sketchPickHitCur == NULL)
		sketchPickHitCur	 = LoadCursor(hInstance, MAKEINTRESOURCE(IDC_SKETCHPICKCUR1));


	if (circleCur[0] == NULL)
		circleCur[0]	 = LoadCursor(hInstance, MAKEINTRESOURCE(IDC_CURSOR1));
	if (circleCur[1] == NULL)
		circleCur[1]	 = LoadCursor(hInstance, MAKEINTRESOURCE(IDC_CURSOR2));
	if (circleCur[2] == NULL)
		circleCur[2]	 = LoadCursor(hInstance, MAKEINTRESOURCE(IDC_CURSOR3));
	if (circleCur[3] == NULL)
		circleCur[3]	 = LoadCursor(hInstance, MAKEINTRESOURCE(IDC_CURSOR4));
	if (circleCur[4] == NULL)
		circleCur[4]	 = LoadCursor(hInstance, MAKEINTRESOURCE(IDC_CURSOR5));
	if (circleCur[5] == NULL)
		circleCur[5]	 = LoadCursor(hInstance, MAKEINTRESOURCE(IDC_CURSOR6));
	if (circleCur[6] == NULL)
		circleCur[6]	 = LoadCursor(hInstance, MAKEINTRESOURCE(IDC_CURSOR7));
	if (circleCur[7] == NULL)
		circleCur[7]	 = LoadCursor(hInstance, MAKEINTRESOURCE(IDC_CURSOR8));
	if (circleCur[8] == NULL)
		circleCur[8]	 = LoadCursor(hInstance, MAKEINTRESOURCE(IDC_CURSOR9));
	if (circleCur[9] == NULL)
		circleCur[9]	 = LoadCursor(hInstance, MAKEINTRESOURCE(IDC_CURSOR10));
	if (circleCur[10] == NULL)
		circleCur[10]	 = LoadCursor(hInstance, MAKEINTRESOURCE(IDC_CURSOR11));
	if (circleCur[11] == NULL)
		circleCur[11]	 = LoadCursor(hInstance, MAKEINTRESOURCE(IDC_CURSOR12));
	if (circleCur[12] == NULL)
		circleCur[12]	 = LoadCursor(hInstance, MAKEINTRESOURCE(IDC_CURSOR13));
	if (circleCur[13] == NULL)
		circleCur[13]	 = LoadCursor(hInstance, MAKEINTRESOURCE(IDC_CURSOR14));
	if (circleCur[14] == NULL)
		circleCur[14]	 = LoadCursor(hInstance, MAKEINTRESOURCE(IDC_CURSOR15));
	if (circleCur[15] == NULL)
		circleCur[15]	 = LoadCursor(hInstance, MAKEINTRESOURCE(IDC_CURSOR16));



	rotCur   = ip->GetSysCursor(SYSCUR_ROTATE);
	scaleCur = ip->GetSysCursor(SYSCUR_USCALE);


	// Add our sub object type
	// TSTR type1(GetString(IDS_PW_SELECTFACE));
	// TSTR type2 (GetString(IDS_PW_FACEMAP));
	// TSTR type3 (GetString(IDS_PW_PLANAR));
	// const TCHAR *ptype[] = {type1};
	// This call is obsolete. Please see BaseObject::NumSubObjTypes() and BaseObject::GetSubObjType()
	// ip->RegisterSubObjectTypes(ptype, 1);

	selectMode = new SelectModBoxCMode(this,ip);
	moveGizmoMode = new MoveModBoxCMode(this,ip);
	rotGizmoMode       = new RotateModBoxCMode(this,ip);
	uscaleGizmoMode    = new UScaleModBoxCMode(this,ip);
	nuscaleGizmoMode   = new NUScaleModBoxCMode(this,ip);
	squashGizmoMode    = new SquashModBoxCMode(this,ip);	
	mSplineMap.CreateCommandModes(this,ip);

/*
	offsetControl = NewDefaultPoint3Controller();
	Point3 p(0.5f,0.5f,0.0f);
	offsetControl->SetValue(0,&p,CTRL_ABSOLUTE);

	scaleControl = NewDefaultPoint3Controller();
	Point3 sp(0.25f,0.25f,0.0f);
	scaleControl->SetValue(0,&sp,CTRL_ABSOLUTE);

	rotateControl = NewDefaultFloatController();
	float a = 0.0f;
	rotateControl->SetValue(0,&a,CTRL_ABSOLUTE);
*/

	this->ip = ip;
	editMod  = this;
	TimeValue t = ip->GetTime();
	//aszabo|feb.05.02
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_BEGIN_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_ON);
	SetAFlag(A_MOD_BEING_EDITED);

	hParams  = ip->AddRollupPage( 
		hInstance, 
		MAKEINTRESOURCE(IDD_UNWRAP_SELPARAMS),
		UnwrapSelRollupWndProc,
		GetString(IDS_PW_SELPARAMS),
		(LPARAM)this);

	hParams  = ip->AddRollupPage( 
		hInstance, 
		MAKEINTRESOURCE(IDD_UNWRAP_PARAMS),
		UnwrapRollupWndProc,
		GetString(IDS_RB_PARAMETERS),
		(LPARAM)this);
	//PELT
	hMapParams  = ip->AddRollupPage( 
		hInstance, 
		MAKEINTRESOURCE(IDD_UNWRAP_MAPPARAMS),
		UnwrapMapRollupWndProc,
		GetString(IDS_RB_MAPPARAMETERS),
		(LPARAM)this);
	peltData.SetRollupHandle(hMapParams);
	peltData.SetSelRollupHandle(hSelRollup);
	peltData.SetParamRollupHandle(hParams);


	ip->RegisterTimeChangeCallback(this);

	SetNumSelLabel();


	//	actionTable = new UnwrapActionCB(this);
	//	ip->GetActionManager()->ActivateActionTable(actionTable, kUnwrapActions);


	peltPointToPointMode = new PeltPointToPointMode(this,ip);
	tweakMode = new TweakMode(this,ip);

	RegisterNotification(PreSave, this,NOTIFY_FILE_PRE_SAVE);
	RegisterNotification(PostSave, this,NOTIFY_FILE_POST_SAVE);

	mUnwrapNodeDisplayCallback.mod = this;
	INodeDisplayControl* ndc = GetNodeDisplayControl(ip);
	ndc->RegisterNodeDisplayCallback(&mUnwrapNodeDisplayCallback);
	ndc->SetNodeCallback(&mUnwrapNodeDisplayCallback);


	ActivateActionTable();

	if ((alwaysEdit) && (!popUpDialog))
		fnEdit();


}


void UnwrapMod::EndEditParams(
							  IObjParam *ip,ULONG flags,Animatable *next)
{
	if (!this->ip)
		this->ip = ip;

	peltData.SubObjectUpdateSuspend();
	fnSetMapMode(0);
	peltData.SubObjectUpdateResume();

	ClearAFlag(A_MOD_BEING_EDITED);

	//	ip->GetActionManager()->DeactivateActionTable(actionTable, kUnwrapActions);
	//	delete actionTable;

	if (hWnd) DestroyWindow(hWnd);


	ip->UnRegisterTimeChangeCallback(this);

	if (hSelRollup) ip->DeleteRollupPage(hSelRollup);
	hSelRollup  = NULL;	

	if (hParams) ip->DeleteRollupPage(hParams);
	hParams  = NULL;	

	//PELT
	if (hMapParams) ip->DeleteRollupPage(hMapParams);
	hMapParams  = NULL;	
	peltData.SetRollupHandle(NULL);
	peltData.SetSelRollupHandle(NULL);
	peltData.SetParamRollupHandle(NULL);



	ip->DeleteMode(selectMode);

	if (selectMode) delete selectMode;
	selectMode = NULL;

	ip->DeleteMode(moveGizmoMode);
	ip->DeleteMode(rotGizmoMode);
	ip->DeleteMode(uscaleGizmoMode);
	ip->DeleteMode(nuscaleGizmoMode);
	ip->DeleteMode(squashGizmoMode);

	if (moveGizmoMode) delete moveGizmoMode;
	moveGizmoMode = NULL;
	if (rotGizmoMode) delete rotGizmoMode;
	rotGizmoMode = NULL;
	if (uscaleGizmoMode) delete uscaleGizmoMode;
	uscaleGizmoMode = NULL;
	if (nuscaleGizmoMode) delete nuscaleGizmoMode;
	nuscaleGizmoMode = NULL;
	if (squashGizmoMode) delete squashGizmoMode;
	squashGizmoMode = NULL;


	fnSplineMap_EndMapMode();
	mSplineMap.DestroyCommandModes();


	if (peltData.GetPointToPointSeamsMode())
		peltData.SetPointToPointSeamsMode(this,FALSE);

	ip->DeleteMode(peltPointToPointMode);	
	if (peltPointToPointMode) delete peltPointToPointMode;
	peltPointToPointMode = NULL;

	peltData.SetPeltMapMode(this,FALSE);
	mapMapMode = NOMAP;


	ip->DeleteMode(tweakMode);	
	if (tweakMode) delete tweakMode;
	tweakMode = NULL;

	ReleaseISpinner(iMapID); iMapID = NULL;
	ReleaseISpinner(iPlanarThreshold); iPlanarThreshold = NULL;
	ReleaseISpinner(iMatID); iMatID = NULL;
	ReleaseISpinner(iSG); iSG = NULL;


	//	ip->GetRightClickMenuManager()->Unregister(&rMenu);


	TimeValue t =ip->GetTime();
	//aszabo|feb.05.02
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_END_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_OFF);
	this->ip = NULL;

	editMod  = NULL;

	if (CurrentMap == 0)
		ShowCheckerMaterial(FALSE);

	INodeDisplayControl* ndc = GetNodeDisplayControl(ip);
	if (ndc)
	{
		ndc->SetNodeCallback(NULL);
		ndc->UnRegisterNodeDisplayCallback(&mUnwrapNodeDisplayCallback);
	}


	UnRegisterNotification(PreSave, this,NOTIFY_FILE_PRE_SAVE);
	UnRegisterNotification(PostSave, this,NOTIFY_FILE_POST_SAVE);
	DeActivateActionTable();

	SetMode(ID_TOOL_MOVE,FALSE);
}




class NullView: public View {
public:
	Point2 ViewToScreen(Point3 p) { return Point2(p.x,p.y); }
	NullView() { worldToView.IdentityMatrix(); screenW=640.0f; screenH = 480.0f; }
};

static AdjFaceList *BuildAdjFaceList(Mesh &mesh)
{
	AdjEdgeList ae(mesh);
	return new AdjFaceList(mesh,ae);
}

/*
Box3 UnwrapMod::BuildBoundVolume(Object *obj)

{
	Box3 b;
	b.Init();
	if (objType == IS_PATCH)
	{
		PatchObject *pobj = (PatchObject*)obj;
		for (int i = 0; i < pobj->patch.patchSel.GetSize(); i++)
		{
			if (pobj->patch.patchSel[i])
			{
				int pcount = 3;
				if (pobj->patch.patches[i].type == PATCH_QUAD) pcount = 4;
				for (int j = 0; j < pcount; j++)
				{
					int index = pobj->patch.patches[i].v[j];

					b+= pobj->patch.verts[index].p;
				}
			}	
		}

	}	
	else if (objType == IS_MESH)
	{
		TriObject *tobj = (TriObject*)obj;
		for (int i = 0; i < tobj->GetMesh().faceSel.GetSize(); i++)
		{
			if (tobj->GetMesh().faceSel[i])
			{
				for (int j = 0; j < 3; j++)
				{
					int index = tobj->GetMesh().faces[i].v[j];

					b+= tobj->GetMesh().verts[index];
				}
			}	
		}
	}
	else if (objType == IS_MNMESH)
	{
		PolyObject *tobj = (PolyObject*)obj;
		BitArray sel;
		tobj->GetMesh().getFaceSel (sel);
		for (int i = 0; i < sel.GetSize(); i++)
		{
			if (sel[i])
			{
				int ct;
				ct = tobj->GetMesh().f[i].deg;
				for (int j = 0; j < ct; j++)
				{
					int index = tobj->GetMesh().f[i].vtx[j];

					b+= tobj->GetMesh().v[index].p;
				}
			}	
		}
	}
	return b;
}
*/

void UnwrapMod::InitControl(TimeValue t)
{
	Box3 box;
	Matrix3 tm(1);

	if (tmControl==NULL) 
	{
		ReplaceReference(0,NewDefaultMatrix3Controller()); 
		NotifyDependents(FOREVER,0,REFMSG_CONTROLREF_CHANGE);
	}		

	if (flags&CONTROL_INIT) {
		SuspendAnimate();
		AnimateOff();	

		// get our bounding box

			Box3 bounds;
			bounds.Init();

			// get our center
			for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
			{
				MeshTopoData *ld = mMeshTopoData[ldID];

				Matrix3 tm = mMeshTopoData.GetNodeTM(t,ldID);
				for (int i = 0; i < ld->GetNumberGeomVerts(); i++)//TVMaps.geomPoints.Count(); i++)
				{
					bounds += ld->GetGeomVert(i) * tm; //TVMaps.geomPoints[i] * tm;
				}
			}

			if (!bounds.IsEmpty())
			{
				Point3 center = bounds.Center();
				// build the scale
				Point3 scale(bounds.Width().x ,bounds.Width().y , bounds.Width().z);
				if (scale.x < 0.01f)
					scale.x = 1.0f;
				if (scale.y < 0.01f)
					scale.y = 1.0f;
				if (scale.z < 0.01f)
					scale.z = 1.0f;
				tm.SetRow(0,Point3(scale.x,0.0f,0.0f));
				tm.SetRow(1,Point3(0.0f,scale.y,0.0f));
				tm.SetRow(2,Point3(0.0f,0.0f,scale.z));
				tm.SetRow(3,center);
			}

			Matrix3 ptm(1), id(1);
			SetXFormPacket tmpck(tm,ptm);
			tmControl->SetValue(t,&tmpck,TRUE,CTRL_RELATIVE);
		

		ResumeAnimate();
	}
	flags = 0;
}



Matrix3 UnwrapMod::GetMapGizmoMatrix(TimeValue t)
{
	Matrix3 tm(1);
	Interval valid;

	//	int type = GetMapType();

	if (tmControl) 
	{
		tmControl->GetValue(t,&tm,valid,CTRL_RELATIVE);
	}

	return tm;
}

static int lStart[12] = {0,1,3,2,4,5,7,6,0,1,2,3};
static int lEnd[12]   = {1,3,2,0,5,7,6,4,4,5,6,7};

static void DoBoxIcon(BOOL sel,float length, PolyLineProc& lp)
{
	Point3 pt[3];

	length *= 0.5f;
	Box3 box;
	box.pmin = Point3(-length,-length,-length);
	box.pmax = Point3( length, length, length);

	if (sel) //lp.SetLineColor(1.0f,1.0f,0.0f);
		lp.SetLineColor(GetUIColor(COLOR_SEL_GIZMOS));
	else //lp.SetLineColor(0.85f,0.5f,0.0f);		
		lp.SetLineColor(GetUIColor(COLOR_GIZMOS));

	for (int i=0; i<12; i++) {
		pt[0] = box[lStart[i]];
		pt[1] = box[lEnd[i]];
		lp.proc(pt,2);
	}
}


void UnwrapMod::ComputeSelectedFaceData()
{
	TimeValue t = GetCOREInterface()->GetTime();
	mMapGizmoTM = Matrix3(1);
	int dir = GetQMapAlign();
	if (dir == 0) //x
	{
		mMapGizmoTM.SetRow(1,Point3(0.0f,0.0f,1.0f));
		mMapGizmoTM.SetRow(0,Point3(0.0f,1.0f,0.0f));
		mMapGizmoTM.SetRow(2,Point3(1.0f,0.0f,0.0f));
	}
	else if (dir == 1) //y
	{	
		mMapGizmoTM.SetRow(0,Point3(1.0f,0.0f,0.0f));
		mMapGizmoTM.SetRow(1,Point3(0.0f,0.0f,1.0f));
		mMapGizmoTM.SetRow(2,Point3(0.0f,-1.0f,0.0f));
	}	
	else if (dir == 2) //z
	{
		mMapGizmoTM.SetRow(0,Point3(1.0f,0.0f,0.0f));
		mMapGizmoTM.SetRow(1,Point3(0.0f,1.0f,0.0f));
		mMapGizmoTM.SetRow(2,Point3(0.0f,0.0f,1.0f));
	}
	else
	{
		//compute our normal
		Point3 norm(0.0f,0.0f,0.0f);
		for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
		{
		
			MeshTopoData *ld = mMeshTopoData[ldID];
			Matrix3 toWorld = mMeshTopoData.GetNodeTM(t,ldID);
			Point3 pnorm(0.0f,0.0f,0.0f);
			int ct = 0;
			for (int k = 0; k < ld->GetNumberFaces(); k++) 
			{
				if (ld->GetFaceSelected(k))
				{
					// Grap the three points, xformed
					int pcount = 3;
					pcount = ld->GetFaceDegree(k);//gfaces[k]->count;

					Point3 temp_point[4];
					for (int j = 0; j < pcount; j++) {
						int index = ld->GetFaceGeomVert(k,j);//gfaces[k]->t[j];
						//					Point3 temp_point;
						Point3 p = ld->GetGeomVert(index);
						if (j < 4)
							temp_point[j] = p;//gverts.d[index].p;

					}
					pnorm += VectorTransform(Normalize(temp_point[1]-temp_point[0]^temp_point[2]-temp_point[1]),toWorld);
					ct++;
				}
			}
			if(ct > 0)
			{
				pnorm = pnorm/(float)ct;
				pnorm = Normalize(pnorm);
				UnwrapMatrixFromNormal(pnorm,mMapGizmoTM);
			}
		}
	}

	//compute the center
	Box3 bounds;
	bounds.Init();

	Box3 worldBounds;
	worldBounds.Init();
	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];
		BitArray vSel;
		ld->GetGeomVertSelFromFace(vSel);
		Matrix3 toWorld = mMeshTopoData.GetNodeTM(t,ldID);
		Matrix3 toGizmoSpace = Inverse(mMapGizmoTM);
		for (int i = 0; i < ld->GetNumberGeomVerts(); i++)
		{
			if (vSel[i])
			{
				Point3 p = ld->GetGeomVert(i) * toWorld;
				worldBounds += p;
				p = p * toGizmoSpace;
				bounds += p;
			}
		}
	}
	float xScale = 10.0f;
	float yScale = 10.0f;
	if (!bounds.IsEmpty())
	{
		xScale = (bounds.pmax.x - bounds.pmin.x) ;
		yScale = (bounds.pmax.y - bounds.pmin.y) ;
		if (xScale > yScale)
			yScale = xScale;
		else xScale = yScale;
	}

	mMapGizmoTM.SetRow(3,worldBounds.Center());
	mMapGizmoTM.SetRow(0,mMapGizmoTM.GetRow(0) * xScale);
	mMapGizmoTM.SetRow(1,mMapGizmoTM.GetRow(1) * yScale);


/*
	Point3 pnorm(0.0f,0.0f,0.0f);
	int ct = 0;
	gCenter.x = 0.0f;
	gCenter.y = 0.0f;
	gCenter.z = 0.0f;
	int gCt = 0;
	int dir = GetQMapAlign();
	if (dir == 0) //x
	{
		gNormal.x = 1.0f; 
		gNormal.y = 0.0f; 
		gNormal.z = 0.0f; 
	}
	else if (dir == 1) //y
	{	
		gNormal.y = 1.0f; 
		gNormal.x = 0.0f; 
		gNormal.z = 0.0f; 
	}	
	else if (dir == 2) //z
	{
		gNormal.z = 1.0f; 
		gNormal.x = 0.0f; 
		gNormal.y = 0.0f; 
	}
	else
	{
		
		for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
		{
		
			MeshTopoData *ld = mMeshTopoData[ldID];
			Matrix3 toWorld = mMeshTopoData.GetNodeTM(t,ldID);
			for (int k = 0; k < ld->GetNumberFaces(); k++) 
			{
				if (ld->GetFaceSelected(k))
				{
					// Grap the three points, xformed
					int pcount = 3;
					pcount = ld->GetFaceDegree(k);//gfaces[k]->count;

					Point3 temp_point[4];
					for (int j = 0; j < pcount; j++) {
						int index = ld->GetFaceTVVert(k,j);//gfaces[k]->t[j];
						//					Point3 temp_point;
						Point3 p = ld->GetGeomPoint(index);
						if (j < 4)
							temp_point[j] = p;//gverts.d[index].p;
						gCenter += p;//ld->GetGeomPoint(index);//gverts.d[index].p;

					}
					pnorm += VectorTransform(Normalize(temp_point[1]-temp_point[0]^temp_point[2]-temp_point[1]),toWorld);
					ct++;
				}
			}
		}

			// Skip divide by zero situation, make it equal to zero, which happens when gfaces is empty
		if(ct > 0)
		{
			gNormal = pnorm/(float)ct;
			gNormal = Normalize(gNormal);

		}

	}
	gCenter.x = 0.0f;
	gCenter.y = 0.0f;
	gCenter.z = 0.0f;

	gCt = 0;
	float minx = 99999999999.9f,miny = 99999999999.9f,maxx= -999999999.0f,maxy= -9999999999.0f;
	float minz = 99999999999.9f,maxz = -99999999999.9f;


	Box3 bounds;
	bounds.Init();
	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{		
		MeshTopoData *ld = mMeshTopoData[ldID];

		Matrix3 toWorld = mMeshTopoData.GetNodeTM(t,ldID);
		for (int k = 0; k < ld->GetNumberFaces(); k++) 
		{
			if (ld->GetFaceSelected(k))
			{
				// Grap the three points, xformed
				int pcount = 3;
				pcount = ld->GetFaceDegree(k);//gfaces[k]->count;

				Point3 temp_point[4];
				for (int j = 0; j < pcount; j++) 
				{
					int index = ld->GetFaceGeomVertIndex(k,j);
					Point3 p = ld->GetGeomPoint(index) * toWorld;
					bounds += p;
				}
			}
		}
	}
	gCenter = bounds.Center();

	Matrix3 tm;
	UnwrapMatrixFromNormal(gNormal,tm);
	tm.SetTrans(gCenter);
	tm = Inverse(tm);
	minx = 99999999999.9f;
	miny = 99999999999.9f;
	maxx= -999999999.0f;
	maxy= -9999999999.0f;
	minz = 99999999999.9f;
	maxz = -99999999999.9f;


	bounds.Init();
	
	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{		
		MeshTopoData *ld = mMeshTopoData[ldID];

		Matrix3 toGizmoSpace = mMeshTopoData.GetNodeTM(t,ldID)*tm;
		for (int k = 0; k < ld->GetNumberFaces(); k++) 
		{
			if (ld->GetFaceSelected(k))
			{
				// Grap the three points, xformed
				int pcount = 3;
				pcount = ld->GetFaceDegree(k);//gfaces[k]->count;
				for (int j=0; j<pcount; j++) 
				{
					int index = ld->GetFaceGeomVertIndex(k,j);//gfaces[k]->t[j];
					Point3 pd2 = ld->GetGeomPoint(index) * toGizmoSpace;//gverts.d[index].p*toGizmoSpace;
					if (pd2.x < minx) minx = pd2.x;
					if (pd2.y < miny) miny = pd2.y;
					if (pd2.z < minz) minz = pd2.z;
					if (pd2.x > maxx) maxx = pd2.x;
					if (pd2.y > maxy) maxy = pd2.y;
					if (pd2.z > maxz) maxz = pd2.z;
				}
			}
		}
	}
	
	gXScale = (float) fabs(maxx-minx)/2.0f ;
	gYScale = (float) fabs(maxy-miny)/2.0f ;
	gZScale = (float) fabs(maxz-minz)/2.0f ;

	if (gXScale == 0.0f) gXScale = 1.0f;
	if (gYScale == 0.0f) gYScale = 1.0f;
	if (gZScale == 0.0f) gZScale = 1.0f;

*/
}
/*
void UnwrapMod::DoIcon(PolyLineProc& lp,BOOL sel)
{
	//	int type = GetMapType();	
	type = MAP_PLANAR;
	switch (type) 
	{
		case MAP_BOX: DoBoxIcon(sel,2.0f,lp); break;
		case MAP_PLANAR: DoPlanarMapIcon(sel,2.0f,2.0f,lp); break;
		case MAP_BALL:
		case MAP_SPHERICAL: DoSphericalMapIcon(sel,1.0f,lp); break;
		case MAP_CYLINDRICAL: DoCylindricalMapIcon(sel,1.0f,1.0f,lp); break;
	}
}
*/

void Draw3dEdge(GraphicsWindow *gw, float size, Point3 a, Point3 b, Color c)

{
	Matrix3 tm;
	Point3 vec = Normalize(a-b);
	MatrixFromNormal(vec, tm);
	Point3 vecA,vecB,vecC;
	vecA = tm.GetRow(0)*size;
	vecB = tm.GetRow(1)*size;
	Point3 p[3];
	Point3 color[3];
	Point3 ca = Point3(c);
	color[0] = ca;
	color[1] = ca;
	color[2] = ca;

	p[0] = a + vecA + vecB;
	p[1] = b + vecA + vecB;
	p[2] = a - vecA + vecB;
	gw->triangle(p,color);

	p[0] = b - vecA + vecB;
	p[1] = a - vecA + vecB;
	p[2] = b + vecA + vecB;
	gw->triangle(p,color);

	p[2] = a + vecA - vecB;
	p[1] = b + vecA - vecB;
	p[0] = a - vecA - vecB;
	gw->triangle(p,color);

	p[2] = b - vecA - vecB;
	p[1] = a - vecA - vecB;
	p[0] = b + vecA - vecB;
	gw->triangle(p,color);


	p[2] = a + vecB + vecA;
	p[1] = b + vecB + vecA;
	p[0] = a - vecB + vecA;
	gw->triangle(p,color);

	p[2] = b - vecB + vecA;
	p[1] = a - vecB + vecA;
	p[0] = b + vecB + vecA;
	gw->triangle(p,color);

	p[0] = a + vecB - vecA;
	p[1] = b + vecB - vecA;
	p[2] = a - vecB - vecA;
	gw->triangle(p,color);

	p[0] = b - vecB - vecA;
	p[1] = a - vecB - vecA;
	p[2] = b + vecB - vecA;
	gw->triangle(p,color);
}

//PELT
void UnwrapMod::BuildVisibleFaces(GraphicsWindow *gw, MeshTopoData *ld, BitArray &visibleFaces)
{


	visibleFaces.SetSize(ld->GetNumberFaces());
	visibleFaces.ClearAll();

	gw->clearHitCode();

	HitRegion hr;

	hr.type = RECT_RGN;
	hr.crossing  = true;
	int w = gw->getWinSizeX();
	int h = gw->getWinSizeY();
	hr.rect.left = 0;
	hr.rect.top = 0;
	hr.rect.right = w;
	hr.rect.bottom = h;




	if (ld->GetMesh())
	{
		gw->setHitRegion(&hr);


		SubObjHitList hitList;
		MeshSubHitRec *rec;	


		Mesh &mesh = *ld->GetMesh();
		//				mesh.faceSel = ((MeshTopoData*)mc->localData)->faceSel;
		mesh.SubObjectHitTest(gw, gw->getMaterial(), &hr,
			flags|SUBHIT_FACES|SUBHIT_SELSOLID, hitList);

		rec = hitList.First();
		while (rec) {
			int hitface = rec->index;
			if (hitface < visibleFaces.GetSize())
				visibleFaces.Set(hitface,true);
			rec = rec->Next();
		}
	}
	else if (ld->GetPatch())
	{
		SubPatchHitList hitList;


		PatchMesh &patch = *ld->GetPatch();
		//				patch.patchSel = ((MeshTopoData*)mc->localData)->faceSel;

		//				if (ignoreBackFaceCull)
		patch.SubObjectHitTest(gw, gw->getMaterial(), &hr,
			flags|SUBHIT_PATCH_PATCHES|SUBHIT_SELSOLID|SUBHIT_PATCH_IGNORE_BACKFACING, hitList);
		//				else res = patch.SubObjectHitTest(gw, gw->getMaterial(), &hr,
		//					flags|SUBHIT_PATCH_PATCHES|SUBHIT_SELSOLID, hitList);

		PatchSubHitRec *rec = hitList.First();
		while (rec) {
			int hitface = rec->index;
			if (hitface < visibleFaces.GetSize())
				visibleFaces.Set(hitface,true);
			rec = rec->Next();
		}

	}
	else if (ld->GetMNMesh())
	{
		SubObjHitList hitList;
		MeshSubHitRec *rec;	


		MNMesh &mesh = *ld->GetMNMesh();
		//				mesh.FaceSelect(((MeshTopoData*)mc->localData)->faceSel);
		//			mesh.faceSel = ((MeshTopoData*)mc->localData)->faceSel;
		mesh.SubObjectHitTest(gw, gw->getMaterial(), &hr,
			flags|SUBHIT_MNFACES|SUBHIT_SELSOLID, hitList);

		rec = hitList.First();
		while (rec) {
			int hitface = rec->index;
			if (hitface < visibleFaces.GetSize())
				visibleFaces.Set(hitface,true);
			rec = rec->Next();
		}

	}

}

int UnwrapMod::Display(
					   TimeValue t, INode* inode, ViewExp *vpt, int flags, 
					   ModContext *mc) 
{	
	int iret = 0;

	if (mc->localData == NULL) 
		return 1;

	if (!inode->Selected())
		return iret;

	Matrix3 modmat, ntm = inode->GetObjectTM(t);

	GraphicsWindow *gw = vpt->getGW();
	Matrix3 viewTM;
	vpt->GetAffineTM(viewTM);
	Point3 nodeCenter = ntm.GetRow(3);
	nodeCenter = nodeCenter ;

	float thickLineSize = vpt->GetVPWorldWidth(nodeCenter)/gw->getWinSizeX() *0.5f;
	Point3 sizeVec(thickLineSize,thickLineSize,thickLineSize);
	sizeVec = VectorTransform(Inverse(ntm),sizeVec);
	thickLineSize = Length(sizeVec);

	MeshTopoData *ld = (MeshTopoData*) mc->localData;

	Matrix3 vtm(1);
	Interval iv;
	if (inode) 
		vtm = inode->GetObjectTM(t,&iv);

	gw->setTransform(vtm);

//DRAW SPLINE MAPPING INFO
	INode *splineNode = NULL;
	pblock->GetValue(unwrap_splinemap_node,t,splineNode,FOREVER);
	BOOL displaySplineMap = FALSE;
	pblock->GetValue(unwrap_splinemap_display,t,displaySplineMap,FOREVER);

	if (splineNode && displaySplineMap && (fnGetMapMode() ==  SPLINEMAP) )
	{
		Tab<Point3> faceCenters;
		faceCenters.SetCount(ld->GetNumberFaces());
		for (int i = 0; i < ld->GetNumberFaces(); i++)
		{
			faceCenters[i] = Point3(0.0f,0.0f,0.0f);
			int deg = ld->GetFaceDegree(i);
			if (deg)
			{
				for (int j = 0; j < deg; j++)
				{				
					int index = ld->GetFaceGeomVert(i,j);
					Point3 p = ld->GetGeomVert(index);
					faceCenters[i] += p;
				}
				faceCenters[i] /= deg;
			}
		}

		SplineMapProjectionTypes projType = kCircular;
		int val = 0;
		pblock->GetValue(unwrap_splinemap_projectiontype,t,val,FOREVER);
		if (val == 1)
			projType = kPlanar;
		mSplineMap.Display(gw,ld,vtm,faceCenters,projType);		
	}


		
	gw->setTransform(vtm);

	DWORD limits = gw->getRndLimits();

	BOOL boxMode = vpt->getGW()->getRndLimits() & GW_BOX_MODE;

	//draw our selected vertices
	if  (ip && (ip->GetSubObjectLevel() == 1) )
	{

		COLORREF vertColor = ColorMan()->GetColor(GEOMVERTCOLORID);
		
		Color c(vertColor);

		if (ld->HasIncomingFaceSelection())
			c =  GetUIColor(COLOR_SEL_GIZMOS);

		gw->setColor(LINE_COLOR,c);
		gw->startMarkers();

		BOOL useDot = getUseVertexDots();
		int mType = getVertexDotType();
		for (int i = 0; i < ld->GetNumberGeomVerts(); i++)
		{
			if (ld->GetGeomVertSelected(i)/*(i < gvsel.GetSize()) && gvsel[i]*/)
			{
				Point3 p  = ld->GetGeomVert(i);//TVMaps.geomPoints[i];
				if (useDot)
					gw->marker(&p,VERTEX_DOT_MARKER(mType));								
				else gw->marker(&p,PLUS_SIGN_MRKR);								
			}
		}
		gw->endMarkers();
		iret =  1;		
	}

	
	float size = 0.5f;
	size = thickLineSize;
	
	gw->setRndLimits(gw->getRndLimits() | GW_BACKCULL | GW_TWO_SIDED | GW_FLAT & ~GW_WIREFRAME | GW_ILLUM);
	COLORREF egdeSelectedColor = ColorMan()->GetColor(GEOMEDGECOLORID);
	COLORREF seamColorRef = ColorMan()->GetColor(PELTSEAMCOLORID);

	Color seamColor(seamColorRef);
	Color selectedEdgeColor(egdeSelectedColor);
	Color gizmoColor = GetUIColor(COLOR_SEL_GIZMOS);	
	Color oEdgeColor(openEdgeColor);

	Material selectedEdgeMaterial;
	selectedEdgeMaterial.Kd = selectedEdgeColor;
	selectedEdgeMaterial.dblSided = 1;
	selectedEdgeMaterial.opacity = 1.0f;
	selectedEdgeMaterial.selfIllum = 1.0f;

	Material gizmoMaterial;
	gizmoMaterial.Kd = gizmoColor;
	gizmoMaterial.dblSided = 1;
	gizmoMaterial.opacity = 1.0f;
	gizmoMaterial.selfIllum = 1.0f;

	Material openEdgeMaterial;
	openEdgeMaterial.Kd = oEdgeColor;
	openEdgeMaterial.dblSided = 1;
	openEdgeMaterial.opacity = 1.0f;
	openEdgeMaterial.selfIllum = 1.0f;

	Material seamMaterial;
	seamMaterial.Kd = seamColor;
	seamMaterial.dblSided = 1;
	seamMaterial.opacity = 1.0f;
	seamMaterial.selfIllum = 1.0f;


	BOOL hasIncSelection = ld->HasIncomingFaceSelection();



	//now draw our selected edges, open edges, seams, and selected faces if in subobject mode
	//loop through edges
	if (hasIncSelection)
	{
		gw->setMaterial(gizmoMaterial);
		gw->setColor(LINE_COLOR,gizmoColor);	
	}
	else
	{
		gw->setMaterial(selectedEdgeMaterial);
		gw->setColor(LINE_COLOR,selectedEdgeColor);	
	}
	if (thickOpenEdges)
		gw->startTriangles();
	else gw->startSegments();
	if ( ip && (ip->GetSubObjectLevel() == 2) )
	{
		for (int i = 0; i < ld->GetNumberGeomEdges(); i++)
		{
			//if selected draw it and in egde mode	
			if (ld->GetGeomEdgeSelected(i) )
				ld->DisplayGeomEdge(gw,i,size,thickOpenEdges,selectedEdgeColor);
		}
	}
	if (thickOpenEdges)
		gw->endTriangles();
	else gw->endSegments();


	//draw our seams
	gw->setMaterial(seamMaterial);
	gw->setColor(LINE_COLOR,seamColor);	
	if (thickOpenEdges)
		gw->startTriangles();
	else gw->startSegments();
	if (!boxMode && fnGetAlwayShowPeltSeams())
	{
		for (int i = 0; i < ld->GetNumberGeomEdges(); i++)
		{
			if ((i < ld->mSeamEdges.GetSize()) && ld->mSeamEdges[i])
				ld->DisplayGeomEdge(gw,i,size,thickOpenEdges,seamColor);
		}
	}
	if (thickOpenEdges)
		gw->endTriangles();
	else gw->endSegments();


	//if open edge draw it
	gw->setMaterial(openEdgeMaterial);
	gw->setColor(LINE_COLOR,oEdgeColor);	
	if (thickOpenEdges)
		gw->startTriangles();
	else gw->startSegments();
	if (!boxMode && viewportOpenEdges )
	{
		//loop through our tv edges looking for open faces
		for (int i = 0; i < ld->GetNumberTVEdges(); i++)
		{
			int ct = ld->GetTVEdgeNumberTVFaces(i);
			BOOL skip = FALSE;
			if (ip && (ip->GetSubObjectLevel() == 2) )
			{
				if (ld->GetTVEdgeSelected(i))
					skip = TRUE;
			}
			if ((ct == 1) && (!skip))
			{ 
				//find the face for this edge
				int faceIndex = ld->GetTVEdgeConnectedTVFace(i,0);
				int degree = ld->GetFaceDegree(faceIndex);
				int tvA = ld->GetTVEdgeVert(i,0);
				int tvB = ld->GetTVEdgeVert(i,1);
				int a = -1, b = -1,va,vb;
				va = -1;
				vb = -1;
				BOOL hit = FALSE;
				for (int j = 0; j < degree; j++)
				{
					int testA = ld->GetFaceTVVert(faceIndex,j);
					int testB = ld->GetFaceTVVert(faceIndex,(j+1)%degree);
					if ( ((testA == tvA) && (testB == tvB)) ||
						 ((testB == tvA) && (testA == tvB)) )
					{
						a = ld->GetFaceGeomVert(faceIndex,j);
						b = ld->GetFaceGeomVert(faceIndex,(j+1)%degree);
						if (ld->GetPatch())
						{
							va = ld->GetPatch()->numVerts+ld->GetPatch()->patches[faceIndex].vec[j*2];
							vb = ld->GetPatch()->numVerts+ld->GetPatch()->patches[faceIndex].vec[j*2+1];
						}
						hit = TRUE;
						j = degree;
					}

				}
				if (hit)
				{
					ld->DisplayGeomEdge(gw,a,va,b,vb,size,thickOpenEdges,oEdgeColor);				
				}

			}			
		}

	}
	if (thickOpenEdges)
		gw->endTriangles();
	else gw->endSegments();


	//if in face mode and has incoming edge draw and face selected draw it
	//draw our seams
	gw->setMaterial(gizmoMaterial);
	gw->setColor(LINE_COLOR,gizmoColor);	
	if (thickOpenEdges)
		gw->startTriangles();
	else gw->startSegments();
	if (!boxMode && hasIncSelection && ip && (ip->GetSubObjectLevel() == 3))
	{
		for (int i = 0; i < ld->GetNumberGeomEdges(); i++)
		{
			int ct = ld->GetGeomEdgeNumberOfConnectedFaces(i);
			BOOL sel = FALSE;
			for (int j = 0; j < ct; j++)
			{
				int faceIndex = ld->GetGeomEdgeConnectedFace(i,j);
				if (ld->GetFaceSelected(faceIndex))
					sel = TRUE;
			}
			if (sel)
			{
				ld->DisplayGeomEdge(gw,i,size,thickOpenEdges,gizmoColor);
			}
		}
	}
	if (thickOpenEdges)
		gw->endTriangles();
	else gw->endSegments();
	


	//draw the mapping gizmo
	if ( (ip && (ip->GetSubObjectLevel() == 3) && (fnGetMapMode() != NOMAP)) )
	{
		DrawGizmo(t,inode,gw);
		iret = 1;
	}
	//draw the quick mapping gizmo	
	else if ( (ip && (ip->GetSubObjectLevel() == 3) && (GetQMapPreview()) ))
	{
		ComputeSelectedFaceData();
		gw->setTransform(mMapGizmoTM);
		Point3 zero(0.0f,0.0f,0.0f);
		gw->setColor(LINE_COLOR,GetUIColor(COLOR_SEL_GIZMOS));

		gw->marker(&zero,DOT_MRKR);

		Point3 line[3];
		gw->startSegments();

		line[0] = zero;
		line[1] = Point3(0.0f,0.0f,10.0f);		
		gw->segment(line,1);

		line[0] = Point3(-0.5f,-0.5f,0.0f);
		line[1] = Point3(0.5f,-0.5f,0.0f);
		gw->segment(line,1);

		line[0] = Point3(-0.5f,0.5f,0.0f);
		line[1] = Point3(0.5f,0.5f,0.0f);
		gw->segment(line,1);

		line[0] = Point3(-0.5f,-0.5f,0.0f);
		line[1] = Point3(-0.5f,0.5f,0.0f);
		gw->segment(line,1);

		line[0] = Point3(0.5f,-0.5f,0.0f);
		line[1] = Point3(0.5f,0.5f,0.0f);
		gw->segment(line,1);

		line[0] = Point3(0.0f,0.0f,0.0f);
		line[1] = Point3(0.0f,0.5f,0.0f);
		gw->segment(line,1);

		line[0] = Point3(0.0f,0.0f,0.0f);
		line[1] = Point3(0.25f,0.0f,0.0f);
		gw->segment(line,1);
		gw->endSegments();

	}
	if (ip && (ip->GetSubObjectLevel() == 3) && ip->GetShowEndResult() && inode->Selected())
	{
		gw->setTransform(vtm);	

		for (int i = 0; i < ld->GetNumberGeomEdges(); i++)
		{
			//if selected draw it and in edge mode	
			int ct = ld->GetGeomEdgeNumberOfConnectedFaces(i);
			BOOL selected = FALSE;
			for (int j = 0; j < ct; j++)
			{
				int index = ld->GetGeomEdgeConnectedFace(i,j);
				if (ld->GetFaceSelected(index))
					selected = TRUE;

			}
			if (selected)
				ld->DisplayGeomEdge(gw,i,size,FALSE,selectedEdgeColor);
		}
	}

	gw->setRndLimits(limits);
	return iret;	
}



void UnwrapMod::GetWorldBoundBox(
								 TimeValue t,INode* inode, ViewExp *vpt, Box3& box, 
								 ModContext *mc) 
{
	gizmoBounds.Init();

	if ( (ip && (ip->GetSubObjectLevel() == 3) && (fnGetMapMode() == SPLINEMAP) ))
	{
		Box3 bounds;
		bounds.Init();
		bounds = mSplineMap.GetWorldsBounds();

		box = bounds;

	}
	else if ( (ip && (ip->GetSubObjectLevel() == 3) && (fnGetMapMode() != NOMAP) ))
	{
		Box3 bounds;
		bounds.Init();
		bounds += Point3(1.0f,1.0f,1.0f);
		bounds += Point3(-1.0f,-1.0f,-1.0f);
		box.Init();
		Matrix3 ntm(1);
		Matrix3 tm = GetMapGizmoMatrix(t);
		for (int i = 0; i < 8; i++)
		{
			box += bounds[i] * tm;
		}
		gizmoBounds = box;

		//		cb->TM(modmat,0);
	}
	else if  (ip && (ip->GetSubObjectLevel() > 0) )
	{
		Box3 bounds;
		bounds.Init();
		bounds += Point3(1.0f,1.0f,1.0f);
		bounds += Point3(-1.0f,-1.0f,-1.0f);
		box.Init();


		for (int i = 0; i < 8; i++)
		{
			box += bounds[i] * mMapGizmoTM;//mGizmoTM;
		}
		gizmoBounds = box;
	}
}

void UnwrapMod::GetSubObjectCenters(
									SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc)
{	

	if ( (ip && (ip->GetSubObjectLevel() == 3) && (fnGetMapMode() == SPLINEMAP) ))
	{
		Tab<int> selSplines;
		Tab<int> selCrossSections;
		mSplineMap.GetSelectedCrossSections(selSplines, selCrossSections);
		//see if we have a cross selection selected
		//if not use the face selection
		if (selCrossSections.Count() == 0)
		{
			BOOL hasSelection = FALSE;
			for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
			{
				MeshTopoData *ld = mMeshTopoData[ldID];
				BitArray faceSel = ld->GetFaceSel();
				int numberSet = (int)faceSel.NumberSet();
				if ((int)faceSel.NumberSet() > 0)
				{
					hasSelection = TRUE;
				}
			}
			Box3 bounds;
			bounds.Init();
			for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
			{
				Matrix3 tm = mMeshTopoData.GetNodeTM(t,ldID);
				MeshTopoData *ld = mMeshTopoData[ldID];
				if (hasSelection)
				{
					bounds += ld->GetSelFaceBoundingBox()* tm;
				}
				else
				{
					bounds += ld->GetBoundingBox() * tm;
				}
			}

			cb->Center(bounds.Center(),0);
		}
		else
		{
			SplineCrossSection *section = mSplineMap.GetCrossSection(selSplines[0],selCrossSections[0]);
			Point3 p = section->mTM.GetTrans();

			cb->Center(p,0);
		}
	}
	else if ( (ip && (ip->GetSubObjectLevel() == 3) && (fnGetMapMode() != NOMAP) ))
	{
		cb->Center(mGizmoTM.GetTrans(),0);
	}
	else
	{
		

		int ct = 0;
		Box3 box;
		if (ip && (ip->GetSubObjectLevel() == 1))
		{
			for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
			{
				Matrix3 tm = GetMeshTopoDataNode(ldID)->GetObjectTM(t);	
				MeshTopoData *ld = mMeshTopoData[ldID];
				for (int i = 0; i < ld->GetNumberGeomVerts(); i++) 
				{
					if (ld->GetGeomVertSelected(i))
					{
						box += ld->GetGeomVert(i) * tm;
						ct++;
					}
				}
			}
		}
		else if (ip && (ip->GetSubObjectLevel() == 2))
		{
			for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
			{
				MeshTopoData *ld = mMeshTopoData[ldID];
				Matrix3 tm = GetMeshTopoDataNode(ldID)->GetObjectTM(t);	
				for (int i = 0; i < ld->GetNumberGeomEdges(); i++) 
				{
					if (ld->GetGeomEdgeSelected(i))
					{
						int a = ld->GetGeomEdgeVert(i,0);//TVMaps.gePtrList[i]->a;
						box += ld->GetGeomVert(a) * tm;
						a = ld->GetGeomEdgeVert(i,1);//TVMaps.gePtrList[i]->b;
						box += ld->GetGeomVert(a) * tm;
						ct++;
					}
				}
			}
		}
		else if (ip && (ip->GetSubObjectLevel() == 3))
		{
			for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
			{
				MeshTopoData *ld = mMeshTopoData[ldID];
				Matrix3 tm = GetMeshTopoDataNode(ldID)->GetObjectTM(t);	
				for (int i = 0; i < ld->GetNumberFaces(); i++) 
				{
					if (ld->GetFaceSelected(i))
					{
						int degree = ld->GetFaceDegree(i);
						for (int j = 0; j < degree; j++) 
						{
							int id = ld->GetFaceGeomVert(i,j);//TVMaps.f[i]->v[j];
							box += ld->GetGeomVert(id) * tm;
							ct++;
						}
					}
				}
			}
		}

		if (ct ==0) return;

		mUnwrapNodeDisplayCallback.mBounds = box;

		cb->Center (box.Center(),0);
	}
	
}

void UnwrapMod::GetSubObjectTMs(
								SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc)
{

	if ( (ip && (ip->GetSubObjectLevel() == 3) && (fnGetMapMode() == SPLINEMAP) ))
	{
		Tab<int> selSplines;
		Tab<int> selCrossSections;
		mSplineMap.GetSelectedCrossSections(selSplines, selCrossSections);
		//see if we have a cross selection selected
		//if not use the face selection
		if (selCrossSections.Count() == 0)
		{
			BOOL hasSelection = FALSE;
			for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
			{
				MeshTopoData *ld = mMeshTopoData[ldID];
				BitArray faceSel = ld->GetFaceSel();
				int numberSet = (int)faceSel.NumberSet();
				if ((int)faceSel.NumberSet() > 0)
				{
					hasSelection = TRUE;
				}
			}
			Box3 bounds;
			bounds.Init();
			for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
			{
				Matrix3 tm = mMeshTopoData.GetNodeTM(t,ldID);
				MeshTopoData *ld = mMeshTopoData[ldID];
				if (hasSelection)
				{
					bounds += ld->GetSelFaceBoundingBox()* tm;
				}
				else
				{
					bounds += ld->GetBoundingBox() * tm;
				}
			}
			Matrix3 tm(1);
			tm.SetTrans(bounds.Center());
			cb->TM(tm,0);
		}
		else
		{
			SplineCrossSection *section = mSplineMap.GetCrossSection(selSplines[0],selCrossSections[0]);
			Point3 p = section->mTM.GetTrans();

			Matrix3 tm(1);
			tm = section->mTM;
			tm.NoScale();
//			tm.SetTrans(p);
			cb->TM(tm,0);
		}
	}

	else if ( (ip && (ip->GetSubObjectLevel() == 3)  && (fnGetMapMode() != NOMAP) ))
	{
		Matrix3 ntm = node->GetObjectTM(t), modmat;
		modmat = GetMapGizmoMatrix(t);
		cb->TM(modmat,0);
	}
}



int UnwrapMod::HitTest (TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt, ModContext* mc) {
	Interval valid;
	int savedLimits = 0, res = 0;
	GraphicsWindow *gw = vpt->getGW();
	HitRegion hr;
	MeshTopoData *ld = (MeshTopoData *) mc->localData;


	if ( fnGetMapMode() == SPLINEMAP)
	{		
		INode *splineNode = NULL;
		pblock->GetValue(unwrap_splinemap_node,t,splineNode,FOREVER);
		BOOL displaySplineMap = FALSE;
		pblock->GetValue(unwrap_splinemap_display,t,displaySplineMap,FOREVER);
		if (splineNode && displaySplineMap)
		{
				SplineMapProjectionTypes projType = kCircular;
				int val = 0;
				pblock->GetValue(unwrap_splinemap_projectiontype,t,val,FOREVER);
				if (val == 1)
					projType = kPlanar;
				MakeHitRegion(hr,type, crossing,4,p);
				gw->setHitRegion(&hr);
				Matrix3 tm = inode->GetObjectTM(t);
				res = mSplineMap.HitTest(vpt,inode,mc, tm, hr,flags,projType, TRUE );
				return res;
		}
	}
	else if ( (peltData.GetEditSeamsMode()))
	{
		MakeHitRegion(hr,type, crossing,4,p);
		gw->setHitRegion(&hr);
		Matrix3 mat = inode->GetObjectTM(t);
		gw->setTransform(mat);	
		gw->setRndLimits(((savedLimits = gw->getRndLimits()) | GW_PICK) & ~GW_ILLUM);

		if (oldSelMethod)
		{
			if (type == HITTYPE_POINT)
				gw->setRndLimits(gw->getRndLimits() & GW_BACKCULL);
			else gw->setRndLimits(gw->getRndLimits() & ~GW_BACKCULL);
		}
		else
		{
			if (ignoreBackFaceCull) gw->setRndLimits(gw->getRndLimits() |  GW_BACKCULL);
			else gw->setRndLimits(gw->getRndLimits() & ~GW_BACKCULL);
		}

		res = peltData.HitTestEdgeMode(this, ld, vpt,gw,hr,inode, mc, flags,type);
		gw->setRndLimits(savedLimits);	
		return res;	
	}
	else if (ip && (ip->GetSubObjectLevel() ==1) )
	{

		if (( mode == ID_UNWRAP_WELD) ||
               (mode == ID_TOOL_WELD) )
			   type = HITTYPE_POINT;
		MakeHitRegion(hr,type, crossing,4,p);
		gw->setHitRegion(&hr);
		Matrix3 mat = inode->GetObjectTM(t);
		gw->setTransform(mat);	
		gw->setRndLimits(((savedLimits = gw->getRndLimits()) | GW_PICK) & ~GW_ILLUM);

		if (oldSelMethod)
		{
			if (type == HITTYPE_POINT)
				gw->setRndLimits(gw->getRndLimits() & GW_BACKCULL);
			else gw->setRndLimits(gw->getRndLimits() & ~GW_BACKCULL);
		}
		else
		{
			if (ignoreBackFaceCull) gw->setRndLimits(gw->getRndLimits() |  GW_BACKCULL);
			else gw->setRndLimits(gw->getRndLimits() & ~GW_BACKCULL);
		}

		gw->clearHitCode();

		BOOL add = GetKeyState(VK_CONTROL)<0;
		BOOL sub = GetKeyState(VK_MENU)<0;
		BOOL polySelect = !(GetKeyState(VK_SHIFT)<0);

		if (add)
			ip->ReplacePrompt( GetString(IDS_PW_MOUSE_ADD));
		else if (sub)
			ip->ReplacePrompt( GetString(IDS_PW_MOUSE_SUBTRACT));
		else if (!polySelect)
			ip->ReplacePrompt( GetString(IDS_PW_MOUSE_SELECTTRI));
		else ip->ReplacePrompt( GetString(IDS_PW_MOUSE_SELECTFACE));

		//build our visible face
		BitArray visibleFaces;
		BuildVisibleFaces(gw, ld, visibleFaces);

		Tab<UVWHitData> hitVerts;
		HitGeomVertData(ld,hitVerts,gw,  hr);


		BitArray visibleVerts;
		visibleVerts.SetSize(ld->GetNumberGeomVerts());


		if (fnGetBackFaceCull())
		{
			visibleVerts.ClearAll();

			for (int i = 0; i < ld->GetNumberFaces(); i++)
			{
				if (visibleFaces[i])
				{
					int deg = ld->GetFaceDegree(i);
					for (int j = 0; j < deg; j++)
					{
						visibleVerts.Set(ld->GetFaceGeomVert(i,j),TRUE);
					}
				}
			}

		}
		else
		{
			visibleVerts.SetAll();
		}



		for (int hi = 0; hi < hitVerts.Count(); hi++)
		{
			int i = hitVerts[hi].index;
			//						if (hitEdges[i])
			{



				if (fnGetBackFaceCull())
				{
					if (visibleVerts[i])
					{
						if ( (ld->GetGeomVertSelected(i) && (flags&HIT_SELONLY)) ||
							!(flags&(HIT_UNSELONLY|HIT_SELONLY)))
							vpt->LogHit(inode,mc,0.0f,i,NULL);
						else if ( (!ld->GetGeomVertSelected(i) && (flags&HIT_UNSELONLY)) ||
							!(flags&(HIT_UNSELONLY|HIT_SELONLY)))
							vpt->LogHit(inode,mc,0.0f,i,NULL);

					}
				}
				else
				{

					if ( (ld->GetGeomVertSelected(i) && (flags&HIT_SELONLY)) ||
						!(flags&(HIT_UNSELONLY|HIT_SELONLY)))
						vpt->LogHit(inode,mc,0.0f,i,NULL);
					else if ( (!ld->GetGeomVertSelected(i) && (flags&HIT_UNSELONLY)) ||
						!(flags&(HIT_UNSELONLY|HIT_SELONLY)))
						vpt->LogHit(inode,mc,0.0f,i,NULL);

				}
			}
		}

	}
	if (ip && (ip->GetSubObjectLevel() ==2) )
	{

		MakeHitRegion(hr,type, crossing,4,p);
		gw->setHitRegion(&hr);
		Matrix3 mat = inode->GetObjectTM(t);
		gw->setTransform(mat);	
		gw->setRndLimits(((savedLimits = gw->getRndLimits()) | GW_PICK) & ~GW_ILLUM);

		if (oldSelMethod)
		{
			if (type == HITTYPE_POINT)
				gw->setRndLimits(gw->getRndLimits() & GW_BACKCULL);
			else gw->setRndLimits(gw->getRndLimits() & ~GW_BACKCULL);
		}
		else
		{
			if (ignoreBackFaceCull) gw->setRndLimits(gw->getRndLimits() |  GW_BACKCULL);
			else gw->setRndLimits(gw->getRndLimits() & ~GW_BACKCULL);
		}

		gw->clearHitCode();

		BOOL add = GetKeyState(VK_CONTROL)<0;
		BOOL sub = GetKeyState(VK_MENU)<0;
		BOOL polySelect = !(GetKeyState(VK_SHIFT)<0);

		if (add)
			ip->ReplacePrompt( GetString(IDS_PW_MOUSE_ADD));
		else if (sub)
			ip->ReplacePrompt( GetString(IDS_PW_MOUSE_SUBTRACT));
		else if (!polySelect)
			ip->ReplacePrompt( GetString(IDS_PW_MOUSE_SELECTTRI));
		else ip->ReplacePrompt( GetString(IDS_PW_MOUSE_SELECTFACE));

		//build our visible face


		if (peltData.GetPointToPointSeamsMode())
		{
			res = peltData.HitTestPointToPointMode(this,ld,vpt,gw,p,hr,inode,mc);
		}
		else
		{
			BitArray visibleFaces;

			BuildVisibleFaces(gw, ld, visibleFaces);

			//hit test our geom edges
			Tab<UVWHitData> hitEdges;
			HitGeomEdgeData(ld,hitEdges,gw,  hr);		
			res = hitEdges.Count();
			if (type == HITTYPE_POINT)
			{
				int closestIndex = -1;
				float closest=  0.0f;
				Matrix3 toView(1);
				vpt->GetAffineTM(toView);
				toView = mat * toView;
				for (int hi = 0; hi < hitEdges.Count(); hi++)
				{
					int eindex = hitEdges[hi].index;
					int a = ld->GetGeomEdgeVert(eindex,0);

					Point3 p = ld->GetGeomVert(a) * toView;
					if ((p.z > closest) || (closestIndex==-1))
					{
						closest = p.z ;
						closestIndex = hi;
					}
				}
				if (closestIndex != -1)
				{
					Tab<UVWHitData> tempHitEdge;
					tempHitEdge.Append(1,&hitEdges[closestIndex],1);
					hitEdges = tempHitEdge;
				}
			}

			for (int hi = 0; hi < hitEdges.Count(); hi++)
			{
				int i = hitEdges[hi].index;
				{
					BOOL visibleFace = FALSE;
					BOOL selFace = FALSE;
					int ct = ld->GetGeomEdgeNumberOfConnectedFaces(i);
					for (int j = 0; j < ct; j++)
					{
						int faceIndex =ld->GetGeomEdgeConnectedFace(i,j);
						if ((faceIndex < ld->GetNumberFaces()))
							selFace = TRUE;
						if ((faceIndex < visibleFaces.GetSize()) && (visibleFaces[faceIndex]))
							visibleFace = TRUE;

					}

					if (fnGetBackFaceCull())
					{
						if (selFace && visibleFace)
						{
							
							if ( (ld->GetGeomEdgeSelected(i) && (flags&HIT_SELONLY)) ||
								!(flags&(HIT_UNSELONLY|HIT_SELONLY)))
								vpt->LogHit(inode,mc,0.0f,i,NULL);
							else if ( (!ld->GetGeomEdgeSelected(i) && (flags&HIT_UNSELONLY)) ||
								!(flags&(HIT_UNSELONLY|HIT_SELONLY)))
								vpt->LogHit(inode,mc,0.0f,i,NULL);

						}
					}
					else
					{
						if (selFace )
						{
							if ( (ld->GetGeomEdgeSelected(i) && (flags&HIT_SELONLY)) ||
								!(flags&(HIT_UNSELONLY|HIT_SELONLY)))
								vpt->LogHit(inode,mc,0.0f,i,NULL);
							else if ( (!ld->GetGeomEdgeSelected(i) && (flags&HIT_UNSELONLY)) ||
								!(flags&(HIT_UNSELONLY|HIT_SELONLY)))
								vpt->LogHit(inode,mc,0.0f,i,NULL);

						}
					}
				}
			}
		}


	}
	else if (ip && (ip->GetSubObjectLevel() == 3) )
	{
		MakeHitRegion(hr,type, crossing,4,p);
		gw->setHitRegion(&hr);
		Matrix3 mat = inode->GetObjectTM(t);
		gw->setTransform(mat);	
		gw->setRndLimits(((savedLimits = gw->getRndLimits()) | GW_PICK) & ~GW_ILLUM);

		if (oldSelMethod)
		{
			if (type == HITTYPE_POINT)
				gw->setRndLimits(gw->getRndLimits() & GW_BACKCULL);
			else gw->setRndLimits(gw->getRndLimits() & ~GW_BACKCULL);
		}
		else
		{
			if (ignoreBackFaceCull) gw->setRndLimits(gw->getRndLimits() |  GW_BACKCULL);
			else gw->setRndLimits(gw->getRndLimits() & ~GW_BACKCULL);
		}

		gw->clearHitCode();

		BOOL add = GetKeyState(VK_CONTROL)<0;
		BOOL sub = GetKeyState(VK_MENU)<0;
		BOOL polySelect = !(GetKeyState(VK_SHIFT)<0);

		if (add)
			ip->ReplacePrompt( GetString(IDS_PW_MOUSE_ADD));
		else if (sub)
			ip->ReplacePrompt( GetString(IDS_PW_MOUSE_SUBTRACT));
		else if (!polySelect)
			ip->ReplacePrompt( GetString(IDS_PW_MOUSE_SELECTTRI));
		else ip->ReplacePrompt( GetString(IDS_PW_MOUSE_SELECTFACE));


		if ( ( fnGetMapMode() != NOMAP) && ( fnGetMapMode() != SPLINEMAP) )
		{
			if ( peltData.GetPeltMapMode() 
				&& (peltData.GetPointToPointSeamsMode() || peltData.GetEditSeamsMode())
				)
			{
			}
			else
			{
				DrawGizmo(t, inode, gw);
				if (gw->checkHitCode()) 
				{
					vpt->LogHit(inode, mc, gw->getHitDistance(), 0, NULL); 
					res = 1;
				}
			}
		}
		else if (((MeshTopoData*)mc->localData)->GetMesh())
		{
			SubObjHitList hitList;
			MeshSubHitRec *rec;	


			Mesh &mesh = *((MeshTopoData*)mc->localData)->GetMesh();
			mesh.faceSel = ((MeshTopoData*)mc->localData)->GetFaceSel();

			//PELT
			if ((fnGetMapMode() == NOMAP) || (fnGetMapMode() == SPLINEMAP))
			{
				res = mesh.SubObjectHitTest(gw, gw->getMaterial(), &hr,
					flags|SUBHIT_FACES|SUBHIT_SELSOLID, hitList);

				rec = hitList.First();
				while (rec) {
					vpt->LogHit(inode,mc,rec->dist,rec->index,NULL);
					rec = rec->Next();
				}
			}

		}
		else if (((MeshTopoData*)mc->localData)->GetPatch())
		{
			SubPatchHitList hitList;


			PatchMesh &patch = *((MeshTopoData*)mc->localData)->GetPatch();
			patch.patchSel = ((MeshTopoData*)mc->localData)->GetFaceSel();

			if ((fnGetMapMode() == NOMAP) || (fnGetMapMode() == SPLINEMAP))
			{
				if (ignoreBackFaceCull)
					res = patch.SubObjectHitTest(gw, gw->getMaterial(), &hr,
					flags|SUBHIT_PATCH_PATCHES|SUBHIT_SELSOLID|SUBHIT_PATCH_IGNORE_BACKFACING, hitList);
				else res = patch.SubObjectHitTest(gw, gw->getMaterial(), &hr,
					flags|SUBHIT_PATCH_PATCHES|SUBHIT_SELSOLID, hitList);

				PatchSubHitRec *rec = hitList.First();
				while (rec) {
					vpt->LogHit(inode,mc,rec->dist,rec->index,NULL);
					rec = rec->Next();
				}
			}

		}
		else if (((MeshTopoData*)mc->localData)->GetMNMesh())
		{
			SubObjHitList hitList;
			MeshSubHitRec *rec;	


			MNMesh &mesh = *((MeshTopoData*)mc->localData)->GetMNMesh();
			mesh.FaceSelect(((MeshTopoData*)mc->localData)->GetFaceSel());

			if ((fnGetMapMode() == NOMAP) || (fnGetMapMode() == SPLINEMAP))
			{
				res = mesh.SubObjectHitTest(gw, gw->getMaterial(), &hr,
					flags|SUBHIT_MNFACES|SUBHIT_SELSOLID, hitList);

				rec = hitList.First();
				while (rec) {
					vpt->LogHit(inode,mc,rec->dist,rec->index,NULL);
					rec = rec->Next();
				}
			}
		}
	}
	gw->setRndLimits(savedLimits);	

	return res;	
}




void UnwrapMod::SelectSubComponent (HitRecord *hitRec, BOOL selected, BOOL all, BOOL invert) 
{





	BitArray set;
	//BitArray tempset;
	BOOL sub = GetKeyState(VK_MENU)<0;
	BOOL polySelect = !(GetKeyState(VK_SHIFT)<0);

	GetCOREInterface()->ClearCurNamedSelSet();
	MeshTopoData* ld = NULL;

	if (theHold.Holding()) 
		HoldSelection();



	//spline map selection
	if (fnGetMapMode() == SPLINEMAP )
	{
		if (theHold.Holding()) 
			mSplineMap.HoldData();

		
			while (hitRec) 
			{	
				int splineIndex = hitRec->hitInfo;
				int index = hitRec->distance;
				BOOL state = selected;

				if (invert) 
					state = !mSplineMap.CrossSectionIsSelected(splineIndex,index);
				bool newState = false;
				if (state) 
					newState = true;					
				else       
					newState = false;
				mSplineMap.CrossSectionSelect(splineIndex,index,newState);
				TSTR functionCall;
				functionCall.printf("$.modifiers[#unwrap_uvw].unwrap6.splineMap_selectCrossSection");
				macroRecorder->FunctionCall(functionCall, 3, 0,
					mr_int,splineIndex+1,
					mr_int,index+1,
					mr_bool,newState);
				macroRecorder->EmitScript();
				if (!all) break;

				hitRec = hitRec->Next();
			}
		
		NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
		if (ip) ip->RedrawViews(ip->GetTime());
		return;
	}
	//PELT
	if ((peltData.GetEditSeamsMode()))
	{
		
		MeshTopoData *ld = (MeshTopoData*)hitRec->modContext->localData;
		if (ld->mSeamEdges.GetSize() != ld->GetNumberGeomEdges())//TVMaps.gePtrList.Count())
		{
			ld->mSeamEdges.SetSize(ld->GetNumberGeomEdges());//TVMaps.gePtrList.Count());
			ld->mSeamEdges.ClearAll();
		}

		if (theHold.Holding()) 
			theHold.Put (new UnwrapPeltSeamRestore (this,ld));

		//		if (objType == IS_MESH)
		{
			while (hitRec) 
			{					
				//				d = (MeshTopoData*)hitRec->modContext->localData;	


				set.ClearAll();
				int index = hitRec->hitInfo;
				if ((index >= 0) && (index < ld->mSeamEdges.GetSize()))
				{

					BOOL state = selected;

					if (invert) state = !ld->mSeamEdges[hitRec->hitInfo];
					if (state) ld->mSeamEdges.Set(hitRec->hitInfo);
					else       ld->mSeamEdges.Clear(hitRec->hitInfo);
					if (!all) break;
				}

				hitRec = hitRec->Next();

			}
		}

		TSTR functionCall;
		INode *node = NULL;
		for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
		{
			if (ld == mMeshTopoData[ldID])
				node = mMeshTopoData.GetNode(ldID);
		}
		functionCall.printf("$%s.modifiers[#unwrap_uvw].unwrap6.setPeltSelectedSeamsByNode",node->GetName());
		macroRecorder->FunctionCall(functionCall, 2, 0,
			mr_bitarray,&(ld->mSeamEdges),
			mr_reftarg,node
			);
		NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
		if (ip) ip->RedrawViews(ip->GetTime());
		
	}
	else
	{
		if ( (ip && (ip->GetSubObjectLevel() == 1)  ))
		{
			HitRecord *fistHitRec = hitRec;
			for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
			{
				BitArray holdSel =  mMeshTopoData[ldID]->GetGeomVertSelection();//gvsel;
				BOOL hit = FALSE;
				while (hitRec) 
				{					
					ld = (MeshTopoData*)hitRec->modContext->localData;

					if (ld == mMeshTopoData[ldID])
					{
						BOOL state = selected;
						//6-29--99 watje
						if (hitRec->hitInfo < ld->GetNumberGeomVerts())
						{
							if (invert) 
								state = !ld->GetGeomVertSelected(hitRec->hitInfo);
							ld->SetGeomVertSelected(hitRec->hitInfo,state);
							hit = TRUE;
						}

						if (!all) break;
					}

					hitRec = hitRec->Next();
				}
				
				if (geomElemMode && hit) 
					SelectGeomElement(mMeshTopoData[ldID],!sub,&holdSel);
				hitRec = fistHitRec;
				MeshTopoData *ld = mMeshTopoData[ldID];
				TSTR functionCall;
				INode *node = mMeshTopoData.GetNode(ldID);
				functionCall.printf("$%s.modifiers[#unwrap_uvw].unwrap6.setSelectedGeomVertsByNode",node->GetName());
				macroRecorder->FunctionCall(functionCall, 2, 0,
					mr_bitarray,ld->GetGeomVertSelectionPtr(),
					mr_reftarg,node
					);
					
			}
		}
		
		else if ( (ip && (ip->GetSubObjectLevel() == 2)  ))
		{
			HitRecord *fistHitRec = hitRec;
			for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
			{
				BOOL hit = FALSE;
				
				BitArray holdSel;// = ld->GetGeomEdgeSelection();//gesel;
				while (hitRec) 
				{		
					ld = (MeshTopoData*)hitRec->modContext->localData;
					if (ld == mMeshTopoData[ldID])
					{
						if (!hit)
							holdSel = ld->GetGeomEdgeSelection();//gesel;

						BOOL state = selected;
						//6-29--99 watje
						if (invert) 
							state = !ld->GetGeomEdgeSelected(hitRec->hitInfo);//gesel[hitRec->hitInfo];
						if (state) 
							ld->SetGeomEdgeSelected(hitRec->hitInfo,TRUE);
						else       
							ld->SetGeomEdgeSelected(hitRec->hitInfo,FALSE);
						hit = TRUE;
						if (!all) break;

						
					}
					hitRec = hitRec->Next();
				}
				if (geomElemMode && hit) 
					SelectGeomElement(mMeshTopoData[ldID],!sub,&holdSel);

				MeshTopoData *ld = mMeshTopoData[ldID];
				TSTR functionCall;
				INode *node = mMeshTopoData.GetNode(ldID);
				functionCall.printf("$%s.modifiers[#unwrap_uvw].unwrap6.setSelectedGeomEdgesByNode",node->GetName());
				macroRecorder->FunctionCall(functionCall, 2, 0,
					mr_bitarray,ld->GetGeomEdgeSelectionPtr(),
					mr_reftarg,node
					);

				hitRec = fistHitRec;
			}
		}		
		else if ( (ip && (ip->GetSubObjectLevel() == 3)  ) && (( fnGetMapMode() == NOMAP) || ( fnGetMapMode() == SPLINEMAP)) )
		{
			//we need to collect like LD together
			HitRecord *fistHitRec = hitRec;
			MeshTopoData *firstLD = (MeshTopoData *) fistHitRec->modContext->localData;

			for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
			{
				INode *node = mMeshTopoData.GetNode(ldID);
				MeshTopoData *ld = NULL;
				AdjFaceList *al = NULL;

				BitArray holdFaceSel = mMeshTopoData[ldID]->GetFaceSelection();

				BitArray saveSel;

				BOOL hit = FALSE;
				while (hitRec) 
				{
					if ((ld == NULL) && (mMeshTopoData[ldID] == hitRec->modContext->localData) || (!all))
					{
						if (all)
							ld = mMeshTopoData[ldID];
						else
							ld = firstLD;
						if (ld->GetMesh())
						{
							if (polySelect)
							{
								al = BuildAdjFaceList(*ld->GetMesh());
								set.SetSize(ld->GetMesh()->numFaces);
							}
						}
						else if (ld->GetMNMesh())
						{
						}
						else if (ld->GetPatch())
						{
						}
						
						if ((planarMode) && (sub))  //hmm hack alert since we are doing a planar selection removal we need to trick the system
						{
							saveSel.SetSize(ld->GetFaceSel().GetSize());  //save our current sel
							saveSel = ld->GetFaceSel();					  //then clear it ans set the system to normal select
							BitArray clearSel = ld->GetFaceSel();	
							clearSel.ClearAll();				  //we will then fix the selection after the hitrecs
							ld->SetFaceSel(clearSel);
							selected = TRUE;
							hit = TRUE;
						}
					}
					if (ld && (mMeshTopoData[ldID] == hitRec->modContext->localData) || (!all))
					{
						if (ld->GetMesh() && polySelect)
						{
							set.ClearAll();

							BOOL degenerateFace = FALSE;
							for (int k = 0; k < 3; k++)
							{
								int idA = ld->GetFaceGeomVert(hitRec->hitInfo,k);
								int idB = ld->GetFaceGeomVert(hitRec->hitInfo,(k+1)%3);
								if (idA == idB)
									degenerateFace = TRUE;
							}

							
							if (degenerateFace)
							{
								set.SetSize(ld->GetNumberFaces());
								set.ClearAll();
								set.Set(hitRec->hitInfo);
							}
							else
							{
								ld->GetMesh()->PolyFromFace (hitRec->hitInfo, set, 45.0, FALSE, al);
							}
							BitArray faceSel = ld->GetFaceSelection();
							
							


							if (invert) 
								faceSel ^= set;
							else if (selected) 
								faceSel |= set;
							else 
								faceSel &= ~set;
							ld->SetFaceSelection(faceSel);
							hit = TRUE;
							if (!all) 
							{
								ldID = mMeshTopoData.Count();
								break;							
							}
						}
						else 
						{
							BOOL state = selected;
							//6-29--99 watje
							if (invert) 
								state = !ld->GetFaceSelected(hitRec->hitInfo);
							ld->SetFaceSelected(hitRec->hitInfo,state);
							hit = TRUE;
							if (!all) 
							{
								ldID = mMeshTopoData.Count();
								break;
							}

						}
					}
					hitRec = hitRec->Next();
				}
				if (al) 
					delete al;
				hitRec = fistHitRec;

				if (ld)
				{
					if (planarMode) 
						SelectGeomFacesByAngle(ld);

					if ((planarMode) && (sub))
					{
						saveSel &= ~ld->GetFaceSelection();//faceSel;
						ld->SetFaceSelection(saveSel);
	//					d->faceSel = saveSel;
					}
					
					if (geomElemMode && hit) 
						SelectGeomElement(ld,!sub,&holdFaceSel);


					TSTR functionCall;					
					CleanUpGeoSelection(ld);
					functionCall.printf("$%s.modifiers[#unwrap_uvw].unwrap6.selectFacesByNode",node->GetName());
					macroRecorder->FunctionCall(functionCall, 2, 0,
							mr_bitarray,ld->GetFaceSelectionPtr(),
							mr_reftarg,node
							);
				}
			}
			
			if (filterSelectedFaces == 1) 
			{
				for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
				{
					mMeshTopoData[ldID]->BuilFilterSelectedFaces(filterSelectedFaces);
				}				
				InvalidateView();
			}

		}
		
		ComputeSelectedFaceData();

		

		if (fnGetSyncSelectionMode()) 
			fnSyncTVSelection();
		RebuildDistCache();
		theHold.Accept (GetString (IDS_DS_SELECT));

		NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
		SetNumSelLabel();
	}
}

void UnwrapMod::ClearSelection(int selLevel)
{

	if ( fnGetMapMode() == SPLINEMAP)
	{
		BOOL selectCrossSection = FALSE;
		INode *splineNode = NULL;
		TimeValue t = GetCOREInterface()->GetTime();
		pblock->GetValue(unwrap_splinemap_node,t,splineNode,FOREVER);
		BOOL displaySplineMap = FALSE;
		pblock->GetValue(unwrap_splinemap_display,t,displaySplineMap,FOREVER);
		if (splineNode && displaySplineMap)
		{
				if (theHold.Holding()) 
					mSplineMap.HoldData();

					fnSplineMap_ClearCrossSectionSelection();
					macroRecorder->FunctionCall(_T("$.modifiers[#unwrap_uvw].unwrap6.SplineMap_ClearSelectCrossSection"), 0, 0);


				NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
				if (ip) ip->RedrawViews(ip->GetTime());
				return;
		}
	}

	//PELT
	//we dont clear the selection when in seams mode since the user can easily wipe out 
	//his previous seams
	if ((peltData.GetPeltMapMode()) && (peltData.GetEditSeamsMode()))
	{
		return;
	}
	if ( (peltData.GetPointToPointSeamsMode()))
	{		
		return;
	}


	//loop through the local data list
	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];
		if (!ld) 
			continue;


		//hold our current selection
		if (theHold.Holding()) 
			theHold.Put (new TSelRestore (this,ld));

		switch (selLevel) 
		{
			case 1:
				{
					BitArray gvsel = ld->GetGeomVertSelection();
					gvsel.ClearAll();
					ld->SetGeomVertSelection(gvsel);


					BitArray vsel = ld->GetTVVertSelection();
					vsel.ClearAll();
					ld->SetTVVertSelection(vsel);
					break;
				}
			case 2:
				{
					BitArray gesel = ld->GetGeomEdgeSelection();
					gesel.ClearAll();
					ld->SetGeomEdgeSelection(gesel);

//					ld->gesel.ClearAll();
					break;
				}
			case 3:
				{
					if ((fnGetMapMode() == NOMAP) || (fnGetMapMode() == SPLINEMAP))
					{
						BitArray fsel = ld->GetFaceSel();
						fsel.ClearAll();
						ld->SetFaceSel(fsel);
					}
					break;
				}
		}
	}


	theHold.Suspend();
	if (fnGetSyncSelectionMode()) 
		fnSyncTVSelection();
	theHold.Resume();


	GetCOREInterface()->ClearCurNamedSelSet();
	NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	SetNumSelLabel();
	//update our views to show new faces
	InvalidateView();


}
void UnwrapMod::SelectAll(int selLevel)
{


	if ( fnGetMapMode() == SPLINEMAP)
	{
		BOOL selectCrossSection = FALSE;
		INode *splineNode = NULL;
		TimeValue t = GetCOREInterface()->GetTime();
		pblock->GetValue(unwrap_splinemap_node,t,splineNode,FOREVER);
		BOOL displaySplineMap = FALSE;
		pblock->GetValue(unwrap_splinemap_display,t,displaySplineMap,FOREVER);
		if (splineNode && displaySplineMap)
		{
			{	
				if (theHold.Holding()) 
					mSplineMap.HoldData();
				for (int i = 0; i < mSplineMap.NumberOfSplines(); i++)
				{
					if (mSplineMap.IsSplineSelected(i))
					{
						{
							for (int j = 0; j < mSplineMap.NumberOfCrossSections(i); j++)
								mSplineMap.CrossSectionSelect(i,j,TRUE);
						}
					}
				}
				NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
				TimeValue t = GetCOREInterface()->GetTime();
				if (ip) ip->RedrawViews(t);
				return;
			}
		}
	}

	//PELT
	if ((peltData.GetPeltMapMode()) && (peltData.GetEditSeamsMode()))
	{
		for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
		{
			MeshTopoData *ld = mMeshTopoData[ldID];
			ld->mSeamEdges.SetAll();
		}
//		peltData.seamEdges.SetAll();
		return;
	}
	if ((peltData.GetPeltMapMode()) && (peltData.GetPointToPointSeamsMode()))
	{		
		return;
	}

	if (theHold.Holding()) 
		HoldSelection();

	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];
		if (!ld) 
			continue;


		switch (selLevel) 
		{
		case 1:
			ld->AllSelection(TVVERTMODE);
			break;
		case 2:
			ld->AllSelection(TVEDGEMODE);
			break;
		case 3:
			ld->AllSelection(TVFACEMODE);
			break;
		}

	}

	GetCOREInterface()->ClearCurNamedSelSet();
	if (fnGetSyncSelectionMode()) 
		fnSyncTVSelection();
	NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	SetNumSelLabel();
	//update our views to show new faces

	InvalidateView();
}

void UnwrapMod::InvertSelection(int selLevel)
{

	if ( fnGetMapMode() == SPLINEMAP)
	{
		BOOL selectCrossSection = FALSE;
		INode *splineNode = NULL;
		TimeValue t = GetCOREInterface()->GetTime();
		pblock->GetValue(unwrap_splinemap_node,t,splineNode,FOREVER);
		BOOL displaySplineMap = FALSE;
		pblock->GetValue(unwrap_splinemap_display,t,displaySplineMap,FOREVER);
		if (splineNode && displaySplineMap)
		{
				if (theHold.Holding()) 
					mSplineMap.HoldData();
				for (int i = 0; i < mSplineMap.NumberOfSplines(); i++)
				{
					if (mSplineMap.IsSplineSelected(i))
					{
						{
							for (int j = 0; j < mSplineMap.NumberOfCrossSections(i); j++)
							{
								if (mSplineMap.CrossSectionIsSelected(i,j))
									mSplineMap.CrossSectionSelect(i,j,FALSE);
								else
									mSplineMap.CrossSectionSelect(i,j,TRUE);

							}
						}
					}
				}
				NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
				TimeValue t = GetCOREInterface()->GetTime();
				if (ip) ip->RedrawViews(t);

				return;
		}
	}

	//PELT
	if ((peltData.GetPeltMapMode()) && (peltData.GetEditSeamsMode()))
		return;

	if (theHold.Holding()) 
		HoldSelection();

	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];
		if (!ld) 
			continue;
		switch (selLevel) 
		{
		case 1:
			{
				ld->InvertSelection(TVVERTMODE);
				break;
			}
		case 2:
			{
				ld->InvertSelection(TVEDGEMODE);
				break;
			}
		case 3:
			{
				ld->InvertSelection(TVFACEMODE);
				break;
			}
		}
	}

	GetCOREInterface()->ClearCurNamedSelSet();
	if (fnGetSyncSelectionMode()) 
		fnSyncTVSelection();
	NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	SetNumSelLabel();
	//update our views to show new faces
	InvalidateView();


}




void UnwrapMod::ActivateSubobjSel(int level, XFormModes& modes) {
	// Fill in modes with our sub-object modes

	TimeValue t = GetCOREInterface()->GetTime();
	if (level ==0)
	{


		if (fnGetMapMode() != PELTMAP)
			fnSetMapMode(NOMAP);
		if (peltData.GetPointToPointSeamsMode())
			peltData.SetPointToPointSeamsMode(this,FALSE);

		modes = XFormModes(NULL,NULL,NULL,NULL,NULL,selectMode);
		EnableMapButtons(FALSE);
		EnableFaceSelectionUI(FALSE);
		EnableEdgeSelectionUI(FALSE);
		EnableSubSelectionUI(FALSE);
		if (fnGetMapMode() == PELTMAP)
			EnableAlignButtons(FALSE);
		peltData.EnablePeltButtons(hMapParams, FALSE);
		peltData.SetPointToPointSeamsMode(this,FALSE);
		peltData.SetEditSeamsMode(this,FALSE);


	}
	else if (level == 1)
	{		

		fnSetTVSubMode(TVVERTMODE);
		if (peltData.GetPointToPointSeamsMode())
			peltData.SetPointToPointSeamsMode(this,FALSE);

		SetupNamedSelDropDown();
		if (fnGetMapMode() != PELTMAP)
			fnSetMapMode(NOMAP);
		modes = XFormModes(NULL,NULL,NULL,NULL,NULL,selectMode);
		EnableMapButtons(FALSE);
		EnableFaceSelectionUI(FALSE);
		EnableEdgeSelectionUI(FALSE);
		EnableSubSelectionUI(TRUE); 	
		if (fnGetMapMode() == PELTMAP)
			EnableAlignButtons(FALSE);
		peltData.EnablePeltButtons(hMapParams, TRUE);

		peltData.SetPointToPointSeamsMode(this,FALSE);
		peltData.SetEditSeamsMode(this,FALSE);

	}
	else if (level == 2)
	{

		fnSetTVSubMode(TVEDGEMODE);
		SetupNamedSelDropDown();
		if (fnGetMapMode() != PELTMAP)
			fnSetMapMode(NOMAP);
		modes = XFormModes(NULL,NULL,NULL,NULL,NULL,selectMode);
		EnableMapButtons(FALSE);
		EnableFaceSelectionUI(FALSE);
		EnableEdgeSelectionUI(TRUE);
		EnableSubSelectionUI(TRUE);				
		if (fnGetMapMode() == PELTMAP)
			EnableAlignButtons(FALSE);
		peltData.EnablePeltButtons(hMapParams, TRUE);

		peltData.SetPointToPointSeamsMode(this,FALSE);
		peltData.SetEditSeamsMode(this,FALSE);

	}
	else if (level==3) 
	{
		if (peltData.GetPointToPointSeamsMode())
			peltData.SetPointToPointSeamsMode(this,FALSE);

		fnSetTVSubMode(TVFACEMODE);
		SetupNamedSelDropDown();
		//face select
		if (fnGetMapMode() == NOMAP)
			modes = XFormModes(NULL,NULL,NULL,NULL,NULL,selectMode);
		else if (fnGetMapMode() == SPLINEMAP)
		{
			modes = XFormModes(moveGizmoMode,rotGizmoMode,nuscaleGizmoMode,uscaleGizmoMode,squashGizmoMode,NULL);
			EnableAlignButtons(FALSE);				
		}
		else modes = XFormModes(moveGizmoMode,rotGizmoMode,nuscaleGizmoMode,uscaleGizmoMode,squashGizmoMode,NULL);
		EnableMapButtons(TRUE);		
		EnableFaceSelectionUI(TRUE);
		EnableEdgeSelectionUI(FALSE);
		EnableSubSelectionUI(TRUE);	

		if (fnGetMapMode() == PELTMAP)
		{
			if (peltData.peltDialog.hWnd)
				EnableAlignButtons(FALSE);				
			else EnableAlignButtons(TRUE); 
		}
		peltData.EnablePeltButtons(hMapParams, TRUE);

		peltData.SetPointToPointSeamsMode(this,FALSE);
		peltData.SetEditSeamsMode(this,FALSE);

	}
	/*
	if (level==4) 
	{
	SetupNamedSelDropDown();
	//face select
	modes = XFormModes(moveGizmoMode,rotGizmoMode,nuscaleGizmoMode,uscaleGizmoMode,squashGizmoMode,NULL);

	EnableMapButtons(TRUE);
	}
	*/
	SetNumSelLabel ();
	MoveScriptUI();

	InvalidateView();
	NotifyDependents(FOREVER,PART_DISPLAY,REFMSG_CHANGE);

}


Mtl* UnwrapMod::GetMultiMaterials(Mtl *base)
{
	Tab<Mtl*> materialStack;
	materialStack.Append(1,&base);
	while (materialStack.Count() != 0)
	{
		Mtl* topMaterial = materialStack[0];
		materialStack.Delete(0,1);
		//append any mtl
		for (int i = 0; i < topMaterial->NumSubMtls(); i++)
		{
			Mtl* addMat = topMaterial->GetSubMtl(i);
			if (addMat)
				materialStack.Append(1,&addMat,100);
		}


		if  (topMaterial->ClassID() == Class_ID(MULTI_CLASS_ID,0))
			return topMaterial;
	}
	return NULL;
}

Texmap*  UnwrapMod::GetActiveMap()
{
	if ((CurrentMap >= 0) && (CurrentMap < pblock->Count(unwrap_texmaplist)))
	{
		Texmap *map;
		pblock->GetValue(unwrap_texmaplist,0,map,FOREVER,CurrentMap);
		return map;
	}
	else return NULL;
}

Mtl* UnwrapMod::GetCheckerMap()
{
	Mtl *mtl = NULL;
	pblock->GetValue(unwrap_checkmtl,0,mtl,FOREVER);
	return mtl;
}
Mtl* UnwrapMod::GetOriginalMap()
{
	Mtl *mtl = NULL;
	pblock->GetValue(unwrap_checkmtl,0,mtl,FOREVER);
	return mtl;
}

void UnwrapMod::ResetMaterialList()
{
	//set the param block to 1
	pblock->SetCount(unwrap_texmaplist,0);
	pblock->SetCount(unwrap_texmapidlist,0);
	//get our mat list
	
	Mtl* checkerMat = GetCheckerMap();
	Mtl *baseMtl = NULL;

	pblock->SetCount(unwrap_originalmtl_list,mMeshTopoData.Count());
	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		INode *node = mMeshTopoData.GetNode(ldID);
		
		baseMtl = node->GetMtl();
		if (baseMtl == checkerMat)
		{
			pblock->GetValue(unwrap_originalmtl_list,0,baseMtl,FOREVER,ldID);
		}
		else
		{
			pblock->SetValue(unwrap_originalmtl_list,0,baseMtl,ldID);
		}
	}



	//add it to the pblock
	//fixpw
//	if (baseMtl)
	{
		Tab<Texmap*> tmaps;
		Tab<int> matIDs;
		for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
		{
			INode *node = mMeshTopoData.GetNode(ldID);
			baseMtl = node->GetMtl();
			if (baseMtl)
				ParseMaterials(baseMtl, tmaps, matIDs);
		}

		pblock->SetCount(unwrap_texmaplist,tmaps.Count());
		pblock->SetCount(unwrap_texmapidlist,tmaps.Count());



		if (checkerMat)
		{
			pblock->SetCount(unwrap_texmaplist,1+tmaps.Count());
			pblock->SetCount(unwrap_texmapidlist,1+tmaps.Count());

			Texmap *checkMap = NULL;
			checkMap = (Texmap *) checkerMat->GetSubTexmap(1);
			if (checkMap)
			{
				pblock->SetValue(unwrap_texmaplist,0,checkMap,0);
				int id = -1;
				pblock->SetValue(unwrap_texmapidlist,0,id,0);
			}
		}
		if ((baseMtl != NULL))
		{
			for (int i = 0; i < tmaps.Count(); i++)
			{
				pblock->SetValue(unwrap_texmaplist,0,tmaps[i],i+1);
				pblock->SetValue(unwrap_texmapidlist,0,matIDs[i],i+1);
			}
		}
	}

}
void UnwrapMod::AddToMaterialList(Texmap *map, int id)
{
	pblock->Append(unwrap_texmaplist,1,&map);
	pblock->Append(unwrap_texmapidlist,1,&id);
}
void UnwrapMod::DeleteFromMaterialList(int index)
{
	pblock->Delete(unwrap_texmaplist,index,1);
	pblock->Delete(unwrap_texmapidlist,index,1);
}

void UnwrapMod::ParseMaterials(Mtl *base, Tab<Texmap*> &tmaps, Tab<int> &matIDs)
{
	if (base==NULL) return;
	Tab<Mtl*> materialStack;
	materialStack.Append(1,&base);
	while (materialStack.Count() != 0)
	{
		Mtl* topMaterial = materialStack[0];
		materialStack.Delete(0,1);
		//append any mtl
		for (int i = 0; i < topMaterial->NumSubMtls(); i++)
		{
			Mtl* addMat = topMaterial->GetSubMtl(i);
			if (addMat &&  (topMaterial->ClassID() != Class_ID(MULTI_CLASS_ID,0)) )
				materialStack.Append(1,&addMat,100);
		}

		IDxMaterial *dxMaterial = (IDxMaterial *) topMaterial->GetInterface(IDXMATERIAL_INTERFACE);
		if (dxMaterial != NULL)
		{
			int numberBitmaps = dxMaterial->GetNumberOfEffectBitmaps();
			for (int i = 0; i < numberBitmaps; i++)
			{
				PBBitmap *pmap= dxMaterial->GetEffectBitmap(i);
				if (pmap)
				{
					//create new bitmap texture
					BitmapTex *bmtex = (BitmapTex *) CreateInstance(TEXMAP_CLASS_ID, Class_ID(BMTEX_CLASS_ID,0));
					//add it to the list			
					TSTR mapName;
					mapName.printf("%s",pmap->bi.Name());
					bmtex->SetMapName(mapName);
					Texmap *map = (Texmap *) bmtex;
					tmaps.Append(1,&map,100);
					int id = -1;
					matIDs.Append(1,&id,100);
				}
			}
		}
		else if  (topMaterial->ClassID() == Class_ID(MULTI_CLASS_ID,0))
		{
			MultiMtl *mtl = (MultiMtl*) topMaterial;
			IParamBlock2 *pb = mtl->GetParamBlock(0);
			if (pb)
			{	
				int numMaterials = pb->Count(multi_mtls);
				for (int i = 0; i < numMaterials; i++)
				{
					int id;
					Mtl *mat;
					pb->GetValue(multi_mtls,0,mat,FOREVER,i);
					pb->GetValue(multi_ids,0,id,FOREVER,i);

					if (mat)
					{
						int tex_count = mat->NumSubTexmaps();
						for (int j = 0; j < tex_count; j++)
						{
							Texmap *tmap;
							tmap = mat->GetSubTexmap(j);

							if (tmap != NULL)
							{
								tmaps.Append(1,&tmap,100);								
								matIDs.Append(1,&id,100);
							}
						}
					}
				}
			}
		}
		else
		{
			int tex_count = topMaterial->NumSubTexmaps();
			for (int i = 0; i < tex_count; i++)
			{
				Texmap *tmap = topMaterial->GetSubTexmap(i);
				if (tmap != NULL)
				{
					tmaps.Append(1,&tmap,100);	
					int id = -1;
					matIDs.Append(1,&id,100);
				}
			}
		}

	}
}
/*
Mtl *UnwrapMod::GetBaseMtl()
{
	MyEnumProc dep;              
	DoEnumDependents(&dep);
	if (dep.Nodes.Count() > 0)
		return dep.Nodes[0]->GetMtl();
	return NULL;
}
*/
/*
void UnwrapMod::LoadMaterials()
{
	//no entries on our material list lets add some based on the current assigned material in MtlBase


	Mtl *baseMtl = GetBaseMtl();
	if ((baseMtl != NULL))
	{
		Tab<Texmap*> tmaps;
		Tab<int> matIDs;
		ParseMaterials(baseMtl, tmaps, matIDs);
	}
}
*/

void UnwrapMod::BuildMatIDList()
{


	filterMatID.ZeroCount();

	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];
		for (int i = 0; i < ld->GetNumberFaces(); i++)//TVMaps.f.Count(); i++)
		{
			int found = 0;
			if (!ld->GetFaceDead(i))//(TVMaps.f[i]->flags & FLAG_DEAD))
			{
				for (int j = 0; j < filterMatID.Count(); j++)
				{
					if (filterMatID[j] == ld->GetFaceMatID(i))//TVMaps.f[i]->MatID) 
					{
						found = 1;
						j = filterMatID.Count();
					}
				}
			}
			else found = 1;
			if (found == 0)
			{
				int id = ld->GetFaceMatID(i);
				filterMatID.Append(1,&id);//TVMaps.f[i]->MatID,1);
			}
		}
	}
}


void UnwrapMod::ComputeFalloff(float &u, int ftype)

{
	if (u<= 0.0f) u = 0.0f;
	else if (u>= 1.0f) u = 1.0f;
	else switch (ftype)
	{
case (3) : u = u*u*u; break;
	//	case (BONE_FALLOFF_X2_FLAG) : u = u*u; break;
case (0) : u = u; break;
case (1) : u = 1.0f-((float)cos(u*PI) + 1.0f)*0.5f; break;
	//	case (BONE_FALLOFF_2X_FLAG) : u = (float) sqrt(u); break;
case (2) : u = (float) pow(u,0.3f); break;

	}

}


void UnwrapMod::DisableUI()
{
	EnableMapButtons(FALSE);
	EnableFaceSelectionUI(FALSE);
	EnableEdgeSelectionUI(FALSE);
	EnableSubSelectionUI(FALSE);
	EnableAlignButtons(FALSE);
	
	peltData.EnablePeltButtons(hMapParams, FALSE);

	EnableWindow(GetDlgItem(hParams,IDC_RADIO1),FALSE);
	EnableWindow(GetDlgItem(hParams,IDC_RADIO2),FALSE);
	EnableWindow(GetDlgItem(hParams,IDC_RADIO3),FALSE);
	EnableWindow(GetDlgItem(hParams,IDC_RADIO4),FALSE);
	EnableWindow(GetDlgItem(hParams,IDC_UNWRAP_EDIT),FALSE);
	EnableWindow(GetDlgItem(hParams,IDC_UNWRAP_RESET),FALSE);
	EnableWindow(GetDlgItem(hParams,IDC_UNWRAP_SAVE),FALSE);
	EnableWindow(GetDlgItem(hParams,IDC_UNWRAP_LOAD),FALSE);
	EnableWindow(GetDlgItem(hParams,IDC_MAP_CHAN1),FALSE);
	EnableWindow(GetDlgItem(hParams,IDC_MAP_CHAN2),FALSE);
	EnableWindow(GetDlgItem(hParams,IDC_DONOTREFLATTEN_CHECK),FALSE);

	EnableWindow(GetDlgItem(hParams,IDC_ALWAYSSHOWPELTSEAMS_CHECK),FALSE);
	EnableWindow(GetDlgItem(hParams,IDC_SHOWMAPSEAMS_CHECK),FALSE);
	EnableWindow(GetDlgItem(hParams,IDC_THINSEAM),FALSE);
	EnableWindow(GetDlgItem(hParams,IDC_THICKSEAM),FALSE);
}

void UnwrapMod::ModifyObject(
							 TimeValue t, ModContext &mc, ObjectState *os, INode *node)
{
	//if we have the apply to whole object flip the subobject state
	if (applyToWholeObject)
	{
#ifndef NO_PATCHES // orb 02-05-03
		if (os->obj->IsSubClassOf(patchObjectClassID))
		{
			PatchObject *pobj = (PatchObject*)os->obj;
			pobj->patch.selLevel = PATCH_OBJECT;
			pobj->patch.patchSel.ClearAll();
		}
		else 
#endif // NO_PATCHES
			if (os->obj->IsSubClassOf(triObjectClassID))
			{
//				isMesh = TRUE;
				TriObject *tobj = (TriObject*)os->obj;
				tobj->GetMesh().selLevel = MESH_OBJECT;
				tobj->GetMesh().faceSel.ClearAll();
			}
			else if (os->obj->IsSubClassOf(polyObjectClassID))
			{
				PolyObject *pobj = (PolyObject*)os->obj;
				pobj->GetMesh().selLevel = MNM_SL_OBJECT; 
				BitArray s;
				pobj->GetMesh().getFaceSel(s);
				s.ClearAll();
				pobj->GetMesh().FaceSelect(s);
			}
	}

	//poll for material on mesh
	int CurrentChannel = 0;

	if (channel == 0)
	{
		CurrentChannel = 1;
		//should be from scroller;
	}
	else if (channel == 1)
	{
		CurrentChannel = 0;
	}
	else CurrentChannel = channel;


//	isMesh = FALSE;
	currentTime = t;



	// Prepare the controller and set up mats

	//Get our local data
	MeshTopoData *d  = (MeshTopoData*)mc.localData;
	//if we dont have local data we need to build it and initialize it
	if (d==NULL)
	{
		d = new MeshTopoData(os,CurrentChannel);
		mc.localData = d;
	}

	//if we are not a mesh, poly mesh or patch convert and copy it into our object
	if (
#ifndef NO_PATCHES // orb 02-05-03
		(!os->obj->IsSubClassOf(patchObjectClassID)) && 
#endif // NO_PATCHES		
		(!os->obj->IsSubClassOf(triObjectClassID)) &&
		(!os->obj->IsSubClassOf(polyObjectClassID)) )
	{
		//neither patch or poly convert to a mesh
		if (os->obj->CanConvertToType(triObjectClassID))
		{
			TriObject *tri = (TriObject *) os->obj->ConvertToType(t, Class_ID(TRIOBJ_CLASS_ID, 0));
			os->obj = tri;
			os->obj->UnlockObject();
		}
	}

	//this just makes sure we have some cache geo data since onload the cache meshes 
	//are null which triggers a reset
	d->SetGeoCache(os);

	//next check to make sure we have a valid mesh cache in the local data
	//we need this the mesh topology and or type can change
	BOOL reset = d->HasCacheChanged(os);


	int g1 = os->obj->NumPoints();
	int g2 = d->GetNumberGeomVerts();

	//if the topochanges and the user has forceUpdate off do nothing
	//this is for cases when they apply unwrap to an object that has changed topology but
	//will come back at render time	

	if ((reset) && (!forceUpdate))
	{
		os->obj->UpdateValidity(TEXMAP_CHAN_NUM,GetValidity(t));	
		return;
	}
	else if (reset )
	{
		//this will rebuild all the caches and put back the original mapping or apply a default one if one does not exist
		d->Reset(os,CurrentChannel);
	}

	//update our geo list since this can change so we need to update
	//this includes hidden faces and geometry point position
	d->UpdateGeoData(t,os);

	//this pushes the data on any of the controller back into the local data
	d->UpdateControllers(t,this);


	//this fills out local data list
	MyEnumProc dep;              
	DoEnumDependents(&dep);
	mMeshTopoData.SetCount(0);
	for (int i = 0; i < dep.Nodes.Count(); i++)
	{
		if (dep.Nodes[i]->Selected())
			IsInStack(dep.Nodes[i]);
	}

	//if we are viz and multiple instances disable the UI
	#ifdef DESIGN_VER
		if (mMeshTopoData.Count() > 1)
		{
			DisableUI();
		}
	#endif

	//see if we old Max data in the modifier and if so move it to the local data
	if (mTVMaps_Max9.f.Count() || mTVMaps_Max9.v.Count() && d->IsLoaded())
	{
		d->ResolveOldData(mTVMaps_Max9);
		//fix up the controllers now since we dont keep a full list any more
		for (int i = 0; i < mTVMaps_Max9.v.Count(); i++)
		{
			if (i < mUVWControls.cont.Count())
			{
				if (mUVWControls.cont[i])
					d->SetTVVertControlIndex(i, i);
			}
		}
		d->ClearLoaded();
	}
	//check to see if all the tv are cleared and if so clear the max9 data
	BOOL clearOldData = TRUE;
	for (int i = 0; i < mMeshTopoData.Count(); i++)
	{
		if (mMeshTopoData[i]->IsLoaded())
			clearOldData = FALSE;
	}
	if (clearOldData)
	{
		mTVMaps_Max9.FreeEdges();
		mTVMaps_Max9.FreeFaces();
		mTVMaps_Max9.v.SetCount(0);
		mTVMaps_Max9.f.SetCount(0);
	}

	//if the user flipped this flag get the selection from the stack and copy it to our cache and uvw faces
	if (getFaceSelectionFromStack)
	{
#ifndef NO_PATCHES // orb 02-05-03
		if (os->obj->IsSubClassOf(patchObjectClassID))
		{
			GetFaceSelectionFromPatch(os,mc, t);
		}
		else 
#endif // NO_PATCHES
		if (os->obj->IsSubClassOf(triObjectClassID))
		{
			GetFaceSelectionFromMesh(os,mc, t);
		}
		else if (os->obj->IsSubClassOf(polyObjectClassID))
		{
			GetFaceSelectionFromMNMesh(os,mc, t);
		}

		getFaceSelectionFromStack = FALSE;
	}

	//this inits our tm control for the mapping gizmos
	if (!tmControl || (flags&CONTROL_OP) || (flags&CONTROL_INITPARAMS)) 
		InitControl(t);

	//now copy our selection into the object flowing up the stack for display purposes
	d->CopyFaceSelection(os);

	d->ApplyMapping(os, CurrentChannel,t);

	Interval iv = GetValidity(t);
	iv = iv & os->obj->ChannelValidity(t,GEOM_CHAN_NUM);
	iv = iv & os->obj->ChannelValidity(t,TOPO_CHAN_NUM);
	os->obj->UpdateValidity(TEXMAP_CHAN_NUM,iv);

	if ((popUpDialog) && (alwaysEdit))
		fnEdit();
	popUpDialog = FALSE;
}

Interval UnwrapMod::LocalValidity(TimeValue t)
{
	// aszabo|feb.05.02 If we are being edited, 
	// return NEVER to forces a cache to be built after previous modifier.
	if (TestAFlag(A_MOD_BEING_EDITED))
		return NEVER;

	return GetValidity(t);
}

//aszabo|feb.06.02 - When LocalValidity is called by ModifyObject,
// it returns NEVER and thus the object channels are marked non valid
// As a result, the mod stack enters and infinite evaluation of the modifier
// ModifyObject now calls GetValidity and CORE calls LocalValidity to
// allow for building a cache on the input of this modifier when it's 
// being edited 
Interval UnwrapMod::GetValidity(TimeValue t)
{

	Interval iv = FOREVER;
	for (int i=0; i<mUVWControls.cont.Count(); i++) {
		if (mUVWControls.cont[i]) 
		{			
			Point3 p;
			mUVWControls.cont[i]->GetValue(t,&p,iv);
		}
	}
	return iv;
}



RefTargetHandle UnwrapMod::GetReference(int i)
{
	if (i==0) return tmControl;
	else if (i<91) return  map[i-1];
	else if (i==95) return pblock;
	else if (i > 110) return mUVWControls.cont[i-11-100];
	else if (i == 100) return checkerMat;
	else if (i == 101) return originalMat;
	return NULL;
}



void UnwrapMod::SetReference(int i, RefTargetHandle rtarg)
{
	if (i==0) tmControl = (Control*)rtarg;
	else if (i<91) map[i-1] = (Texmap*)rtarg;
	else if (i == 95)  pblock=(IParamBlock2*)rtarg;
	else if (i == 100) checkerMat = (StdMat*) rtarg;
	else if (i == 101) originalMat = (ReferenceTarget*) rtarg;
	else
	{
		int index = i-11-100;
		if (index >= mUVWControls.cont.Count())
		{
			int startIndex = mUVWControls.cont.Count();
			mUVWControls.cont.SetCount(index+1);
			for (int id = startIndex; id < mUVWControls.cont.Count(); id++)
				mUVWControls.cont[id] = NULL;
		}
		mUVWControls.cont[index] = (Control*)rtarg;
	}


}
// ref 0 - the tm control for gizmo
// ref 1-90 - map references
// ref 100 the checker texture
// ref 111+ the vertex controllers

int UnwrapMod::RemapRefOnLoad(int iref) 
{
	if (version == 1)
	{
		if (iref == 0)
			return 1;
		else if (iref > 0)
			return iref + 10 + 100;

	}
	else if (version < 8)
	{
		if (iref > 10)
			return iref + 100;
	}
	return iref;
}

Animatable* UnwrapMod::SubAnim(int i)
{
	return mUVWControls.cont[i];
}

TSTR UnwrapMod::SubAnimName(int i)
{
	TSTR buf;
	//	buf.printf(_T("Point %d"),i+1);
	buf.printf(_T("%s %d"),GetString(IDS_PW_POINT),i+1);
	return buf;
}


RefTargetHandle UnwrapMod::Clone(RemapDir& remap)
{
	UnwrapMod *mod = new UnwrapMod;



	mod->zoom      = zoom;
	mod->aspect    = aspect;
	mod->xscroll   = xscroll;
	mod->yscroll   = yscroll;
	mod->uvw       = uvw;
	mod->showMap   = showMap;
	mod->update    = update;
	mod->lineColor = lineColor;
	mod->openEdgeColor = openEdgeColor;
	mod->selColor  =	selColor;
	mod->rendW     = rendW;
	mod->rendH     = rendH;
	mod->isBitmap =  isBitmap;
	mod->isBitmap =  pixelSnap;
	mod->useBitmapRes = useBitmapRes;
	mod->channel = channel;
	mod->ReplaceReference(PBLOCK_REF,remap.CloneRef(pblock));
	mod->ReplaceReference(0,remap.CloneRef(tmControl));


	mod->mUVWControls.cont.SetCount(mUVWControls.cont.Count());
	for (int i=0; i<mUVWControls.cont.Count(); i++) {
		mod->mUVWControls.cont[i] = NULL;		
		if (mUVWControls.cont[i]) mod->ReplaceReference(i+11+100,remap.CloneRef(mUVWControls.cont[i]));
	}

/*
	if (instanced)
	{
		for (int i=0; i<mod->TVMaps.cont.Count(); i++) mod->DeleteReference(i+11+100);
		mod->TVMaps.v.Resize(0);
		mod->TVMaps.f.Resize(0);
		mod->TVMaps.cont.Resize(0);
		mod->vsel.SetSize(0);
		mod->updateCache = TRUE;
		mod->instanced = FALSE;
	}
*/
	mod->loadDefaults = FALSE;
	mod->showIconList = showIconList;

	BaseClone(this, mod, remap);
	return mod;
}

#define NAMEDSEL_STRING_CHUNK	0x2809
#define NAMEDSEL_ID_CHUNK		0x2810

#define NAMEDVSEL_STRING_CHUNK	0x2811
#define NAMEDVSEL_ID_CHUNK		0x2812

#define NAMEDESEL_STRING_CHUNK	0x2813
#define NAMEDESEL_ID_CHUNK		0x2814


RefResult UnwrapMod::NotifyRefChanged(
									  Interval changeInt, 
									  RefTargetHandle hTarget, 
									  PartID& partID, 
									  RefMessage message)
{

	if (suspendNotify) return REF_STOP;

	switch (message) 
	{
	case REFMSG_CHANGE:
		if (editMod==this && hView) 
		{
			InvalidateView();
		}
		break;



	}
	return REF_SUCCEED;
}




IOResult UnwrapMod::Save(ISave *isave)
{
	ULONG nb = 0;
	Modifier::Save(isave);
	version = CURRENTVERSION;


	isave->BeginChunk(ZOOM_CHUNK);
	isave->Write(&zoom, sizeof(zoom), &nb);
	isave->EndChunk();

	isave->BeginChunk(ASPECT_CHUNK);
	isave->Write(&aspect, sizeof(aspect), &nb);
	isave->EndChunk();

	isave->BeginChunk(XSCROLL_CHUNK);
	isave->Write(&xscroll, sizeof(xscroll), &nb);
	isave->EndChunk();

	isave->BeginChunk(YSCROLL_CHUNK);
	isave->Write(&yscroll, sizeof(yscroll), &nb);
	isave->EndChunk();

	isave->BeginChunk(IWIDTH_CHUNK);
	isave->Write(&rendW, sizeof(rendW), &nb);
	isave->EndChunk();

	isave->BeginChunk(IHEIGHT_CHUNK);
	isave->Write(&rendH, sizeof(rendH), &nb);
	isave->EndChunk();

	isave->BeginChunk(UVW_CHUNK);
	isave->Write(&uvw, sizeof(uvw), &nb);
	isave->EndChunk();

	isave->BeginChunk(SHOWMAP_CHUNK);
	isave->Write(&showMap, sizeof(showMap), &nb);
	isave->EndChunk();

	isave->BeginChunk(UPDATE_CHUNK);
	isave->Write(&update, sizeof(update), &nb);
	isave->EndChunk();

	isave->BeginChunk(LINECOLOR_CHUNK);
	isave->Write(&lineColor, sizeof(lineColor), &nb);
	isave->EndChunk();

	isave->BeginChunk(SELCOLOR_CHUNK);
	isave->Write(&selColor, sizeof(selColor), &nb);
	isave->EndChunk();

	isave->BeginChunk(CHANNEL_CHUNK);
	isave->Write(&channel, sizeof(channel), &nb);
	isave->EndChunk();

	isave->BeginChunk(PREFS_CHUNK);
	isave->Write(&lineColor, sizeof(lineColor), &nb);
	isave->Write(&selColor, sizeof(selColor), &nb);
	isave->Write(&weldThreshold, sizeof(weldThreshold), &nb);
	isave->Write(&update, sizeof(update), &nb);
	isave->Write(&showVerts, sizeof(showVerts), &nb);
	isave->Write(&midPixelSnap, sizeof(midPixelSnap), &nb);
	isave->EndChunk();

	if (namedSel.Count()) {
		isave->BeginChunk(0x2806);			
		for (int i=0; i<namedSel.Count(); i++) 
		{
			isave->BeginChunk(NAMEDSEL_STRING_CHUNK);
			isave->WriteWString(*namedSel[i]);
			isave->EndChunk();

			isave->BeginChunk(NAMEDSEL_ID_CHUNK);
			isave->Write(&ids[i],sizeof(DWORD),&nb);
			isave->EndChunk();
		}

		for (int i=0; i<namedVSel.Count(); i++) 
		{
			isave->BeginChunk(NAMEDVSEL_STRING_CHUNK);
			isave->WriteWString(*namedVSel[i]);
			isave->EndChunk();

			isave->BeginChunk(NAMEDVSEL_ID_CHUNK);
			isave->Write(&idsV[i],sizeof(DWORD),&nb);
			isave->EndChunk();
		}

		for (int i=0; i<namedESel.Count(); i++) 
		{
			isave->BeginChunk(NAMEDESEL_STRING_CHUNK);
			isave->WriteWString(*namedESel[i]);
			isave->EndChunk();

			isave->BeginChunk(NAMEDESEL_ID_CHUNK);
			isave->Write(&idsE[i],sizeof(DWORD),&nb);
			isave->EndChunk();
		}

		isave->EndChunk();
	}
	if (useBitmapRes)
	{
		isave->BeginChunk(USEBITMAPRES_CHUNK);
		isave->EndChunk();
	}



	isave->BeginChunk(LOCKASPECT_CHUNK);
	isave->Write(&lockAspect, sizeof(lockAspect), &nb);
	isave->EndChunk();

	isave->BeginChunk(MAPSCALE_CHUNK);
	isave->Write(&mapScale, sizeof(mapScale), &nb);
	isave->EndChunk();

	isave->BeginChunk(WINDOWPOS_CHUNK);
	isave->Write(&windowPos, sizeof(WINDOWPLACEMENT), &nb);
	isave->EndChunk();

	isave->BeginChunk(FORCEUPDATE_CHUNK);
	isave->Write(&forceUpdate, sizeof(forceUpdate), &nb);
	isave->EndChunk();



	if (bTile)
	{
		isave->BeginChunk(TILE_CHUNK);
		isave->EndChunk();
	}

	isave->BeginChunk(TILECONTRAST_CHUNK);
	isave->Write(&fContrast, sizeof(fContrast), &nb);
	isave->EndChunk();

	isave->BeginChunk(TILELIMIT_CHUNK);
	isave->Write(&iTileLimit, sizeof(iTileLimit), &nb);
	isave->EndChunk();

	isave->BeginChunk(SOFTSELLIMIT_CHUNK);
	isave->Write(&limitSoftSel, sizeof(limitSoftSel), &nb);
	isave->Write(&limitSoftSelRange, sizeof(limitSoftSelRange), &nb);
	isave->EndChunk();


	isave->BeginChunk(FLATTENMAP_CHUNK);
	isave->Write(&flattenAngleThreshold, sizeof(flattenAngleThreshold), &nb);
	isave->Write(&flattenSpacing, sizeof(flattenSpacing), &nb);
	isave->Write(&flattenNormalize, sizeof(flattenNormalize), &nb);
	isave->Write(&flattenRotate, sizeof(flattenRotate), &nb);
	isave->Write(&flattenCollapse, sizeof(flattenCollapse), &nb);
	isave->EndChunk();

	isave->BeginChunk(NORMALMAP_CHUNK);
	isave->Write(&normalMethod, sizeof(normalMethod), &nb);
	isave->Write(&normalSpacing, sizeof(normalSpacing), &nb);
	isave->Write(&normalNormalize, sizeof(normalNormalize), &nb);
	isave->Write(&normalRotate, sizeof(normalRotate), &nb);
	isave->Write(&normalAlignWidth, sizeof(normalAlignWidth), &nb);
	isave->EndChunk();


	isave->BeginChunk(UNFOLDMAP_CHUNK);
	isave->Write(&unfoldMethod, sizeof(unfoldMethod), &nb);
	isave->EndChunk();


	isave->BeginChunk(STITCH_CHUNK);
	isave->Write(&bStitchAlign, sizeof(bStitchAlign), &nb);
	isave->Write(&fStitchBias, sizeof(fStitchBias), &nb);
	isave->EndChunk();


	isave->BeginChunk(GEOMELEM_CHUNK);
	isave->Write(&geomElemMode, sizeof(geomElemMode), &nb);
	isave->EndChunk();


	isave->BeginChunk(PLANARTHRESHOLD_CHUNK);
	isave->Write(&planarThreshold, sizeof(planarThreshold), &nb);
	isave->Write(&planarMode, sizeof(planarMode), &nb);
	isave->EndChunk();

	isave->BeginChunk(BACKFACECULL_CHUNK);
	isave->Write(&ignoreBackFaceCull, sizeof(ignoreBackFaceCull), &nb);
	isave->Write(&oldSelMethod, sizeof(oldSelMethod), &nb);
	isave->EndChunk();

	isave->BeginChunk(TVELEMENTMODE_CHUNK);
	isave->Write(&tvElementMode, sizeof(tvElementMode), &nb);
	isave->EndChunk();

	isave->BeginChunk(ALWAYSEDIT_CHUNK);
	isave->Write(&alwaysEdit, sizeof(alwaysEdit), &nb);
	isave->EndChunk();

	isave->BeginChunk(SHOWCONNECTION_CHUNK);
	isave->Write(&showVertexClusterList, sizeof(showVertexClusterList), &nb);
	isave->EndChunk();

	isave->BeginChunk(PACK_CHUNK);
	isave->Write(&packMethod, sizeof(packMethod), &nb);
	isave->Write(&packSpacing, sizeof(packSpacing), &nb);
	isave->Write(&packNormalize, sizeof(packNormalize), &nb);
	isave->Write(&packRotate, sizeof(packRotate), &nb);
	isave->Write(&packFillHoles, sizeof(packFillHoles), &nb);
	isave->EndChunk();


	isave->BeginChunk(TVSUBOBJECTMODE_CHUNK);
	isave->Write(&TVSubObjectMode, sizeof(TVSubObjectMode), &nb);
	isave->EndChunk();


	isave->BeginChunk(FILLMODE_CHUNK);
	isave->Write(&fillMode, sizeof(fillMode), &nb);
	isave->EndChunk();

	isave->BeginChunk(OPENEDGECOLOR_CHUNK);
	isave->Write(&openEdgeColor, sizeof(openEdgeColor), &nb);
	isave->Write(&displayOpenEdges, sizeof(displayOpenEdges), &nb);
	isave->EndChunk();

	isave->BeginChunk(UVEDGEMODE_CHUNK);
	isave->Write(&uvEdgeMode, sizeof(uvEdgeMode), &nb);
	isave->Write(&openEdgeMode, sizeof(openEdgeMode), &nb);
	isave->Write(&displayHiddenEdges, sizeof(displayHiddenEdges), &nb);

	isave->EndChunk();

	isave->BeginChunk(MISCCOLOR_CHUNK);
	isave->Write(&handleColor, sizeof(handleColor), &nb);
	isave->Write(&freeFormColor, sizeof(freeFormColor), &nb);
	isave->EndChunk();

	isave->BeginChunk(HITSIZE_CHUNK);
	isave->Write(&hitSize, sizeof(hitSize), &nb);
	isave->EndChunk();

	isave->BeginChunk(PIVOT_CHUNK);
	isave->Write(&resetPivotOnSel, sizeof(resetPivotOnSel), &nb);
	isave->EndChunk();

	isave->BeginChunk(GIZMOSEL_CHUNK);
	isave->Write(&allowSelectionInsideGizmo, sizeof(allowSelectionInsideGizmo), &nb);
	isave->EndChunk();

	isave->BeginChunk(SHARED_CHUNK);
	isave->Write(&showShared, sizeof(showShared), &nb);
	isave->Write(&sharedColor, sizeof(sharedColor), &nb);
	isave->EndChunk();

	isave->BeginChunk(SHOWICON_CHUNK);
	showIconList.Save(isave);
	isave->EndChunk();

	isave->BeginChunk(SYNCSELECTION_CHUNK);
	isave->Write(&syncSelection, sizeof(syncSelection), &nb);
	isave->EndChunk();

	isave->BeginChunk(BRIGHTCENTER_CHUNK);
	isave->Write(&brightCenterTile, sizeof(brightCenterTile), &nb);
	isave->Write(&blendTileToBackGround, sizeof(blendTileToBackGround), &nb);
	isave->EndChunk();

	isave->BeginChunk(CURSORSIZE_CHUNK);
	isave->Write(&sketchCursorSize, sizeof(sketchCursorSize), &nb);
	isave->Write(&paintSize, sizeof(paintSize), &nb);
	isave->EndChunk();


	isave->BeginChunk(TICKSIZE_CHUNK);
	isave->Write(&tickSize, sizeof(tickSize), &nb);
	isave->EndChunk();

	//new
	isave->BeginChunk(GRID_CHUNK);
	isave->Write(&gridSize, sizeof(gridSize), &nb);
	isave->Write(&gridSnap, sizeof(gridSnap), &nb);
	isave->Write(&gridVisible, sizeof(gridVisible), &nb);
	isave->Write(&gridColor, sizeof(gridColor), &nb);
	isave->Write(&gridStr, sizeof(gridStr), &nb);
	isave->Write(&autoMap, sizeof(autoMap), &nb);
	isave->EndChunk();

	isave->BeginChunk(PREVENTFLATTENING_CHUNK);
	isave->Write(&preventFlattening, sizeof(preventFlattening), &nb);
	isave->EndChunk();

	isave->BeginChunk(ENABLESOFTSELECTION_CHUNK);
	isave->Write(&enableSoftSelection, sizeof(enableSoftSelection), &nb);
	isave->EndChunk();


	isave->BeginChunk(CONSTANTUPDATE_CHUNK);
	isave->Write(&update, sizeof(update), &nb);
	isave->Write(&loadDefaults, sizeof(loadDefaults), &nb);	
	isave->EndChunk();

	isave->BeginChunk(APPLYTOWHOLEOBJECT_CHUNK);
	isave->Write(&applyToWholeObject, sizeof(applyToWholeObject), &nb);
	isave->EndChunk();

	isave->BeginChunk(THICKOPENEDGE_CHUNK);
	isave->Write(&thickOpenEdges, sizeof(thickOpenEdges), &nb);
	isave->EndChunk();	

	isave->BeginChunk(VIEWPORTOPENEDGE_CHUNK);
	isave->Write(&viewportOpenEdges, sizeof(viewportOpenEdges), &nb);
	isave->EndChunk();	

	isave->BeginChunk(ABSOLUTETYPEIN_CHUNK);
	isave->Write(&absoluteTypeIn, sizeof(absoluteTypeIn), &nb);
	isave->EndChunk();	

	isave->BeginChunk(STITCHSCALE_CHUNK);
	isave->Write(&bStitchScale, sizeof(bStitchScale), &nb);
	isave->EndChunk();	

//	isave->BeginChunk(SEAM_CHUNK);
//	peltData.seamEdges.Save(isave);
//	isave->EndChunk();	

	isave->BeginChunk(VERSION_CHUNK);
	isave->Write(&version, sizeof(version), &nb);
	isave->EndChunk();

	isave->BeginChunk(CURRENTMAP_CHUNK);
	isave->Write(&CurrentMap, sizeof(CurrentMap), &nb);
	isave->EndChunk();





	isave->BeginChunk(RELAX_CHUNK);
	isave->Write(&relaxAmount, sizeof(relaxAmount), &nb);
	isave->Write(&relaxStretch, sizeof(relaxStretch), &nb);
	isave->Write(&relaxIteration, sizeof(relaxIteration), &nb);
	isave->Write(&relaxType, sizeof(relaxType), &nb);
	isave->Write(&relaxBoundary, sizeof(relaxBoundary), &nb);
	isave->Write(&relaxSaddle, sizeof(relaxSaddle), &nb);
	isave->EndChunk();

	isave->BeginChunk(FALLOFFSPACE_CHUNK);
	isave->Write(&falloffSpace, sizeof(falloffSpace), &nb);
	isave->Write(&falloffStr, sizeof(falloffStr), &nb);
	isave->EndChunk();

	isave->BeginChunk(SHOWPELTSEAMS_CHUNK);
	isave->Write(&alwaysShowSeams, sizeof(alwaysShowSeams), &nb);
	isave->EndChunk();	

	isave->BeginChunk(SPLINEMAP_V2_CHUNK);
	mSplineMap.Save(isave);
	isave->EndChunk();	

	isave->BeginChunk(CONTROLLER_COUNT_CHUNK);
	int controllerCount = mUVWControls.cont.Count();
	isave->Write(&controllerCount, sizeof(controllerCount), &nb);
	isave->EndChunk();	



	return IO_OK;
}

#define UVWVER	4

void UnwrapMod::LoadUVW(HWND hWnd)
{

	theHold.Begin();
	HoldPointsAndFaces();	

	static TCHAR fname[256] = {'\0'};
	OPENFILENAME ofn;
	memset(&ofn,0,sizeof(ofn));
	FilterList fl;
	fl.Append( GetString(IDS_PW_UVWFILES));
	fl.Append( _T("*.uvw"));		

	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];

		TSTR title;
		title.printf("%s : %s",mMeshTopoData.GetNode(ldID)->GetName(),GetString(IDS_PW_LOADOBJECT));

		ofn.lStructSize     = sizeof(OPENFILENAME);  // No OFN_ENABLEHOOK
		ofn.hwndOwner       = GetCOREInterface()->GetMAXHWnd();
		ofn.lpstrFilter     = fl;
		ofn.lpstrFile       = fname;
		ofn.nMaxFile        = 256;    
		//ofn.lpstrInitialDir = ip->GetDir(APP_EXPORT_DIR);
		ofn.Flags           = OFN_HIDEREADONLY|OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST;
		ofn.FlagsEx         = OFN_EX_NOPLACESBAR;
		ofn.lpstrDefExt     = _T("uvw");
		ofn.lpstrTitle      = title;


		if (GetOpenFileName(&ofn)) 
		{

			//load stuff here  stuff here
			FILE *file = fopen(fname,_T("rb"));

			for (int i=0; i<mUVWControls.cont.Count(); i++) 
				if (mUVWControls.cont[i]) DeleteReference(i+11+100);


			int ver = ld->LoadTVVerts(file);
			ld->LoadFaces(file,ver);
			fclose(file);
		}




	}
	theHold.Accept(_T(GetString(IDS_PW_LOADOBJECT)));

}
void UnwrapMod::SaveUVW(HWND hWnd)
{
	static TCHAR fname[256] = {'\0'};
	OPENFILENAME ofn;
	memset(&ofn,0,sizeof(ofn));
	FilterList fl;
	fl.Append( GetString(IDS_PW_UVWFILES));
	fl.Append( _T("*.uvw"));		
	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];

		TSTR title;
		title.printf("%s : %s",mMeshTopoData.GetNode(ldID)->GetName(),GetString(IDS_PW_SAVEOBJECT));

		ofn.lStructSize     = sizeof(OPENFILENAME);  // No OFN_ENABLEHOOK
		ofn.hwndOwner       = GetCOREInterface()->GetMAXHWnd();
		ofn.lpstrFilter     = fl;
		ofn.lpstrFile       = fname;
		ofn.nMaxFile        = 256;    
		//ofn.lpstrInitialDir = ip->GetDir(APP_EXPORT_DIR);
		ofn.Flags           = OFN_HIDEREADONLY|OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST;
		ofn.FlagsEx         = OFN_EX_NOPLACESBAR;
		ofn.lpstrDefExt     = _T("uvw");
		ofn.lpstrTitle      = title;

	tryAgain:
		if (GetSaveFileName(&ofn)) {
			if (DoesFileExist(fname)) {
				TSTR buf1;
				TSTR buf2 = GetString(IDS_PW_SAVEOBJECT);
				buf1.printf(GetString(IDS_PW_FILEEXISTS),fname);
				if (IDYES!=MessageBox(
					hParams,
					buf1,buf2,MB_YESNO|MB_ICONQUESTION)) {
						goto tryAgain;
				}
			}
			//save stuff here
			// this is timed slice so it will not save animation not sure how to save controller info but will neeed to later on in other plugs

			FILE *file = fopen(fname,_T("wb"));

			int ver = -1;
			fwrite(&ver, sizeof(ver), 1,file);
			ver = UVWVER;
			fwrite(&ver, sizeof(ver), 1,file);

	
			int vct = ld->GetNumberTVVerts();//TVMaps.v.Count(), 
			int fct = ld->GetNumberFaces();//TVMaps.f.Count();

			fwrite(&vct, sizeof(vct), 1,file);

			if (vct) 
			{
				ld->SaveTVVerts(file);
			}

			fwrite(&fct, sizeof(fct), 1,file);

			if (fct) 
			{
				ld->SaveFaces(file);//				TVMaps.SaveFaces(file);
			}
	
			fclose(file);
		}
	}

}


IOResult UnwrapMod::LoadNamedSelChunk(ILoad *iload) {	
	IOResult res;
	DWORD ix=0;
	ULONG nb;

	while (IO_OK==(res=iload->OpenChunk())) 
	{
		switch(iload->CurChunkID())  
		{
		case NAMEDSEL_STRING_CHUNK: 
			{
				TCHAR *name;
				res = iload->ReadWStringChunk(&name);
				//AddSet(TSTR(name),level+1);
				TSTR *newName = new TSTR(name);
				namedSel.Append(1,&newName);				
				ids.Append(1,&ix);
				ix++;
				break;
			}
		case NAMEDSEL_ID_CHUNK:
			iload->Read(&ids[ids.Count()-1],sizeof(DWORD), &nb);
			break;

		case NAMEDVSEL_STRING_CHUNK: 
			{
				TCHAR *name;
				res = iload->ReadWStringChunk(&name);
				//AddSet(TSTR(name),level+1);
				TSTR *newName = new TSTR(name);
				namedVSel.Append(1,&newName);				
				idsV.Append(1,&ix);
				ix++;
				break;
			}
		case NAMEDVSEL_ID_CHUNK:
			iload->Read(&idsV[ids.Count()-1],sizeof(DWORD), &nb);
			break;

		case NAMEDESEL_STRING_CHUNK: 
			{
				TCHAR *name;
				res = iload->ReadWStringChunk(&name);
				//AddSet(TSTR(name),level+1);
				TSTR *newName = new TSTR(name);
				namedESel.Append(1,&newName);				
				idsE.Append(1,&ix);
				ix++;
				break;
			}
		case NAMEDESEL_ID_CHUNK:
			iload->Read(&idsE[ids.Count()-1],sizeof(DWORD), &nb);
			break;

		}
		iload->CloseChunk();
		if (res!=IO_OK) return res;
	}
	return IO_OK;
}


class UnwrapPostLoadCallback:public  PostLoadCallback
{
public:
	UnwrapMod      *s;

	int oldData;
	UnwrapPostLoadCallback(UnwrapMod *r, BOOL b) {s=r;oldData = b;}
	void proc(ILoad *iload);
};

void UnwrapPostLoadCallback::proc(ILoad *iload)
{
	if (!oldData)
	{
		//		for (int i=0; i<10; i++) 
		//			s->ReplaceReference(i+1,NULL);
	}
	delete this;
}



IOResult UnwrapMod::Load(ILoad *iload)
{
	popUpDialog = FALSE;
	version = 2;
	IOResult res =IO_OK;
	ULONG nb = 0;
	Modifier::Load(iload);
	//check for backwards compatibility
	useBitmapRes = FALSE;

	Tab<UVW_TVFaceOldClass> oldData;


	bTile = FALSE;

//	TVMaps.edgesValid = FALSE;

	//5.1.05
	this->autoBackground = FALSE;


	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {

		case CURRENTMAP_CHUNK:
			iload->Read(&CurrentMap, sizeof(CurrentMap), &nb);				
			break;

		case VERSION_CHUNK:
			iload->Read(&version, sizeof(version), &nb);				
			break;
			//5.1.05
		case AUTOBACKGROUND_CHUNK:
			this->autoBackground = TRUE;
			break;

		case 0x2806:
			res = LoadNamedSelChunk(iload);
			break;

		/*************************************************************
		Max9 Legacy loader START
		case VERTCOUNT_CHUNK:
			iload->Read(&ct, sizeof(ct), &nb);
			mTVMaps_Max9.v.SetCount(ct);
			mTVMaps_Max9.cont.SetCount(ct);
			for (int i=0; i<ct; i++) T
				mTVMaps_Max9.cont[i] = NULL;          
			break;
		case FACECOUNT_CHUNK:
			iload->Read(&ct, sizeof(ct), &nb);
			mTVMaps_Max9.SetCountFaces(ct);
			break;
		case GEOMPOINTSCOUNT_CHUNK:
			iload->Read(&ct, sizeof(ct), &nb);
			mTVMaps_Max9.geomPoints.SetCount(ct);
			break;

			//old way here for legacy reason only
		case FACE_CHUNK:
			{
				version = 1;
				oldDataPresent = TRUE;
				Tab<TVFace> f;
				f.SetCount(mTVMaps_Max9.f.Count());
				iload->Read(f.Addr(0), sizeof(TVFace)*mTVMaps_Max9.f.Count(), &nb);
				for (int i=0;i<mTVMaps_Max9.f.Count();i++)
				{

					mTVMaps_Max9.f[i]->count  = 3;
					mTVMaps_Max9.f[i]->t = new int[mTVMaps_Max9.f[i]->count];
					mTVMaps_Max9.f[i]->v = new int[mTVMaps_Max9.f[i]->count];

					mTVMaps_Max9.f[i]->t[0] = f[i].t[0];
					mTVMaps_Max9.f[i]->t[1] = f[i].t[1];
					mTVMaps_Max9.f[i]->t[2] = f[i].t[2];
					mTVMaps_Max9.f[i]->flags = 0;
					mTVMaps_Max9.f[i]->vecs = NULL;
				}

				break;
			}
			//old way here for legacy reason only
		case VERTS_CHUNK:
			{
				Tab<Point3> p;
				p.SetCount(mTVMaps_Max9.v.Count());
				oldDataPresent = TRUE;

				iload->Read(p.Addr(0), sizeof(Point3)*mTVMaps_Max9.v.Count(), &nb);

				for (int i=0;i<mTVMaps_Max9.v.Count();i++)
				{
					mTVMaps_Max9.v[i].p = p[i];
					mTVMaps_Max9.v[i].flags = 0;
					mTVMaps_Max9.v[i].influence = 0.0f;
				}
				break;
			}
		case FACE2_CHUNK:

			oldData.SetCount(mTVMaps_Max9.f.Count());
			iload->Read(oldData.Addr(0), sizeof(UVW_TVFaceOldClass)*oldData.Count(), &nb);
			for (int i = 0; i < TVMaps.f.Count(); i++)
			{
				//fix for bug 281118 it was checking an uniitiliazed flag
				if (oldData[i].flags & 8)  // not this was FLAG_QUAD but this define got removed
					mTVMaps_Max9.f[i]->count=4;
				else mTVMaps_Max9.f[i]->count=3;

				mTVMaps_Max9.f[i]->t = new int[mTVMaps_Max9.f[i]->count];
				mTVMaps_Max9.f[i]->v = new int[mTVMaps_Max9.f[i]->count];

				for (int j = 0; j < mTVMaps_Max9.f[i]->count; j++)
					mTVMaps_Max9.f[i]->t[j] = oldData[i].t[j];
				mTVMaps_Max9.f[i]->FaceIndex = oldData[i].FaceIndex;
				mTVMaps_Max9.f[i]->MatID = oldData[i].MatID;
				mTVMaps_Max9.f[i]->flags = oldData[i].flags;
				mTVMaps_Max9.f[i]->vecs = NULL;


			}
			//now compute the geom points
			for (int i = 0; i < mTVMaps_Max9.f.Count(); i++)
			{
				int pcount = 3;
				//					if (TVMaps.f[i].flags & FLAG_QUAD) pcount = 4;
				pcount = mTVMaps_Max9.f[i]->count;

				for (int j =0; j < pcount; j++)
				{
					BOOL found = FALSE;
					int index;
					for (int k =0; k < mTVMaps_Max9.geomPoints.Count();k++)
					{
						if (oldData[i].pt[j] == mTVMaps_Max9.geomPoints[k])
						{
							found = TRUE;
							index = k;
							k = mTVMaps_Max9.geomPoints.Count();
						}
					}
					if (found)
					{
						mTVMaps_Max9.f[i]->v[j] = index;
					}
					else
					{
						mTVMaps_Max9.f[i]->v[j] = mTVMaps_Max9.geomPoints.Count();
						mTVMaps_Max9.geomPoints.Append(1,&oldData[i].pt[j],1);
					}
				}

			}

			break;

		case FACE4_CHUNK:
			TVMaps.LoadFaces(iload);	

			break;

		case VERTS2_CHUNK:
			iload->Read(mTVMaps_Max9.v.Addr(0), sizeof(UVW_TVVertClass_Max9)*mTVMaps_Max9.v.Count(), &nb);
			break;
		case GEOMPOINTS_CHUNK:
			iload->Read(mTVMaps_Max9.geomPoints.Addr(0), sizeof(Point3)*mTVMaps_Max9.geomPoints.Count(), &nb);
			break;
//		Max9 Legacy loader END
		***************************************************************/


		case ZOOM_CHUNK:
			iload->Read(&zoom, sizeof(zoom), &nb);
			break;
		case ASPECT_CHUNK:
			iload->Read(&aspect, sizeof(aspect), &nb);
			break;
		case LOCKASPECT_CHUNK:
			iload->Read(&lockAspect, sizeof(lockAspect), &nb);
			break;
		case MAPSCALE_CHUNK:
			iload->Read(&mapScale, sizeof(mapScale), &nb);
			break;

		case XSCROLL_CHUNK:
			iload->Read(&xscroll, sizeof(xscroll), &nb);
			break;
		case YSCROLL_CHUNK:
			iload->Read(&yscroll, sizeof(yscroll), &nb);
			break;
		case IWIDTH_CHUNK:
			iload->Read(&rendW, sizeof(rendW), &nb);
			break;
		case IHEIGHT_CHUNK:
			iload->Read(&rendH, sizeof(rendH), &nb);
			break;
		case SHOWMAP_CHUNK:
			iload->Read(&showMap, sizeof(showMap), &nb);
			break;
		case UPDATE_CHUNK:
			iload->Read(&update, sizeof(update), &nb);
			break;
		case LINECOLOR_CHUNK:
			iload->Read(&lineColor, sizeof(lineColor), &nb);
			break;
		case SELCOLOR_CHUNK:
			iload->Read(&selColor, sizeof(selColor), &nb);
			break;			
		case UVW_CHUNK:
			iload->Read(&uvw, sizeof(uvw), &nb);
			break;
		case CHANNEL_CHUNK:
			iload->Read(&channel, sizeof(channel), &nb);
			break;			
		case WINDOWPOS_CHUNK:
			iload->Read(&windowPos, sizeof(WINDOWPLACEMENT), &nb);

			windowPos.flags = 0;
			windowPos.showCmd = SW_SHOWNORMAL;
			break;			
		case PREFS_CHUNK:
			iload->Read(&lineColor, sizeof(lineColor), &nb);
			iload->Read(&selColor, sizeof(selColor), &nb);
			iload->Read(&weldThreshold, sizeof(weldThreshold), &nb);
			iload->Read(&update, sizeof(update), &nb);
			iload->Read(&showVerts, sizeof(showVerts), &nb);
			iload->Read(&midPixelSnap, sizeof(midPixelSnap), &nb);
			break;
		case USEBITMAPRES_CHUNK:
			useBitmapRes = TRUE;
			break;			

		case FORCEUPDATE_CHUNK:
			break;
			//tile stuff
		case TILE_CHUNK:
			bTile = TRUE;
			break;			
		case TILECONTRAST_CHUNK:
			iload->Read(&fContrast, sizeof(fContrast), &nb);
			break;			
		case TILELIMIT_CHUNK:
			iload->Read(&iTileLimit, sizeof(iTileLimit), &nb);
			break;			
		case SOFTSELLIMIT_CHUNK:
			iload->Read(&limitSoftSel, sizeof(limitSoftSel), &nb);
			iload->Read(&limitSoftSelRange, sizeof(limitSoftSelRange), &nb);
			break;			
		case FLATTENMAP_CHUNK:
			iload->Read(&flattenAngleThreshold, sizeof(flattenAngleThreshold), &nb);
			iload->Read(&flattenSpacing, sizeof(flattenSpacing), &nb);
			iload->Read(&flattenNormalize, sizeof(flattenNormalize), &nb);
			iload->Read(&flattenRotate, sizeof(flattenRotate), &nb);
			iload->Read(&flattenCollapse, sizeof(flattenCollapse), &nb);
			break;			
		case NORMALMAP_CHUNK:
			iload->Read(&normalMethod, sizeof(normalMethod), &nb);
			iload->Read(&normalSpacing, sizeof(normalSpacing), &nb);
			iload->Read(&normalNormalize, sizeof(normalNormalize), &nb);
			iload->Read(&normalRotate, sizeof(normalRotate), &nb);
			iload->Read(&normalAlignWidth, sizeof(normalAlignWidth), &nb);
			break;			

		case UNFOLDMAP_CHUNK:
			iload->Read(&unfoldMethod, sizeof(unfoldMethod), &nb);
			break;			

		case STITCH_CHUNK:
			iload->Read(&bStitchAlign, sizeof(bStitchAlign), &nb);
			iload->Read(&fStitchBias, sizeof(fStitchBias), &nb);
			break;	

		case STITCHSCALE_CHUNK:
			iload->Read(&bStitchScale, sizeof(bStitchScale), &nb);
			break;

		case GEOMELEM_CHUNK:
			iload->Read(&geomElemMode, sizeof(geomElemMode), &nb);
			break;			
		case PLANARTHRESHOLD_CHUNK:
			iload->Read(&planarThreshold, sizeof(planarThreshold), &nb);
			iload->Read(&planarMode, sizeof(planarMode), &nb);
			break;			
		case BACKFACECULL_CHUNK:
			iload->Read(&ignoreBackFaceCull, sizeof(ignoreBackFaceCull), &nb);
			iload->Read(&oldSelMethod, sizeof(oldSelMethod), &nb);
			break;			
		case TVELEMENTMODE_CHUNK:
			iload->Read(&tvElementMode, sizeof(tvElementMode), &nb);
			break;			
		case ALWAYSEDIT_CHUNK:
			iload->Read(&alwaysEdit, sizeof(alwaysEdit), &nb);
			break;			

		case SHOWCONNECTION_CHUNK:
			iload->Read(&showVertexClusterList, sizeof(showVertexClusterList), &nb);
			break;			
		case PACK_CHUNK:
			iload->Read(&packMethod, sizeof(packMethod), &nb);
			iload->Read(&packSpacing, sizeof(packSpacing), &nb);
			iload->Read(&packNormalize, sizeof(packNormalize), &nb);
			iload->Read(&packRotate, sizeof(packRotate), &nb);
			iload->Read(&packFillHoles, sizeof(packFillHoles), &nb);
			break;			
		case TVSUBOBJECTMODE_CHUNK:
			iload->Read(&TVSubObjectMode, sizeof(TVSubObjectMode), &nb);
			break;			
		case FILLMODE_CHUNK:
			iload->Read(&fillMode, sizeof(fillMode), &nb);
			break;			

		case OPENEDGECOLOR_CHUNK:
			iload->Read(&openEdgeColor, sizeof(openEdgeColor), &nb);
			iload->Read(&displayOpenEdges, sizeof(displayOpenEdges), &nb);
			break;
		case THICKOPENEDGE_CHUNK:
			iload->Read(&thickOpenEdges, sizeof(thickOpenEdges), &nb);
			break;
		case VIEWPORTOPENEDGE_CHUNK:
			iload->Read(&viewportOpenEdges, sizeof(viewportOpenEdges), &nb);
			break;
		case UVEDGEMODE_CHUNK:
			iload->Read(&uvEdgeMode, sizeof(uvEdgeMode), &nb);
			iload->Read(&openEdgeMode, sizeof(openEdgeMode), &nb);
			iload->Read(&displayHiddenEdges, sizeof(displayHiddenEdges), &nb);
			break;
		case MISCCOLOR_CHUNK:
			iload->Read(&handleColor, sizeof(handleColor), &nb);
			iload->Read(&freeFormColor, sizeof(freeFormColor), &nb);
			break;

		case HITSIZE_CHUNK:
			iload->Read(&hitSize, sizeof(hitSize), &nb);
			break;
		case PIVOT_CHUNK:
			iload->Read(&resetPivotOnSel, sizeof(resetPivotOnSel), &nb);
			break;
		case GIZMOSEL_CHUNK:
			iload->Read(&allowSelectionInsideGizmo, sizeof(allowSelectionInsideGizmo), &nb);
			break;
		case SHARED_CHUNK:
			iload->Read(&showShared, sizeof(showShared), &nb);
			iload->Read(&sharedColor, sizeof(sharedColor), &nb);
			break;
		case SHOWICON_CHUNK:
			showIconList.Load(iload);

			break;
		case SYNCSELECTION_CHUNK:
			iload->Read(&syncSelection, sizeof(syncSelection), &nb);
			break;
		case BRIGHTCENTER_CHUNK:
			iload->Read(&brightCenterTile, sizeof(brightCenterTile), &nb);
			iload->Read(&blendTileToBackGround, sizeof(blendTileToBackGround), &nb);
			break;

		case CURSORSIZE_CHUNK:
			iload->Read(&sketchCursorSize, sizeof(sketchCursorSize), &nb);
			iload->Read(&paintSize, sizeof(paintSize), &nb);
			break;


		case TICKSIZE_CHUNK:
			iload->Read(&tickSize, sizeof(tickSize), &nb);
			break;
			//new
		case GRID_CHUNK:
			iload->Read(&gridSize, sizeof(gridSize), &nb);
			iload->Read(&gridSnap, sizeof(gridSnap), &nb);
			iload->Read(&gridVisible, sizeof(gridVisible), &nb);
			iload->Read(&gridColor, sizeof(gridColor), &nb);
			iload->Read(&gridStr, sizeof(gridStr), &nb);
			iload->Read(&autoMap, sizeof(autoMap), &nb);
			break;

		case PREVENTFLATTENING_CHUNK:
			iload->Read(&preventFlattening, sizeof(preventFlattening), &nb);
			break;

		case ENABLESOFTSELECTION_CHUNK:
			iload->Read(&enableSoftSelection, sizeof(enableSoftSelection), &nb);
			break;

		case CONSTANTUPDATE_CHUNK:
			iload->Read(&update, sizeof(update), &nb);
			iload->Read(&loadDefaults, sizeof(loadDefaults), &nb);
			break;
		case APPLYTOWHOLEOBJECT_CHUNK:
			iload->Read(&applyToWholeObject, sizeof(applyToWholeObject), &nb);
			break;

		case ABSOLUTETYPEIN_CHUNK:
			iload->Read(&absoluteTypeIn, sizeof(absoluteTypeIn), &nb);
			break;

//		case SEAM_CHUNK:
//			peltData.seamEdges.Load(iload);
//			break;

		case RELAX_CHUNK:
			iload->Read(&relaxAmount, sizeof(relaxAmount), &nb);
			iload->Read(&relaxStretch, sizeof(relaxStretch), &nb);
			iload->Read(&relaxIteration, sizeof(relaxIteration), &nb);
			iload->Read(&relaxType, sizeof(relaxType), &nb);
			iload->Read(&relaxBoundary, sizeof(relaxBoundary), &nb);
			iload->Read(&relaxSaddle, sizeof(relaxSaddle), &nb);
			break;
		case FALLOFFSPACE_CHUNK:
			iload->Read(&falloffSpace, sizeof(falloffSpace), &nb);
			iload->Read(&falloffStr, sizeof(falloffStr), &nb);
			break;

		case SHOWPELTSEAMS_CHUNK:
			iload->Read(&alwaysShowSeams, sizeof(alwaysShowSeams), &nb);
			break;
		case SPLINEMAP_CHUNK:
			mSplineMap.Load(iload, 1);
			break;
		case SPLINEMAP_V2_CHUNK:
			mSplineMap.Load(iload, 2);
			break;
		case CONTROLLER_COUNT_CHUNK:
			{
				int controllerCount = 0;
				iload->Read(&controllerCount, sizeof(controllerCount), &nb);				
				mUVWControls.cont.SetCount(controllerCount);
				for (int i = 0; i < mUVWControls.cont.Count(); i++)
					mUVWControls.cont[i] = NULL;
			}
			break;
		default:
			mTVMaps_Max9.LoadOlderVersions(iload);
			//load the older versions here now
			mUVWControls.cont.SetCount(mTVMaps_Max9.v.Count());

			for (int i = 0; i < mUVWControls.cont.Count(); i++)
				mUVWControls.cont[i] = NULL;


			break;
		}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
	}


	UnwrapPostLoadCallback* unwrapplcb = new UnwrapPostLoadCallback(this,oldDataPresent);
	iload->RegisterPostLoadCallback(unwrapplcb);


	return IO_OK;
}

#define FACESEL_CHUNKID			0x0210
#define FSELSET_CHUNK			0x2846


#define ESELSET_CHUNK			0x2847


#define VSELSET_CHUNK			0x2848

IOResult UnwrapMod::SaveLocalData(ISave *isave, LocalModData *ld) {	
	MeshTopoData *d = (MeshTopoData*)ld;

	if (d == NULL)
		return IO_OK;

/*
	isave->BeginChunk(FACESEL_CHUNKID);
	BitArray fsel = d->GetFaceSel();
	fsel.Save(isave);
	isave->EndChunk();
*/
	
	if (d->fselSet.Count()) {
		isave->BeginChunk(FSELSET_CHUNK);
		d->fselSet.Save(isave);
		isave->EndChunk();
	}



	if (d->vselSet.Count()) {
		isave->BeginChunk(VSELSET_CHUNK);
		d->vselSet.Save(isave);
		isave->EndChunk();
	}

	if (d->eselSet.Count()) {
		isave->BeginChunk(ESELSET_CHUNK);
		d->eselSet.Save(isave);
		isave->EndChunk();
	}


//	int vct = d->GetNumberTVVerts(),//TVMaps.v.Count(), 
//		fct = d->GetNumberFaces();//TVMaps.f.Count();


//	isave->BeginChunk(VERTCOUNT_CHUNK);
//	isave->Write(&vct, sizeof(vct), &nb);
//	isave->EndChunk();

	if (d->GetNumberTVVerts()) 
	{
		isave->BeginChunk(VERTS3_CHUNK);
		d->SaveTVVerts(isave);
//		isave->Write(TVMaps.v.Addr(0), sizeof(UVW_TVVertClass)*vct, &nb);
		isave->EndChunk();
	}


	if ( d->GetNumberFaces()) {
		isave->BeginChunk(FACE5_CHUNK);
		d->SaveFaces(isave);
		isave->EndChunk();
	}



	if (d->GetNumberGeomVerts()) {
		isave->BeginChunk(GEOMPOINTS_CHUNK);
		d->SaveGeoVerts(isave);
		isave->EndChunk();
	}

	isave->BeginChunk(VERTSEL_CHUNK);
	d->SaveTVVertSelection(isave);
	isave->EndChunk();

	isave->BeginChunk(GVERTSEL_CHUNK);
	d->SaveGeoVertSelection(isave);
	isave->EndChunk();



	isave->BeginChunk(UEDGESELECTION_CHUNK);
	d->SaveTVEdgeSelection(isave);
	isave->EndChunk();

	isave->BeginChunk(GEDGESELECTION_CHUNK);
	d->SaveGeoEdgeSelection(isave);
	isave->EndChunk();

	isave->BeginChunk(FACESELECTION_CHUNK);
	d->SaveFaceSelection(isave);
	isave->EndChunk();

	isave->BeginChunk(SEAM_CHUNK);
	d->mSeamEdges.Save(isave);
	isave->EndChunk();	


	
	

	return IO_OK;
}

IOResult UnwrapMod::LoadLocalData(ILoad *iload, LocalModData **pld) {
	MeshTopoData *d = new MeshTopoData;
	*pld = d;
	IOResult res;	
	//need to invalidate the TVs edges since they are not saved and need to be regenerated
	d->SetTVEdgeInvalid();
	d->SetGeoEdgeInvalid();
	d->SetLoaded();
	while (IO_OK==(res=iload->OpenChunk())) 
	{
		switch(iload->CurChunkID())  
		{
/*
		case FACESEL_CHUNKID:
			{
				BitArray fsel;
				fsel.Load(iload);
				d->SetFaceSel(fsel);
				break;
			}
*/
		case FSELSET_CHUNK:
			res = d->fselSet.Load(iload);
			break;
		case VSELSET_CHUNK:
			res = d->vselSet.Load(iload);
			break;
		case ESELSET_CHUNK:
			res = d->eselSet.Load(iload);
			break;

/*
		case VERTCOUNT_CHUNK:
			iload->Read(&ct, sizeof(ct), &nb);
			TVMaps.v.SetCount(ct);
//			TVMaps.cont.SetCount(ct);
//			vsel.SetSize(ct);
//			for (int i=0; i<ct; i++) TVMaps.cont[i] = NULL;          
			break;
		case FACECOUNT_CHUNK:
			iload->Read(&ct, sizeof(ct), &nb);
			TVMaps.SetCountFaces(ct);
			break;

		case GEOMPOINTSCOUNT_CHUNK:
			iload->Read(&ct, sizeof(ct), &nb);
			TVMaps.geomPoints.SetCount(ct);
			break;
*/

		case FACE5_CHUNK:
			d->LoadFaces(iload);	
			break;

		case VERTS3_CHUNK:
			d->LoadTVVerts(iload);
//			iload->Read(TVMaps.v.Addr(0), sizeof(UVW_TVVertClass)*TVMaps.v.Count(), &nb);
			break;
		case GEOMPOINTS_CHUNK:
			d->LoadGeoVerts(iload);
//			iload->Read(TVMaps.geomPoints.Addr(0), sizeof(Point3)*TVMaps.geomPoints.Count(), &nb);
			break;

		case VERTSEL_CHUNK:
			{
			d->LoadTVVertSelection(iload);
			break;
			}
		case GVERTSEL_CHUNK:
			{
			d->LoadGeoVertSelection(iload);
			break;
			}
		case GEDGESELECTION_CHUNK:
			{
			d->LoadGeoEdgeSelection(iload);
			break;
			}
		case UEDGESELECTION_CHUNK:
			{

			d->LoadTVEdgeSelection(iload);//esel.Load(iload);
			break;
			}
		case FACESELECTION_CHUNK:
			{
			d->LoadFaceSelection(iload);//fsel.Load(iload);
			break;
			}

		case SEAM_CHUNK:
			{
			d->mSeamEdges.Load(iload);
			break;
			}

		}

		iload->CloseChunk();
		if (res!=IO_OK) return res;
	}
	d->FixUpLockedFlags();
	return IO_OK;
}


/*
void UnwrapMod::SynchWithMesh(Mesh &mesh)
{
int ct=0;
if (mesh.selLevel==MESH_FACE) {
for (int i=0; i<mesh.getNumFaces(); i++) {
if (mesh.faceSel[i]) ct++;
}
} else {
ct = mesh.getNumFaces();
}
if (ct != tvFace.Count()) {
DeleteAllRefsFromMe();
tvert.Resize(0);
cont.Resize(0);
tvFace.SetCount(ct);

TVFace *tvFaceM = mesh.tvFace;
Point3 *tVertsM = mesh.tVerts;
int numTV = channel ? mesh.getNumVertCol() : mesh.getNumTVerts();
if (channel) {
tvFaceM = mesh.vcFace;
tVertsM = mesh.vertCol;
}

if (mesh.selLevel==MESH_FACE) {
// Mark tverts that are used by selected faces
BitArray used;
if (tvFaceM) used.SetSize(numTV);
else used.SetSize(mesh.getNumVerts());
for (int i=0; i<mesh.getNumFaces(); i++) {
if (mesh.faceSel[i]) {
if (tvFaceM) {
for (int j=0; j<3; j++) 
used.Set(tvFaceM[i].t[j]);
} else {
for (int j=0; j<3; j++) 
used.Set(mesh.faces[i].v[j]);
}
}
}

// Now build a vmap
Tab<DWORD> vmap;
vmap.SetCount(used.GetSize());
int ix=0;
for (int i=0; i<used.GetSize(); i++) {
if (used[i]) vmap[i] = ix++;
else vmap[i] = UNDEFINED;				
}

// Copy in tverts
tvert.SetCount(ix);
cont.SetCount(ix);
vsel.SetSize(ix);
ix = 0;
Box3 box = mesh.getBoundingBox();
for (int i=0; i<used.GetSize(); i++) {
if (used[i]) {
cont[ix] = NULL;
if (tvFaceM) tvert[ix++] = tVertsM[i];
else {
// Do a planar mapping if there are no tverts
tvert[ix].x = mesh.verts[i].x/box.Width().x + 0.5f;
tvert[ix].y = mesh.verts[i].y/box.Width().y + 0.5f;
tvert[ix].z = mesh.verts[i].z/box.Width().z + 0.5f;
ix++;
}
}
}

// Copy in face and remap indices		
ix = 0;
for (int i=0; i<mesh.getNumFaces(); i++) {
if (mesh.faceSel[i]) {
if (tvFaceM) tvFace[ix] = tvFaceM[i];
else {
for (int j=0; j<3; j++) 
tvFace[ix].t[j] = mesh.faces[i].v[j];
}

for (int j=0; j<3; j++) {
tvFace[ix].t[j] = vmap[tvFace[ix].t[j]];
}
ix++;
}
}
} else {
// Just copy all the tverts and faces
if (tvFaceM) {
tvert.SetCount(numTV);
cont.SetCount(numTV);
vsel.SetSize(numTV);
for (int i=0; i<numTV; i++) {
tvert[i] = tVertsM[i];
cont[i]  = NULL;
}
for (int i=0; i<mesh.getNumFaces(); i++) {
tvFace[i] = tvFaceM[i];
}
} else {
Box3 box = mesh.getBoundingBox();
tvert.SetCount(mesh.getNumVerts());
cont.SetCount(mesh.getNumVerts());
vsel.SetSize(mesh.getNumVerts());
for (int i=0; i<mesh.getNumVerts(); i++) {
// Do a planar mapping if there are no tverts
tvert[i].x = mesh.verts[i].x/box.Width().x + 0.5f;
tvert[i].y = mesh.verts[i].y/box.Width().y + 0.5f;
tvert[i].z = mesh.verts[i].z/box.Width().z + 0.5f;
cont[i]  = NULL;
}
for (int i=0; i<mesh.getNumFaces(); i++) {
for (int j=0; j<3; j++) 
tvFace[i].t[j] = mesh.faces[i].v[j];
}
}
}
if (hView && editMod==this) {
InvalidateView();
}
}
}
*/
void UnwrapMod::GetUVWIndices(int &i1, int &i2)
{
	switch (uvw) {
case 0: i1 = 0; i2 = 1; break;
case 1: i1 = 1; i2 = 2; break;
case 2: i1 = 0; i2 = 2; break;
	}
}


//--- Floater Dialog -------------------------------------------------


#define TOOL_HEIGHT		30
#define SPINNER_HEIGHT	30

#define WM_SETUPMOD	WM_USER+0x18de

static HIMAGELIST hToolImages = NULL;
static HIMAGELIST hOptionImages = NULL;
static HIMAGELIST hViewImages = NULL;
static HIMAGELIST hVertexImages = NULL;

class DeleteResources {
public:
	~DeleteResources() {
		if (hToolImages) ImageList_Destroy(hToolImages);			
		if (hOptionImages) ImageList_Destroy(hOptionImages);			
		if (hViewImages) ImageList_Destroy(hViewImages);			
		if (hVertexImages) ImageList_Destroy(hVertexImages);			
	}
};
static DeleteResources	theDelete;





INT_PTR CALLBACK UnwrapFloaterDlgProc(
									  HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	UnwrapMod *mod = DLGetWindowLongPtr<UnwrapMod*>(hWnd);
	//POINTS p = MAKEPOINTS(lParam);	commented out by sca 10/7/98 -- causing warning since unused.
	switch (msg) {
case WM_INITDIALOG:

	mod = (UnwrapMod*)lParam;
	DLSetWindowLongPtr(hWnd, lParam);
	SendMessage(hWnd, WM_SETICON, ICON_SMALL, GetClassLongPtr(GetCOREInterface()->GetMAXHWnd(), GCLP_HICONSM)); // mjm - 3.12.99
	::SetWindowContextHelpId(hWnd, idh_unwrap_edit);
	GetCOREInterface()->RegisterDlgWnd(hWnd);
	mod->SetupDlg(hWnd);

	delEvent.SetEditMeshMod(mod);
	GetCOREInterface()->RegisterDeleteUser(&delEvent);

	//			SendMessage( mod->hTextures, CB_SETCURSEL, mod->CurrentMap, 0 );
	//			mod->SetupImage();
	mod->UpdateListBox();
	SendMessage( mod->hMatIDs, CB_SETCURSEL, mod->matid+1, 0 );

	if (mod->iFalloff) mod->iFalloff->SetCurFlyOff(mod->falloff,FALSE);
	if (mod->iFalloffSpace) mod->iFalloffSpace->SetCurFlyOff(mod->falloffSpace,FALSE);
	//			mod->iIncSelected->SetCurFlyOff(mod->incSelected,FALSE);
	if (mod->iMirror) mod->iMirror->SetCurFlyOff(mod->mirror,FALSE);
	if (mod->iUVW) mod->iUVW->SetCurFlyOff(mod->uvw,FALSE);

	if (mod->iHide) mod->iHide->SetCurFlyOff(mod->hide,FALSE);
	if (mod->iFreeze) mod->iFreeze->SetCurFlyOff(mod->freeze,FALSE);

	if (mod->iFilterSelected)
	{
		if (mod->filterSelectedFaces)
			mod->iFilterSelected->SetCheck(TRUE);
		else mod->iFilterSelected->SetCheck(FALSE);
	}


	if (mod->iSnap)
	{
		mod->iSnap->SetCurFlyOff(mod->initialSnapState);
		if (mod->pixelSnap)
		{
			mod->iSnap->SetCurFlyOff(1);
			mod->iSnap->SetCheck(TRUE);
		}
		if (mod->fnGetGridSnap())
		{
			mod->iSnap->SetCurFlyOff(0);
			mod->iSnap->SetCheck(TRUE);
		}
		else mod->iSnap->SetCheck(FALSE);
	}

	if (mod->iShowMap)
	{
		if (mod->showMap)
			mod->iShowMap->SetCheck(TRUE);
		else mod->iShowMap->SetCheck(FALSE);
	}

	if (mod->windowPos.length != 0) {
		mod->windowPos.flags = 0;
		mod->windowPos.showCmd = SW_SHOWNORMAL;
		SetWindowPlacement(hWnd,&mod->windowPos);
	}
#ifdef DESIGN_VER
	mod->ZoomExtents();
#endif
	break;
case WM_SHOWWINDOW:
	{
		if (!wParam)
		{
			if (mod->peltData.peltDialog.hWnd)
				ShowWindow(mod->peltData.peltDialog.hWnd,SW_HIDE);
			if (mod->hOptionshWnd)
				ShowWindow(mod->hOptionshWnd,SW_HIDE);
			if (mod->hRelaxDialog)
				ShowWindow(mod->hRelaxDialog,SW_HIDE);
			if (mod->renderUVWindow)
				ShowWindow(mod->renderUVWindow,SW_HIDE);



		}
		else 
		{
			if (mod->peltData.peltDialog.hWnd)
				ShowWindow(mod->peltData.peltDialog.hWnd,SW_SHOW);
			if (mod->hOptionshWnd)
				ShowWindow(mod->hOptionshWnd,SW_SHOW);
			if (mod->hRelaxDialog)
				ShowWindow(mod->hRelaxDialog,SW_SHOW);
			if (mod->renderUVWindow)
				ShowWindow(mod->renderUVWindow,SW_SHOW);


		}
		return FALSE;
		break;
	}

case WM_SYSCOMMAND:
	if ((wParam & 0xfff0) == SC_CONTEXTHELP) 
	{
		DoHelp(HELP_CONTEXT, idh_unwrap_edit); 
	}
	return FALSE;
	break;

case WM_SIZE:
	if (mod)
	{


		mod->minimized = FALSE;
		if (wParam == SIZE_MAXIMIZED)
		{
			WINDOWPLACEMENT floaterPos;
			floaterPos.length = sizeof(WINDOWPLACEMENT);
			GetWindowPlacement(hWnd,&floaterPos);



			Rect rect;
			GetWindowRect(hWnd,&rect);
			int w,h;
			w = rect.right - rect.left;
			h = rect.bottom - rect.top;
			if (w > mod->maximizeWidth)
				mod->maximizeWidth = w;
			if (h > mod->maximizeHeight)
				mod->maximizeHeight = h;

			SetWindowPos(hWnd,NULL,rect.left,rect.top,mod->maximizeWidth-2-mod->xWindowOffset,mod->maximizeHeight-mod->yWindowOffset,SWP_NOZORDER );
			mod->SizeDlg();
			mod->MoveScriptUI();
			mod->bringUpPanel = TRUE;
			return 0;
		}
		mod->SizeDlg();

		if (wParam == SIZE_MINIMIZED)
		{
			if (mod->peltData.peltDialog.hWnd)
				ShowWindow(mod->peltData.peltDialog.hWnd,SW_HIDE);
			if (mod->hOptionshWnd)
				ShowWindow(mod->hOptionshWnd,SW_HIDE);
			if (mod->hRelaxDialog)
				ShowWindow(mod->hRelaxDialog,SW_HIDE);
			if (mod->renderUVWindow)
				ShowWindow(mod->renderUVWindow,SW_HIDE);

			mod->minimized = TRUE;
			mod->MoveScriptUI();

			return 0;

		}
		else 
		{
			if (mod->peltData.peltDialog.hWnd)
				ShowWindow(mod->peltData.peltDialog.hWnd,SW_SHOW);
			if (mod->hOptionshWnd)
				ShowWindow(mod->hOptionshWnd,SW_SHOW);
			if (mod->hRelaxDialog)
				ShowWindow(mod->hRelaxDialog,SW_SHOW);
			if (mod->renderUVWindow)
				ShowWindow(mod->renderUVWindow,SW_SHOW);


			mod->bringUpPanel = TRUE;
		}
	}
	break;
case WM_MOVE:
	if (mod) 
	{
		mod->MoveScriptUI();

	}
	break;

case WM_ACTIVATE:


	if (LOWORD(wParam) == WA_INACTIVE) 
	{
		mod->floaterWindowActive = FALSE;
		mod->enableActionItems = FALSE;

	}
	else 
	{
		mod->enableActionItems = TRUE;
		if ((LOWORD(wParam) == WA_CLICKACTIVE) &&  (!mod->floaterWindowActive))
		{
			mod->bringUpPanel = TRUE;
		}

		mod->floaterWindowActive = TRUE;

	}

	break;

case WM_PAINT: {
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hWnd,&ps);
	Rect rect;
	GetClientRect(hWnd,&rect);			
	rect.top += TOOL_HEIGHT-2;
	SelectObject(hdc,GetStockObject(WHITE_BRUSH));			
	WhiteRect3D(hdc,rect,TRUE);
	EndPaint(hWnd,&ps);
	break;
			   }
case CC_SPINNER_BUTTONDOWN:
	if (LOWORD(wParam) != IDC_UNWRAP_STRSPIN) 
	{
		theHold.SuperBegin();				
		theHold.Begin();

	}
	break;
	/*
	case WM_ACTIVATE:
	{
	switch ( LOWORD(wParam) )
	{
	case WA_ACTIVE:
	case WA_CLICKACTIVE:
	{
	GetCOREInterface()->GetActionManager()->ActivateActionTable(mod->actionTable, kIUVWUnwrapQuad);
	break;
	}
	case WA_INACTIVE:
	{
	GetCOREInterface()->GetActionManager()->DeactivateActionTable(mod->actionTable, kIUVWUnwrapQuad);
	break;
	}
	}
	break;
	}
	*/			
	/*
	case WM_RBUTTONDOWN:
	{
	// Get the current vieport quad menu from the menu manager.
	// SCM 4/12/00
	//				IMenuContext *pContext = GetCOREInterface()->GetMenuManager()->GetMenuMan().GetContext(kIUVWUnwrapQuad);
	IMenuContext *pContext = GetCOREInterface()->GetMenuManager()->GetContext(kIUVWUnwrapQuad);
	DbgAssert(pContext);
	DbgAssert(pContext->GetType() == kMenuContextQuadMenu);
	IQuadMenuContext *pQMContext = (IQuadMenuContext *)pContext;
	IQuadMenu *pMenu = pQMContext->GetMenu( pQMContext->GetCurrentMenuIndex() );
	DbgAssert(pMenu);
	pMenu->TrackMenu(hWnd);
	return TRUE;
	break;
	}
	*/


case CC_SPINNER_CHANGE:
	if (LOWORD(wParam) == IDC_UNWRAP_STRSPIN) 
	{
		mod->RebuildDistCache();
		UpdateWindow(hWnd);
		mod->InvalidateView();
	}
	else
	{
		if (!theHold.Holding()) {
			theHold.SuperBegin();
			theHold.Begin();
		}


		switch (LOWORD(wParam)) {
case IDC_UNWRAP_USPIN:
	mod->tempWhich = 0;
	mod->TypeInChanged(0);
	break;
case IDC_UNWRAP_VSPIN:
	mod->tempWhich = 1;
	mod->TypeInChanged(1);
	break;
case IDC_UNWRAP_WSPIN:
	mod->tempWhich = 2;
	mod->TypeInChanged(2);
	break;
		}

		UpdateWindow(hWnd);
	}
	break;

case WM_CUSTEDIT_ENTER:
case CC_SPINNER_BUTTONUP:
	if ( (LOWORD(wParam) == IDC_UNWRAP_STR) || (LOWORD(wParam) == IDC_UNWRAP_STRSPIN) )
	{
		float str = mod->iStr->GetFVal();
		TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap.setFalloffDist"));
		macroRecorder->FunctionCall(mstr, 1, 0,
			mr_float,str);
		macroRecorder->EmitScript();
		mod->RebuildDistCache();
		mod->InvalidateView();
		UpdateWindow(hWnd);
	}
	else
	{
		if (HIWORD(wParam) || msg==WM_CUSTEDIT_ENTER) {

			if (theHold.Holding())
			{
				theHold.Accept(_T(GetString(IDS_PW_MOVE_UVW)));
				theHold.SuperAccept(_T(GetString(IDS_PW_MOVE_UVW)));
			}


			if (mod->tempWhich ==0)
			{
				TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap.moveX"));
				macroRecorder->FunctionCall(mstr, 1, 0,
				mr_float,mod->tempAmount);
			}
			else if (mod->tempWhich ==1)
			{
				TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap.moveY"));
				macroRecorder->FunctionCall(mstr, 1, 0,
				mr_float,mod->tempAmount);
			}
			else if (mod->tempWhich ==2)
			{
				TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap.moveZ"));
				macroRecorder->FunctionCall(mstr, 1, 0,
				mr_float,mod->tempAmount);
			}

			if (mod->fnGetRelativeTypeInMode())
				mod->SetupTypeins();


		} else {
			theHold.Cancel();
			theHold.SuperCancel();

			mod->NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);
			mod->InvalidateView();
			UpdateWindow(hWnd);
			GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
			if (mod->fnGetRelativeTypeInMode())
				mod->SetupTypeins();

		}
	}
	break;

case 0x020A:
	{
		int delta = (short) HIWORD(wParam);

		int xPos = GET_X_LPARAM(lParam); 
		int yPos = GET_Y_LPARAM(lParam); 


		RECT rect;
		GetWindowRect(mod->hView,&rect);
		xPos = xPos - rect.left;
		yPos = yPos - rect.top;

		float z;
		if (delta<0)
			z = (1.0f/(1.0f-0.0025f*delta));
		else z = (1.0f+0.0025f*delta);
		mod->zoom = mod->zoom * z;

		if (mod->middleMode->inDrag)
		{
			mod->xscroll = mod->xscroll*z;
			mod->yscroll = mod->yscroll*z;
		}
		else
		{

			Rect rect;
			GetClientRect(mod->hView,&rect);	
			int centerX = (rect.w()-1)/2-xPos;
			int centerY = (rect.h()-1)/2-yPos;


			mod->xscroll = (mod->xscroll + centerX)*z;
			mod->yscroll = (mod->yscroll + centerY)*z;


			mod->xscroll -= centerX;
			mod->yscroll -= centerY;


		}
		//watje tile
		mod->tileValid = FALSE;

		mod->middleMode->ResetInitialParams();

		mod->InvalidateView();

		break;
	}

case WM_COMMAND:
	{
		switch (LOWORD(wParam)) {
case ID_TOOL_MOVE:

case ID_TOOL_ROTATE:
case ID_TOOL_SCALE:
case ID_TOOL_WELD:
case ID_TOOL_PAN:
case ID_TOOL_ZOOM:
case ID_TOOL_ZOOMREG:
case ID_UNWRAP_MOVE:
case ID_UNWRAP_ROTATE:
case ID_UNWRAP_SCALE:
case ID_UNWRAP_PAN:
case ID_UNWRAP_WELD:
case ID_UNWRAP_ZOOM:
case ID_UNWRAP_ZOOMREGION:
case ID_FREEFORMMODE:
case ID_SKETCHMODE:
case ID_PAINTSELECTMODE:

	if (mod->iMove)
	{
		mod->move = mod->iMove->GetCurFlyOff();
		if (mod->move == 0)
			mod->iMove->SetTooltip(TRUE,GetString(IDS_RB_MOVE));
		else if (mod->move == 1)
			mod->iMove->SetTooltip(TRUE,GetString(IDS_PW_MOVEH));
		else if (mod->move == 2)
			mod->iMove->SetTooltip(TRUE,GetString(IDS_PW_MOVEV));
	}

	if (mod->iScale)
	{
		mod->scale = mod->iScale->GetCurFlyOff();
		if (mod->scale == 0)
			mod->iScale->SetTooltip(TRUE,GetString(IDS_RB_SCALE));
		else if (mod->scale == 1)
			mod->iScale->SetTooltip(TRUE,GetString(IDS_PW_SCALEH));
		else if (mod->scale == 2)
			mod->iScale->SetTooltip(TRUE,GetString(IDS_PW_SCALEV));
	}

	mod->SetMode(LOWORD(wParam));
	break;				

case ID_TOOL_FALLOFF:
	{
		if (mod->iFalloff) mod->falloff = mod->iFalloff->GetCurFlyOff();
		
		TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap.setFalloffType"));
		macroRecorder->FunctionCall(mstr, 1, 0,
			mr_int,mod->falloff+1);
		macroRecorder->EmitScript();

		mod->RebuildDistCache();
		mod->InvalidateView();
		break;
	}
case ID_TOOL_FALLOFF_SPACE:
	{
		if (mod->iFalloffSpace) 
		{
			mod->falloffSpace = mod->iFalloffSpace->GetCurFlyOff();
			if (mod->falloffSpace)
				mod->iFalloffSpace->SetTooltip(TRUE,GetString(IDS_PW_FALLOFFSPACEUVW));
			else mod->iFalloffSpace->SetTooltip(TRUE,GetString(IDS_PW_FALLOFFSPACE));
		}

		TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap.setFalloffSpace"));
		macroRecorder->FunctionCall(mstr, 1, 0,
			mr_int,mod->falloffSpace+1);
		macroRecorder->EmitScript();


		mod->RebuildDistCache();
		mod->InvalidateView();
		break;
	}

case ID_TOOL_DECSELECTED:
	{
		if (mod->iDecSelected) mod->iDecSelected->SetTooltip(TRUE,GetString(IDS_PW_CONTRACTSELECTION));
		TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap.contractSelection"));
		macroRecorder->FunctionCall(mstr, 0, 0);
		macroRecorder->EmitScript();
		mod->fnContractSelection();
		break;
	}

case ID_TOOL_INCSELECTED:
	{
		if (mod->iIncSelected) mod->iIncSelected->SetTooltip(TRUE,GetString(IDS_PW_EXPANDSELECTION)); 
		TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap.expandSelection"));
		macroRecorder->FunctionCall(mstr, 0, 0);
		macroRecorder->EmitScript();
		mod->fnExpandSelection();
		break;
	}
case ID_UNWRAP_MIRROR:
case ID_TOOL_MIRROR:
	if (mod->iMirror)
	{
		mod->mirror = mod->iMirror->GetCurFlyOff();

		if (mod->mirror ==0)
		{
			mod->iMirror->SetTooltip(TRUE,GetString(IDS_PW_MIRRORH));
			TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap.mirrorh"));
			macroRecorder->FunctionCall(mstr, 0, 0);
			macroRecorder->EmitScript();
			mod->MirrorPoints( mod->mirror);
		}
		else if (mod->mirror ==1)
		{
			mod->iMirror->SetTooltip(TRUE,GetString(IDS_PW_MIRRORV));
			TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap.mirrorv"));
			macroRecorder->FunctionCall(mstr, 0, 0);
			macroRecorder->EmitScript();
			mod->MirrorPoints( mod->mirror);
		}
		else if (mod->mirror ==2)
		{
			mod->iMirror->SetTooltip(TRUE,GetString(IDS_PW_FLIPH));
			TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap.fliph"));
			macroRecorder->FunctionCall(mstr, 0, 0);
			macroRecorder->EmitScript();
			mod->FlipPoints(mod->mirror-2);
		}
		else if (mod->mirror ==3)
		{
			mod->iMirror->SetTooltip(TRUE,GetString(IDS_PW_FLIPV));
			TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap.flipv"));
			macroRecorder->FunctionCall(mstr, 0, 0);
			macroRecorder->EmitScript();
			mod->FlipPoints(mod->mirror-2);

		}
	}



	break;
case ID_TOOL_LOCKSELECTED:
	{
	if (mod->iLockSelected) mod->lockSelected = mod->iLockSelected->IsChecked();
	TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap.lock"));
	macroRecorder->FunctionCall(mstr, 0, 0);
	macroRecorder->EmitScript();
	break;
	}
case ID_TOOL_FILTER_SELECTEDFACES:
	{
	if (mod->iFilterSelected) 
		mod->filterSelectedFaces = mod->iFilterSelected->IsChecked();
	mod->BuildFilterSelectedFacesData();
	mod->InvalidateView();
	UpdateWindow(hWnd);
	TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap.filterselected"));
	macroRecorder->FunctionCall(mstr, 0, 0);
	macroRecorder->EmitScript();

	break;
	}
case ID_UNWRAP_EXTENT:
case ID_TOOL_ZOOMEXT:
	{
		if (mod->iZoomExt) mod->zoomext = mod->iZoomExt->GetCurFlyOff();
		//watje tile
		mod->tileValid = FALSE;

		if (mod->zoomext == 0)
		{
			mod->ZoomExtents();
			TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap.fit"));
			macroRecorder->FunctionCall(mstr, 0, 0);
		}
		else if (mod->zoomext == 1)
		{
			mod->ZoomSelected();
			TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap.fitselected"));
			macroRecorder->FunctionCall(mstr, 0, 0);
		}
		else if (mod->zoomext == 2)
		{
			mod->FrameSelectedElement();
			TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap5.fitSelectedElement"));
			macroRecorder->FunctionCall(mstr, 0, 0);
		}


		macroRecorder->EmitScript();

		break;
	}

case ID_TOOL_FILTER_MATID:
	if ( HIWORD(wParam) == CBN_SELCHANGE ) {
		//get count
		mod->matid = SendMessage( mod->hMatIDs, CB_GETCURSEL, 0, 0 )-1;
		mod->SetMatFilters();
		TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap.setMatID"));
		macroRecorder->FunctionCall(mstr, 1, 0,
			mr_int, mod->matid+2);
		macroRecorder->EmitScript();

		mod->UpdateListBox();
		if (mod->dropDownListIDs.Count() >= 2)
		{
			mod->CurrentMap = mod->dropDownListIDs[1];
			SendMessage( mod->hTextures, CB_SETCURSEL, 1, 0 );
		}
		else
		{
			SendMessage( mod->hTextures, CB_SETCURSEL, 0, 0 );
			mod->CurrentMap = mod->dropDownListIDs[0];
		}


		mod->SetupImage();


		mod->InvalidateView();
		SetFocus(hWnd);  //kinda hack, once the user has selected something we immediatetly change focus so he middle mouse scroll does not cycle the drop list
	}
	break;
case ID_TOOL_TEXTURE_COMBO:
	if ( HIWORD(wParam) == CBN_SELCHANGE ) {
		//get count
		SetFocus(hWnd); //kinda hack, once the user has selected something we immediatetly change focus so he middle mouse scroll does not cycle the drop list
		int ct = SendMessage( mod->hTextures, CB_GETCOUNT, 0, 0 );
		int res = SendMessage( mod->hTextures, CB_GETCURSEL, 0, 0 );
		//pick a new map
		if (res == (ct -4))
		{
			mod->PickMap();
			mod->SetupImage();
			//			mod->UpdateListBox();			
			SendMessage( mod->hTextures, CB_SETCURSEL, mod->CurrentMap, 0 );
		}
		if (res == (ct -3))
		{
			mod->DeleteFromMaterialList(mod->CurrentMap);
			mod->SetupImage();
			mod->UpdateListBox();
			SendMessage( mod->hTextures, CB_SETCURSEL, mod->CurrentMap, 0 );
		}
		if (res == (ct -2))
		{
			mod->ResetMaterialList();
			mod->UpdateListBox();
			mod->SetupImage();
			SendMessage( mod->hTextures, CB_SETCURSEL, mod->CurrentMap, 0 );
		}
		else if (res < (ct-4))
			//select a current
		{
			//			mod->CurrentMap = res;
			if ((res >= 0) && (res < mod->dropDownListIDs.Count()))
			{
				mod->CurrentMap = mod->dropDownListIDs[res];

				if (mod->CurrentMap == 0)
					mod->ShowCheckerMaterial(TRUE);
				else mod->ShowCheckerMaterial(FALSE);

				mod->SetupImage();
			}


			TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap.setCurrentMap"));
			macroRecorder->FunctionCall(mstr, 1, 0,
				mr_int, mod->CurrentMap+1);
			macroRecorder->EmitScript();

		}

	}
	break;



case ID_TOOL_UVW:
	{
		if (mod->iUVW) mod->uvw = mod->iUVW->GetCurFlyOff();
		TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap.setUVSpace"));
		macroRecorder->FunctionCall(mstr, 1, 0,
			mr_int, mod->uvw+1);
		macroRecorder->EmitScript();

		mod->InvalidateView();
		break;
	}

case ID_TOOL_PROP:
	{
		SetFocus(hWnd);
		TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap.options"));
		macroRecorder->FunctionCall(mstr, 0, 0);
		macroRecorder->EmitScript();

		mod->PropDialog();

		break;
	}

case ID_TOOL_SHOWMAP:
	{
		if (mod->iShowMap)
			mod->showMap = mod->iShowMap->IsChecked();
		TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap.DisplayMap"));
		macroRecorder->FunctionCall(mstr, 1, 0,
			mr_bool, mod->showMap);
		macroRecorder->EmitScript();

		mod->InvalidateView();
		break;
	}
case ID_TOOL_SNAP:
	{
		if (mod->iSnap->GetCurFlyOff() == 1)
		{
			if (mod->iSnap) mod->pixelSnap = mod->iSnap->IsChecked();
			mod->gridSnap = FALSE;
		}
		else
		{
			if (mod->iSnap) mod->fnSetGridSnap(mod->iSnap->IsChecked());
		}

		//					if (mod->iSnap) mod->pixelSnap = mod->iSnap->IsChecked();
		//					mod->InvalidateView();
		TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap.snap"));
		macroRecorder->FunctionCall(mstr, 0, 0);
		macroRecorder->EmitScript();
		break;
	}

case ID_UNWRAP_BREAK:
case ID_TOOL_BREAK:
	{
		mod->BreakSelected();
		mod->InvalidateView();		
		TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap.breakSelected"));
		macroRecorder->FunctionCall(mstr, 0, 0);
		macroRecorder->EmitScript();
		break;
	}
case ID_UNWRAP_WELDSELECTED:
case ID_TOOL_WELD_SEL:
	{
		mod->WeldSelected(TRUE,TRUE);					
		//					mod->InvalidateView();
		TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap.weldSelected"));
		macroRecorder->FunctionCall(mstr, 0, 0);
		macroRecorder->EmitScript();
		break;
	}
case ID_TOOL_UPDATE:
	{
		mod->SetupImage();
		mod->UpdateListBox();
		mod->InvalidateView();
		TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap.updateMap"));
		macroRecorder->FunctionCall(mstr, 0, 0);
		macroRecorder->EmitScript();
		break;
	}
case ID_TOOL_HIDE:
	{
		if (mod->iHide) mod->hide = mod->iHide->GetCurFlyOff();
		if (mod->hide == 0)
		{
			if (mod->iHide) mod->iHide->SetTooltip(TRUE,GetString(IDS_PW_HIDE));
			mod->HideSelected();
			TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap.hide"));
			macroRecorder->FunctionCall(mstr, 0, 0);
			macroRecorder->EmitScript();

		}
		else{
			if (mod->iHide) mod->iHide->SetTooltip(TRUE,GetString(IDS_PW_UNHIDE));
			mod->UnHideAll();
			TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap.unhide"));
			macroRecorder->FunctionCall(mstr, 0, 0);
			macroRecorder->EmitScript();

		}
		mod->InvalidateView();
		break;
	}

case ID_TOOL_FREEZE:
	{
		if (mod->iFreeze) mod->freeze = mod->iFreeze->GetCurFlyOff();
		if (mod->freeze == 0)
		{
			if (mod->iFreeze) mod->iFreeze->SetTooltip(TRUE,GetString(IDS_PW_FREEZE));
			mod->FreezeSelected();
			TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap.freeze"));
			macroRecorder->FunctionCall(mstr, 0, 0);
			macroRecorder->EmitScript();
		}
		else
		{
			if (mod->iFreeze) mod->iFreeze->SetTooltip(TRUE,GetString(IDS_PW_UNFREEZE));
			mod->UnFreezeAll();
			TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap.unfreeze"));
			macroRecorder->FunctionCall(mstr, 0, 0);
			macroRecorder->EmitScript();

		}
		mod->InvalidateView();
		break;
	}
case ID_ABSOLUTETYPEIN:
	{
		BOOL abs = FALSE;
		if (mod->iUVWSpinAbsoluteButton)
		{
			if (mod->iUVWSpinAbsoluteButton->IsChecked())
				abs = FALSE;
			else abs = TRUE;
			mod->SetAbsoluteTypeInMode(abs);
		}
		TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap4.SetRelativeTypeIn"));
		if (abs)
		{
			macroRecorder->FunctionCall(mstr, 1, 0,
			mr_bool, FALSE);
		}
		else
		{
			macroRecorder->FunctionCall(mstr, 1, 0,
			mr_bool, TRUE);
		}
		macroRecorder->EmitScript();
		break;
	}
case IDOK:
case IDCANCEL:
	break;
default:
	IMenuBarContext* pContext = (IMenuBarContext*) GetCOREInterface()->GetMenuManager()->GetContext(kUnwrapMenuBar);
	if (pContext)
	{
		int id = LOWORD(wParam);
		int hid = HIWORD(wParam);
		if (hid == 0)
			pContext->ExecuteAction(id);


	}
	return TRUE;

	/*
	case ID_TOOL_UNFREEZE:
	mod->UnFreezeAll();
	mod->InvalidateView();
	break;
	*/

		}

		IMenuBarContext* pContext = (IMenuBarContext*) GetCOREInterface()->GetMenuManager()->GetContext(kUnwrapMenuBar);
		if (pContext)
			pContext->UpdateWindowsMenu();
		break;
	}

case WM_CLOSE:
	{
		if ((mod->mode == ID_UNWRAP_WELD)||(mod->mode == ID_TOOL_WELD))
			mod->SetMode(mod->oldMode);
		HWND maxHwnd = GetCOREInterface()->GetMAXHWnd();
		mod->fnSetMapMode(NOMAP);
		SetFocus(maxHwnd);
		DestroyWindow(hWnd);
		if (mod->hRelaxDialog)
		{
			DestroyWindow(mod->hRelaxDialog);
			mod->hRelaxDialog = NULL;
		}
		break;
	}

case WM_DESTROY:
	{
		mod->DestroyDlg();
		GetCOREInterface()->UnRegisterDeleteUser(&delEvent);
		if (mod->hRelaxDialog)
		{
			DestroyWindow(mod->hRelaxDialog);
			mod->hRelaxDialog = NULL;
		}
		break;
	}

default:
	return FALSE;
	}

	return TRUE;
}



static LRESULT CALLBACK UnwrapViewProc(
									   HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	UnwrapMod *mod = DLGetWindowLongPtr<UnwrapMod*>(hWnd);
	switch (msg) {
case WM_CREATE:
	break;

case WM_SIZE:			
	if (mod) 
	{
		mod->iBuf->Resize();
		//watje tile
		mod->tileValid = FALSE;
		mod->iTileBuf->Resize();

		mod->InvalidateView();
		mod->MoveScriptUI();
	}
	break;


case WM_PAINT:
	if (mod) mod->PaintView();
	break;

case 0x020A:
	break;

case WM_LBUTTONDOWN:
case WM_LBUTTONDBLCLK:
case WM_LBUTTONUP:		
case WM_RBUTTONDOWN:
case WM_RBUTTONUP:
case WM_RBUTTONDBLCLK:
case WM_MBUTTONDOWN:
case WM_MBUTTONDBLCLK:
case WM_MBUTTONUP:		

case WM_MOUSEMOVE:

	return mod->mouseMan.MouseWinProc(hWnd,msg,wParam,lParam);

default:
	return DefWindowProc(hWnd,msg,wParam,lParam);
	}	
	return 0;
}

void UnwrapMod::DestroyDlg()
{
	UnwrapMod* mod = DLGetWindowLongPtr<UnwrapMod*>(hWnd);
	if ( mod )
	{
		mod->FreeSnapBuffer();
	}

	if (renderUVWindow != NULL)
	{
		DestroyWindow(renderUVWindow);
	}
	renderUVWindow = NULL;

	ColorMan()->SetColor(LINECOLORID,  lineColor);
	ColorMan()->SetColor(SELCOLORID, selColor);
	ColorMan()->SetColor(OPENEDGECOLORID,  openEdgeColor);
	ColorMan()->SetColor(HANDLECOLORID,  handleColor);
	ColorMan()->SetColor(FREEFORMCOLORID,  freeFormColor);
	ColorMan()->SetColor(SHAREDCOLORID,  sharedColor);
	ColorMan()->SetColor(BACKGROUNDCOLORID,  backgroundColor);

	//new
	ColorMan()->SetColor(GRIDCOLORID,  gridColor);

	//get windowpos
	windowPos.length = sizeof(WINDOWPLACEMENT); 
	GetWindowPlacement(hWnd,&windowPos);
	EndScriptUI();

	ReleaseICustToolbar(iTool);
	iTool = NULL;
	ReleaseICustToolbar(iView);
	iView = NULL;
	ReleaseICustToolbar(iOption);
	iOption = NULL;
	ReleaseICustToolbar(iFilter);
	iFilter = NULL;
	ReleaseICustToolbar(iVertex);
	iVertex = NULL;

	ReleaseICustToolbar(iUVWSpinBar);
	iUVWSpinBar = NULL;
	if (iUVWSpinAbsoluteButton) ReleaseICustButton(iUVWSpinAbsoluteButton   ); 
	iUVWSpinAbsoluteButton    = NULL;


	if (iMove) ReleaseICustButton(iMove   ); 
	iMove    = NULL;


	if (iFreeForm) ReleaseICustButton(iFreeForm   ); iFreeForm    = NULL;

	if (iRot) ReleaseICustButton(iRot    ); 
	iRot     = NULL;
	if (iScale) ReleaseICustButton(iScale  ); 
	iScale   = NULL;

	if (iFalloff) ReleaseICustButton(iFalloff   ); 
	iFalloff    = NULL;
	if (iFalloffSpace) ReleaseICustButton(iFalloffSpace   ); 
	iFalloffSpace    = NULL;

	if (iMirror) ReleaseICustButton(iMirror  ); 
	iMirror   = NULL;
	if (iWeld) ReleaseICustButton(iWeld   ); 
	iWeld    = NULL;

	if (iPan) ReleaseICustButton(iPan); 
	iPan     = NULL;

	if (iZoom) ReleaseICustButton(iZoom   ); 
	iZoom    = NULL;
	if (iUpdate) ReleaseICustButton(iUpdate ); 
	iUpdate  = NULL;
	if (iZoomReg) ReleaseICustButton(iZoomReg); 
	iZoomReg = NULL;
	if (iZoomExt) ReleaseICustButton(iZoomExt); 
	iZoomExt = NULL;
	if (iUVW) ReleaseICustButton(iUVW	   ); 
	iUVW	   = NULL;
	if (iProp) ReleaseICustButton(iProp   ); 
	iProp    = NULL;
	if (iShowMap) ReleaseICustButton(iShowMap); 
	iShowMap = NULL;
	if (lockSelected) ReleaseICustButton(iLockSelected); 
	iLockSelected = NULL;

	if (iFilterSelected) ReleaseICustButton(iFilterSelected); 
	iFilterSelected = NULL;

	if (iHide) ReleaseICustButton(iHide); 
	iHide = NULL;
	if (iFreeze) ReleaseICustButton(iFreeze); 
	iFreeze = NULL;
	if (iIncSelected) ReleaseICustButton(iIncSelected); 
	iIncSelected = NULL;

	if (iDecSelected) 
		ReleaseICustButton(iDecSelected); 
	iDecSelected = NULL;

	if (iSnap) ReleaseICustButton(iSnap); iSnap = NULL;
	if (iU) ReleaseISpinner(iU); 
	iU = NULL;
	if (iV) ReleaseISpinner(iV); 
	iV = NULL;
	if (iW) ReleaseISpinner(iW); 
	iW = NULL;

	ReleaseISpinner(iStr); iStr = NULL;

	if (iWeldSelected) ReleaseICustButton(iWeldSelected); 
	iWeldSelected = NULL;

	if (iBreak) 
		ReleaseICustButton(iBreak); 
	iBreak = NULL;

	DestroyIOffScreenBuf(iBuf); iBuf   = NULL;

	//watje tile
	tileValid = FALSE;
	DestroyIOffScreenBuf(iTileBuf); iTileBuf   = NULL;

	delete moveMode; moveMode = NULL;
	delete freeFormMode; freeFormMode = NULL;
	delete sketchMode; sketchMode = NULL;

	delete paintSelectMode; paintSelectMode = NULL;

	delete rotMode; rotMode = NULL;
	delete scaleMode; scaleMode = NULL;
	delete panMode; panMode = NULL;
	delete zoomMode; zoomMode = NULL;
	delete zoomRegMode; zoomRegMode = NULL;
	delete weldMode; weldMode = NULL;
	//PELT
	delete peltStraightenMode; peltStraightenMode = NULL;

	delete rightMode; rightMode = NULL;	
	delete middleMode; middleMode = NULL;	
	mouseMan.SetMouseProc(NULL,LEFT_BUTTON,0);
	mouseMan.SetMouseProc(NULL,RIGHT_BUTTON,0);
	mouseMan.SetMouseProc(NULL,MIDDLE_BUTTON,0);
	GetCOREInterface()->UnRegisterDlgWnd(hWnd);


	if (mode == ID_SKETCHMODE)
		SetMode(ID_UNWRAP_MOVE);

	this->hWnd = NULL;

	if (CurrentMap == 0)
		ShowCheckerMaterial(FALSE);


}



void UnwrapMod::SetupDlg(HWND hWnd)
{
	if ((CurrentMap < 0) || (CurrentMap >= pblock->Count(unwrap_texmaplist)))
		CurrentMap = 0;

	lineColor = ColorMan()->GetColor(LINECOLORID);
	selColor = ColorMan()->GetColor(SELCOLORID );
	openEdgeColor = ColorMan()->GetColor(OPENEDGECOLORID  );
	handleColor = ColorMan()->GetColor(HANDLECOLORID  );
	freeFormColor = ColorMan()->GetColor(FREEFORMCOLORID  );
	sharedColor = ColorMan()->GetColor(SHAREDCOLORID  );
	backgroundColor = ColorMan()->GetColor(BACKGROUNDCOLORID  );

	//new
	gridColor = ColorMan()->GetColor(GRIDCOLORID  );

	this->hWnd = hWnd;

	hView = GetDlgItem(hWnd,IDC_UNWRAP_VIEW);
	DLSetWindowLongPtr(hView, this);
	iBuf = CreateIOffScreenBuf(hView);
	iBuf->SetBkColor(backgroundColor);
	viewValid    = FALSE;
	typeInsValid = FALSE;

	//watje tile
	iTileBuf = CreateIOffScreenBuf(hView);
	iTileBuf->SetBkColor(backgroundColor);
	tileValid = FALSE;

	moveMode = new MoveMode(this);
	rotMode = new RotateMode(this);
	scaleMode = new ScaleMode(this);
	panMode = new PanMode(this);
	zoomMode = new ZoomMode(this);
	zoomRegMode = new ZoomRegMode(this);
	weldMode = new WeldMode(this);
	//PELT
	peltStraightenMode = new PeltStraightenMode(this);

	rightMode = new RightMouseMode(this);
	middleMode = new MiddleMouseMode(this);
	freeFormMode = new FreeFormMode(this);
	sketchMode = new SketchMode(this);
	paintSelectMode = new PaintSelectMode(this);

	mouseMan.SetMouseProc(rightMode,RIGHT_BUTTON,1);
	mouseMan.SetMouseProc(middleMode,MIDDLE_BUTTON,2);

	iU = GetISpinner(GetDlgItem(hWnd,IDC_UNWRAP_USPIN));
	iU->LinkToEdit(GetDlgItem(hWnd,IDC_UNWRAP_U),EDITTYPE_FLOAT);
	iU->SetLimits(-9999999, 9999999, FALSE);
	iU->SetAutoScale();

	iV = GetISpinner(GetDlgItem(hWnd,IDC_UNWRAP_VSPIN));
	iV->LinkToEdit(GetDlgItem(hWnd,IDC_UNWRAP_V),EDITTYPE_FLOAT);
	iV->SetLimits(-9999999, 9999999, FALSE);
	iV->SetAutoScale();

	iW = GetISpinner(GetDlgItem(hWnd,IDC_UNWRAP_WSPIN));
	iW->LinkToEdit(GetDlgItem(hWnd,IDC_UNWRAP_W),EDITTYPE_FLOAT);
	iW->SetLimits(-9999999, 9999999, FALSE);
	iW->SetAutoScale();	

	if (showIconList[19]) 		
	{
		ShowWindow(GetDlgItem(hWnd,IDC_UNWRAP_USPIN),SW_SHOW);
		ShowWindow(GetDlgItem(hWnd,IDC_UNWRAP_U),SW_SHOW);
		ShowWindow(GetDlgItem(hWnd,IDC_UNWRAP_ULABEL),SW_SHOW);
	}
	else
	{
		ShowWindow(GetDlgItem(hWnd,IDC_UNWRAP_USPIN),SW_HIDE);
		ShowWindow(GetDlgItem(hWnd,IDC_UNWRAP_U),SW_HIDE);
		ShowWindow(GetDlgItem(hWnd,IDC_UNWRAP_ULABEL),SW_HIDE);
	}


	if (showIconList[20]) 		
	{
		ShowWindow(GetDlgItem(hWnd,IDC_UNWRAP_VSPIN),SW_SHOW);
		ShowWindow(GetDlgItem(hWnd,IDC_UNWRAP_V),SW_SHOW);
		ShowWindow(GetDlgItem(hWnd,IDC_UNWRAP_VLABEL),SW_SHOW);

	}
	else
	{
		ShowWindow(GetDlgItem(hWnd,IDC_UNWRAP_VSPIN),SW_HIDE);
		ShowWindow(GetDlgItem(hWnd,IDC_UNWRAP_V),SW_HIDE);
		ShowWindow(GetDlgItem(hWnd,IDC_UNWRAP_VLABEL),SW_HIDE);
	}

	if (showIconList[21]) 		
	{
		ShowWindow(GetDlgItem(hWnd,IDC_UNWRAP_WSPIN),SW_SHOW);
		ShowWindow(GetDlgItem(hWnd,IDC_UNWRAP_W),SW_SHOW);
		ShowWindow(GetDlgItem(hWnd,IDC_UNWRAP_WLABEL),SW_SHOW);
	}
	else
	{
		ShowWindow(GetDlgItem(hWnd,IDC_UNWRAP_WSPIN),SW_HIDE);
		ShowWindow(GetDlgItem(hWnd,IDC_UNWRAP_W),SW_HIDE);
		ShowWindow(GetDlgItem(hWnd,IDC_UNWRAP_WLABEL),SW_HIDE);
	}

	iStr = GetISpinner(GetDlgItem(hWnd,IDC_UNWRAP_STRSPIN));
	iStr->LinkToEdit(GetDlgItem(hWnd,IDC_UNWRAP_STR),EDITTYPE_FLOAT);
	iStr->SetLimits(0, 9999999, FALSE);
	iStr->SetAutoScale();	
	iStr->SetValue(falloffStr, FALSE);


	iTool = GetICustToolbar(GetDlgItem(hWnd,IDC_UNWARP_TOOLBAR));
	iTool->SetBottomBorder(TRUE);	
	iTool->SetImage(hToolImages);

	toolSize = 5;

	iTool->AddTool(ToolSeparatorItem(5));
	int toolCt = 0;

	if (showIconList[1])
	{
		iTool->AddTool(ToolButtonItem(CTB_CHECKBUTTON,0, 0, 1, 1, 16, 15, 23, 22, ID_TOOL_MOVE));
		toolSize += 23;
		toolCt++;
	}
	if (showIconList[2])
	{
		iTool->AddTool(ToolButtonItem(CTB_CHECKBUTTON,2, 2, 3, 3, 16, 15, 23, 22, ID_TOOL_ROTATE));
		toolSize += 23;
		toolCt++;
	}
	if (showIconList[3])
	{
		iTool->AddTool(ToolButtonItem(CTB_CHECKBUTTON,4, 4, 5, 5, 16, 15, 23, 22, ID_TOOL_SCALE));
		toolSize += 23;
		toolCt++;
	}
	if (showIconList[4])
	{
		iTool->AddTool(ToolButtonItem(CTB_CHECKBUTTON,38, 38, 39, 39, 16, 15, 23, 22, ID_FREEFORMMODE));
		toolSize += 23;
		toolCt++;
	}

	if (toolCt > 0)
	{
		iTool->AddTool(ToolSeparatorItem(5));
		toolSize += 5;
	}
	toolCt = 0;
	if (showIconList[5])
	{
		iTool->AddTool(ToolButtonItem(CTB_PUSHBUTTON,14, 14, 15, 15, 16, 15, 23, 22, ID_TOOL_MIRROR));
		toolSize += 23;
		toolCt++;
	}
	if (showIconList[6])
	{
		iTool->AddTool(	ToolButtonItem(CTB_PUSHBUTTON,18, 18, 19, 19, 16, 15, 23, 22, ID_TOOL_INCSELECTED));
		toolSize += 23;
		toolCt++;
	}
	if (showIconList[7])
	{
		iTool->AddTool(ToolButtonItem(CTB_PUSHBUTTON,20, 20, 21, 21, 16, 15, 23, 22, ID_TOOL_DECSELECTED));
		toolSize += 23;
		toolCt++;
	}
	if (toolCt > 0)
	{
		iTool->AddTool(ToolSeparatorItem(5));
		toolSize += 5;
	}
	toolCt = 0;
	if (showIconList[8])
	{
		iTool->AddTool(	ToolButtonItem(CTB_PUSHBUTTON,	24, 24, 25, 25, 16, 15, 23, 22, ID_TOOL_FALLOFF));
		toolSize += 23;
		toolCt++;
	}

	if (showIconList[9])
	{
		iTool->AddTool(ToolButtonItem(CTB_PUSHBUTTON,32, 32, 33, 33, 16, 15, 23, 22, ID_TOOL_FALLOFF_SPACE));
		toolSize += 23;
		toolCt++;
	}

	if (showIconList[10])
	{
		ShowWindow(GetDlgItem(hWnd,IDC_UNWRAP_STR),SW_SHOW);
		ShowWindow(GetDlgItem(hWnd,IDC_UNWRAP_STRSPIN),SW_SHOW);
	}
	else
	{
		ShowWindow(GetDlgItem(hWnd,IDC_UNWRAP_STR),SW_HIDE);
		ShowWindow(GetDlgItem(hWnd,IDC_UNWRAP_STRSPIN),SW_HIDE);
	}

	vertSize = 0;

	iVertex = GetICustToolbar(GetDlgItem(hWnd,IDC_UNWRAP_VERTS_TOOLBAR));
	iVertex->SetBottomBorder(TRUE);	
	iVertex->SetImage(hVertexImages);

	if ((showIconList[11]) || (showIconList[12]) || (showIconList[13]) )
	{
		iVertex->AddTool(ToolSeparatorItem(5));
		vertSize += 5;
	}

	if (showIconList[11])
	{
		iVertex->AddTool(ToolButtonItem(CTB_PUSHBUTTON,	0, 1, 1, 1, 16, 15, 23, 22, ID_TOOL_BREAK));
		vertSize += 23;
	}
	if (showIconList[12])
	{
		iVertex->AddTool(ToolButtonItem(CTB_CHECKBUTTON,2, 2, 3, 3, 16, 15, 23, 22, ID_TOOL_WELD));
		vertSize += 23;
	}
	if (showIconList[13])
	{
		iVertex->AddTool(ToolButtonItem(CTB_PUSHBUTTON, 4,  4,  5,  5, 16, 15, 23, 22, ID_TOOL_WELD_SEL));
		vertSize += 23;
	}


	iOption = GetICustToolbar(GetDlgItem(hWnd,IDC_UNWRAP_OPTION_TOOLBAR));
	iOption->SetBottomBorder(TRUE);	
	iOption->SetImage(hOptionImages);
	optionSize = 0;
	if ((showIconList[14]) || (showIconList[15]) || (showIconList[16]) || (showIconList[17]) )
	{
		iOption->AddTool(ToolSeparatorItem(5));
		optionSize += 5;
	}
	if (showIconList[14]) 
	{
		iOption->AddTool(ToolButtonItem(CTB_PUSHBUTTON,0, 0, 0, 0, 16, 15, 70, 22, ID_TOOL_UPDATE));
		optionSize += 70;
	}
	if (showIconList[15]) 
	{
		iOption->AddTool(ToolButtonItem(CTB_CHECKBUTTON,0, 0, 1, 1, 16, 15, 23, 22, ID_TOOL_SHOWMAP));
		optionSize += 23;
	}
	if (showIconList[16]) 
	{
		iOption->AddTool(ToolButtonItem(CTB_PUSHBUTTON,2, 2, 2, 2, 16, 15, 23, 22, ID_TOOL_UVW));
		optionSize += 23;
	}
	if (showIconList[17]) 
	{
		iOption->AddTool(ToolButtonItem(CTB_PUSHBUTTON,5, 5, 5, 5, 16, 15, 23, 22, ID_TOOL_PROP));
		optionSize += 23;
	}

	//	iOption->AddTool(ToolSeparatorItem(5));

	// get the objects materail texture list

	if (showIconList[18]) 
	{
		//5.1.01
		iOption->AddTool(ToolOtherItem(_T("combobox"), 170,	430, ID_TOOL_TEXTURE_COMBO,
			CBS, 2, NULL, 0));
		hTextures = iOption->GetItemHwnd(ID_TOOL_TEXTURE_COMBO);
		//	SendMessage(hTextures, WM_SETFONT, (WPARAM)GetAppHFont(), MAKELONG(0, 0));

		HFONT hFont;
		hFont = CreateFont(GetUIFontHeight(),0,0,0,FW_LIGHT,0,0,0,GetUIFontCharset(),0,0,0, VARIABLE_PITCH | FF_SWISS, _T(""));
		SendMessage(hTextures, WM_SETFONT, (WPARAM)hFont, MAKELONG(0, 0));
	}



	iUVWSpinBar = GetICustToolbar(GetDlgItem(hWnd,IDC_UNWRAP_ABSOLUTE_TOOLBAR));
	iUVWSpinBar->SetBottomBorder(FALSE);	
	iUVWSpinBar->SetImage(hVertexImages);
	iUVWSpinBar->AddTool(ToolSeparatorItem(5));
	iUVWSpinBar->AddTool(ToolButtonItem(CTB_CHECKBUTTON,26, 27, 26, 27, 16, 15, 23, 22, ID_ABSOLUTETYPEIN));
	iUVWSpinAbsoluteButton = iUVWSpinBar->GetICustButton(ID_ABSOLUTETYPEIN);
	if (fnGetRelativeTypeInMode())
		iUVWSpinAbsoluteButton->SetCheck(TRUE);
	else iUVWSpinAbsoluteButton->SetCheck(FALSE);



	iView = GetICustToolbar(GetDlgItem(hWnd,IDC_UNWARP_VIEW_TOOLBAR));
	iView->SetBottomBorder(FALSE);	
	iView->SetImage(hViewImages);
	iView->AddTool(ToolSeparatorItem(5));

	viewSize = 5;

	if (showIconList[27]) 
	{
		iView->AddTool(ToolButtonItem(CTB_CHECKBUTTON,0, 0, 1, 1, 16, 15, 23, 22, ID_TOOL_PAN));
		viewSize += 23;
	}

	if (showIconList[28]) 
	{
		iView->AddTool(ToolButtonItem(CTB_CHECKBUTTON,2, 2, 3, 3, 16, 15, 23, 22, ID_TOOL_ZOOM));
		viewSize += 23;
	}	
	if (showIconList[29]) 
	{
		iView->AddTool(ToolButtonItem(CTB_CHECKBUTTON, 4,  4,  5,  5, 16, 15, 23, 22, ID_TOOL_ZOOMREG));
		viewSize += 23;
	}	
	if (showIconList[30]) 
	{
		iView->AddTool(ToolButtonItem(CTB_PUSHBUTTON, 6,  6,  7,  7, 16, 15, 23, 22, ID_TOOL_ZOOMEXT));
		viewSize += 23;
	}
	if (showIconList[31]) 
	{
		iView->AddTool(ToolButtonItem(CTB_CHECKBUTTON, 10, 10,  10,  10, 16, 15, 23, 22, ID_TOOL_SNAP));
		viewSize += 23;
	}

	/*View->AddTool(
	ToolButtonItem(CTB_CHECKBUTTON,
	11, 11,  11,  11, 16, 15, 23, 22, ID_TOOL_LOCKSELECTED));
	*/


	//	iVertex->AddTool(ToolSeparatorItem(10));

	iFilter = GetICustToolbar(GetDlgItem(hWnd,IDC_UNWRAP_FILTER_TOOLBAR));
	iFilter->SetBottomBorder(FALSE);	
	iFilter->SetImage(hVertexImages);
	iFilter->AddTool(ToolSeparatorItem(5));

	filterSize = 5;
	int filterCount = 0;
	if (showIconList[22]) 
	{
		iFilter->AddTool(ToolButtonItem(CTB_CHECKBUTTON, 14, 16,  15,  17, 16, 15, 23, 22, ID_TOOL_LOCKSELECTED));
		filterSize += 23;
		filterCount++;
	}

	if (showIconList[23]) 
	{
		iFilter->AddTool(ToolButtonItem(CTB_PUSHBUTTON, 6,  6,  7,  7, 16, 15, 23, 22, ID_TOOL_HIDE));
		filterSize += 23;
		filterCount++;
	}

	if (showIconList[24]) 
	{
		iFilter->AddTool(ToolButtonItem(CTB_PUSHBUTTON, 10,  10,  11,  11, 16, 15, 23, 22, ID_TOOL_FREEZE));
		filterSize += 23;
		filterCount++;
	}

	if (showIconList[25]) 
	{
		iFilter->AddTool(ToolButtonItem(CTB_CHECKBUTTON,18, 19, 26, 26, 16, 15, 23, 22, ID_TOOL_FILTER_SELECTEDFACES));
		filterSize += 23;
		filterCount++;
	}

	if ( showIconList[26] && (filterCount >0))
	{
		iFilter->AddTool(ToolSeparatorItem(10));
		filterSize += 10;
	}	

	if (showIconList[26])
	{
		WORD LanguageID = PRIMARYLANGID(MaxSDK::Util::GetLanguageID());

		switch (LanguageID)
		{
			case LANG_JAPANESE:
				// Japanese
				iFilter->AddTool(ToolOtherItem(_T("combobox"), 100,	280, ID_TOOL_FILTER_MATID,
					CBS, 2, NULL, 0));
				break;

			case LANG_CHINESE:
				// Chinese
				iFilter->AddTool(ToolOtherItem(_T("combobox"), 130,	280, ID_TOOL_FILTER_MATID,
					CBS, 2, NULL, 0));
				break;

			case LANG_KOREAN:
				// Korean
				iFilter->AddTool(ToolOtherItem(_T("combobox"), 130,	280, ID_TOOL_FILTER_MATID,
					CBS, 2, NULL, 0));
				break;

			default:
				// English, French, and German
				iFilter->AddTool(ToolOtherItem(_T("combobox"), 140,	280, ID_TOOL_FILTER_MATID,
					CBS, 2, NULL, 0));
		}

		hMatIDs = iFilter->GetItemHwnd(ID_TOOL_FILTER_MATID);
		//	SendMessage(hTextures, WM_SETFONT, (WPARAM)GetAppHFont(), MAKELONG(0, 0));
		//FIX THIS make res id
		//5.1.01
		{
			HFONT hFont;			// Add for Japanese version
			hFont = CreateFont(GetUIFontHeight(),0,0,0,FW_LIGHT,0,0,0,GetUIFontCharset(),0,0,0, VARIABLE_PITCH | FF_SWISS, _T(""));
			SendMessage(hMatIDs, WM_SETFONT, (WPARAM)hFont, MAKELONG(0, 0));
		}
		SendMessage(hMatIDs, CB_ADDSTRING, 0, (LPARAM)_T(GetString(IDS_PW_ID_ALLID)));	
		SendMessage(hMatIDs,CB_SETCURSEL, (WPARAM)0, (LPARAM)0 );
		filterSize += 140;
	}



	iWeld    = iVertex->GetICustButton(ID_TOOL_WELD);
	iLockSelected = iFilter->GetICustButton(ID_TOOL_LOCKSELECTED);
	iHide	 = iFilter->GetICustButton(ID_TOOL_HIDE);
	iFreeze	 = iFilter->GetICustButton(ID_TOOL_FREEZE);
	iFilterSelected = iFilter->GetICustButton(ID_TOOL_FILTER_SELECTEDFACES);

	iMove    = iTool->GetICustButton(ID_TOOL_MOVE);
	iRot     = iTool->GetICustButton(ID_TOOL_ROTATE);
	iScale   = iTool->GetICustButton(ID_TOOL_SCALE);	
	iMirror   = iTool->GetICustButton(ID_TOOL_MIRROR);	
	iIncSelected = iTool->GetICustButton(ID_TOOL_INCSELECTED);	
	iDecSelected = iTool->GetICustButton(ID_TOOL_DECSELECTED);	
	iFalloff = iTool->GetICustButton(ID_TOOL_FALLOFF);	
	iFalloffSpace = iTool->GetICustButton(ID_TOOL_FALLOFF_SPACE);

	iPan     = iView->GetICustButton(ID_TOOL_PAN);	
	iZoom    = iView->GetICustButton(ID_TOOL_ZOOM);
	iZoomExt = iView->GetICustButton(ID_TOOL_ZOOMEXT);
	iZoomReg = iView->GetICustButton(ID_TOOL_ZOOMREG);
	iSnap = iView->GetICustButton(ID_TOOL_SNAP);

	iBreak = iVertex->GetICustButton(ID_TOOL_BREAK);
	iWeldSelected = iVertex->GetICustButton(ID_TOOL_WELD_SEL);

	iFreeForm    = iTool->GetICustButton(ID_FREEFORMMODE);


	iUpdate  = iOption->GetICustButton(ID_TOOL_UPDATE);
	iUVW	 = iOption->GetICustButton(ID_TOOL_UVW);
	iProp	 = iOption->GetICustButton(ID_TOOL_PROP);
	iShowMap = iOption->GetICustButton(ID_TOOL_SHOWMAP);

	if (iMove) iMove->SetTooltip(TRUE,GetString(IDS_RB_MOVE));
	if (iRot) iRot->SetTooltip(TRUE,GetString(IDS_RB_ROTATE));
	if (iScale) iScale->SetTooltip(TRUE,GetString(IDS_RB_SCALE));
	if (iPan) iPan->SetTooltip(TRUE,GetString(IDS_RB_PAN));
	if (iZoom) iZoom->SetTooltip(TRUE,GetString(IDS_RB_ZOOM));
	if (iUpdate) iUpdate->SetTooltip(TRUE,GetString(IDS_RB_UPDATE));
	if (iZoomExt) iZoomExt->SetTooltip(TRUE,GetString(IDS_RB_ZOOMEXT));
	if (iZoomReg) iZoomReg->SetTooltip(TRUE,GetString(IDS_RB_ZOOMREG));
	if (iUVW) iUVW->SetTooltip(TRUE,GetString(IDS_RB_UVW));
	if (iProp) iProp->SetTooltip(TRUE,GetString(IDS_RB_PROP));
	if (iShowMap) iShowMap->SetTooltip(TRUE,GetString(IDS_RB_SHOWMAP));
	if (iSnap) iSnap->SetTooltip(TRUE,GetString(IDS_PW_SNAP));
	if (iWeld) iWeld->SetTooltip(TRUE,GetString(IDS_PW_WELD));

	if (iWeldSelected) iWeldSelected->SetTooltip(TRUE,GetString(IDS_PW_WELDSELECTED));
	if (iBreak) iBreak->SetTooltip(TRUE,GetString(IDS_PW_BREAK));

	if (iFreeForm) iFreeForm->SetTooltip(TRUE,GetString(IDS_PW_FREEFORMMODE));


	if (iMirror) iMirror->SetTooltip(TRUE,GetString(IDS_PW_MIRRORH));
	if (iIncSelected) iIncSelected->SetTooltip(TRUE,GetString(IDS_PW_EXPANDSELECTION));
	if (iDecSelected) iDecSelected->SetTooltip(TRUE,GetString(IDS_PW_CONTRACTSELECTION));
	if (iFalloff) iFalloff->SetTooltip(TRUE,GetString(IDS_PW_FALLOFF)); 
	if (iFalloffSpace) iFalloffSpace->SetTooltip(TRUE,GetString(IDS_PW_FALLOFFSPACE)); 
	//need break tool tip	iBreak->SetTooltip(TRUE,GetString(IDS_PW_BREAK)); 
	//need weld selecetd	iBreak->SetTooltip(TRUE,GetString(IDS_PW_BREAK)); 
	if (iLockSelected) iLockSelected->SetTooltip(TRUE,GetString(IDS_PW_LOCKSELECTED));
	if (iHide) iHide->SetTooltip(TRUE,GetString(IDS_PW_HIDE));
	if (iFreeze) iFreeze->SetTooltip(TRUE,GetString(IDS_PW_FREEZE));
	if (iFilterSelected) iFilterSelected->SetTooltip(TRUE,GetString(IDS_PW_FACEFILTER));

	if (iMove) iMove->SetHighlightColor(GREEN_WASH);
	if (iRot) iRot->SetHighlightColor(GREEN_WASH);
	if (iScale) iScale->SetHighlightColor(GREEN_WASH);
	if (iWeld) iWeld->SetHighlightColor(GREEN_WASH);
	if (iPan) iPan->SetHighlightColor(GREEN_WASH);
	if (iZoom) iZoom->SetHighlightColor(GREEN_WASH);	
	if (iZoomReg) iZoomReg->SetHighlightColor(GREEN_WASH);	
	if (iMirror) iMirror->SetHighlightColor(GREEN_WASH);	
	if (iFalloff) iFalloff->SetHighlightColor(GREEN_WASH);	
	if (iFalloffSpace) iFalloffSpace->SetHighlightColor(GREEN_WASH);	

	if (iFreeForm) iFreeForm->SetHighlightColor(GREEN_WASH);

	if (iUpdate) iUpdate->SetImage(NULL,0,0,0,0,0,0);
	if (iUpdate) iUpdate->SetText(GetString(IDS_RB_UPDATE));

	FlyOffData fdata1[] = 
	{
		{ 2, 2 ,  2,  2},
		{ 3,  3,  3,  3},
		{ 4,  4,  4,  4}
	};
	if (iUVW) iUVW->SetFlyOff(3,fdata1,GetCOREInterface()->GetFlyOffTime(),uvw,FLY_DOWN);

	FlyOffData fdata2[] = 
	{
		{ 0,  0,  1,  1},
		{10, 10, 11, 11},
		{12, 12, 13, 13}
	};
	if (iMove) iMove->SetFlyOff(3,fdata2,GetCOREInterface()->GetFlyOffTime(),move,FLY_DOWN);

	FlyOffData fdata2a[] = 
	{
		{24, 24, 25, 25},
		{26, 26, 27, 27},
		{28, 28, 29, 29},
		{30, 30, 31, 31}
	};
	if (iFalloff) iFalloff->SetFlyOff(4,fdata2a,GetCOREInterface()->GetFlyOffTime(),falloff,FLY_DOWN);


	FlyOffData fdata2b[] = 
	{
		{32, 32, 32, 32},
		{33, 33, 33, 33},
	};
	if (iFalloffSpace) iFalloffSpace->SetFlyOff(2,fdata2b,GetCOREInterface()->GetFlyOffTime(),falloffSpace,FLY_DOWN);



	FlyOffData fdata3[] = 
	{
		{ 4,  4,  5,  5},
		{ 6,  6,  7,  7},
		{ 8,  8,  9,  9}
	};
	if (iScale) iScale->SetFlyOff(3,fdata3,GetCOREInterface()->GetFlyOffTime(),scale,FLY_DOWN);

	FlyOffData fdata5[] = 
	{
		{ 14,  14,  15,  15},
		{ 16,  16,  17,  17},
		{ 34,  34,  35,  35},
		{ 36,  36,  37,  37}
	};
	if (iMirror) iMirror->SetFlyOff(4,fdata5,GetCOREInterface()->GetFlyOffTime(),mirror,FLY_DOWN);

	FlyOffData fdata4[] = 
	{
		{ 6,  6,  7,  7},
		{ 8,  8,  9,  9},
		{ 30,  30,  31,  31}
	};
	if (iZoomExt) iZoomExt->SetFlyOff(3,fdata4,GetCOREInterface()->GetFlyOffTime(),zoomext,FLY_UP);

	FlyOffData fdata6[] = 
	{
		{ 6,  6,  7,  7},
		{ 8,  8,  9,  9}
	};
	if (iHide) iHide->SetFlyOff(2,fdata6,GetCOREInterface()->GetFlyOffTime(),hide,FLY_UP);

	FlyOffData fdata7[] = {
		{ 10,  11,  11,  11},
		{ 12,  13,  13,  13}
	};
	if (iFreeze) iFreeze->SetFlyOff(2,fdata7,GetCOREInterface()->GetFlyOffTime(),hide,FLY_UP);


	FlyOffData fdata8[] = 
	{
		{ 11,  11,  11,  11},
		{ 10,  10,  10,  10}
	};
	if (iSnap) iSnap->SetFlyOff(2,fdata8,GetCOREInterface()->GetFlyOffTime(),hide,FLY_UP);

	if (iShowMap)
	{
		iShowMap->SetCheck(showMap);
		if (image) iShowMap->Enable();
		else	iShowMap->Disable();
	}


	if (iSnap) 
	{
		if (pixelSnap)
		{
			iSnap->SetCurFlyOff(1);
			iSnap->SetCheck(TRUE);
		}
		if (gridSnap)
		{
			iSnap->SetCurFlyOff(0);
			iSnap->SetCheck(TRUE);
		}
		else iSnap->SetCheck(FALSE);
	}

	if (iFilterSelected) 
		iFilterSelected->SetCheck(filterSelectedFaces);
	BuildFilterSelectedFacesData();

	if (iLockSelected) iLockSelected->SetCheck(lockSelected);

	SizeDlg();
	SetMode(mode,FALSE);

	//5.1.02 adds new bitmap bg management
	//							matPairList.ZeroCount();

	

	Tab<int> matIDs;
	Tab<Mtl*> matList;
	MultiMtl* mtl = NULL;
	matid = -1;
	BuildMatIDList();
	SetMatFilters();

	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		INode *node = mMeshTopoData.GetNode(ldID);
		Mtl *baseMtl = node->GetMtl();//GetBaseMtl();
		if (baseMtl)
		{
			mtl = (MultiMtl*) GetMultiMaterials(baseMtl);
			if (mtl)
			{
				IParamBlock2 *pb = mtl->GetParamBlock(0);
				if (pb)
				{	

					int numMaterials = pb->Count(multi_mtls);
//					matIDs.SetCount(numMaterials);
//					matList.SetCount(numMaterials);
					for (int i = 0; i < numMaterials; i++)
					{
						int id;
						Mtl *mat;
						pb->GetValue(multi_mtls,0,mat,FOREVER,i);
						pb->GetValue(multi_ids,0,id,FOREVER,i);

						BOOL dupe = FALSE;
						for (int m = 0; m < matIDs.Count(); m++)
						{
							if (matIDs[m] == id)
								dupe = TRUE;
						}
						if (!dupe)
						{
							matIDs.Append(1,&id,100);
							matList.Append(1,&mat,100);
						}
//						matIDs[i] = id;											
//						matList[i] = mat;																						
					}
				}
			}
		}
	}


	for (int i = 0; i<filterMatID.Count();i++)
	{
		char st[200];
		if (mtl) 
		{
			int id = filterMatID[i];

			int matchMat = -1;
			for (int j = 0; j < matIDs.Count(); j++)
			{
				if (id == matIDs[j])
					matchMat = j;
			}
			if ((matchMat == -1) || (matList[matchMat] == NULL))
				sprintf(st,"%d",filterMatID[i]+1);
			else sprintf(st,"%d:%s",filterMatID[i]+1,matList[matchMat]->GetFullName());

		}
		else sprintf(st,"%d",filterMatID[i]+1);
		SendMessage(hMatIDs, CB_ADDSTRING , 0, (LPARAM) (TCHAR*) st);
	}


	
	SendMessage(hMatIDs, CB_SETCURSEL, matid+1, 0 );



	if ((!GetCOREInterface()->IsSubObjectSelectionEnabled())&& (iFilterSelected))
		iFilterSelected->Enable(FALSE);

	if (pblock->Count(unwrap_texmaplist) == 0)
		ResetMaterialList();

}

static void SetWindowYPos(HWND hWnd,int y)
{
	Rect rect;
	GetClientRectP(hWnd,&rect);
	SetWindowPos(hWnd,NULL,rect.left,y,0,0,SWP_NOSIZE|SWP_NOZORDER);
}

static void SetWindowXPos(HWND hWnd,int x)
{
	Rect rect;
	GetClientRectP(hWnd,&rect);
	SetWindowPos(hWnd,NULL,x,rect.top,0,0,SWP_NOSIZE|SWP_NOZORDER);
}

void UnwrapMod::SizeDlg()
{
	Rect rect;
	GetClientRect(hWnd,&rect);
	MoveWindow(GetDlgItem(hWnd,IDC_UNWARP_TOOLBAR),
		0, 0, toolSize, TOOL_HEIGHT, TRUE);

	SetWindowXPos(GetDlgItem(hWnd,IDC_UNWRAP_STR),toolSize);
	SetWindowYPos(GetDlgItem(hWnd,IDC_UNWRAP_STR),6);
	SetWindowXPos(GetDlgItem(hWnd,IDC_UNWRAP_STRSPIN),toolSize+32);
	SetWindowYPos(GetDlgItem(hWnd,IDC_UNWRAP_STRSPIN),6);

	MoveWindow(GetDlgItem(hWnd,IDC_UNWRAP_VERTS_TOOLBAR),
		toolSize+56, 0, vertSize, TOOL_HEIGHT, TRUE);


	MoveWindow(GetDlgItem(hWnd,IDC_UNWRAP_OPTION_TOOLBAR),
		310+64, 0, 320, TOOL_HEIGHT, TRUE);


	/*
	MoveWindow(GetDlgItem(hWnd,IDC_UNWRAP_TEXTURE_COMBO),
	480, 4, 100, TOOL_HEIGHT, TRUE);
	MoveWindow(GetDlgItem(hWnd,IDC_MATID_STRING),
	582, 0, 36, TOOL_HEIGHT-4, TRUE);
	MoveWindow(GetDlgItem(hWnd,IDC_MATID_COMBO),
	620, 4, 50, TOOL_HEIGHT, TRUE);
	*/

	MoveWindow(GetDlgItem(hWnd,IDC_UNWRAP_VIEW),
		2, TOOL_HEIGHT, rect.w()-5, rect.h()-TOOL_HEIGHT-SPINNER_HEIGHT-3,FALSE);


	int ys = rect.h()-TOOL_HEIGHT+3;
	int yl = rect.h()-TOOL_HEIGHT+5;


	SetWindowYPos(GetDlgItem(hWnd,IDC_UNWRAP_ULABEL),yl);
	SetWindowYPos(GetDlgItem(hWnd,IDC_UNWRAP_VLABEL),yl);
	SetWindowYPos(GetDlgItem(hWnd,IDC_UNWRAP_WLABEL),yl);

	SetWindowYPos(GetDlgItem(hWnd,IDC_UNWRAP_U),ys);
	SetWindowYPos(GetDlgItem(hWnd,IDC_UNWRAP_V),ys);
	SetWindowYPos(GetDlgItem(hWnd,IDC_UNWRAP_W),ys);

	SetWindowYPos(GetDlgItem(hWnd,IDC_UNWRAP_USPIN),ys);
	SetWindowYPos(GetDlgItem(hWnd,IDC_UNWRAP_VSPIN),ys);
	SetWindowYPos(GetDlgItem(hWnd,IDC_UNWRAP_WSPIN),ys);

	SetWindowYPos(GetDlgItem(hWnd,IDC_UNWRAP_ABSOLUTE_TOOLBAR),ys);

	{
		int w = rect.w() - filterSize - viewSize - 4;
		if (w < 220+12)
			w = 220+12;

		MoveWindow(GetDlgItem(hWnd,IDC_UNWRAP_FILTER_TOOLBAR),
			w, ys-5, filterSize, TOOL_HEIGHT, TRUE);

		MoveWindow(GetDlgItem(hWnd,IDC_UNWARP_VIEW_TOOLBAR),
			w + filterSize, ys-5, viewSize, TOOL_HEIGHT, TRUE);

	}

	InvalidateRect(hWnd,NULL,TRUE);
	InvalidateRect(GetDlgItem(hWnd,IDC_UNWRAP_FILTER_TOOLBAR),NULL,FALSE);
	InvalidateRect(GetDlgItem(hWnd,IDC_UNWARP_VIEW_TOOLBAR),NULL,FALSE);

}

Point2 UnwrapMod::UVWToScreen(Point3 pt,float xzoom, float yzoom,int w,int h)
{	
	int i1, i2;
	GetUVWIndices(i1,i2);
	int tx = (w-int(xzoom))/2;
	int ty = (h-int(yzoom))/2;
	return Point2(pt[i1]*xzoom+xscroll+tx, (float(h)-pt[i2]*yzoom)+yscroll-ty);
}

void UnwrapMod::ComputeZooms(
							 HWND hWnd, float &xzoom, float &yzoom,int &width, int &height)
{
	Rect rect;
	GetClientRect(hWnd,&rect);	
	width = rect.w()-1;
	height = rect.h()-1;

	if (zoom < 0.000001f) zoom = 0.000001f;
	if (zoom > 100000.0f) zoom = 100000.0f;

	if (lockAspect)
		xzoom = zoom*aspect*float(height);
	else xzoom = zoom*aspect*float(width);
	yzoom = zoom*float(height);

}


void UnwrapMod::SetMatFilters()
{
	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		int id = -1;
		if ((matid >= 0) && matid < filterMatID.Count())
			id = filterMatID[matid];
		mMeshTopoData[ldID]->BuilMatIDFilter(id);
	}
}


void UnwrapMod::InvalidateView()
{
	InvalidateTypeins();
	viewValid = FALSE;
	if (hView) {
		InvalidateRect(hView,NULL,TRUE);
	}
}

void UnwrapMod::SetMode(int m, BOOL updateMenuBar)
{
	BOOL invalidView = FALSE;
	if ((mode == ID_SKETCHMODE) && (m != ID_SKETCHMODE))
	{

		if (theHold.Holding())
			theHold.Accept(_T(GetString(IDS_PW_MOVE_UVW)));		

		invalidView = TRUE;
		sketchSelMode = restoreSketchSelMode;
		sketchType = restoreSketchType;
		sketchDisplayPoints = restoreSketchDisplayPoints;
		sketchInteractiveMode = restoreSketchInteractiveMode;

		sketchCursorSize = restoreSketchCursorSize; 


	}

	switch (mode) {
case ID_FREEFORMMODE:
	if (iFreeForm) iFreeForm->SetCheck(FALSE);
	oldMode = mode;
	invalidView = TRUE;
	break;
case ID_TOOL_MOVE:
case ID_UNWRAP_MOVE:
	if (iMove) iMove->SetCheck(FALSE);   
	oldMode = mode;
	break;
case ID_UNWRAP_ROTATE:
case ID_TOOL_ROTATE:  
	if (iRot) iRot->SetCheck(FALSE);    
	oldMode = mode;
	break;
case ID_UNWRAP_SCALE:
case ID_TOOL_SCALE:   
	if (iScale) iScale->SetCheck(FALSE);  
	oldMode = mode;
	break;
case ID_UNWRAP_PAN:
case ID_TOOL_PAN:     if (iPan) iPan->SetCheck(FALSE);    break;
case ID_UNWRAP_ZOOM:
case ID_TOOL_ZOOM:    
	if (iZoom) iZoom->SetCheck(FALSE);   
	break;
case ID_UNWRAP_ZOOMREGION:
case ID_TOOL_ZOOMREG: 
	if (iZoomReg) iZoomReg->SetCheck(FALSE);break;
case ID_UNWRAP_WELD:
case ID_TOOL_WELD:	  
	if (iWeld) iWeld->SetCheck(FALSE);
	oldMode = mode;
	break;
case ID_TOOL_PELTSTRAIGHTEN:	
	peltData.peltDialog.SetStraightenButton(FALSE);
	//	peltData.peltDialog.SetStraightenMode(FALSE);
	oldMode = mode;
	break;
	}

	int prevMode = mode;
	mode = m;

	if (prevMode == ID_PAINTSELECTMODE)
		MoveScriptUI();

	switch (mode) {
case ID_PAINTSELECTMODE:  
	{
		TSTR mstr = GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap.setPaintSelectMode"));
		macroRecorder->FunctionCall(mstr, 1, 0,mr_bool,TRUE);
		macroRecorder->EmitScript();
		mouseMan.SetMouseProc(paintSelectMode, LEFT_BUTTON);
		break;
	}
case ID_SKETCHMODE:   
	{
		TSTR mstr = GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap.sketchNoParams"));
		macroRecorder->FunctionCall(mstr, 0, 0);
		macroRecorder->EmitScript();
		sketchMode->pointCount = 0;
		mouseMan.SetMouseProc(sketchMode, LEFT_BUTTON);
		break;
	}
case ID_FREEFORMMODE:   
	{
		TSTR mstr = GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap2.setFreeFormMode"));
		macroRecorder->FunctionCall(mstr, 1, 0,
			mr_bool,TRUE);
		macroRecorder->EmitScript();
		if (iFreeForm) iFreeForm->SetCheck(TRUE);  
		mouseMan.SetMouseProc(freeFormMode, LEFT_BUTTON);
		invalidView = TRUE;
		break;
	}
case ID_UNWRAP_MOVE:   
case ID_TOOL_MOVE:   
	{
		if (iMove)
		{
			if (iMove->GetCurFlyOff() == 0)
			{
				TSTR mstr = GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap.move"));
				macroRecorder->FunctionCall(mstr, 0, 0);
			}
			else if (iMove->GetCurFlyOff() == 1)
			{
				TSTR mstr = GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap.moveh"));
				macroRecorder->FunctionCall(mstr, 0, 0);
			}
			else if (iMove->GetCurFlyOff() == 2)
			{
				TSTR mstr = GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap.movev"));
				macroRecorder->FunctionCall(mstr, 0, 0);
			}
			macroRecorder->EmitScript();
			iMove->SetCheck(TRUE);  

		}
		mouseMan.SetMouseProc(moveMode, LEFT_BUTTON);
		break;
	}

case ID_UNWRAP_ROTATE:   
case ID_TOOL_ROTATE: 
	{
		TSTR mstr = GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap.rotate"));
		macroRecorder->FunctionCall(mstr, 0, 0);
		macroRecorder->EmitScript();

		if (iRot) iRot->SetCheck(TRUE);   
		mouseMan.SetMouseProc(rotMode, LEFT_BUTTON);
		break;
	}

case ID_UNWRAP_SCALE:   
case ID_TOOL_SCALE:  
	{
		if (iScale)
		{
			if (iScale->GetCurFlyOff() == 0)
			{
				TSTR mstr = GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap.scale"));
				macroRecorder->FunctionCall(mstr, 0, 0);
			}
			else if (iScale->GetCurFlyOff() == 1)
			{
				TSTR mstr = GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap.scaleh"));
				macroRecorder->FunctionCall(mstr, 0, 0);
			}
			else if (iScale->GetCurFlyOff() == 2)
			{
				TSTR mstr = GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap.scalev"));
				macroRecorder->FunctionCall(mstr, 0, 0);
			}
			macroRecorder->EmitScript();
			iScale->SetCheck(TRUE); 
		}

		mouseMan.SetMouseProc(scaleMode, LEFT_BUTTON);
		break;
	}
case ID_UNWRAP_WELD:
case ID_TOOL_WELD:   
	{
		if (fnGetTVSubMode() != TVFACEMODE)
		{
			TSTR mstr = GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap.weld"));
			macroRecorder->FunctionCall(mstr, 0, 0);
			macroRecorder->EmitScript();
			if (iWeld) iWeld->SetCheck(TRUE);  

			for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
			{
				MeshTopoData *ld = mMeshTopoData[ldID];
				if (fnGetTVSubMode() == TVVERTMODE)
					ld->ClearSelection(TVVERTMODE);//vsel.ClearAll();			
				else if (fnGetTVSubMode() == TVEDGEMODE)
					ld->ClearSelection(TVEDGEMODE);//esel.ClearAll();			
			}

			if (ip) ip->RedrawViews(ip->GetTime());

			mouseMan.SetMouseProc(weldMode, LEFT_BUTTON);
		}
		break;
	}

case ID_TOOL_PELTSTRAIGHTEN:	  
	{
		peltData.peltDialog.SetStraightenButton(TRUE);
		if (ip) ip->RedrawViews(ip->GetTime());
		mouseMan.SetMouseProc(peltStraightenMode, LEFT_BUTTON);
		break;
	}
case ID_UNWRAP_PAN:
case ID_TOOL_PAN:    
	{
		TSTR mstr = GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap.pan"));
		macroRecorder->FunctionCall(mstr, 0, 0);
		macroRecorder->EmitScript();

		if (iPan) iPan->SetCheck(TRUE);   
		mouseMan.SetMouseProc(panMode, LEFT_BUTTON);
		break;
	}

case ID_UNWRAP_ZOOM:
case ID_TOOL_ZOOM:   
	{
		TSTR mstr = GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap.zoom"));
		macroRecorder->FunctionCall(mstr, 0, 0);
		macroRecorder->EmitScript();

		if (iZoom) iZoom->SetCheck(TRUE);
		mouseMan.SetMouseProc(zoomMode, LEFT_BUTTON);
		break;
	}
case ID_UNWRAP_ZOOMREGION:
case ID_TOOL_ZOOMREG:
	{
		TSTR mstr = GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap.zoomRegion"));
		macroRecorder->FunctionCall(mstr, 0, 0);
		macroRecorder->EmitScript();

		if (iZoomReg) iZoomReg->SetCheck(TRUE);
		mouseMan.SetMouseProc(zoomRegMode, LEFT_BUTTON);
		break;
	}
}



	if (updateMenuBar)
	{
		if (hWnd)
		{
			IMenuBarContext* pContext = (IMenuBarContext*) GetCOREInterface()->GetMenuManager()->GetContext(kUnwrapMenuBar);
			if (pContext)
				pContext->UpdateWindowsMenu();
		}
	}
	if (invalidView) InvalidateView();

}

void UnwrapMod::RegisterClasses()
{
	if (!hToolImages) {
		HBITMAP hBitmap, hMask;	
		hToolImages = ImageList_Create(16, 15, TRUE, 4, 0);
		hBitmap = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_UNWRAP_TRANSFORM));
		hMask   = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_UNWRAP_TRANSFORM_MASK));
		ImageList_Add(hToolImages,hBitmap,hMask);
		DeleteObject(hBitmap);
		DeleteObject(hMask);
	}

	if (!hOptionImages) {
		HBITMAP hBitmap, hMask;	
		hOptionImages = ImageList_Create(16, 15, TRUE, 4, 0);
		hBitmap = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_UNWRAP_OPTION));
		hMask   = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_UNWRAP_OPTION_MASK));
		ImageList_Add(hOptionImages,hBitmap,hMask);
		DeleteObject(hBitmap);
		DeleteObject(hMask);
	}

	if (!hViewImages) {
		HBITMAP hBitmap, hMask;	
		hViewImages = ImageList_Create(16, 15, TRUE, 4, 0);
		hBitmap = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_UNWRAP_VIEW));
		hMask   = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_UNWRAP_VIEW_MASK));
		ImageList_Add(hViewImages,hBitmap,hMask);
		DeleteObject(hBitmap);
		DeleteObject(hMask);
	}

	if (!hVertexImages) {
		HBITMAP hBitmap, hMask;	
		hVertexImages = ImageList_Create(16, 15, TRUE, 4, 0);
		hBitmap = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_UNWRAP_VERT));
		hMask   = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_UNWRAP_VERT_MASK));
		ImageList_Add(hVertexImages,hBitmap,hMask);
		DeleteObject(hBitmap);
		DeleteObject(hMask);
	}

	static BOOL registered = FALSE;
	if (!registered) {
		registered = TRUE;
		WNDCLASS  wc;
		wc.style         = 0;
		wc.hInstance     = hInstance;
		wc.hIcon         = NULL;
		wc.hCursor       = NULL;
		wc.hbrBackground = NULL; //(HBRUSH)GetStockObject(WHITE_BRUSH);	
		wc.lpszMenuName  = NULL;
		wc.cbClsExtra    = 0;
		wc.cbWndExtra    = 0;
		wc.lpfnWndProc   = UnwrapViewProc;
		wc.lpszClassName = _T(GetString(IDS_PW_UNWRAPVIEW));
		RegisterClass(&wc);

	}

}

//static int lStart[12] = {0,1,3,2,4,5,7,6,0,1,2,3};
//static int lEnd[12]   = {1,3,2,0,5,7,6,4,4,5,6,7};
/*
static void DoBoxIcon(BOOL sel,float length, PolyLineProc& lp)
{
Point3 pt[3];

length *= 0.5f;
Box3 box;
box.pmin = Point3(-length,-length,-length);
box.pmax = Point3( length, length, length);

if (sel) //lp.SetLineColor(1.0f,1.0f,0.0f);
lp.SetLineColor(GetUIColor(COLOR_SEL_GIZMOS));
else //lp.SetLineColor(0.85f,0.5f,0.0f);		
lp.SetLineColor(GetUIColor(COLOR_GIZMOS));

for (int i=0; i<12; i++) {
pt[0] = box[lStart[i]];
pt[1] = box[lEnd[i]];
lp.proc(pt,2);
}
}

*/

/*************************************************************

Modified from Graphics Gem 3 Fast Liner intersection by Franklin Antonio

**************************************************************/



BOOL UnwrapMod::LineIntersect(Point3 p1, Point3 p2, Point3 q1, Point3 q2)
{


	float a, b, c, d, det;  /* parameter calculation variables */
	float max1, max2, min1, min2; /* bounding box check variables */

	/*  First make the bounding box test. */
	max1 = maxmin(p1.x, p2.x, min1);
	max2 = maxmin(q1.x, q2.x, min2);
	if((max1 < min2) || (min1 > max2)) return(FALSE); /* no intersection */
	max1 = maxmin(p1.y, p2.y, min1);
	max2 = maxmin(q1.y, q2.y, min2);
	if((max1 < min2) || (min1 > max2)) return(FALSE); /* no intersection */

	/* See if the endpoints of the second segment lie on the opposite
	sides of the first.  If not, return 0. */
	a = (q1.x - p1.x) * (p2.y - p1.y) -
		(q1.y - p1.y) * (p2.x - p1.x);
	b = (q2.x - p1.x) * (p2.y - p1.y) -
		(q2.y - p1.y) * (p2.x - p1.x);
	if(a!=0.0f && b!=0.0f && SAME_SIGNS(a, b)) return(FALSE);

	/* See if the endpoints of the first segment lie on the opposite
	sides of the second.  If not, return 0.  */
	c = (p1.x - q1.x) * (q2.y - q1.y) -
		(p1.y - q1.y) * (q2.x - q1.x);
	d = (p2.x - q1.x) * (q2.y - q1.y) -
		(p2.y - q1.y) * (q2.x - q1.x);
	if(c!=0.0f && d!=0.0f && SAME_SIGNS(c, d) ) return(FALSE);

	/* At this point each segment meets the line of the other. */
	det = a - b;
	if(det == 0.0f) return(FALSE); /* The segments are colinear.  Determining */
	return(TRUE);
}
/*
int UnwrapMod::PolyIntersect(Point3 p1, int i1, int i2, BitArray *ignoredFaces)
{

	static int startFace = 0;
	int ct = 0;
	Point3 p2 = p1;
	float x = FLT_MIN;
	for (int i =0; i < TVMaps.v.Count(); i++)
	{
		if (!(TVMaps.v[i].flags & FLAG_DEAD))
		{
			float tx = TVMaps.v[i].p[i1];
			if (tx > x) x = tx;
		}
	}
	p2.x = x+10.0f;

	if (startFace >= TVMaps.f.Count()) startFace = 0;


	while (ct != TVMaps.f.Count())
	{
		int pcount = TVMaps.f[startFace]->count;
		int hit = 0;
		BOOL bail = FALSE;
		if (ignoredFaces)
		{
			if ((*ignoredFaces)[startFace])
				bail = TRUE;
		}
		if (IsFaceVisible(startFace) && (!bail))
		{
			int frozen = 0, hidden = 0;
			for (int j=0; j<pcount; j++) 
			{
				int index = TVMaps.f[startFace]->t[j];
				if (TVMaps.v[index].flags & FLAG_HIDDEN) hidden++;
				if (TVMaps.v[index].flags & FLAG_FROZEN) frozen++;
			}
			if ((frozen == pcount) || (hidden == pcount))
			{
			}	
			else if ( (objType == IS_PATCH) && (!(TVMaps.f[startFace]->flags & FLAG_CURVEDMAPPING)) && (TVMaps.f[startFace]->vecs))
			{
				Spline3D spl;
				spl.NewSpline();
				int i = startFace;
				for (int j=0; j<pcount; j++) 
				{
					Point3 in, p, out;
					int index = TVMaps.f[i]->t[j];
					p = GetPoint(currentTime,index);

					index = TVMaps.f[i]->vecs->handles[j*2];
					out = GetPoint(currentTime,index);
					if (j==0)
						index = TVMaps.f[i]->vecs->handles[pcount*2-1];
					else index = TVMaps.f[i]->vecs->handles[j*2-1];

					in = GetPoint(currentTime,index);

					SplineKnot kn(KTYPE_BEZIER_CORNER, LTYPE_CURVE, p, in, out);
					spl.AddKnot(kn);

					spl.SetClosed();
					spl.ComputeBezPoints();
				}
				//draw curves
				Point3 ptList[7*4];
				int ct = 0;
				for (int j=0; j<pcount; j++) 
				{
					int jNext = j+1;
					if (jNext >= pcount) jNext = 0;
					Point3 p;
					int index = TVMaps.f[i]->t[j];
					if (j==0)
						ptList[ct++] = GetPoint(currentTime,index);

					for (int iu = 1; iu <= 5; iu++)
					{
						float u = (float) iu/5.f;
						ptList[ct++] = spl.InterpBezier3D(j, u);

					}



				}
				for (int j=0; j < ct; j++) 
				{
					int index;
					if (j == (ct-1))
						index = 0;
					else index = j+1;
					Point3 a(0.0f,0.0f,0.0f),b(0.0f,0.0f,0.0f);
					a.x = ptList[j][i1];
					a.y = ptList[j][i2];

					b.x = ptList[index][i1];
					b.y = ptList[index][i2];

					if (LineIntersect(p1, p2, a,b)) hit++;
					//					if (LineIntersect(p1, p2, ptList[j],ptList[index])) hit++;
				}


			}
			else
			{
				for (int i = 0; i < pcount; i++)
				{
					int faceIndexA;  
					faceIndexA = TVMaps.f[startFace]->t[i];
					int faceIndexB;
					if (i == (pcount-1))
						faceIndexB  = TVMaps.f[startFace]->t[0];
					else faceIndexB  = TVMaps.f[startFace]->t[i+1];
					Point3 a(0.0f,0.0f,0.0f),b(0.0f,0.0f,0.0f);
					a.x = TVMaps.v[faceIndexA].p[i1];
					a.y = TVMaps.v[faceIndexA].p[i2];
					b.x = TVMaps.v[faceIndexB].p[i1];
					b.y = TVMaps.v[faceIndexB].p[i2];
					if (LineIntersect(p1, p2, a,b)) 
						hit++;
				}
			}


			if ((hit%2) == 1) 
				return startFace;
		}
		startFace++;
		if (startFace >= TVMaps.f.Count()) startFace = 0;
		ct++;
	}

	return -1;
}
*/

BOOL UnwrapMod::HitTest(Rect rect,Tab<TVHitData> &hits,BOOL selOnly,BOOL circleMode)
{
	//we dont let hit testing happen during the inertactive mapping so they cannot change there selection
	if( (fnGetMapMode() == PLANARMAP) || (fnGetMapMode() == CYLINDRICALMAP) || (fnGetMapMode() == SPHERICALMAP) || (fnGetMapMode() == BOXMAP))
		return FALSE;

	//if you are in pelt mode but dont have the dialog up also prevent the selection
	if( (fnGetMapMode() == PELTMAP) && (peltData.peltDialog.hWnd == NULL))
		return FALSE;

	Point2 pt;
	float xzoom, yzoom;
	int width,height;
	ComputeZooms(hView,xzoom,yzoom,width,height);


	Point2 center = UVWToScreen(Point3(0.0f,0.0f,0.0f),xzoom,yzoom,width,height);
	Point3 p1(10000.f,10000.f,10000.f), p2(-10000.f,-10000.f,-10000.f);

	Rect smallRect = rect;

	if (circleMode)
	{

	}
	else
	{
		//just builds a quick vertex bounding box
		if (fnGetTVSubMode() == TVVERTMODE)
		{
			if ( (abs(rect.left-rect.right) <= 4) && (abs(rect.bottom-rect.top) <= 4) )
			{
				rect.left -= hitSize;
				rect.right += hitSize;
				rect.top -= hitSize;
				rect.bottom += hitSize;

				smallRect.left -= 1;
				smallRect.right += 1;
				smallRect.top -= 1;
				smallRect.bottom += 1;
			}
		}
	}

	Rect rectFF = rect;
	if ( (abs(rect.left-rect.right) <= 4) && (abs(rect.bottom-rect.top) <= 4) )
	{
		rectFF.left -= hitSize;
		rectFF.right += hitSize;
		rectFF.top -= hitSize;
		rectFF.bottom += hitSize;
	}


	int i1,i2;
	GetUVWIndices(i1,i2);

	p1[i1] = ((float)rectFF.left-center.x)/xzoom;
	p2[i1] = ((float)rectFF.right-center.x)/xzoom;

	p1[i2] = -((float)rectFF.top-center.y)/yzoom;
	p2[i2] = -((float)rectFF.bottom-center.y)/yzoom;

	Box3 boundsFF;
	boundsFF.Init();
	boundsFF += p1;
	boundsFF += p2;

	p1[i1] = ((float)rect.left-center.x)/xzoom;
	p2[i1] = ((float)rect.right-center.x)/xzoom;

	p1[i2] = -((float)rect.top-center.y)/yzoom;
	p2[i2] = -((float)rect.bottom-center.y)/yzoom;

	Box3 bounds;
	bounds.Init();
	bounds += p1;
	bounds += p2;




	//compute a screen rect bounding box which we use to do the mouse pick
	Point3 smallp1(10000.f,10000.f,10000.f), smallp2(-10000.f,-10000.f,-10000.f);
	smallp1[i1] = ((float)smallRect.left-center.x)/xzoom;
	smallp2[i1] = ((float)smallRect.right-center.x)/xzoom;

	smallp1[i2] = -((float)smallRect.top-center.y)/yzoom;
	smallp2[i2] = -((float)smallRect.bottom-center.y)/yzoom;

	Box3 smallBounds;
	smallBounds.Init();
	smallBounds += smallp1;
	smallBounds += smallp2;


	mMouseHitLocalData = NULL;

	
	// if we are doing a grid snap we need to find the anchor vertex which is the first vertex
	// that use clicked on
	if (gridSnap)
	{
		//loop through all our local data
		for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
		{
			MeshTopoData *ld = mMeshTopoData[ldID];
			if (ld)
			{
				//check to see if we hit a vertex
				for (int i=0; i<ld->GetNumberTVVerts(); i++) 
				{
					if (selOnly && !ld->GetTVVertSelected(i)) continue;
					if (ld->GetTVVertHidden(i)) continue;
					if (ld->GetTVVertFrozen(i)) continue;

					if (!ld->IsTVVertVisible(i)) continue;

					if (bounds.Contains(ld->GetTVVert(i)))
					{
						mMouseHitLocalData = ld;
						mMouseHitVert = i;
						i = ld->GetNumberTVVerts();
						ldID = mMeshTopoData.Count();
					}
				}
			}
		}
	}

	if (fnGetTVSubMode() == TVVERTMODE)
	{
		//loop through all our local data
		for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
		{
			MeshTopoData *ld = mMeshTopoData[ldID];
			if (ld)
			{
				//check to see if we hit a vertex
				for (int i=0; i<ld->GetNumberTVVerts(); i++) 
				{

					if (selOnly && !ld->GetTVVertSelected(i)) continue;
					if (ld->GetTVVertHidden(i)) continue;
					if (ld->GetTVVertFrozen(i)) continue;
					if (!ld->IsTVVertVisible(i)) continue;

					Point3 p(0.0f,0.0f,0.0f);
					p[i1] = ld->GetTVVert(i)[i1];
					p[i2] = ld->GetTVVert(i)[i2];
					
					if (bounds.Contains(p))
					{
						TVHitData d;
						d.mLocalDataID = ldID;
						d.mID = i;
						hits.Append(1,&d,1000);
					}
				}

			}
		}
	}

	else if (fnGetTVSubMode() == TVEDGEMODE)
	{
		//loop through all our local data
		for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
		{
			MeshTopoData *ld = mMeshTopoData[ldID];
			if (ld)
			{
				//check if single click
				if ( (abs(rect.left-rect.right) <= 4) && (abs(rect.bottom-rect.top) <= 4) )
				{
					Point3 center = bounds.Center();
					float threshold = (1.0f/xzoom) * 2.0f;
					int edgeIndex = ld->EdgeIntersect(center,threshold,i1,i2);

					if (edgeIndex >=0)
					{
						BitArray sel;
						sel.SetSize(ld->GetNumberTVVerts());
						sel.ClearAll();

						BitArray tempVSel;
						if (selOnly)
							ld->GetVertSelFromEdge(tempVSel);

						int index1 = ld->GetTVEdgeVert(edgeIndex, 0);
						if (selOnly && !tempVSel[index1]) edgeIndex = -1;
						if (ld->GetTVVertHidden(index1)) edgeIndex = -1;
						if (ld->GetTVVertFrozen(index1)) edgeIndex = -1;
						if (!ld->IsTVVertVisible(index1)) edgeIndex = -1;

						if (edgeIndex >=0)
						{
							int index2 = ld->GetTVEdgeVert(edgeIndex, 1);

							if (selOnly && !tempVSel[index2]) edgeIndex = -1;
							if (ld->GetTVVertHidden(index2)) edgeIndex = -1;
							if (ld->GetTVVertFrozen(index2)) edgeIndex = -1;
							if (!ld->IsTVVertVisible(index2)) edgeIndex = -1;
						}
					}

					if (edgeIndex >= 0)
					{
						if (selOnly && !ld->GetTVEdgeSelected(edgeIndex)) 
						{
						}
						else
						{

							TVHitData d;
							d.mLocalDataID = ldID;
							d.mID = edgeIndex;
							hits.Append(1,&d,10);
						}
					}
/*
					if (gridSnap)
					{
						float closestDist = 0.0f;
						Point3 tempCenter = center;
						//				tempCenter.z = 0.0f;
						if (mode == ID_FREEFORMMODE)
						{

							for (int j =0; j < esel.GetSize(); j++)
							{
								if (esel[j])
								{
									int index = j;
									int a = TVMaps.ePtrList[index]->a;
									//							Point3 p = GetPoint(t,a);
									//							p.z = 0.0f;
									Point3 p(0.0f,0.0f,0.0f);
									Point3 tp = GetPoint(t,a);
									p[i1] = tp[i1];
									p[i2] = tp[i2];

									float dist = LengthSquared(tempCenter-p);
									if ((mouseHitVert == -1) || (dist < closestDist))
									{
										mouseHitVert = a;
										closestDist = dist;
									}
									a = TVMaps.ePtrList[index]->b;
									//							p = GetPoint(t,a);
									//							p.z = 0.0f;
									p = Point3(0.0f,0.0f,0.0f);
									tp = GetPoint(t,j);
									p[i1] = tp[i1];
									p[i2] = tp[i2];

									dist = LengthSquared(tempCenter-p);

									if ((mouseHitVert == -1) || (dist < closestDist))
									{
										mouseHitVert = a;
										closestDist = dist;
									}
								}
							}	
						}
						else
						{
							for (int j =0; j < hits.Count(); j++)
							{
								int index = hits[j];
								int a = TVMaps.ePtrList[index]->a;
								Point3 p(0.0f,0.0f,0.0f);
								Point3 tp = GetPoint(t,a);
								p[i1] = tp[i1];
								p[i2] = tp[i2];

								float dist = LengthSquared(tempCenter-p);
								if ((mouseHitVert == -1) || (dist < closestDist))
								{
									mouseHitVert = a;
									closestDist = dist;
								}
								a = TVMaps.ePtrList[index]->b;
								p = Point3(0.0f,0.0f,0.0f);
								tp = GetPoint(t,a);
								p[i1] = tp[i1];
								p[i2] = tp[i2];


								dist = LengthSquared(tempCenter-p);

								if ((mouseHitVert == -1) || (dist < closestDist))
								{
									mouseHitVert = a;
									closestDist = dist;
								}
							}	

						}	
					}
*/
					//loop through all the faces and see if the point is contained by it
				}	
				
				//else it is a drag rect

				else
				{
					BitArray sel;
					sel.SetSize(ld->GetNumberTVVerts());
					sel.ClearAll();

					BitArray tempVSel;
					if (selOnly)
						ld->GetVertSelFromEdge(tempVSel);
					Tab<Point2> screenSpaceList; 
					screenSpaceList.SetCount(ld->GetNumberTVVerts()); 
					for (int i = 0; i < screenSpaceList.Count(); i++)
						screenSpaceList[i] = UVWToScreen(ld->GetTVVert(i),xzoom,yzoom,width,height);

					Point3 rectPoints[4];

					rectPoints[0].x = rect.left; 
					rectPoints[0].y = rect.top; 
					rectPoints[0].z = 0.0f;

					rectPoints[1].x = rect.right; 
					rectPoints[1].y = rect.top; 
					rectPoints[1].z = 0.0f;

					rectPoints[2].x = rect.left; 
					rectPoints[2].y = rect.bottom; 
					rectPoints[2].z = 0.0f;

					rectPoints[3].x = rect.right; 
					rectPoints[3].y = rect.bottom; 
					rectPoints[3].z = 0.0f;

					BOOL cross = GetCOREInterface()->GetCrossing();

					for (int i=0; i<ld->GetNumberTVEdges(); i++) 
					{
						int index1 = ld->GetTVEdgeVert(i,0);//TVMaps.ePtrList[i]->a;

						if (selOnly && !tempVSel[index1]) continue;
						if (ld->GetTVVertHidden(index1)) continue;
						if (ld->GetTVVertFrozen(index1)) continue;
						if (!ld->IsTVVertVisible(index1)) continue;

						int index2 = ld->GetTVEdgeVert(i,1);

						if (selOnly && !tempVSel[index2]) continue;
						if (ld->GetTVVertHidden(index2)) continue;
						if (ld->GetTVVertFrozen(index2)) continue;
						if (!ld->IsTVVertVisible(index2)) continue;

						Point2 a = screenSpaceList[index1];
						Point2 b = screenSpaceList[index2];

						BOOL hitEdge = FALSE;
						if ( (a.x >= rect.left) && (a.x <= rect.right) && 
							(a.y <= rect.bottom) && (a.y >= rect.top) && 
							(b.x >= rect.left) && (b.x <= rect.right) && 
							(b.y <= rect.bottom) && (b.y >= rect.top) )
						{
							TVHitData d;
							d.mLocalDataID = ldID;
							d.mID = i;
							hits.Append(1,&d,500);
							hitEdge = TRUE;
						}

						Point3 ap,bp;
						ap.x = a.x;
						ap.y = a.y;
						ap.z = 0.0f;
						bp.x = b.x;
						bp.y = b.y;
						bp.z = 0.0f;

						if ((cross) && (!hitEdge))
						{
							if (LineIntersect(ap, bp, rectPoints[0],rectPoints[1]))
							{
								TVHitData d;
								d.mLocalDataID = ldID;
								d.mID = i;
								hits.Append(1,&d,500);
							}
							else if (LineIntersect(ap, bp, rectPoints[2],rectPoints[3]))
							{
								TVHitData d;
								d.mLocalDataID = ldID;
								d.mID = i;
								hits.Append(1,&d,500);
							}								
							else if (LineIntersect(ap, bp, rectPoints[0],rectPoints[2]))
							{
								TVHitData d;
								d.mLocalDataID = ldID;
								d.mID = i;
								hits.Append(1,&d,500);
							}
							else if (LineIntersect(ap, bp, rectPoints[1],rectPoints[3]))
							{
								TVHitData d;
								d.mLocalDataID = ldID;
								d.mID = i;
								hits.Append(1,&d,500);
							}
						}

					}

				}

			}
		}
	}

	else if (fnGetTVSubMode() == TVFACEMODE)
	{
		//check if single click
		if ( (abs(rect.left-rect.right) <= 4) && (abs(rect.bottom-rect.top) <= 4) )
		{
			Point3 center(0.0f,0.0f,0.0f);

			Point3 tcent = bounds.Center();
			center.x = tcent[i1];
			center.y = tcent[i2];

			int faceIndex = -1;

			//loop through all our local data
			for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
			{
				MeshTopoData *ld = mMeshTopoData[ldID];
				if (ld)
				{
					if (selOnly)
					{
						BitArray ignoreFaces;
						ignoreFaces = ~ld->GetFaceSelection();
						faceIndex = ld->PolyIntersect(center,i1,i2,&ignoreFaces);
					}
					else faceIndex = ld->PolyIntersect(center,i1,i2);

					if ((faceIndex >= 0) && !ld->GetFaceHidden(faceIndex))
					{
						if (selOnly && !ld->GetFaceSelected(faceIndex)) 
						{
						}
						else
						{
							TVHitData d;
							d.mLocalDataID = ldID;
							d.mID = faceIndex;
							hits.Append(1,&d,500);
						}
					}
				}
			}

			//loop through all the faces and see if the point is contained by it
/*
			if (gridSnap)
			{
				float closestDist = 0.0f;
				Point3 tempCenter = center;
				//				tempCenter.z = 0.0f;
				for (int j =0; j < hits.Count(); j++)
				{
					int index = hits[j];
					for (int k = 0; k < TVMaps.f[index]->count; k++)
					{
						int a = TVMaps.f[index]->t[k];
						//						Point3 p = GetPoint(t,a);
						//						p.z = 0.0f;
						Point3 p(0.0f,0.0f,0.0f);
						Point3 tp = GetPoint(t,a);
						p[i1] = tp[i1];
						p[i2] = tp[i2];

						float dist = LengthSquared(tempCenter-p);
						if ((mouseHitVert == -1) || (dist < closestDist))
						{
							mouseHitVert = a;
							closestDist = dist;
						}

					}
				}
			}
*/
		}	
		//else it is a drag rect
		else
		{



			//loop through all our local data
			for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
			{
				MeshTopoData *ld = mMeshTopoData[ldID];
				if (ld)
				{
/*
					BitArray sel;
					sel.SetSize(TVMaps.v.Count());
					sel.ClearAll();
*/
					BitArray tempVSel;
					if (selOnly)
						ld->GetVertSelFromFace(tempVSel);

					Tab<Point2> screenSpaceList; 
					screenSpaceList.SetCount(ld->GetNumberTVVerts()); 
					for (int i = 0; i < screenSpaceList.Count(); i++)
						screenSpaceList[i] = UVWToScreen(ld->GetTVVert(i),xzoom,yzoom,width,height);


					Point3 rectPoints[4];

					rectPoints[0].x = rect.left; 
					rectPoints[0].y = rect.top; 
					rectPoints[0].z = 0.0f;

					rectPoints[1].x = rect.right; 
					rectPoints[1].y = rect.top; 
					rectPoints[1].z = 0.0f;

					rectPoints[2].x = rect.left; 
					rectPoints[2].y = rect.bottom; 
					rectPoints[2].z = 0.0f;

					rectPoints[3].x = rect.right; 
					rectPoints[3].y = rect.bottom; 
					rectPoints[3].z = 0.0f;

					BOOL cross = GetCOREInterface()->GetCrossing();


					for (int i=0; i<ld->GetNumberFaces(); i++) 
					{
						int faceIndex = i;
						int total = 0;
						int pcount = ld->GetFaceDegree(faceIndex);
						BOOL frozen = FALSE;
						for (int k = 0; k <  pcount; k++)
						{
							int index = ld->GetFaceTVVert(faceIndex,k);
							if (ld->GetTVVertFrozen(index)) 
								frozen = TRUE;
							if (!ld->IsTVVertVisible(index)) 
								frozen = TRUE;
							Point2 a = screenSpaceList[index];			
							if ( (a.x >= rect.left) && (a.x <= rect.right) && 
								(a.y <= rect.bottom) && (a.y >= rect.top)  )
								total++;
						}
						if (frozen) 
						{
							total=0;
						}
						else if (total == pcount)
						{
							TVHitData d;
							d.mLocalDataID = ldID;
							d.mID = i;
							hits.Append(1,&d,500);
						}
						else if (cross)
						{
							for (int k = 0; k <  pcount; k++)
							{
								int index = ld->GetFaceTVVert(faceIndex,k);
								int index2;
								if (k == (pcount -1))
									index2 = ld->GetFaceTVVert(faceIndex,0);
								else index2 = ld->GetFaceTVVert(faceIndex,k+1);

								Point2 a = screenSpaceList[index];			
								Point2 b = screenSpaceList[index2];			

								Point3 ap,bp;
								ap.x = a.x;
								ap.y = a.y;
								ap.z = 0.0f;
								bp.x = b.x;
								bp.y = b.y;
								bp.z = 0.0f;
								if (LineIntersect(ap, bp, rectPoints[0],rectPoints[1]))
								{
									TVHitData d;
									d.mLocalDataID = ldID;
									d.mID = i;
									hits.Append(1,&d,500);
									k =  pcount;
								}
								else if (LineIntersect(ap, bp, rectPoints[2],rectPoints[3]))
								{
									TVHitData d;
									d.mLocalDataID = ldID;
									d.mID = i;
									hits.Append(1,&d,500);
									k =  pcount;
								}
								else if (LineIntersect(ap, bp, rectPoints[0],rectPoints[2]))
								{
									TVHitData d;
									d.mLocalDataID = ldID;
									d.mID = i;
									hits.Append(1,&d,500);
									k =  pcount;
								}
								else if (LineIntersect(ap, bp, rectPoints[1],rectPoints[3]))
								{
									TVHitData d;
									d.mLocalDataID = ldID;
									d.mID = i;
									hits.Append(1,&d,500);
									k =  pcount;
								}
							}
						}
					}


				}

			}


		}
	}



	BOOL bail = FALSE;
	int vselSet = 0;
	int eselSet = 0;
	int fselSet = 0;

	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];
		if (ld)
		{
			vselSet += ld->GetTVVertSelection().NumberSet();
			eselSet += ld->GetTVEdgeSelection().NumberSet();
			fselSet += ld->GetFaceSelection().NumberSet();
		}
	}
	if (fnGetTVSubMode() == TVVERTMODE)
	{
		if (vselSet == 0 ) 
			bail = TRUE;
		freeFormSubMode = ID_TOOL_SELECT;
	}
	else if (fnGetTVSubMode() == TVEDGEMODE)
	{
		if (eselSet == 0 ) 
			bail = TRUE;
		freeFormSubMode = ID_TOOL_SELECT;
	}
	else if (fnGetTVSubMode() == TVFACEMODE)
	{
		if (fselSet == 0 ) 
			bail = TRUE;
		freeFormSubMode = ID_TOOL_SELECT;
	}

	if ( (mode == ID_FREEFORMMODE) && (!bail))
	{
		//check if cursor is inside bounds
		Point3 center = bounds.Center();
		Point3 hold1, hold2;
		hold1 = p1;
		hold2 = p2;
		p1 = Point3(0.0f,0.0f,0.0f);
		p2 = Point3(0.0f,0.0f,0.0f);

		p1[i1] = hold1[i1];
		p1[i2] = hold1[i2];

		p2[i1] = hold2[i1];
		p2[i2] = hold2[i2];

		Point2 min = UVWToScreen(freeFormCorners[0],xzoom,yzoom,width,height);
		Point2 max = UVWToScreen(freeFormCorners[3],xzoom,yzoom,width,height);

		bounds = boundsFF;

		if ((fabs(max.x - min.x) < 40.0f) || (fabs(min.y - max.y) < 40.0f))
		{
			bounds = smallBounds;
		}


		BOOL forceMove = FALSE;
		if (fnGetTVSubMode() == TVVERTMODE)
		{
			if (vselSet <= 1 ) 
				forceMove = TRUE;
		}
		else if (fnGetTVSubMode() == TVEDGEMODE)
		{
		}
		else if (fnGetTVSubMode() == TVFACEMODE)
		{
		}
		if ((!forceMove) && (smallBounds.Contains(selCenter+freeFormPivotOffset)) )
		{
			freeFormSubMode = ID_TOOL_MOVEPIVOT;
			return TRUE;
		}
		else if ( (bounds.Contains(freeFormCorners[0]) || bounds.Contains(freeFormCorners[1]) ||
			bounds.Contains(freeFormCorners[2]) || bounds.Contains(freeFormCorners[3]) ))
		{
			freeFormSubMode = ID_TOOL_SCALE;
			if (bounds.Contains(freeFormCorners[0]))
			{

				scaleCorner = 0;
				scaleCornerOpposite = 2;
			}
			else if (bounds.Contains(freeFormCorners[1]))
			{

				scaleCorner = 3;
				scaleCornerOpposite = 1;
			}
			else if (bounds.Contains(freeFormCorners[2]))
			{

				scaleCorner = 1;
				scaleCornerOpposite = 3;
			}
			else if (bounds.Contains(freeFormCorners[3]))
			{

				scaleCorner = 2;
				scaleCornerOpposite = 0;
			}

			return TRUE;
		}
		BOOL useMoveMode = FALSE;

		if (allowSelectionInsideGizmo)
		{
			if ((hits.Count() == 0) || forceMove)
				useMoveMode = TRUE;
			else useMoveMode = FALSE;
		}
		else
		{

			useMoveMode = TRUE;

		}

		if ( (bounds.Contains(freeFormEdges[0]) || bounds.Contains(freeFormEdges[1]) ||
			bounds.Contains(freeFormEdges[2]) || bounds.Contains(freeFormEdges[3]) ))
		{
			freeFormSubMode = ID_TOOL_ROTATE;
			return TRUE;
		}
		else if ((freeFormBounds.Contains(p1) && freeFormBounds.Contains(p2)) && (useMoveMode) )
		{
			freeFormSubMode = ID_TOOL_MOVE;
			return TRUE;
		}
		else 
		{
			freeFormSubMode = ID_TOOL_SELECT;
		}


	}
/*
	if ((mode == ID_TOOL_WELD) &&  (hits.Count()>0))
	{
		if (fnGetTVSubMode() == TVFACEMODE) 
			hits.SetCount(0);
		else hits.SetCount(1);
	}
	*/
	return hits.Count();
}

void UnwrapMod::InvalidateTypeins()
{
	typeInsValid = FALSE;	
}

void UnwrapMod::SetupTypeins()
{
	if (typeInsValid) return;
	typeInsValid = TRUE;

	Point3 uv(0,0,0);
	BOOL found = FALSE;
	BOOL u = TRUE, v = TRUE, w = TRUE;


	TransferSelectionStart();

	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];
		for (int i=0; i < ld->GetNumberTVVerts(); i++)//TVMaps.v.Count(); i++) {
		{
			if (!ld->GetTVVertSelected(i)/*vsel[i]*/) continue;

			if (found) 
			{
				Point3 p = ld->GetTVVert(i);
				if (uv.x!=p.x) 
				{
					u = FALSE;				
				}
				if (uv.y!=p.y) 
				{
					v = FALSE;				
				}
				if (uv.z!=p.z) 
				{
					w = FALSE;				
				}			
			} else {
				uv = ld->GetTVVert(i);//TVMaps.v[i].p;
				found = TRUE;
			}
		}
	}
	TransferSelectionEnd(FALSE,FALSE);

	if (!found) {
		if (iU) iU->Disable();
		if (iV) iV->Disable();
		if (iW) iW->Disable();
	} else {
		if (absoluteTypeIn)
		{
			if (iU) iU->Enable();
			if (iV) iV->Enable();
			if (iW) iW->Enable();
			if ((u) && (iU)) {
				iU->SetIndeterminate(FALSE);
				iU->SetValue(uv.x,FALSE);
			} else  if (iU) {
				iU->SetIndeterminate(TRUE);
			}

			if ((v) && (iV)){
				iV->SetIndeterminate(FALSE);
				iV->SetValue(uv.y,FALSE);
			} else if (iV) {
				iV->SetIndeterminate(TRUE);
			}

			if ((w) && (iW)){
				iW->SetIndeterminate(FALSE);
				iW->SetValue(uv.z,FALSE);
			} else  if (iW) {
				iW->SetIndeterminate(TRUE);
			}
		}

		else
		{
			iU->SetIndeterminate(FALSE);
			iU->SetValue(0.0f,FALSE);

			iV->SetIndeterminate(FALSE);
			iV->SetValue(0.0f,FALSE);

			iW->SetIndeterminate(FALSE);
			iW->SetValue(0.0f,FALSE);

		}
	}	
}

void UnwrapMod::Select(Tab<TVHitData> &hits,BOOL toggle,BOOL subtract,BOOL all)
{

	if (!fnGetPaintMode()) 
		HoldSelection();

	BitArray usedLDs;
	usedLDs.SetSize(mMeshTopoData.Count());
	usedLDs.ClearAll();


	if (fnGetTVSubMode() == TVVERTMODE)
	{
		for (int i=0; i<hits.Count(); i++) 
		{
			int vID = hits[i].mID;
			int ldID = hits[i].mLocalDataID;
			
			MeshTopoData *ld = mMeshTopoData[ldID];
			if (!ld) 
				continue;

			usedLDs.Set(ldID,TRUE);
			if ( ld->IsTVVertVisible(vID) &&  (!ld->GetTVVertFrozen(vID)) && (!ld->GetTVVertHidden(vID)) )
			{
				if (toggle) 
					ld->SetTVVertSelected(vID,!ld->GetTVVertSelected(vID));					
				else if (subtract) 
					ld->SetTVVertSelected(vID,FALSE);
				else 
					ld->SetTVVertSelected(vID,TRUE);
				if (!all) break;
			}
		}	
		if ((!subtract) && (mode != ID_TOOL_WELD) )
			SelectHandles(0);
	}

	else if (fnGetTVSubMode() == TVEDGEMODE)
	{
		for (int i=0; i<hits.Count(); i++) 
		{
			int eID = hits[i].mID;
			int ldID = hits[i].mLocalDataID;
			
			MeshTopoData *ld = mMeshTopoData[ldID];
			if (ld)
			{
				usedLDs.Set(ldID,TRUE);
				if (toggle) 
					ld->SetTVEdgeSelected(eID,!ld->GetTVEdgeSelected(eID));
				else if (subtract) 
					ld->SetTVEdgeSelected(eID,FALSE);
				else 
					ld->SetTVEdgeSelected(eID,TRUE);
				if (!all) 
					break;
			}
		}	

	}

	else if (fnGetTVSubMode() == TVFACEMODE)
	{
		for (int i=0; i<hits.Count(); i++) 
		{
			int fID = hits[i].mID;
			int ldID = hits[i].mLocalDataID;

			MeshTopoData *ld = mMeshTopoData[ldID];
			if (ld)
			{
				usedLDs.Set(ldID,TRUE);
				if ( (ld->IsFaceVisible(fID)) && (!ld->GetFaceFrozen(fID) ))
				{

					if (toggle) 
						ld->SetFaceSelected(fID,!ld->GetFaceSelected(fID));
					else if (subtract) 
						ld->SetFaceSelected(fID,FALSE);
					else 
						ld->SetFaceSelected(fID,TRUE);
					if (!all) 
						break;
				}
			}
		}	
//		ld->BuilFilterSelectedFaces(filterSelectedFaces);
	}


	if ( (tvElementMode) && (mode != ID_TOOL_WELD)) 
	{
		if (subtract)
			SelectElement(FALSE);
		else SelectElement(TRUE);
	}

	if ( (uvEdgeMode) && (fnGetTVSubMode() == TVEDGEMODE))
	{
		if (!fnGetPaintMode())
			SelectUVEdge(FALSE);
	}
	if ( (openEdgeMode) && (fnGetTVSubMode() == TVEDGEMODE))
	{
		if (!fnGetPaintMode()) 
			SelectOpenEdge();
	}
	if ( (polyMode) && (fnGetTVSubMode() == TVFACEMODE))
	{
		theHold.Suspend();
		if (subtract)
			fnPolySelect2(FALSE);
		else fnPolySelect();
		theHold.Resume();
	}

	

	if (!fnGetPaintMode())
	{
		for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
		{
			if (usedLDs[ldID])
			{
				MeshTopoData *ld = mMeshTopoData[ldID];
				INode *node = mMeshTopoData.GetNode(ldID);

				TSTR functionCall;
				


				if (fnGetTVSubMode() == TVVERTMODE)
				{
					functionCall.printf("$%s.modifiers[#unwrap_uvw].unwrap6.selectVerticesByNode",node->GetName());
					macroRecorder->FunctionCall(functionCall, 2, 0,
						mr_bitarray,ld->GetTVVertSelectionPtr(),
						mr_reftarg,node
						);
				}
				else if (fnGetTVSubMode() == TVEDGEMODE)
				{
					functionCall.printf("$%s.modifiers[#unwrap_uvw].unwrap6.selectEdgesByNode",node->GetName());
					macroRecorder->FunctionCall(functionCall, 2, 0,
						mr_bitarray,ld->GetTVEdgeSelectionPtr(),
						mr_reftarg,node
						);
				}
				else if (fnGetTVSubMode() == TVFACEMODE)
				{
					functionCall.printf("$%s.modifiers[#unwrap_uvw].unwrap6.selectFacesByNode",node->GetName());
					macroRecorder->FunctionCall(functionCall, 2, 0,
						mr_bitarray,ld->GetFaceSelectionPtr(),
						mr_reftarg,node
						);
				}

				macroRecorder->EmitScript();
				

			}
		}
	}

	if ((mode == ID_FREEFORMMODE) && (fnGetResetPivotOnSel()))
	{
		freeFormPivotOffset.x = 0.0f;
		freeFormPivotOffset.y = 0.0f;
		freeFormPivotOffset.z = 0.0f;
	}

	if (fnGetSyncSelectionMode() && (!fnGetPaintMode())) 
		fnSyncGeomSelection();

	RebuildDistCache();
}

void UnwrapMod::ClearSelect()
{
	if ((fnGetMapMode() == PLANARMAP) || (fnGetMapMode() == CYLINDRICALMAP)  || (fnGetMapMode() == SPHERICALMAP) || (fnGetMapMode() == BOXMAP))
		return;

	if ((fnGetMapMode() == PELTMAP) && (peltData.peltDialog.hWnd==NULL))
		return;

	HoldSelection();
	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];
		ld->ClearSelection(fnGetTVSubMode());
	}
	if (fnGetSyncSelectionMode()) fnSyncGeomSelection();

}

void UnwrapMod::HoldPoints()
{
	if (theHold.Holding() && !TestAFlag(A_HELD)) {
		SetAFlag(A_HELD);
		for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
		{
			MeshTopoData *ld = mMeshTopoData[ldID];
			theHold.Put(new TVertRestore(this,ld,FALSE));
		}
	}
}


void UnwrapMod::HoldSelection()
{
	if (theHold.Holding() && !TestAFlag(A_HELD)) {	
		SetAFlag(A_HELD);
		for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
		{
			MeshTopoData *ld = mMeshTopoData[ldID];
			theHold.Put(new TSelRestore(this,ld));
		}
	}
}

void UnwrapMod::HoldPointsAndFaces(bool hidePeltDialog)
{
	if (theHold.Holding() && !TestAFlag(A_HELD)) {
		SetAFlag(A_HELD);
		for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
		{
			MeshTopoData *ld = mMeshTopoData[ldID];
			theHold.Put(new TVertAndTFaceRestore(this,ld,hidePeltDialog));
		}
	}
}


void UnwrapMod::TypeInChanged(int which)
{
	theHold.Restore();
	TimeValue t = GetCOREInterface()->GetTime();
	HoldPoints();

	TransferSelectionStart();

	Point3 uvw(0.0f,0.0f,0.0f);
	if (iU) uvw[0] =  iU->GetFVal();
	if (iV) uvw[1] =  iV->GetFVal();
	if (iW) uvw[2] =  iW->GetFVal();


	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];

		for (int i=0; i < ld->GetNumberTVVerts(); i++)//TVMaps.v.Count(); i++) 
		{

			if (absoluteTypeIn)
			{
				if (ld->GetTVVertSelected(i))//vsel[i]) 
				{
					Point3 p = ld->GetTVVert(i);
					p[which] = uvw[which];
					ld->SetTVVert(t,i,p,this);
//					if (TVMaps.cont[i]) TVMaps.cont[i]->GetValue(t,&TVMaps.v[i].p,FOREVER);					
//					TVMaps.v[i].p[which] = uvw[which];
//					if (TVMaps.cont[i]) TVMaps.cont[i]->SetValue(t,&TVMaps.v[i].p);
				}
				else if(ld->GetTVVertInfluence(i) != 0.0f)
				{
					float infl = ld->GetTVVertInfluence(i);//TVMaps.v[i].influence;
					Point3 p = ld->GetTVVert(i);
					p[which] += (uvw[which] - p[which])*infl;
					ld->SetTVVert(t,i,p,this);
//					if (TVMaps.cont[i]) TVMaps.cont[i]->GetValue(t,&TVMaps.v[i].p,FOREVER);
//					TVMaps.v[i].p[which] += (uvw[which] - TVMaps.v[i].p[which])*infl;
//					if (TVMaps.cont[i]) TVMaps.cont[i]->SetValue(t,&TVMaps.v[i].p);

				}
			}
			else
			{
				if (ld->GetTVVertSelected(i))//(vsel[i]) {
				{
					Point3 p = ld->GetTVVert(i);
					p[which] += uvw[which];
					ld->SetTVVert(t,i,p,this);
//					if (TVMaps.cont[i]) TVMaps.cont[i]->GetValue(t,&TVMaps.v[i].p,FOREVER);
//					TVMaps.v[i].p[which] += uvw[which];
//					if (TVMaps.cont[i]) TVMaps.cont[i]->SetValue(t,&TVMaps.v[i].p);
				}
				else if (ld->GetTVVertInfluence(i)!= 0.0f)
				{
					float infl = ld->GetTVVertInfluence(i);//TVMaps.v[i].influence;
					Point3 p = ld->GetTVVert(i);
					p[which] += uvw[which] * infl;;
					ld->SetTVVert(t,i,p,this);
//					if (TVMaps.cont[i]) TVMaps.cont[i]->GetValue(t,&TVMaps.v[i].p,FOREVER);
//					TVMaps.v[i].p[which] += uvw[which] * infl;
//					if (TVMaps.cont[i]) TVMaps.cont[i]->SetValue(t,&TVMaps.v[i].p);

				}
			}

		}
	}

	TransferSelectionEnd(FALSE,FALSE);

	tempAmount = uvw[which];

	NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);
	InvalidateView();
	if (ip) ip->RedrawViews(ip->GetTime());
}

void UnwrapMod::ChannelChanged(int which, float x)
{
	TimeValue t = GetCOREInterface()->GetTime();
	theHold.Begin();
	HoldPoints();
	Point3 uvw;
	uvw[0] = 0.0f;
	uvw[1] = 0.0f;
	uvw[2] = 0.0f;

	uvw[which] = x;

	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];


		for (int i = 0; i < ld->GetNumberTVVerts(); i++)//TVMaps.v.Count(); i++) {
		{
			if (ld->GetTVVertSelected(i))//vsel[i]) 
			{
				Point3 p = ld->GetTVVert(i);
				p[which] = uvw[which];
				ld->SetTVVert(t,i,p,this);
//				if (TVMaps.cont[i]) TVMaps.cont[i]->GetValue(t,&TVMaps.v[i].p,FOREVER);
//				TVMaps.v[i].p[which] = uvw[which];
//				if (TVMaps.cont[i]) TVMaps.cont[i]->SetValue(t,&TVMaps.v[i].p);
			}
			else if (ld->GetTVVertInfluence(i) != 0.0f)
			{
				float infl = ld->GetTVVertInfluence(i);
				Point3 p = ld->GetTVVert(i);
				p[which] += (uvw[which] - p[which])*infl;
				ld->SetTVVert(t,i,p,this);

//				if (TVMaps.cont[i]) TVMaps.cont[i]->GetValue(t,&TVMaps.v[i].p,FOREVER);
//				TVMaps.v[i].p[which] += (uvw[which] - TVMaps.v[i].p[which])*infl;
//				if (TVMaps.cont[i]) TVMaps.cont[i]->SetValue(t,&TVMaps.v[i].p);

			}
		}
	}

	theHold.Accept(_T(GetString(IDS_PW_MOVE_UVW)));

	NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);
	InvalidateView();
	if (ip) ip->RedrawViews(ip->GetTime());
}

void UnwrapMod::SetVertexPosition(MeshTopoData *ld, TimeValue t, int which, Point3 pos, BOOL hold, BOOL update)
{
	if (hold)
	{
		theHold.Begin();
		HoldPoints();
	}

		if (ld)
		{
			if ((which >=0) && (which < ld->GetNumberTVVerts()))
			{
				ld->SetTVVert(t,which,pos,this);
			}

		}

	if (hold)
	{
		theHold.Accept(_T(GetString(IDS_PW_MOVE_UVW)));
	}

	if (update)
	{
		NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);
		InvalidateView();
		if (ip) ip->RedrawViews(ip->GetTime());
	}
}


void UnwrapMod::DeleteSelected()
{

/*
	theHold.Begin();

	HoldPointsAndFaces();

	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{

		MeshTopoData *ld = mMeshTopoData[ldID];

		for (int i = 0; i < ld->GetNumberTVVerts(); i++)//TVMaps.v.Count(); i++) 
		{
			if ( ld->GetTVVertSelected(i) && (!ld->GetTVVertDead(i)))//(vsel[i]) && (!(TVMaps.v[i].flags & FLAG_DEAD)) )
			{
				ld->SetTVVertDead(i,TRUE);//TVMaps.v[i].flags |= FLAG_DEAD;
				ld->SetTVVertSelected(i,FALSE);//vsel.Set(i,FALSE);
			}
		}

		for (int i = 0; i < ld->GetNumberFaces(); i++)//TVMaps.f.Count(); i++) 
		{
			if (!ld->GetFaceDead(i))//(!(TVMaps.f[i]->flags & FLAG_DEAD))
			{
				for (int j=0; j<3; j++) 
				{
					int index = ld->GetFaceTVVert(i,j);//TVMaps.f[i]->t[j];
					if ((index < 0) || (index >= ld->GetNumberTVVerts()))//TVMaps.v.Count()))
					{
						DbgAssert(1);
					}
					else if (ld->GetTVVertDead(index))//TVMaps.v[index].flags & FLAG_DEAD)
					{
						ld->SetFaceDead(i,TRUE);//TVMaps.f[i]->flags |= FLAG_DEAD;
					}
				}
			}
		}
	}
	// loop through faces all
	theHold.Accept(_T(GetString(IDS_PW_DELETE_SELECTED)));
*/
}

void UnwrapMod::HideSelected()
{

	
	theHold.Begin();
	HoldPoints();	
	theHold.Accept(_T(GetString(IDS_PW_HIDE_SELECTED)));

	//convert our sub selection type to vertex selection
	TransferSelectionStart();

	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];
		for (int i=0; i< ld->GetNumberTVVerts(); i++)//TVMaps.v.Count(); i++) 
		{
			if ( (ld->GetTVVertSelected(i) && (!ld->GetTVVertDead(i))))//vsel[i]) && (!(TVMaps.v[i].flags & FLAG_DEAD)) )
			{
				
				ld->SetTVVertHidden(i,TRUE);//TVMaps.v[i].flags |= FLAG_HIDDEN;
				ld->SetTVVertSelected(i,FALSE);//	vsel.Set(i,FALSE);
			}
		}
	}

	
	//put back our old vertex selection if need be
	TransferSelectionEnd(FALSE,FALSE);
	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];
		if (fnGetTVSubMode() == TVVERTMODE)
			ld->ClearTVVertSelection();//vsel.ClearAll();
		else if (fnGetTVSubMode() == TVEDGEMODE)
			ld->ClearTVEdgeSelection();//esel.ClearAll();
		if (fnGetTVSubMode() == TVFACEMODE)
			ld->ClearFaceSelection();//fsel.ClearAll();
	}

}

void UnwrapMod::UnHideAll()
{

	theHold.Begin();
	HoldPoints();	
	theHold.Accept(_T(GetString(IDS_PW_UNHIDEALL)));


	//convert our sub selection type to vertex selection
	TransferSelectionStart();

	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];
		for (int i=0; i< ld->GetNumberTVVerts(); i++)//TVMaps.v.Count(); i++) 
		{
			if ( (ld->GetTVVertHidden(i) && (!ld->GetTVVertDead(i))))//if ( (TVMaps.v[i].flags & FLAG_HIDDEN) && (!(TVMaps.v[i].flags & FLAG_DEAD)) )
			{
				ld->SetTVVertHidden(i,FALSE);//TVMaps.v[i].flags -= FLAG_HIDDEN;
			}
		}
	}


	//put back our old vertex selection if need be
	TransferSelectionEnd(FALSE,FALSE);

}


void UnwrapMod::FreezeSelected()
{

	theHold.Begin();
	HoldPoints();
	theHold.Accept(_T(GetString(IDS_PW_FREEZE_SELECTED)));

	//convert our sub selection type to vertex selection
	TransferSelectionStart();

	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];
		for (int i=0; i<ld->GetNumberTVVerts(); i++)//TVMaps.v.Count(); i++) 
		{
			if (ld->GetTVVertSelected(i) && (!ld->GetTVVertDead(i)))//( (vsel[i]) && (!(TVMaps.v[i].flags & FLAG_DEAD)) )
			{
				ld->SetTVVertFrozen(i,TRUE);//TVMaps.v[i].flags |= FLAG_FROZEN;
				ld->SetTVVertSelected(i,FALSE);//vsel.Set(i,FALSE);
			}
		}
	}

	//put back our old vertex selection if need be
	TransferSelectionEnd(FALSE,FALSE);

	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];
		if (fnGetTVSubMode() == TVVERTMODE)
			ld->ClearTVVertSelection();//vsel.ClearAll();
		else if (fnGetTVSubMode() == TVEDGEMODE)
			ld->ClearTVEdgeSelection();//esel.ClearAll();
		if (fnGetTVSubMode() == TVFACEMODE)
			ld->ClearFaceSelection();//fsel.ClearAll();

		for (int i=0; i<ld->GetNumberTVVerts(); i++)//TVMaps.v.Count(); i++) 
		{
			if (ld->GetTVVertSelected(i) && ld->GetTVVertFrozen(i))//( (vsel[i]) && (TVMaps.v[i].flags & FLAG_FROZEN) )
			{			
				ld->SetTVVertSelected(i,TRUE);//	vsel.Set(i,FALSE);
			}
		}

		for (int i = 0; i < ld->GetNumberFaces(); i++)//TVMaps.f.Count(); i++)
		{
			int deg = ld->GetFaceDegree(i);//TVMaps.f[i]->count;
			BOOL frozen = FALSE;
			for (int j = 0; j < deg; j++)
			{
				int index = ld->GetFaceTVVert(i,j);//TVMaps.f[i]->t[j];
				if (ld->GetTVVertFrozen(index))//(TVMaps.v[index].flags & FLAG_FROZEN)
					frozen = TRUE;
			}
			if (frozen)
			{
				ld->SetFaceSelected(i,FALSE);//fsel.Set(i,FALSE);
			}
		}

		for (int i = 0; i < ld->GetNumberTVEdges(); i++)//TVMaps.ePtrList.Count(); i++)
		{
			int a = ld->GetTVEdgeVert(i,0);//TVMaps.ePtrList[i]->a;
			int b = ld->GetTVEdgeVert(i,1);//TVMaps.ePtrList[i]->b;
			if (ld->GetTVVertFrozen(a) || ld->GetTVVertFrozen(b))
//				(TVMaps.v[a].flags & FLAG_FROZEN) ||
//				(TVMaps.v[b].flags & FLAG_FROZEN) )
			{
				ld->SetTVEdgeSelected(i,FALSE);//esel.Set(i,FALSE);
			}
		}

	}
}

void UnwrapMod::UnFreezeAll()
{

	theHold.Begin();
	HoldPoints();	
	theHold.Accept(_T(GetString(IDS_PW_UNFREEZEALL)));

	//convert our sub selection type to vertex selection
	TransferSelectionStart();

	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];
		for (int i=0; i < ld->GetNumberTVVerts(); i++)//TVMaps.v.Count(); i++) 
		{
			if ( !ld->GetTVVertDead(i))//(TVMaps.v[i].flags & FLAG_DEAD)) 
			{
				if ( ld->GetTVVertFrozen(i))//(TVMaps.v[i].flags & FLAG_FROZEN)) 
					ld->SetTVVertFrozen(i,FALSE);//TVMaps.v[i].flags -= FLAG_FROZEN;
			}
		}
	}



	//put back our old vertex selection if need be
	TransferSelectionEnd(FALSE,FALSE);

}

void UnwrapMod::WeldSelected(BOOL hold, BOOL notify)
{

	if (hold)
	{
		theHold.Begin();
		HoldPointsAndFaces();	
	}

	//convert our sub selection type to vertex selection
	TransferSelectionStart();

	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];
		ld->WeldSelectedVerts(weldThreshold,this);
	}

	if (hold)
	{
		theHold.Accept(_T(GetString(IDS_PW_WELDSELECTED)));
	}	

	//put back our old vertex selection if need be
	TransferSelectionEnd(FALSE,TRUE);
	
	if (notify)
	{
		NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);
		if (ip) ip->RedrawViews(ip->GetTime());
		InvalidateView();
	}

}

BOOL UnwrapMod::WeldPoints(HWND h, IPoint2 m)
{

	BOOL holdNeeded = FALSE;


	Point3 p(0.0f,0.0f,0.0f);
	Point2 mp;
	mp.x = (float) m.x;
	mp.y = (float) m.y;

	float xzoom, yzoom;
	int width,height;
	ComputeZooms(h,xzoom,yzoom,width,height);
	
	

	Rect rect;
	rect.left = m.x - 2;
	rect.right = m.x + 2;
	rect.top = m.y - 2;
	rect.bottom = m.y + 2;

	mode = 0;
	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		Tab<TVHitData> hits;
		int index = -1;

		MeshTopoData *ld = mMeshTopoData[ldID];
		if (HitTest(rect,hits,FALSE))
		{
			MeshTopoData *selID = NULL;
			for (int i =0; i < hits.Count(); i++)
			{
				MeshTopoData *testLD = mMeshTopoData[hits[i].mLocalDataID];
				int index = hits[i].mID;
				if (fnGetTVSubMode() == TVVERTMODE) 
				{
					if (testLD->GetTVVertSelected(index) && (testLD == ld) )
						selID = ld;

				}
				else if (fnGetTVSubMode() == TVEDGEMODE) 
				{
					if (testLD->GetTVEdgeSelected(index) && (testLD == ld))
					{
						selID = ld;
					}
				}
			}

			for (int i=0; i<hits.Count(); i++) 
			{
				int hindex = hits[i].mID;
				MeshTopoData *hitLD = mMeshTopoData[hits[i].mLocalDataID];
				if (selID && (hitLD == selID))
				{
					if (fnGetTVSubMode() == TVVERTMODE)
					{

						if (!ld->GetTVVertSelected(hindex))//vsel[hits[i]]) 
						{
							index = hindex;//hits[i];
							i = hits.Count();
						}
					}
					else if (fnGetTVSubMode() == TVEDGEMODE)
					{
						if (ld == tWeldHitLD)
							index = tWeldHit;

					}

				}
			}
		}
		mode = ID_TOOL_WELD;


		Tab<int> selected, hit;
		if (fnGetTVSubMode() == TVVERTMODE)
		{
			selected.SetCount(1);
			hit.SetCount(1);
			hit[0] = index;
			for (int i = 0; i < ld->GetNumberTVVerts(); i++)//vsel.GetSize(); i++)
			{
				if (ld->GetTVVertSelected(i))//vsel[i]) 
					selected[0] = i;
			}
		}

		else if ((fnGetTVSubMode() == TVEDGEMODE) && (index != -1))
		{
			selected.SetCount(2);
			hit.SetCount(2);
			hit[0] = ld->GetTVEdgeVert(index,0);//TVMaps.ePtrList[index]->a;
			hit[1] = ld->GetTVEdgeVert(index,1);//TVMaps.ePtrList[index]->b;

			int otherEdgeIndex = 0; 

			for (int i = 0; i < ld->GetNumberTVEdges(); i++)//esel.GetSize(); i++)
			{
				if (ld->GetTVEdgeSelected(i))//esel[i]) 
				{
					otherEdgeIndex = i; 
					selected[0] = ld->GetTVEdgeVert(i,0);//TVMaps.ePtrList[i]->a;
					selected[1] = ld->GetTVEdgeVert(i,1);//TVMaps.ePtrList[i]->b;
				}
			}
			int face1, face2;
			int firstVert1,firstVert2;
			int nextVert1,nextVert2;
			int prevVert1 = -1, prevVert2 = -1;
			face1 = ld->GetTVEdgeConnectedTVFace(index,0);//TVMaps.ePtrList[index]->faceList[0];
			face2 = ld->GetTVEdgeConnectedTVFace(otherEdgeIndex,0);//TVMaps.ePtrList[otherEdgeIndex]->faceList[0];
			firstVert1 = hit[0];
			firstVert2 = selected[0];
			//fix up points
			int pcount = 3;
			pcount = ld->GetFaceDegree(face1);//TVMaps.f[face1]->count;
			for (int i = 0; i < pcount; i++)
			{
				int tvfIndex = ld->GetFaceTVVert(face1,i);//TVMaps.f[face1]->t[i];
				if (tvfIndex == firstVert1)
				{
					if (i != (pcount-1))
						nextVert1 = ld->GetFaceTVVert(face1,i+1);//TVMaps.f[face1]->t[i+1];
					else nextVert1 = ld->GetFaceTVVert(face1,0);//TVMaps.f[face1]->t[0];
					if (i!=0)
						prevVert1 = ld->GetFaceTVVert(face1,i-1);//TVMaps.f[face1]->t[i-1];
					else prevVert1 = ld->GetFaceTVVert(face1,pcount-1);//TVMaps.f[face1]->t[pcount-1];
				}
			}

			pcount = ld->GetFaceDegree(face2);//TVMaps.f[face2]->count;
			for (int i = 0; i < pcount; i++)
			{
				int tvfIndex = ld->GetFaceTVVert(face2,i);//TVMaps.f[face2]->t[i];
				if (tvfIndex == firstVert2)
				{
					if (i != (pcount-1))
						nextVert2 = ld->GetFaceTVVert(face2,i+1);//TVMaps.f[face2]->t[i+1];
					else nextVert2 = ld->GetFaceTVVert(face2,0);//TVMaps.f[face2]->t[0];
					if (i!=0)
						prevVert2 = ld->GetFaceTVVert(face2,i-1);//TVMaps.f[face2]->t[i-1];
					else prevVert2 = ld->GetFaceTVVert(face2,pcount-1);//TVMaps.f[face2]->t[pcount-1];

				}
			}

			if (prevVert1 == hit[1])
			{
				int temp = hit[0];
				hit[0] = hit[1];
				hit[1] = temp;
			}
			if (prevVert2 == selected[1])
			{
				int temp = selected[0];
				selected[0] = selected[1];
				selected[1] = temp;
			}

			int tempSel = selected[0];
			selected[0] = selected[1];
			selected[1] = tempSel;


			if (hit[0] == selected[1])
			{
				hit.Delete(0,1);
				selected.Delete(1,1);
			}

			else if (hit[1] == selected[0])
			{
				hit.Delete(1,1);
				selected.Delete(0,1);
			}

		}


		for (int selIndex = 0; selIndex < selected.Count(); selIndex++)
		{
			int index = hit[selIndex];
			if ( index != -1)
			{
				for (int i=0; i<ld->GetNumberFaces(); i++)//TVMaps.f.Count(); i++) 
				{
					int pcount = 3;
					//		if (TVMaps.f[i].flags & FLAG_QUAD) pcount = 4;
					pcount = ld->GetFaceDegree(i);//TVMaps.f[i]->count;

					for (int j=0; j<pcount; j++) 
					{
						int tvfIndex = ld->GetFaceTVVert(i,j);//TVMaps.f[i]->t[j];
						if (tvfIndex == selected[selIndex])
						{
							ld->SetFaceTVVert(i,j,index);//TVMaps.f[i]->t[j] = index;
							ld->DeleteTVVert(tvfIndex,this);
//							TVMaps.v[tvfIndex].flags |= FLAG_DEAD;
							holdNeeded = TRUE;
						}
						if (ld->GetFaceHasVectors(i))// (TVMaps.f[i]->flags & FLAG_CURVEDMAPPING) &&
//							(TVMaps.f[i]->vecs) 
//							)
						{
							tvfIndex = ld->GetFaceTVHandle(i,j*2);//TVMaps.f[i]->vecs->handles[j*2];
							if (tvfIndex == selected[selIndex])
							{
								ld->SetFaceTVHandle(i,j*2,index);//TVMaps.f[i]->vecs->handles[j*2] = index;
								ld->DeleteTVVert(tvfIndex,this);//TVMaps.v[tvfIndex].flags |= FLAG_DEAD;
								holdNeeded = TRUE;
							}

							tvfIndex = tvfIndex = ld->GetFaceTVHandle(i,j*2+1);//TVMaps.f[i]->vecs->handles[j*2+1];
							if (tvfIndex == selected[selIndex])
							{
								ld->SetFaceTVHandle(i,j*2+1,index);//TVMaps.f[i]->vecs->handles[j*2+1] = index;
								ld->DeleteTVVert(tvfIndex,this);//TVMaps.v[tvfIndex].flags |= FLAG_DEAD;
								holdNeeded = TRUE;
							}

							if (ld->GetFaceHasInteriors(i))//TVMaps.f[i]->flags & FLAG_INTERIOR) 
							{
								tvfIndex = tvfIndex = ld->GetFaceTVHandle(i,j);//TVMaps.f[i]->vecs->interiors[j];
								if ( tvfIndex == selected[selIndex])
								{
									ld->SetFaceTVInterior(i,j,index);//TVMaps.f[i]->vecs->interiors[j] = index;
									ld->DeleteTVVert(tvfIndex,this);//TVMaps.v[tvfIndex].flags |= FLAG_DEAD;
									holdNeeded = TRUE;
								}

							}


						}
					}
				}

			}
		}
		ld->SetTVEdgeInvalid();//TVMaps.edgesValid= FALSE;
	}
	

	NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);
	if (ip) ip->RedrawViews(ip->GetTime());
	InvalidateView();


	return holdNeeded;
}

void UnwrapMod::BreakSelected()
{
	if (fnGetTVSubMode() == TVFACEMODE)
	{
		DetachEdgeVerts();
	}
	else
	{
		
		theHold.Begin();
		HoldPointsAndFaces();
		theHold.Accept(_T(GetString(IDS_PW_BREAK)));

		for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
		{

			MeshTopoData *ld = mMeshTopoData[ldID];

			if (fnGetTVSubMode() != TVEDGEMODE)
				TransferSelectionStart();
			if (fnGetTVSubMode() == TVEDGEMODE)
			{
				ld->BreakEdges(this);
			}
			else
			{
				ld->BreakVerts(this);
			}
			if (fnGetTVSubMode() != TVEDGEMODE)
				TransferSelectionEnd(FALSE,TRUE);			

			NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);	
			if (ip) ip->RedrawViews(ip->GetTime());
			InvalidateView();
		}

	}

}

void UnwrapMod::SnapPoint(Point3 &p)
{
	int i1,i2;
	GetUVWIndices(i1,i2);
	//	int ix, iy;
	float fx,fy;
	double dx,dy;
	//compute in pixel space
	//find closest whole pixel
	fx = (float) modf(( (float) p[i1] * (float) (bitmapWidth) ),&dx);
	fy = (float) modf(( (float) p[i2] * (float) (bitmapHeight) ),&dy);
	if (midPixelSnap)
	{
		//		if (fx > 0.5f) dx+=1.0f;
		//		if (fy > 0.5f) dy+=1.0f;
		dx += 0.5f;
		dy += 0.5f;
	}
	else
	{
		if (fx > 0.5f) dx+=1.0f;
		if (fy > 0.5f) dy+=1.0f;
	}
	//put back in UVW space
	p[i1] = (float)dx/(float)(bitmapWidth);
	p[i2] = (float)dy/(float)(bitmapHeight);
}

Point3 UnwrapMod::SnapPoint(Point3 snapPoint, MeshTopoData *snapLD, int snapIndex)
{
	int i1, i2;
	GetUVWIndices(i1,i2);
	suspendNotify = TRUE;
	Point3 p = snapPoint;

	//VSNAP
	if (gridSnap)
	{
		BOOL vSnap,eSnap, gSnap;
		pblock->GetValue(unwrap_vertexsnap,0,vSnap,FOREVER);
		pblock->GetValue(unwrap_edgesnap,0,eSnap,FOREVER);
		pblock->GetValue(unwrap_gridsnap,0,gSnap,FOREVER);

		BOOL snapped = FALSE;
		if ( vSnap  )
		{
			//convert to screen space
			//get our window width height
			float xzoom, yzoom;
			int width,height;
			ComputeZooms(hView,xzoom,yzoom,width,height);
			Point3 tvPoint = UVWToScreen(p,xzoom,yzoom,width,height);
			//look up that point in our list
			//get our pixel distance
			int gridStr = (int)(fnGetGridStr()*60.0f);
			int x,y;
			x = (int) tvPoint.x;
			y = (int) tvPoint.y;
			int startX, startY, endX, endY;

			int vertID = -1;
			MeshTopoData *hitLD = NULL;
			for (int i = 0; i < gridStr; i++)
			{

				startX = x - i;
				endX = x + i;

				startY = y - i;
				endY = y + i;

				if (startX < 0) startX = 0;
				if (startY < 0) startY = 0;

				if (endX < 0) endX = 0;
				if (endY < 0) endY = 0;

				if (startX >= width) startX = width-1;
				if (startY >= height) startY = height-1;

				if (endX >= width) endX = width-1;
				if (endY >= height) endY = height-1;
				for (int iy = startY; iy <= endY; iy++)
				{
					for (int ix = startX; ix <= endX; ix++)
					{
						int index = iy * width + ix;
						for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
						{
							MeshTopoData *ld = mMeshTopoData[ldID];
							int ID = ld->GetVertexSnapBuffer(index);//vertexSnapBuffer[index];
							if ((ID == snapIndex) && (ld == snapLD))
							{
							}
							else if ((ID != -1))
							{
								vertID = ID;
								hitLD = ld;
								ix = endX;
								iy = endY;
								i = gridStr;
							}
						}
					}
				}
			}

			if ((vertID != -1) && hitLD)
			{
				p = hitLD->GetTVVert(vertID);//TVMaps.v[vertID].p;
				snapped = TRUE;
			}
		}

		if ( eSnap && (!snapped)  )
		{

			//convert to screen space
			//get our window width height
			float xzoom, yzoom;
			int width,height;
			ComputeZooms(hView,xzoom,yzoom,width,height);
			Point3 tvPoint = UVWToScreen(p,xzoom,yzoom,width,height);
			//look up that point in our list
			//get our pixel distance
			int gridStr = (int)(fnGetGridStr()*60.0f);
			int x,y;
			x = (int) tvPoint.x;
			y = (int) tvPoint.y;
			int startX, startY, endX, endY;

			int egdeID = -1;
			MeshTopoData *hitLD = NULL;
			for (int i = 0; i < gridStr; i++)
			{

				startX = x - i;
				endX = x + i;

				startY = y - i;
				endY = y + i;

				if (startX < 0) startX = 0;
				if (startY < 0) startY = 0;

				if (endX < 0) endX = 0;
				if (endY < 0) endY = 0;

				if (startX >= width) startX = width-1;
				if (startY >= height) startY = height-1;

				if (endX >= width) endX = width-1;
				if (endY >= height) endY = height-1;
				for (int iy = startY; iy <= endY; iy++)
				{
					for (int ix = startX; ix <= endX; ix++)
					{
						int index = iy * width + ix;
						for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
						{
							
							int ID = mMeshTopoData[ldID]->GetEdgeSnapBuffer(index);

							if (ID >= 0)
							{
								if ((ID == snapIndex) && (mMeshTopoData[ldID] == snapLD))
								{
									//hit self skip
								}
								else  if (!mMeshTopoData[ldID]->GetEdgesConnectedToSnapvert(ID))//edgesConnectedToSnapvert[ID])
								{
									egdeID = ID;
									hitLD = mMeshTopoData[ldID];
									ix = endX;
									iy = endY;
									i = gridStr;
								}
							}
						}
					}
				}
			}

			if ((egdeID >= 0) && hitLD)
			{
				//          Point3 p(0.0f,0.0f,0.0f);

				int a,b;
				a = hitLD->GetTVEdgeVert(egdeID,0);//TVMaps.ePtrList[egdeID]->a;
				b = hitLD->GetTVEdgeVert(egdeID,1);//TVMaps.ePtrList[egdeID]->b;

				Point3 pa,pb;
				pa = hitLD->GetTVVert(a);//TVMaps.v[a].p;
				Point3 aPoint = UVWToScreen(pa,xzoom,yzoom,width,height);

				pb = hitLD->GetTVVert(b);//TVMaps.v[b].p;
				Point3 bPoint = UVWToScreen(pb,xzoom,yzoom,width,height);

				float screenFLength = Length(aPoint-bPoint);
				float screenSLength = Length(tvPoint-aPoint);
				float per = screenSLength/screenFLength;


				Point3 vec = (pb-pa) * per;
				p = pa + vec;

				snapped = TRUE;
			}
		}

		if ( (gSnap)  && (!snapped) )
		{


			float rem = fmod(p[i1],gridSize);
			float per = rem/gridSize;

			per = gridSize * fnGetGridStr();

			float snapPos ;
			if (p[i1] >= 0)
				snapPos = (int)((p[i1]+(gridSize*0.5f))/gridSize) * gridSize;
			else snapPos = (int)((p[i1]-(gridSize*0.5f))/gridSize) * gridSize;

			if ( fabs(p[i1] - snapPos) < per)
				p[i1] = snapPos;

			if (p[i2] >= 0)
				snapPos = (int)((p[i2]+(gridSize*0.5f))/gridSize) * gridSize;
			else snapPos = (int)((p[i2]-(gridSize*0.5f))/gridSize) * gridSize;

			if ( fabs(p[i2] -snapPos) < per)
				p[i2] = snapPos;


		}
	}

	if ((isBitmap) && (pixelSnap))
	{
		SnapPoint(p);
	}

	return p;

}
void UnwrapMod::MovePoints(Point2 pt)
{

	int i1, i2;
	GetUVWIndices(i1,i2);
	HoldPoints();	
	TimeValue t = GetCOREInterface()->GetTime();


	suspendNotify = TRUE;
	//VSNAP

	if (gridSnap)
	{
		BOOL vSnap,eSnap, gSnap;
		pblock->GetValue(unwrap_vertexsnap,0,vSnap,FOREVER);
		pblock->GetValue(unwrap_edgesnap,0,eSnap,FOREVER);
		pblock->GetValue(unwrap_gridsnap,0,gSnap,FOREVER);

		BOOL snapped = FALSE;

		if ( vSnap && (mMouseHitVert != -1) && mMouseHitLocalData)
		{
			Point3 p = mMouseHitLocalData->GetTVVert(mMouseHitVert);//TVMaps.v[mouseHitVert].p;

			p[i1] += pt.x;
			p[i2] += pt.y;

			//convert to screen space
			//get our window width height
			float xzoom, yzoom;
			int width,height;
			ComputeZooms(hView,xzoom,yzoom,width,height);
			Point3 tvPoint = UVWToScreen(p,xzoom,yzoom,width,height);
			//look up that point in our list
			//get our pixel distance
			int gridStr = (int)(fnGetGridStr()*60.0f);
			int x,y;
			x = (int) tvPoint.x;
			y = (int) tvPoint.y;
			int startX, startY, endX, endY;

			int vertID = -1;
			MeshTopoData *snapLD = NULL;
			for (int i = 0; i < gridStr; i++)
			{

				startX = x - i;
				endX = x + i;

				startY = y - i;
				endY = y + i;

				if (startX < 0) startX = 0;
				if (startY < 0) startY = 0;

				if (endX < 0) endX = 0;
				if (endY < 0) endY = 0;

				if (startX >= width) startX = width-1;
				if (startY >= height) startY = height-1;

				if (endX >= width) endX = width-1;
				if (endY >= height) endY = height-1;
				for (int iy = startY; iy <= endY; iy++)
				{
					for (int ix = startX; ix <= endX; ix++)
					{
						int index = iy * width + ix;
						for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
						{
							MeshTopoData *ld = mMeshTopoData[ldID];
							int ID = ld->GetVertexSnapBuffer(index);
							if ((ID != -1) )
							{
								if ((ld == mMouseHitLocalData) && (ID == mMouseHitVert))
								{
									//skip, dont want to snap to self
								}
								else
								{
									snapLD = ld;
									vertID = ID;
									ix = endX;
									iy = endY;
									i = gridStr;
								}
							}
						}
					}
				}
			}

			if ((vertID != -1) && snapLD)
			{
				Point3 p = snapLD->GetTVVert(vertID);//TVMaps.v[vertID].p;
				pt.x = p[i1] - mMouseHitLocalData->GetTVVert(mMouseHitVert)[i1];//TVMaps.v[mouseHitVert].p[i1];
				pt.y = p[i2] - mMouseHitLocalData->GetTVVert(mMouseHitVert)[i2];//TVMaps.v[mouseHitVert].p[i2];
				snapped = TRUE;
			}
		}

		if ( eSnap && (!snapped) && (mMouseHitVert != -1) && mMouseHitLocalData )
		{
			Point3 p = mMouseHitLocalData->GetTVVert(mMouseHitVert);//TVMaps.v[mouseHitVert].p;

			p[i1] += pt.x;
			p[i2] += pt.y;

			//convert to screen space
			//get our window width height
			float xzoom, yzoom;
			int width,height;
			ComputeZooms(hView,xzoom,yzoom,width,height);
			Point3 tvPoint = UVWToScreen(p,xzoom,yzoom,width,height);
			//look up that point in our list
			//get our pixel distance
			int gridStr = (int)(fnGetGridStr()*60.0f);
			int x,y;
			x = (int) tvPoint.x;
			y = (int) tvPoint.y;
			int startX, startY, endX, endY;

			int egdeID = -1;
			MeshTopoData *snapLD = NULL;
			for (int i = 0; i < gridStr; i++)
			{

				startX = x - i;
				endX = x + i;

				startY = y - i;
				endY = y + i;

				if (startX < 0) startX = 0;
				if (startY < 0) startY = 0;

				if (endX < 0) endX = 0;
				if (endY < 0) endY = 0;

				if (startX >= width) startX = width-1;
				if (startY >= height) startY = height-1;

				if (endX >= width) endX = width-1;
				if (endY >= height) endY = height-1;
				for (int iy = startY; iy <= endY; iy++)
				{
					for (int ix = startX; ix <= endX; ix++)
					{
						int index = iy * width + ix;
						for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
						{
							
							int ID = mMeshTopoData[ldID]->GetEdgeSnapBuffer(index);

							if (ID >= 0)
							{
								if ((ID == mMouseHitVert) && (mMeshTopoData[ldID] == mMouseHitLocalData))
								{
									//hit self skip
								}
								else if (!mMeshTopoData[ldID]->GetEdgesConnectedToSnapvert(ID))
								{
									snapLD = mMeshTopoData[ldID];
									egdeID = ID;
									ix = endX;
									iy = endY;
									i = gridStr;
								}
							}
						}
					}
				}
			}

			if (egdeID >= 0)
			{
				Point3 p(0.0f,0.0f,0.0f);

				int a,b;
				a = snapLD->GetTVEdgeVert(egdeID,0);//TVMaps.ePtrList[egdeID]->a;
				b = snapLD->GetTVEdgeVert(egdeID,1);//TVMaps.ePtrList[egdeID]->b;

				Point3 pa,pb;
				pa = snapLD->GetTVVert(a);//TVMaps.v[a].p;
				Point3 aPoint = UVWToScreen(pa,xzoom,yzoom,width,height);

				pb = snapLD->GetTVVert(b);//TVMaps.v[b].p;
				Point3 bPoint = UVWToScreen(pb,xzoom,yzoom,width,height);

				float screenFLength = Length(aPoint-bPoint);
				float screenSLength = Length(tvPoint-aPoint);
				float per = screenSLength/screenFLength;


				Point3 vec = (pb-pa) * per;
				p = pa + vec;


				pt.x = p[i1] - mMouseHitLocalData->GetTVVert(mMouseHitVert)[i1];//TVMaps.v[mouseHitVert].p[i1];
				pt.y = p[i2] - mMouseHitLocalData->GetTVVert(mMouseHitVert)[i2];//TVMaps.v[mouseHitVert].p[i2];
				snapped = TRUE;
			}
		}

		if ( (gSnap) && (mMouseHitVert != -1) && (!snapped) && mMouseHitLocalData)
		{
			
			Point3 p = mMouseHitLocalData->GetTVVert(mMouseHitVert);//TVMaps.v[mouseHitVert].p;

			p[i1] += pt.x;
			p[i2] += pt.y;

			float rem = fmod(p[i1],gridSize);
			float per = rem/gridSize;

			per = gridSize * fnGetGridStr();

			float snapPos ;
			if (p[i1] >= 0)
				snapPos = (int)((p[i1]+(gridSize*0.5f))/gridSize) * gridSize;
			else snapPos = (int)((p[i1]-(gridSize*0.5f))/gridSize) * gridSize;

			if ( fabs(p[i1] - snapPos) < per)
				p[i1] = snapPos;

			if (p[i2] >= 0)
				snapPos = (int)((p[i2]+(gridSize*0.5f))/gridSize) * gridSize;
			else snapPos = (int)((p[i2]-(gridSize*0.5f))/gridSize) * gridSize;

			if ( fabs(p[i2] -snapPos) < per)
				p[i2] = snapPos;

			pt.x = p[i1] - mMouseHitLocalData->GetTVVert(mMouseHitVert)[i1];//TVMaps.v[mouseHitVert].p[i1];
			pt.y = p[i2] - mMouseHitLocalData->GetTVVert(mMouseHitVert)[i2];//TVMaps.v[mouseHitVert].p[i2];
		}
	}


	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{

		MeshTopoData *ld = mMeshTopoData[ldID];
		if (ld == NULL)
		{
			DbgAssert(0);
			continue;
		}
		for (int i=0; i<ld->GetNumberTVVerts(); i++) {
			if (ld->GetTVVertSelected(i)) 
			{
				//check snap and bitmap
				Point3 p = ld->GetTVVert(i);
				p[i1] += pt.x;
				p[i2] += pt.y;
				if ((isBitmap) && (pixelSnap))
				{
					SnapPoint(p);
				}
				ld->SetTVVert(t,i,p,this);
			}
			else if((ld->GetTVVertInfluence(i) != 0.0f) && (fnGetTVSubMode() == TVVERTMODE))
			{
				//check snap and bitmap
				Point3 p = ld->GetTVVert(i);
				Point3 NewPoint = p;
				NewPoint[i1] += pt.x;
				NewPoint[i2] += pt.y;
				Point3 vec;
				vec = (NewPoint - p) * ld->GetTVVertInfluence(i);
				p += vec;
				if ((isBitmap) && (pixelSnap))
				{
					SnapPoint(p);
				}
				ld->SetTVVert(t,i,p,this);
			}
		}

	}
	suspendNotify = FALSE;
	if (update)
		NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);
	InvalidateView();
}
/*
void UnwrapMod::MoveGizmo(Point2 pt)
{
	int i1, i2;
	GetUVWIndices(i1,i2);


	HoldPoints();	
	TimeValue t = ip->GetTime();

	if (offsetControl) 
		offsetControl->GetValue(t,&gOffset,FOREVER);
	gOffset[i1] += pt.x;
	gOffset[i2] += pt.y;
	if (offsetControl) offsetControl->SetValue(t,&gOffset);

	NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);
	InvalidateView();
}
*/
void UnwrapMod::RotatePoints(HWND h, float ang)
{
	HoldPoints();	
	TimeValue t = GetCOREInterface()->GetTime();

	Point3 cent(0,0,0);


	if (centeron)
	{
		float xzoom, yzoom;
		int width,height;

		ComputeZooms(h,xzoom,yzoom,width,height);


		int tx = (width-int(xzoom))/2;
		int ty = (height-int(yzoom))/2;
		int i1, i2;
		GetUVWIndices(i1,i2);

		cent[i1] = (center.x-tx-xscroll)/xzoom;
		cent[i2] = (center.y+ty-yscroll - height)/-yzoom;
	}
	else
	{

		int ct = 0;
		Box3 bbox;

		bbox.Init();

		for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
		{

			MeshTopoData *ld = mMeshTopoData[ldID];
			if (ld == NULL)
			{
				DbgAssert(0);
				continue;
			}
			for (int i=0; i<ld->GetNumberTVVerts(); i++) 
			{
				if (ld->GetTVVertSelected(i)) {
					cent += ld->GetTVVert(i);
					bbox += ld->GetTVVert(i);
					ct++;
				}
			}
		}

		if (!ct) return;
		cent /= float(ct);
		cent = bbox.Center();
	}

	axisCenter.x = cent.x;
	axisCenter.y = cent.y;
	axisCenter.z = 0.0f;
	Matrix3 mat(1);	
	ang = GetCOREInterface()->SnapAngle(ang,FALSE);

	BOOL respectAspectRatio = rotationsRespectAspect;

	if (aspect == 1.0f)
		respectAspectRatio = FALSE;


	if (respectAspectRatio)
	{
		cent[uvw] *= aspect;
	}

	mat.Translate(-cent);

	currentRotationAngle = ang * 180.0f/PI;
	switch (uvw) {
		case 0: mat.RotateZ(ang); break;
		case 1: mat.RotateX(ang); break;
		case 2: mat.RotateY(ang); break;
	}
	mat.Translate(cent);

	suspendNotify = TRUE;

	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{

		MeshTopoData *ld = mMeshTopoData[ldID];
		if (ld == NULL)
		{
			DbgAssert(0);
			continue;
		}
		for (int i=0; i<ld->GetNumberTVVerts(); i++) {
			if (ld->GetTVVertSelected(i)) 
			{
				//check snap and bitmap
				Point3 p = ld->GetTVVert(i);
				if (respectAspectRatio)
				{
					
					p[uvw] *= aspect;
					p = mat * p;
					p[uvw] /= aspect;					

				}
				else 
					p = mat * p;

				if ((isBitmap) && (pixelSnap))
				{
					SnapPoint(p);
				}
				ld->SetTVVert(t,i,p,this);
			}
			else if ((ld->GetTVVertInfluence(i) != 0.0f) && (fnGetTVSubMode() == TVVERTMODE))
			{
				Point3 p = ld->GetTVVert(i);
				//check snap and bitmap
				Point3 NewPoint = ld->GetTVVert(i);
				NewPoint = mat * ld->GetTVVert(i);

				Point3 vec;
				vec = (NewPoint - p) * ld->GetTVVertInfluence(i);
				p += vec;

				if ((isBitmap) && (pixelSnap))
				{
					SnapPoint(p);
				}
				ld->SetTVVert(t,i,p,this);
			}
		}
	}

	suspendNotify = FALSE;
	if (update)
		NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);

	InvalidateView();

}

void UnwrapMod::RotateAroundAxis(HWND h, float ang, Point3 axis)
{
	HoldPoints();	
	TimeValue t = GetCOREInterface()->GetTime();

	Point3 cent(0,0,0);

	cent = axis;
	Matrix3 mat(1);	
	ang = GetCOREInterface()->SnapAngle(ang,FALSE);

	BOOL respectAspectRatio = rotationsRespectAspect;

	if (aspect == 1.0f)
		respectAspectRatio = FALSE;


	if (respectAspectRatio)
	{
		cent[uvw] *= aspect;
	}

	mat.Translate(-cent);

	currentRotationAngle = ang * 180.0f/PI;
	switch (uvw) 
	{
	case 0: mat.RotateZ(ang); break;
	case 1: mat.RotateX(ang); break;
	case 2: mat.RotateY(ang); break;
	}
	mat.Translate(cent);

	suspendNotify = TRUE;
	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];
		for (int i = 0; i < ld->GetNumberTVVerts(); i++)//TVMaps.v.Count(); i++) 
		{
			if (ld->GetTVVertSelected(i))//vsel[i]) 
			{
				Point3 p = ld->GetTVVert(i);
				//check snap and bitmap
				if (respectAspectRatio)
				{
					p[uvw] *= aspect;
					p = mat * p;
					p[uvw] /= aspect;
				}
				else p = mat * p;

				if ((isBitmap) && (pixelSnap))
				{
					SnapPoint(p);
				}
				ld->SetTVVert(t,i,p,this);
			}
			else if ((ld->GetTVVertInfluence(i) != 0.0f) && (fnGetTVSubMode() == TVVERTMODE))
			{
				//check snap and bitmap
				Point3 NewPoint = ld->GetTVVert(i);//TVMaps.v[i].p;
				Point3 op = NewPoint;
				NewPoint = mat * NewPoint;

				Point3 vec;
				vec = (NewPoint - op) * ld->GetTVVertInfluence(i);//TVMaps.v[i].influence;
				op += vec;

				if ((isBitmap) && (pixelSnap))
				{
					SnapPoint(op);
				}
				ld->SetTVVert(t,i,op,this);
			}

		}
	}
	suspendNotify = FALSE;
	if (update)
		NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);

	InvalidateView();

}

void UnwrapMod::ScalePoints(HWND h, float scale, int direction)
{

	HoldPoints();	
	TimeValue t = GetCOREInterface()->GetTime();

	Point3 cent(0,0,0);
	int i;

	int i1, i2;
	GetUVWIndices(i1,i2);

	if (centeron)
	{
		float xzoom, yzoom;
		int width,height;

		ComputeZooms(h,xzoom,yzoom,width,height);

		int tx = (width-int(xzoom))/2;
		int ty = (height-int(yzoom))/2;
		cent[i1] = (center.x-tx-xscroll)/xzoom;
		cent[i2] = (center.y+ty-yscroll - height)/-yzoom;
		cent.z = 0.0f;
	}
	else
	{
		int ct = 0;
		for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
		{

			MeshTopoData *ld = mMeshTopoData[ldID];
			if (ld == NULL)
			{
				DbgAssert(0);
				continue;
			}
			for (int i=0; i<ld->GetNumberTVVerts(); i++) 
			{
				if (ld->GetTVVertSelected(i) && !ld->GetTVVertDead(i)) 
				{
					cent += ld->GetTVVert(i);
					ct++;
				}
			}
		}
		if (!ct) return;
		cent /= float(ct);
	}

	Matrix3 mat(1);	
	mat.Translate(-cent);
	Point3 sc(1,1,1);

	if (direction == 0)
	{
		sc[i1] = scale;
		sc[i2] = scale;
	}
	else if (direction == 1)
	{
		sc[i1] = scale;
	}
	else if (direction == 2)
	{
		sc[i2] = scale;
	}


	axisCenter.x = cent.x;
	axisCenter.y = cent.y;
	axisCenter.z = 0.0f;

	mat.Scale(sc,TRUE);
	mat.Translate(cent);

	suspendNotify = TRUE;
	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];
		if (ld == NULL)
		{
			DbgAssert(0);
			continue;
		}
		for (i=0; i< ld->GetNumberTVVerts(); i++) 
		{
			if (ld->GetTVVertSelected(i))
			{
				Point3 p = ld->GetTVVert(i);
				//check snap and bitmap
				p = mat * p;
				if ((isBitmap) && (pixelSnap))
				{
					SnapPoint(p);
				}
				ld->SetTVVert(t,i,p,this);
			}
			else if ((ld->GetTVVertInfluence(i) != 0.0f) && (fnGetTVSubMode() == TVVERTMODE))
			{
				Point3 p = ld->GetTVVert(i);
				//check snap and bitmap
				Point3 NewPoint = ld->GetTVVert(i);
				NewPoint = mat * p;
				Point3 vec;
				vec = (NewPoint - p) * ld->GetTVVertInfluence(i);
				p += vec;

				if ((isBitmap) && (pixelSnap))
				{
					SnapPoint(p);
				}
				ld->SetTVVert(t,i,p,this);
			}
		}
	}
	suspendNotify = FALSE;
	if (update)
		NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);
	InvalidateView();

}



void UnwrapMod::ScalePointsXY(HWND h, float scaleX, float scaleY)
{

	HoldPoints();	
	TimeValue t = GetCOREInterface()->GetTime();

	Point3 cent(0,0,0);
	int i;

	float xzoom, yzoom;
	int width,height;

	ComputeZooms(h,xzoom,yzoom,width,height);

	int i1,i2;
	GetUVWIndices(i1,i2);

	int tx = (width-int(xzoom))/2;
	int ty = (height-int(yzoom))/2;
	cent[i1] = (center.x-tx-xscroll)/xzoom;
	cent[i2] = (center.y+ty-yscroll - height)/-yzoom;

	Matrix3 mat(1);	

	axisCenter.x = cent.x;
	axisCenter.y = cent.y;
	axisCenter.z = 0.0f;

	mat.Translate(-cent);
	Point3 sc(1,1,1);
	sc[i1] = scaleX;
	sc[i2] = scaleY;

	mat.Scale(sc,TRUE);
	mat.Translate(cent);

	suspendNotify = TRUE;
	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];
		if (ld)
		{
			for (i=0; i<ld->GetNumberTVVerts(); i++) {
				if (ld->GetTVVertSelected(i)) {
					Point3 p = ld->GetTVVert(i);
					//check snap and bitmap
					p = mat * p;
					if ((isBitmap) && (pixelSnap))
					{
						SnapPoint(p);
					}
					ld->SetTVVert(t,i,p,this);
				}
				else if(ld->GetTVVertInfluence(i) != 0.0f)
				{
					Point3 p = ld->GetTVVert(i);
					Point3 NewPoint = p;
					NewPoint = mat * p;
					Point3 vec;
					vec = (NewPoint - p) * ld->GetTVVertInfluence(i);
					p += vec;

					//check snap and bitmap
					if ((isBitmap) && (pixelSnap))
					{
						SnapPoint(p);
					}
					ld->SetTVVert(t,i,p,this);
				}
			}
		}
	}
	suspendNotify = FALSE;

	NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);
	InvalidateView();

}



void UnwrapMod::ScaleAroundAxis(HWND h, float scaleX, float scaleY, Point3 axis)
{

	HoldPoints();	
	TimeValue t = GetCOREInterface()->GetTime();

	Point3 cent(0,0,0);
	int i;

	float xzoom, yzoom;
	int width,height;

	ComputeZooms(h,xzoom,yzoom,width,height);

	int i1,i2;
	GetUVWIndices(i1,i2);

	cent = axis;

	Matrix3 mat(1);	
	mat.Translate(-cent);
	Point3 sc(1,1,1);
	sc[i1] = scaleX;
	sc[i2] = scaleY;

	mat.Scale(sc,TRUE);
	mat.Translate(cent);

	suspendNotify = TRUE;
	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];
		if (ld)
		{
			for (i = 0; i < ld->GetNumberTVVerts(); i++) 
			{
				if (ld->GetTVVertSelected(i))//vsel[i]) 
				{
					//check snap and bitmap
					Point3 p = ld->GetTVVert(i);
					p = mat * p;
					if ((isBitmap) && (pixelSnap))
					{
						SnapPoint(p);
					}

					ld->SetTVVert(t,i,p,this);
				}
				else if ( ld->GetTVVertInfluence(i) != 0.0f)
				{
					//check snap and bitmap
					//			TVMaps.v[i].p = mat * TVMaps.v[i].p;
					Point3 NewPoint = ld->GetTVVert(i);//TVMaps.v[i].p;
					Point3 p = NewPoint;
					NewPoint = mat * p;
					Point3 vec;
					vec = (NewPoint - p) * ld->GetTVVertInfluence(i);//TVMaps.v[i].influence;
					p += vec;

					if ((isBitmap) && (pixelSnap))
					{
						SnapPoint(p);
					}

					ld->SetTVVert(t,i,p,this);

				}
			}
		}
	}
	suspendNotify = FALSE;

	NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);
	InvalidateView();

}


void UnwrapMod::DetachEdgeVerts(BOOL hold)
{

	if (hold)
	{
		theHold.Begin();
		HoldPointsAndFaces();	
	}

	//convert our sub selection type to vertex selection
	if (fnGetTVSubMode() != TVFACEMODE)
		TransferSelectionStart();

	//we differeniate between faces and verts/edges since with edges we can just split along a seam
	//versus detaching a whole set of faces
	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];
		if (fnGetTVSubMode() == TVFACEMODE)
		{			
			//get our shared verts
			BitArray selVerts;
			BitArray unSelVerts;
			BitArray fsel = ld->GetFaceSelection();

			selVerts.SetSize(ld->GetNumberTVVerts());//TVMaps.v.Count());
			unSelVerts.SetSize(ld->GetNumberTVVerts());//TVMaps.v.Count());
			selVerts.ClearAll();
			unSelVerts.ClearAll();
			//loop though our face selection get an array of vertices that the face selection uses
			//loop through our non selected verts get an array of those vertices
			for (int i = 0; i < ld->GetNumberFaces(); i++)//TVMaps.f.Count(); i++)
			{
				int deg = ld->GetFaceDegree(i);//TVMaps.f[i]->count;
				for (int j = 0; j < deg; j++)
				{
					int index = ld->GetFaceTVVert(i,j);//TVMaps.f[i]->t[j];
					if (fsel[i])
						selVerts.Set(index,TRUE);
					else unSelVerts.Set(index,TRUE);
					if (ld->GetFaceHasVectors(i))//TVMaps.f[i]->vecs)
					{
						int index = ld->GetFaceTVHandle(i, j*2);//TVMaps.f[i]->vecs->handles[j*2];
						if (index != -1)
						{
							if (fsel[i])
								selVerts.Set(index,TRUE);
							else unSelVerts.Set(index,TRUE);
						}
						index = ld->GetFaceTVHandle(i, j*2+1);//TVMaps.f[i]->vecs->handles[j*2+1];
						if (index != -1)
						{
							if (fsel[i])
								selVerts.Set(index,TRUE);
							else unSelVerts.Set(index,TRUE);
						}
						index = ld->GetFaceTVInterior(i, j);//TVMaps.f[i]->vecs->interiors[j];
						if (index != -1)
						{
							if (fsel[i])
								selVerts.Set(index,TRUE);
							else unSelVerts.Set(index,TRUE);
						}
					}

				}
			}

			//loop through for matching verts they are shared
			//create clone of those
			//store a look up
			Tab<int> oldToNewIndex;
			oldToNewIndex.SetCount(ld->GetNumberTVVerts());//TVMaps.v.Count());
			for (int i = 0; i < oldToNewIndex.Count(); i++)
			{
				oldToNewIndex[i] = -1;
				if (selVerts[i] && unSelVerts[i])
				{
					Point3 p = ld->GetTVVert(i);//TVMaps.v[i].p;
					int newIndex = ld->AddTVVert(0,p, this);//AddUVWPoint(p);
					oldToNewIndex[i] = newIndex;
				}
			}

			//go back and fix and faces that use the look up vertices
			for (int i = 0; i <ld->GetNumberFaces(); i++)//TVMaps.f.Count(); i++)
			{
				if (fsel[i])
				{
					int deg = ld->GetFaceDegree(i);//TVMaps.f[i]->count;
					for (int j = 0; j < deg; j++)
					{
						int index = ld->GetFaceTVVert(i,j);//TVMaps.f[i]->t[j];					
						int newIndex = oldToNewIndex[index];
						if (newIndex != -1)
							ld->SetFaceTVVert(i,j,newIndex);
//							TVMaps.f[i]->t[j] = newIndex;
						if (ld->GetFaceHasVectors(i))//TVMaps.f[i]->vecs)
						{
							int index = ld->GetFaceTVHandle(i,j*2);//TVMaps.f[i]->vecs->handles[j*2];
							if (index != -1)
							{
								newIndex = oldToNewIndex[index];
								if (newIndex != -1)
									ld->SetFaceTVHandle(i,j*2,newIndex);
//									TVMaps.f[i]->vecs->handles[j*2] = newIndex;
							}
							index = ld->GetFaceTVHandle(i,j*2+1);//TVMaps.f[i]->vecs->handles[j*2+1];
							if (index != -1)
							{
								newIndex = oldToNewIndex[index];
								if (newIndex != -1)
									ld->SetFaceTVHandle(i,j*2+1,newIndex);
//									TVMaps.f[i]->vecs->handles[j*2+1] = newIndex;
							}
							index = ld->GetFaceTVInterior(i,j);//TVMaps.f[i]->vecs->interiors[j];
							if (index != -1)
							{
								newIndex = oldToNewIndex[index];
								if (newIndex != -1)
									ld->SetFaceTVInterior(i,j,newIndex);
//									TVMaps.f[i]->vecs->interiors[j] = newIndex;
							}
						}
					}
				}
			}			
		}
		else
		{



			//convert our selectoin to faces and then back to verts	
			//this will clean out any invalid vertices
			BitArray fsel = ld->GetFaceSelection();
			BitArray vsel = ld->GetTVVertSelection();
			BitArray holdFace(fsel);
			ld->GetFaceSelFromVert(fsel,FALSE);	
			ld->SetFaceSelection(fsel);
			ld->GetVertSelFromFace(vsel);
			ld->SetTVVertSelection(vsel);
			fsel = holdFace;



			//loop through verts 
			for (int i=0; i<ld->GetNumberTVVerts(); i++)//TVMaps.v.Count(); i++) 
			{
				//check if selected
				if (vsel[i])
				{
					//if selected loop through faces that have this vert
					BOOL first = TRUE;
					int newID = -1;
					Point3 p;

					first = TRUE;
					for (int j=0; j<ld->GetNumberFaces();j++)//TVMaps.f.Count(); j++) 
					{
						//if this vert is not selected  create a new vert and point it to it
						int ct = 0;
						int whichVert=-1;
						int degree = ld->GetFaceDegree(j);
						for (int k =0; k < degree; k++)
						{
							int id;
							id = ld->GetFaceTVVert(j,k);//TVMaps.f[j]->t[k];
							if (vsel[id]) 
								ct++;
							if (id == i)
							{
								whichVert = k;
								p = ld->GetTVVert(id);//TVMaps.v[id].p;
							}
						}
						//this face contains the vert
						if (whichVert != -1)
						{
							//checkif all selected;
							if (ct!=degree)//TVMaps.f[j]->count)
							{
								if (first)
								{
									first = FALSE;
									ld->AddTVVert(0,p, j, whichVert, this, FALSE);
									vsel = ld->GetTVVertSelection();
									newID = ld->GetFaceTVVert(j,whichVert);//TVMaps.f[j]->t[whichVert];
								}
								else
								{
									ld->SetFaceTVVert(j,whichVert,newID);
									//TVMaps.f[j]->t[whichVert] = newID;
								}
							}
						}
					}
					//now do handles if need be
					first = TRUE;
					BOOL removeIsolatedFaces = FALSE;
					int isoVert = -1;
					for (int j=0; j<ld->GetNumberFaces(); j++)//TVMaps.f.Count(); j++) 
					{
						if ( ld->GetFaceHasVectors(j))//(TVMaps.f[j]->flags & FLAG_CURVEDMAPPING) && (TVMaps.f[j]->vecs) )
						{
							int whichVert = -1;
							int ct = 0;
							int degree = ld->GetFaceDegree(j);
							for (int k =0; k < degree*2;k++)
							{
								int id;
								id = ld->GetFaceTVHandle(j,k);//TVMaps.f[j]->vecs->handles[k];
								if (vsel[id]) 
									ct++;
								if (id == i)
								{
									whichVert = k;
									p = ld->GetTVVert(id);//TVMaps.v[id].p;
								}
							}
							//this face contains the vert
							if (whichVert != -1)
							{
								//checkif all selected;
								//if owner selected break
								if (ct!=degree*2)
								{
									if (first)
									{
										first = FALSE;
										isoVert = ld->GetFaceTVHandle(j,whichVert);//TVMaps.f[j]->vecs->handles[whichVert];
										ld->AddTVHandle(0,p, j, whichVert, this, FALSE);
										vsel = ld->GetTVVertSelection();
										newID = ld->GetFaceTVHandle(j,whichVert);//TVMaps.f[j]->vecs->handles[whichVert];
										removeIsolatedFaces = TRUE;
									}
									else
									{
										ld->SetFaceTVHandle(j,whichVert,newID);
//										TVMaps.f[j]->vecs->handles[whichVert] = newID;
									}
								}
							}

						}
					}

					if (removeIsolatedFaces)
					{
						BOOL hit = FALSE;
						for (int j=0; j<ld->GetNumberFaces(); j++)//TVMaps.f.Count(); j++) 
						{
							int degree = ld->GetFaceDegree(j);
							for (int k =0; k < degree*2;k++)
							{
								int id;
								id = ld->GetFaceTVHandle(j,k);//TVMaps.f[j]->vecs->handles[k];
								if (id == isoVert)
								{
									hit = TRUE;
								}
							}
						}
						if (!hit)
						{
							ld->DeleteTVVert(isoVert,this);
//							TVMaps.v[isoVert].flags |= FLAG_DEAD;
							vsel.Set(isoVert,FALSE);
						}
					}
				}
			}
			
		}	

		ld->BuildTVEdges();
		ld->BuildVertexClusterList();
		
	}

	//convert our sub selection type to vertex selection
	if (fnGetTVSubMode() != TVFACEMODE)
		TransferSelectionEnd(FALSE,TRUE);

	if (hold)
		theHold.Accept(_T(GetString(IDS_PW_DETACH)));


	NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);
	if (ip) ip->RedrawViews(ip->GetTime());
	InvalidateView();


}
void UnwrapMod::FlipPoints(int direction)
{
	//loop through faces
	theHold.Begin();

	HoldPointsAndFaces();	
	theHold.Accept(_T(GetString(IDS_PW_FLIP)));


	DetachEdgeVerts(FALSE);
	MirrorPoints( direction,FALSE);

	NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);
	if (ip) ip->RedrawViews(ip->GetTime());
	InvalidateView();


}

void UnwrapMod::MirrorPoints( int direction,BOOL hold )
{

	TimeValue t = GetCOREInterface()->GetTime();


	if (hold)
	{
		theHold.SuperBegin();
		theHold.Begin();
		HoldPoints();	
	}

	//convert our sub selection type to vertex selection
	TransferSelectionStart();

	int ct = 0;
	Box3 bounds;
	bounds.Init();
	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		int i;

		MeshTopoData *ld = mMeshTopoData[ldID];

		BitArray vsel = ld->GetTVVertSelection();
		for (i=0; i<ld->GetNumberTVVerts(); i++)//TVMaps.v.Count(); i++) 
		{
			if (vsel[i]) 
			{
				bounds += ld->GetTVVert(i);//TVMaps.v[i].p;
				ct++;
			}
		}
	}
	if (!ct) 
	{
		if (hold)
		{
			theHold.Cancel();
			theHold.SuperCancel();
		}
		return;
	}
	Point3 cent(0,0,0);
	cent = bounds.Center();

	Matrix3 mat(1);	
	mat.Translate(-cent);
	Point3 sc(1.0f,1.0f,1.0f);
	int i1, i2;
	GetUVWIndices(i1,i2);
	if (direction == 0)
	{
		sc[i1] = -1.0f;
	}
	else if (direction == 1)
	{
		sc[i2] = -1.0f;
	}

	//	sc[i1] = scale;
	//	sc[i2] = scale;

	mat.Scale(sc,TRUE);
	mat.Translate(cent);

	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		int i;

		MeshTopoData *ld = mMeshTopoData[ldID];

		BitArray vsel = ld->GetTVVertSelection();

		for (i=0; i< ld->GetNumberTVVerts(); i++)//TVMaps.v.Count(); i++) 
		{
			if (vsel[i]) 
			{
//				if (TVMaps.cont[i]) 
//					TVMaps.cont[i]->GetValue(t,&TVMaps.v[i].p,FOREVER);

				//check snap and bitmap
				Point3 p = mat * ld->GetTVVert(i);//TVMaps.v[i].p;
				if ((isBitmap) && (pixelSnap))
				{
					SnapPoint(p);
				}

				ld->SetTVVert(t,i,p,this);
//				if (TVMaps.cont[i]) 
//					TVMaps.cont[i]->SetValue(t,&TVMaps.v[i].p);
			}
		}
	}

	if (hold)
	{
		theHold.Accept(_T(GetString(IDS_TH_MIRROR)));
		theHold.SuperAccept(_T(GetString(IDS_TH_MIRROR)));
	}

	//put back our old vertex selection if need be
	TransferSelectionEnd(FALSE,FALSE);


	NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);
	if (ip) ip->RedrawViews(ip->GetTime());
	InvalidateView();

}

void UnwrapMod::UpdateListBox()
{

	int ct = pblock->Count(unwrap_texmaplist);

	for (int i = 0; i < ct; i++)
	{
		Texmap *map;
		pblock->GetValue(unwrap_texmaplist,0,map,FOREVER,i);
		if(map)
		{
			int id = -1;
			pblock->GetValue(unwrap_texmapidlist,0,id,FOREVER,i);
		}
	}
	SendMessage(hTextures, CB_RESETCONTENT, 0, 0);

	HDC hdc = GetDC(hTextures);
	Rect rect;
	GetClientRectP( hTextures, &rect );
	SIZE size;
	int width = rect.w();

	dropDownListIDs.ZeroCount();
	if (ct > 0)
	{
		Texmap *map;
		int i = 0;
		pblock->GetValue(unwrap_texmaplist,0,map,FOREVER,i);
		SendMessage(hTextures, CB_ADDSTRING , 0, (LPARAM) (TCHAR*) map->GetFullName());		

		DLGetTextExtent(hdc, map->GetFullName(), &size);
		if ( size.cx > width ) 
			width = size.cx;

		dropDownListIDs.Append(1,&i,100);
	}

	for (int i = 1; i < ct; i++)
	{
		Texmap *map;
		pblock->GetValue(unwrap_texmaplist,0,map,FOREVER,i);
		if (map != NULL) 
		{
			if (matid == -1)
			{
				SendMessage(hTextures, CB_ADDSTRING , 0, (LPARAM) (TCHAR*) map->GetFullName());
				dropDownListIDs.Append(1,&i,100);

				DLGetTextExtent(hdc, map->GetFullName(), &size);
				if ( size.cx > width ) 
					width = size.cx;
			}
			else
			{
				int id = -1;
				pblock->GetValue(unwrap_texmapidlist,0,id,FOREVER,i);
				if (filterMatID[matid] == id)
				{
					SendMessage(hTextures, CB_ADDSTRING , 0, (LPARAM) (TCHAR*) map->GetFullName());
					dropDownListIDs.Append(1,&i,100);

					DLGetTextExtent(hdc, map->GetFullName(), &size);
					if ( size.cx > width ) 
						width = size.cx;

				}
			}
		}
	}

	SendMessage(hTextures, CB_ADDSTRING, 0, (LPARAM)_T("---------------------"));	
	SendMessage(hTextures, CB_ADDSTRING, 0, (LPARAM)GetString(IDS_PW_PICK));	
	SendMessage(hTextures, CB_ADDSTRING, 0, (LPARAM)GetString(IDS_PW_REMOVE));	
	SendMessage(hTextures, CB_ADDSTRING, 0, (LPARAM)GetString(IDS_PW_RESET));	
	SendMessage(hTextures, CB_ADDSTRING, 0, (LPARAM)_T("---------------------"));

	if (CurrentMap > ct )
	{
		if (ct > 1)
			CurrentMap = 1;
		else CurrentMap = 0;
	}
	SendMessage(hTextures, CB_SETCURSEL, CurrentMap, 0 );

	ReleaseDC(hTextures,hdc); 
	if ( width > 0 ) 
		SendMessage(hTextures, CB_SETDROPPEDWIDTH, width+5, 0);

}




void UnwrapMod::ShowCheckerMaterial(BOOL show)
{


//	MyEnumProc dep;              
//	DoEnumDependents(&dep);
//	if (dep.Nodes.Count() > 0)
	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		INode *SelfNode = mMeshTopoData.GetNode(ldID);//dep.Nodes[0];

		macroRecorder->Disable();
		if (SelfNode)
		{
			Interface *ip = GetCOREInterface();
			TimeValue t = ip->GetTime();
			Mtl *checkerMat = GetCheckerMap();
			Mtl *storedBaseMtl = NULL;
			if (ldID < pblock->Count(unwrap_originalmtl_list))
			{
				pblock->GetValue(unwrap_originalmtl_list,0,storedBaseMtl,FOREVER,ldID);
				Mtl *currentBaseMtl = SelfNode->GetMtl();
				if ((show) && (checkerMat))
				{
					//copy the node material into the ref 101
					if (currentBaseMtl != checkerMat)
						pblock->SetValue(unwrap_originalmtl_list,0,currentBaseMtl,ldID);

					//set the checker to the material
					SelfNode->SetMtl(checkerMat);
					//turn it on
					ip->ActivateTexture(checkerMat->GetSubTexmap(1), checkerMat, 1);

				}
				else 
				{
					if (checkerMat)
						ip->DeActivateTexture(checkerMat->GetSubTexmap(1), checkerMat, 1);
					//copy the original material back into the node
					checkerWasShowing = FALSE;
					if (currentBaseMtl == checkerMat)
					{
						SelfNode->SetMtl((Mtl*)storedBaseMtl);
						checkerWasShowing = TRUE;
					}
					Mtl *nullMat = NULL;
					pblock->SetValue(unwrap_originalmtl_list,0,nullMat,ldID);
				}
				ip->RedrawViews(t);
			}
		}
		macroRecorder->Enable();

	}

}

void UnwrapMod::AddMaterial(MtlBase *mtl, BOOL update)
{

	if (mtl)
	{
		if (mtl->ClassID() == Class_ID(0x243e22c6, 0x63f6a014)) //gnormal material
		{
			if (mtl->GetSubTexmap(0) != NULL)
				mtl = mtl->GetSubTexmap(0);
		}

		int ct = pblock->Count(unwrap_texmaplist);
		for (int i = 0; i < ct; i++)
		{
			Texmap *map;
			pblock->GetValue(unwrap_texmaplist,0,map,FOREVER,i);
			if (map == mtl) 
			{
				return;
			}
		}
		AddToMaterialList((Texmap*) mtl, -1);

	}

	CurrentMap = pblock->Count(unwrap_texmaplist) -1;
	UpdateListBox();


}

void UnwrapMod::PickMap()
{	
	BOOL newMat=FALSE, cancel=FALSE;
	MtlBase *mtl = GetCOREInterface()->DoMaterialBrowseDlg(
		hWnd,
		BROWSE_MAPSONLY|BROWSE_INCNONE|BROWSE_INSTANCEONLY,
		newMat,cancel);
	if (cancel) {
		if (newMat) mtl->MaybeAutoDelete();
		return;
	}

	if (mtl != NULL)
	{
		TSTR mstr = GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap4.AddMap"));
		macroRecorder->FunctionCall(mstr, 1, 0,
			mr_reftarg,mtl);

		AddMaterial(mtl);
	}
}

UBYTE *RenderBitmap(Bitmap *bmp,int w, int h)
{
	AColor col;
	//	SCTex sc;
	//	int scanw = ByteWidth(w*3);
	int scanw = ByteWidth(w);
	//	UBYTE *image = new UBYTE[ByteWidth(w*3)*h];
	UBYTE *image = new UBYTE[ByteWidth(w)*h];
	UBYTE *p1;

	//	sc.scale = 1.0f;
	//	sc.duvw = Point3(du,dv,0.0f);
	//	sc.dpt  = sc.duvw;
	//	sc.uvw.y = 1.0f-0.5f*dv;

	BMM_Color_64 color;
	for (int j=0; j<h; j++) {
		//		sc.scrPos.y = j;
		//		sc.uvw.x = 0.5f*du;				
		p1 = image + (h-j-1)*scanw;
		for (int i=0; i<w; i++) {
			bmp->GetPixels(i,j,1,&color);

			*p1++ = (UBYTE)(color.b>>8);
			*p1++ = (UBYTE)(color.g>>8);
			*p1++ = (UBYTE)(color.r>>8);	

		}		
	}
	return image;
}


void UnwrapMod::SetupImage()
{
	delete image; image = NULL;
	if (GetActiveMap()) {		
		iw = rendW;
		ih = rendH;
		aspect = 1.0f;
		//		Class_ID bid = Class_ID(BMTEX_CLASS_ID);
		Bitmap *bmp = NULL;
		if (GetActiveMap()->ClassID() == Class_ID(BMTEX_CLASS_ID,0) )
		{
			isBitmap = 1;
			BitmapTex *bmt;
			bmt = (BitmapTex *) GetActiveMap();
			bmp = bmt->GetBitmap(GetCOREInterface()->GetTime());
			if (bmp!= NULL)
			{
				if (useBitmapRes)
				{
					bitmapWidth = bmp->Width();
					bitmapHeight = bmp->Height();
					iw = bitmapWidth;
					ih = bitmapHeight;
					aspect = (float)bitmapWidth/(float)bitmapHeight;
				}
				else	
				{
					bitmapWidth = iw;
					bitmapHeight = ih;

					aspect = (float)iw/(float)ih;
				}


			}
		}
		else
		{
			aspect = (float)iw/(float)ih;
			isBitmap = 0;
		}
		if (iw==0 || ih==0) return;
		GetActiveMap()->Update(GetCOREInterface()->GetTime(), FOREVER);
		GetActiveMap()->LoadMapFiles(GetCOREInterface()->GetTime());
		SetCursor(LoadCursor(NULL,IDC_WAIT));
		//		if (isBitmap)
		//			image = RenderTexMap(map[CurrentMap],bitmapWidth,bitmapHeight);
		//		else 
		if (GetActiveMap()->ClassID() == Class_ID(BMTEX_CLASS_ID,0) )
		{
			if (bmp != NULL)
			{
				if (useBitmapRes)
					image = RenderBitmap(bmp,iw,ih);
				else image = RenderTexMap(GetActiveMap(),iw,ih,GetShowImageAlpha());
			}
		}
		else image = RenderTexMap(GetActiveMap(),iw,ih,GetShowImageAlpha());
		SetCursor(LoadCursor(NULL,IDC_ARROW));
		InvalidateView();
	}
	if ((image) && (iShowMap)) iShowMap->Enable();
	else if (iShowMap) iShowMap->Disable();
	tileValid = FALSE;
}


int UnwrapMod::GetAxis()
{
	return 2;
	return axis;
}





void UnwrapMod::ZoomExtents()
{

	for (int k = 0; k < 2; k++)
	{
		Rect brect;
		Point2 pt;
		float xzoom, yzoom;
		int width,height;
		ComputeZooms(hView,xzoom,yzoom,width,height);	
		brect.SetEmpty();
		
		for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
		{
			MeshTopoData *ld = mMeshTopoData[ldID];
			for (int i = 0; i < ld->GetNumberTVVerts(); i++)//TVMaps.v.Count(); i++) 
			{		
				if (!ld->GetTVVertDead(i))//!(TVMaps.v[i].flags & FLAG_DEAD))
				{
					if (ld->IsTVVertVisible(i))
					{
						pt = UVWToScreen(ld->GetTVVert(i)/*GetPoint(t,i)*/,xzoom,yzoom,width,height);
						IPoint2 ipt(int(pt.x),int(pt.y));
						brect += ipt;		
					}
				}
			}
		}

		if (brect.IsEmpty()) return;

		if  (  (brect.w() == 1) || (brect.h() == 1) )
		{
			pt = UVWToScreen(Point3(0.0f,0.0f,0.0f),xzoom,yzoom,width,height);
			IPoint2 ipt(int(pt.x),int(pt.y));
			brect += ipt;		

			pt = UVWToScreen(Point3(1.0f,1.0f,1.0f),xzoom,yzoom,width,height);
			IPoint2 ipt2(int(pt.x),int(pt.y));
			brect += ipt2;		

		}

		Rect srect;
		GetClientRect(hView,&srect);

		float rat1, rat2;
		double bw,bh;
		double sw,sh;

		if (brect.w() == 1) 
		{

			brect.left--;
			brect.right++;
		}

		if (brect.h() == 1) 
		{

			brect.top--;
			brect.bottom++;
		}


		bw = brect.w();
		bh = brect.h();

		sw = srect.w();
		sh = srect.h();




		rat1 = float(sw-1.0f)/float(fabs(double(bw-1.0f)));
		rat2 = float(sh-1.0f)/float(fabs(double(bh-1.0f)));
		float rat = (rat1<rat2?rat1:rat2) * 0.9f;

		BOOL redo = FALSE;
		if (_isnan(rat))
		{
			rat = 1.0f;
			redo = TRUE;
		}
		if (!_finite(rat)) 
		{
			rat = 1.0f;
			redo = TRUE;
		}

		zoom *= rat;



		IPoint2 delta = srect.GetCenter() - brect.GetCenter();
		xscroll += delta.x;
		yscroll += delta.y;
		xscroll *= rat;
		yscroll *= rat;
	}




	InvalidateView();

}

void UnwrapMod::FrameSelectedElement()
{



	Rect brect;
	Point2 pt;
	float xzoom, yzoom;
	int width,height;
	ComputeZooms(hView,xzoom,yzoom,width,height);	
	brect.SetEmpty();
	int found = 0;
	BOOL doAll = TRUE;


	TransferSelectionStart();

	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];
		BitArray vsel = ld->GetTVVertSelection();
		if (vsel.NumberSet() != 0) 
			doAll = FALSE;
	}

	

	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];

		BitArray faceHasSelectedVert;

		BitArray vsel = ld->GetTVVertSelection();
		BitArray tempVSel = vsel;

		faceHasSelectedVert.SetSize(ld->GetNumberFaces());//TVMaps.f.Count());
		faceHasSelectedVert.ClearAll();

		int count = -1;

		while (count != vsel.NumberSet())
		{
			count = vsel.NumberSet();
			for (int i = 0; i < ld->GetNumberFaces(); i++)//TVMaps.f.Count();i++)
			{
				if (!ld->GetFaceDead(i))//(TVMaps.f[i]->flags & FLAG_DEAD))
				{
					int pcount = 3;
					pcount = ld->GetFaceDegree(i);//TVMaps.f[i]->count;
					int totalSelected = 0;
					for (int k = 0; k < pcount; k++)
					{
						int index = ld->GetFaceTVVert(i,k);//TVMaps.f[i]->t[k];
						if (vsel[index])
						{
							totalSelected++;
						}
					}

					if ( (totalSelected != pcount) && (totalSelected!= 0))
					{
						faceHasSelectedVert.Set(i);
					}
				}
			}
			for (int i = 0; i < ld->GetNumberFaces(); i++)//TVMaps.f.Count();i++)
			{
				if (faceHasSelectedVert[i])
				{
					int pcount = 3;
					pcount = ld->GetFaceDegree(i);//TVMaps.f[i]->count;
					for (int k = 0; k < pcount; k++)
					{
						int index = ld->GetFaceTVVert(i,k);//TVMaps.f[i]->t[k];
						vsel.Set(index,1);
					}
				}

			}
		}
		

		for (int i=0; i < ld->GetNumberTVVerts(); i++)//TVMaps.v.Count(); i++) 
		{		
			if (!ld->GetTVVertDead(i))//(TVMaps.v[i].flags & FLAG_DEAD))
			{
				if ((vsel[i]) || doAll)
				{
					pt = UVWToScreen(ld->GetTVVert(i)/*GetPoint(t,i)*/,xzoom,yzoom,width,height);
					IPoint2 ipt(int(pt.x),int(pt.y));
					brect += ipt;		
					found++;
				}
			}
		}
	}

//	vsel = tempVSel;
	TransferSelectionEnd(FALSE,FALSE);

	if (brect.w() < 5)
	{
		brect.left -= 5;
		brect.right += 5;
	}

	if (brect.h() < 5)
	{
		brect.top -= 5;
		brect.bottom += 5;
	}


	if (found <=1) return;
	Rect srect;
	GetClientRect(hView,&srect);		
	float rat1 = 1.0f, rat2 = 1.0f;
	if (brect.w()>2.0f )
		rat1 = float(srect.w()-1)/float(fabs(double(brect.w()-1)));
	if (brect.h()>2.0f )
		rat2 = float(srect.h()-1)/float(fabs(double(brect.h()-1)));
	float rat = (rat1<rat2?rat1:rat2) * 0.9f;
	float tempZoom = zoom *rat;
	if ( tempZoom <= 1000.0f)
	{
		zoom *= rat;
		IPoint2 delta = srect.GetCenter() - brect.GetCenter();
		xscroll += delta.x;
		yscroll += delta.y;
		xscroll *= rat;
		yscroll *= rat;	
		InvalidateView();
	}

}

void UnwrapMod::fnFrameSelectedElement()
{
	if (iZoomExt) 		
		iZoomExt->SetCurFlyOff(2,TRUE);
	FrameSelectedElement();
}

void UnwrapMod::ZoomSelected()
{
	TransferSelectionStart();

	Rect brect;
	Point2 pt;
	float xzoom, yzoom;
	int width,height;
	ComputeZooms(hView,xzoom,yzoom,width,height);	
	brect.SetEmpty();
	int found = 0;
	BOOL doAll = TRUE;
	
	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];
		BitArray vsel = ld->GetTVVertSelection();
		if (vsel.NumberSet() != 0) 
			doAll = FALSE;
	}

	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];
		for (int i = 0; i < ld->GetNumberTVVerts(); i++)//TVMaps.v.Count(); i++) 
		{	
			if (!ld->GetTVVertDead(i))//(TVMaps.v[i].flags & FLAG_DEAD))
			{
				if ((ld->GetTVVertSelected(i)/*vsel[i]*/) || doAll)
				{
					pt = UVWToScreen(ld->GetTVVert(i)/*GetPoint(t,i)*/,xzoom,yzoom,width,height);
					IPoint2 ipt(int(pt.x),int(pt.y));
					brect += ipt;		
					found++;
				}
			}
		}
	}

	TransferSelectionEnd(FALSE,FALSE);

	if (brect.w() < 5)
	{
		brect.left -= 5;
		brect.right += 5;
	}

	if (brect.h() < 5)
	{
		brect.top -= 5;
		brect.bottom += 5;
	}


	if (found <=1) return;
	Rect srect;
	GetClientRect(hView,&srect);		
	float rat1 = 1.0f, rat2 = 1.0f;
	if (brect.w()>2.0f )
		rat1 = float(srect.w()-1)/float(fabs(double(brect.w()-1)));
	if (brect.h()>2.0f )
		rat2 = float(srect.h()-1)/float(fabs(double(brect.h()-1)));
	float rat = (rat1<rat2?rat1:rat2) * 0.9f;
	float tempZoom = zoom *rat;
	if ( tempZoom <= 1000.0f)
	{
		zoom *= rat;
		IPoint2 delta = srect.GetCenter() - brect.GetCenter();
		xscroll += delta.x;
		yscroll += delta.y;
		xscroll *= rat;
		yscroll *= rat;	
		InvalidateView();
	}
}



void UnwrapMod::RebuildFreeFormData()
{
	//compute your zooms and scale
	float xzoom, yzoom;
	int width,height;
	ComputeZooms(hView,xzoom,yzoom,width,height);

	int count = 0;

	TransferSelectionStart();

	count = 0;//vsel.NumberSet();

	
	freeFormBounds.Init();
	if (!inRotation)
		selCenter = Point3(0.0f,0.0f,0.0f);
	int i1, i2;
	GetUVWIndices(i1, i2);

	for (int ldID =0; ldID < mMeshTopoData.Count(); ldID++)
	{
		MeshTopoData *ld = mMeshTopoData[ldID];
		int vselCount = ld->GetNumberTVVerts();//vsel.GetSize();
		for (int i = 0; i < vselCount; i++)
		{
			if (ld->GetTVVertSelected(i))//vsel[i])
			{
				//get bounds
				Point3 p(0.0f,0.0f,0.0f);
				Point3 tv = ld->GetTVVert(i);
				p[i1] = tv[i1];
				p[i2] = tv[i2];
				//			p.z = 0.0f;
				freeFormBounds += p;
				count++;
			}
		}
	}
	Point3 tempCenter;
	if (!inRotation)
		selCenter = freeFormBounds.Center();			
	else tempCenter = freeFormBounds.Center();			

	if (count > 0)
	{	

		//draw gizmo bounds
		Point2 prect[4];
		prect[0] = UVWToScreen(freeFormBounds.pmin,xzoom,yzoom,width,height);
		prect[1] = UVWToScreen(freeFormBounds.pmax,xzoom,yzoom,width,height);
		float xexpand = 15.0f/xzoom;
		float yexpand = 15.0f/yzoom;

		if (!freeFormMode->dragging)
		{
			if ((prect[1].x-prect[0].x) < 30)
			{
				prect[1].x += 15;
				prect[0].x -= 15;
				//expand bounds
				freeFormBounds.pmax.x += xexpand;
				freeFormBounds.pmin.x -= xexpand;
			}
			if ((prect[0].y-prect[1].y) < 30)
			{
				prect[1].y -= 15;
				prect[0].y += 15;
				freeFormBounds.pmax.y += yexpand;
				freeFormBounds.pmin.y -= yexpand;

			}
		}
	}
	TransferSelectionEnd(FALSE,FALSE);

}
//--- Mouse procs for modes -----------------------------------------------

int SelectMode::proc(HWND hWnd, int msg, int point, int flags, IPoint2 m)
{
	switch (msg) 
	{
		case MOUSE_POINT:
		{
			if (point==0) 
			{
				// First click

				region   = FALSE;
				toggle   = flags&MOUSE_CTRL;
				subtract = flags&MOUSE_ALT;

				// Hit test
				Tab<TVHitData> hits;
				Rect rect;
				rect.left = m.x-2;
				rect.right = m.x+2;
				rect.top = m.y-2;
				rect.bottom = m.y+2;
				// First hit test sel only
				mod->centeron = 0;
				if (toggle && subtract)
				{
					mod->centeron = 1;
					return subproc(hWnd,msg,point,flags,m);
				}

				// First hit test sel only
				if  ( ((!toggle && !subtract && mod->HitTest(rect,hits,TRUE)) || (mod->lockSelected==1)) ||
					((mod->freeFormSubMode != ID_TOOL_SELECT) && (mod->mode == ID_FREEFORMMODE)) )
				{
					return subproc(hWnd,msg,point,flags,m);
				} else
					// Next hit test everything
					if (mod->HitTest(rect,hits,subtract)) 
					{
						theHold.Begin();
						mod->HoldSelection();
						theHold.Accept(_T(GetString(IDS_PW_SELECT_UVW)));

						theHold.Suspend();
						if (!toggle && !subtract) 
							mod->ClearSelect();
						mod->Select(hits,toggle,subtract,FALSE);
						mod->InvalidateView();
						theHold.Resume();

						if (mod->showVerts)					
						{
							mod->NotifyDependents(FOREVER,PART_DISPLAY ,REFMSG_CHANGE);
							if (mod->ip) mod->ip->RedrawViews(mod->ip->GetTime());
						}
						if (mod->mode == ID_FREEFORMMODE)
						{
							mod->RebuildFreeFormData();
							mod->HitTest(rect,hits,FALSE);
							if ((toggle || subtract) && (mod->freeFormSubMode!=ID_TOOL_SCALE)) 
								return FALSE;
						}
						else
						{
							if (toggle || subtract) return FALSE;
						}
						return subproc(hWnd,msg,point,flags,m);
					} 
					else 
					{					
						region = TRUE;
						lm = om = m;
						XORDottedRect(hWnd,om,m);
					}				
			} else 
			{
				// Second click
				if (region) 
				{
					Rect rect;
					if ((mod->mode == ID_UNWRAP_WELD) ||
						(mod->mode == ID_TOOL_WELD))
					{
						rect.left = om.x-2;
						rect.right = om.x+2;
						rect.top = om.y-2;
						rect.bottom = om.y+2;
					}
					else
					{					
						rect.left   = om.x;
						rect.top    = om.y;
						rect.right  = m.x;
						rect.bottom = m.y;
						rect.Rectify();					
					}
					Tab<TVHitData> hits;
					theHold.Begin();
					mod->HoldSelection();
					theHold.Accept(_T(GetString(IDS_PW_SELECT_UVW)));
					theHold.Suspend();
					if (!toggle && !subtract) 
						mod->ClearSelect();
					if (mod->HitTest(rect,hits,subtract)) {						
						mod->Select(hits,FALSE,subtract,TRUE);											
					}
					theHold.Resume();

					if (mod->showVerts)					
					{
						mod->NotifyDependents(FOREVER,PART_DISPLAY ,REFMSG_CHANGE);
						if (mod->ip) mod->ip->RedrawViews(mod->ip->GetTime());
					}

					mod->InvalidateView();
				} 
				else 
				{
					return subproc(hWnd,msg,point,flags,m);
				}
			}
			break;
		}

		case MOUSE_MOVE:
		{
			SetCursor(LoadCursor(NULL, IDC_ARROW));
			if (region) 
			{
				XORDottedRect(hWnd,om,lm);
				XORDottedRect(hWnd,om,m);
				lm = m;
			} 
			else 
			{
				SetCursor(GetXFormCur());
				return subproc(hWnd,msg,point,flags,m);
			}
			break;
		}


		case MOUSE_FREEMOVE: 
		{

			Tab<TVHitData> hits;
			Rect rect;
			rect.left = m.x-2;
			rect.right = m.x+2;
			rect.top = m.y-2;
			rect.bottom = m.y+2;

			if ((flags&MOUSE_CTRL) &&  (flags&MOUSE_ALT))
				GetCOREInterface()->ReplacePrompt( GetString(IDS_PW_MOUSE_CENTER));
			else if (flags&MOUSE_CTRL)
				GetCOREInterface()->ReplacePrompt( GetString(IDS_PW_MOUSE_ADD));
			else if (flags&MOUSE_ALT)
				GetCOREInterface()->ReplacePrompt( GetString(IDS_PW_MOUSE_SUBTRACT));
			else if (flags&MOUSE_SHIFT)
				GetCOREInterface()->ReplacePrompt( GetString(IDS_PW_MOUSE_CONSTRAIN));
			else GetCOREInterface()->ReplacePrompt( GetString(IDS_PW_MOUSE_SELECTTV));



			if (mod->HitTest(rect,hits,FALSE)) 
			{
				{
					if (mod->mode == ID_FREEFORMMODE)
					{
						SetCursor(GetXFormCur());
					}
					else if (mod->fnGetTVSubMode() == TVVERTMODE)
					{
						MeshTopoData *ld = NULL;
						int vID = -1;			
						if (hits.Count())
						{
							vID = hits[0].mID;
							int ldID = hits[0].mLocalDataID;
							ld = mod->GetMeshTopoData(ldID);
						}

						if (ld && ld->GetTVVertSelected(vID)) 
						{
							SetCursor(GetXFormCur());
						} 
						else 
						{
							SetCursor(mod->selCur);
						}		
					}

					else if (mod->fnGetTVSubMode() == TVEDGEMODE)
					{
						MeshTopoData *ld = NULL;
						int vID = -1;			
						if (hits.Count())
						{
							vID = hits[0].mID;
							int ldID = hits[0].mLocalDataID;
							ld = mod->GetMeshTopoData(ldID);
						}

						if (ld && ld->GetTVEdgeSelected(vID)) 
						{
							SetCursor(GetXFormCur());
						} 
						else 
						{
							SetCursor(mod->selCur);
						}

					}

					else if (mod->fnGetTVSubMode() == TVFACEMODE)
					{
						hits.ZeroCount();
						mod->HitTest(rect,hits,TRUE);
						if (hits.Count() ) 
						{
							int fID = hits[0].mID;
							int ldID = hits[0].mLocalDataID;
							MeshTopoData *ld = mod->GetMeshTopoData(ldID);
							if (ld && ld->GetFaceSelected(fID))
								SetCursor(GetXFormCur());
							else
								SetCursor(mod->selCur);
						} 
						else 
						{
							SetCursor(mod->selCur);
						}
					}
				} 
			}
			else 
			{
				SetCursor(LoadCursor(NULL, IDC_ARROW));
			}
			return subproc(hWnd,msg,point,flags,m);
		}
		case MOUSE_ABORT:
		{
			if (region) 
			{
				InvalidateRect(hWnd,NULL,FALSE);
			} 
			else 
			{
				return subproc(hWnd,msg,point,flags,m);
			}
			break;
		}
	}
	return 1;
}

int PaintSelectMode::proc(HWND hWnd, int msg, int point, int flags, IPoint2 m)
{

static IPoint2 lastm;
static int lastRadius;

switch (msg) {
case MOUSE_POINT:
	if (point==0) {
		// First click

		toggle   = flags&MOUSE_CTRL;
		subtract = flags&MOUSE_ALT;

		// Hit test
		Tab<TVHitData> hits;
		Rect rect;
		rect.left   = m.x-mod->fnGetPaintSize();
		rect.top    = m.y-mod->fnGetPaintSize();
		rect.right  = m.x+mod->fnGetPaintSize();
		rect.bottom = m.y+mod->fnGetPaintSize();
		// First hit test sel only
		// Next hit test everything
		//hold the selection
		theHold.Begin();
		mod->HoldSelection();//theHold.Put (new TSelRestore (mod));

		if (!toggle && !subtract) 
		{
			mod->ClearSelect();
			mod->InvalidateView();
			UpdateWindow(mod->hWnd);
		}

		if (mod->HitTest(rect,hits,subtract)) 
		{
			mod->Select(hits,toggle,subtract,TRUE);
			mod->InvalidateView();
			if (mod->fnGetSyncSelectionMode()) 
			{
				mod->fnSyncGeomSelection();
			}
			else UpdateWindow(mod->hWnd);
		}
		om = m;
	} 
	else 
	{
		mod->NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
		if (mod->ip) mod->ip->RedrawViews(mod->ip->GetTime());
		theHold.Accept(_T(GetString(IDS_PW_SELECT_UVW)));
	}
	break;			

case MOUSE_MOVE:
	{
		float len = Length(om-m);
		if (len > ((float)mod->fnGetPaintSize()*2.0f))
		{
			int ct = (len/(mod->fnGetPaintSize()*2.0f));
			Point2 start, end,vec;
			start.x = (float)om.x;
			start.y = (float)om.y;
			end.x = (float)m.x;
			end.y = (float)m.y;
			vec = (end-start)/ct;
			Point2 current;
			current = start;
			toggle   = flags&MOUSE_CTRL;
			subtract = flags&MOUSE_ALT;
			BOOL redraw = FALSE;
			for (int i =0; i < (ct+1); i++)
			{
				m.x = (int)current.x;
				m.y = (int)current.y;
				Rect rect;
				rect.left   = m.x-mod->fnGetPaintSize();
				rect.top    = m.y-mod->fnGetPaintSize();
				rect.right  = m.x+mod->fnGetPaintSize();
				rect.bottom = m.y+mod->fnGetPaintSize();
				rect.Rectify();					
				Tab<TVHitData> hits;
				if (mod->HitTest(rect,hits,subtract)) 
				{										
					mod->Select(hits,FALSE,subtract,TRUE);		
					redraw = TRUE;
				}
				current += vec;
			}	
			if (redraw)
			{
				mod->InvalidateView();
				if (mod->fnGetSyncSelectionMode()) 
				{
					mod->fnSyncGeomSelection();
				}
				else UpdateWindow(mod->hWnd);
			}
		}
		else
		{
			Rect rect;
			rect.left   = m.x-mod->fnGetPaintSize();
			rect.top    = m.y-mod->fnGetPaintSize();
			rect.right  = m.x+mod->fnGetPaintSize();
			rect.bottom = m.y+mod->fnGetPaintSize();
			rect.Rectify();					
			Tab<TVHitData> hits;
			toggle   = flags&MOUSE_CTRL;
			subtract = flags&MOUSE_ALT;
			if (mod->HitTest(rect,hits,subtract)) 
			{										
				mod->Select(hits,FALSE,subtract,TRUE);		
				mod->InvalidateView();
				UpdateWindow(mod->hWnd);
			}
			mod->fnSyncGeomSelection();
		}

		om = m;

		IPoint2 r = lastm;
		if (!first)
		{				
			r.x += lastRadius;
			XORDottedCircle(hWnd, lastm,r);
		}
		first = FALSE;

		r = m;
		r.x += mod->fnGetPaintSize();
		XORDottedCircle(hWnd, m,r);

		lastm = m;
		lastRadius = mod->fnGetPaintSize();


		//		mod->NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
		//		mod->ip->RedrawViews(mod->ip->GetTime());

	}
	break;
case MOUSE_FREEMOVE: 
	{
		IPoint2 r = lastm;
		if (!first)
		{				
			r.x += lastRadius;
			XORDottedCircle(hWnd, lastm,r);
		}
		first = FALSE;

		r = m;
		r.x += mod->fnGetPaintSize();
		XORDottedCircle(hWnd, m,r);

		lastm = m;
		lastRadius = mod->fnGetPaintSize();;			
	}

case MOUSE_ABORT:
	{
		//cancel the hold
		theHold.Restore();
		theHold.Cancel();
		break;
	}
	}
return 1;
}


int MoveMode::subproc(HWND hWnd, int msg, int point, int flags, IPoint2 m)
{
switch (msg) 
{
case MOUSE_POINT:
	if (point==0) 
	{
		//VSNAP
		mod->BuildSnapBuffer();
		mod->PlugControllers();

		theHold.SuperBegin();
		theHold.Begin();
		mod->HoldPoints();
		om = m;
		//convert our sub selection type to vertex selection
		mod->tempVert = Point3(0.0f,0.0f,0.0f);
	} 
	else 
	{

		TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap2.MoveSelected"));
		macroRecorder->FunctionCall(mstr, 1, 0,
			mr_point3,&mod->tempVert);

		if (mod->tempVert == Point3(0.0f,0.0f,0.0f))
		{
			theHold.Cancel();
			theHold.SuperCancel();
		}
		else
		{
			theHold.Accept(_T(GetString(IDS_PW_MOVE_UVW)));
			theHold.SuperAccept(_T(GetString(IDS_PW_MOVE_UVW)));
		}


		mod->NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);

		if (mod->ip) mod->ip->RedrawViews(mod->ip->GetTime());
	}
	break;

case MOUSE_MOVE: {
	theHold.Restore();
	float xzoom, yzoom;
	int width, height;
	IPoint2 delta = m-om;
	if (flags&MOUSE_SHIFT && mod->move==0) {
		if (abs(delta.x) > abs(delta.y)) delta.y = 0;
		else delta.x = 0;
	} else if (mod->move==1) {
		delta.y = 0;
	} else if (mod->move==2) {
		delta.x = 0;
	}
	mod->ComputeZooms(hWnd,xzoom,yzoom,width,height);
	Point2 mv;
	mv.x = delta.x/xzoom;
	mv.y = -delta.y/yzoom;
	//check if moving points or gizmo

	mod->tempVert.x = mv.x;
	mod->tempVert.y = mv.y;
	mod->tempVert.z = 0.0f;

	mod->TransferSelectionStart();
	mod->MovePoints(mv);
	mod->TransferSelectionEnd(FALSE,FALSE);



	if (mod->update && mod->ip) mod->ip->RedrawViews(mod->ip->GetTime());
	UpdateWindow(hWnd);
	break;		
				 }


case MOUSE_ABORT:
	theHold.Cancel();
	theHold.SuperCancel();
	if (mod->fnGetConstantUpdate())
		mod->NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);
	if (mod->ip) mod->ip->RedrawViews(mod->ip->GetTime());
	mod->InvalidateView();
	break;
}
return 1;
}

#define ZOOM_FACT	0.01f
#define ROT_FACT	DegToRad(0.5f)

int RotateMode::subproc(HWND hWnd, int msg, int point, int flags, IPoint2 m)
{
	switch (msg) {
case MOUSE_POINT:
	if (point==0) {

		mod->PlugControllers();

		theHold.SuperBegin();
		theHold.Begin();
		mod->HoldPoints();
		mod->center.x = (float) m.x;
		mod->center.y = (float) m.y;
		mod->tempCenter.x = mod->center.x;
		mod->tempCenter.y = mod->center.y;
		om = m;


	} else {

		float angle = float(m.y-om.y)*ROT_FACT;
		mod->tempHwnd = hWnd;
		if (mod->centeron)
		{
			TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap2.RotateSelected"));
			macroRecorder->FunctionCall(mstr, 2, 0,
			mr_float,angle,
			mr_point3,&mod->axisCenter);
		}
		else
		{
			TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap2.RotateSelectedCenter"));
			macroRecorder->FunctionCall(mstr, 1, 0,
			mr_float,angle);
		}
		macroRecorder->EmitScript();

		if (angle == 0.0f)
		{
			theHold.Cancel();
			theHold.SuperCancel();
		}
		else
		{
			theHold.Accept(_T(GetString(IDS_PW_ROTATE_UVW)));
			theHold.SuperAccept(_T(GetString(IDS_PW_ROTATE_UVW)));
		}
		mod->NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);
		if (mod->ip) mod->ip->RedrawViews(mod->ip->GetTime());
	}
	break;

case MOUSE_MOVE:
	theHold.Restore();

	//convert our sub selection type to vertex selection
	mod->TransferSelectionStart();

	mod->RotatePoints(hWnd,float(m.y-om.y)*ROT_FACT);
	mod->TransferSelectionEnd(FALSE,FALSE);

	if (mod->update && mod->ip) mod->ip->RedrawViews(mod->ip->GetTime());
	UpdateWindow(hWnd);
	break;		

case MOUSE_ABORT:

	theHold.Cancel();
	theHold.SuperCancel();
	if (mod->fnGetConstantUpdate())
		mod->NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);
	if (mod->ip) mod->ip->RedrawViews(mod->ip->GetTime());
	mod->InvalidateView();
	break;
	}
	return 1;
}

int ScaleMode::subproc(HWND hWnd, int msg, int point, int flags, IPoint2 m)
{

	switch (msg) {
case MOUSE_POINT:
	if (point==0) {

		mod->PlugControllers();

		theHold.SuperBegin();
		theHold.Begin();
		mod->HoldPoints();
		mod->center.x = (float) m.x;
		mod->center.y = (float) m.y;
		om = m;
		mod->tempCenter.x = mod->center.x;
		mod->tempCenter.y = mod->center.y;
		mod->tempAmount = 1.0f;



	} else {
		mod->tempHwnd = hWnd;
		if (mod->centeron)
		{
			TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap2.ScaleSelected"));
			macroRecorder->FunctionCall(mstr, 3, 0,
				mr_float,mod->tempAmount,
				mr_int,mod->tempDir,
				mr_point3,&mod->tempCenter);
		}
		else
		{
			TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap2.ScaleSelectedCenter"));
			macroRecorder->FunctionCall(mstr, 2, 0,
				mr_float,mod->tempAmount,
				mr_int,mod->tempDir);
		}
		macroRecorder->EmitScript();

		if (mod->tempAmount == 1.0f)
		{
			theHold.Cancel();
			theHold.SuperCancel();
		}
		else
		{
			theHold.Accept(_T(GetString(IDS_PW_SCALE_UVW)));
			theHold.SuperAccept(_T(GetString(IDS_PW_SCALE_UVW)));
		}
		mod->NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);
		if (mod->ip) mod->ip->RedrawViews(mod->ip->GetTime());
	}
	break;

case MOUSE_MOVE: {
	theHold.Restore();
	IPoint2 delta = om-m;
	int direction = 0;
	if (flags&MOUSE_SHIFT ){
		if (abs(delta.x) > abs(delta.y)) 
		{
			delta.y = 0;
			direction = 1;
		}
		else 
		{	
			delta.x = 0;
			direction = 2;
		}
	}
	else if (mod->scale > 0)
	{
		if (mod->scale == 1) 
		{
			delta.y = 0;
			direction = 1;
		}
		else if (mod->scale == 2) 
		{	
			delta.x = 0;
			direction = 2;
		}

	}

	float z = 0.0f;
	if (direction == 0)
	{
		if (delta.y<0)
			z = (1.0f/(1.0f-ZOOM_FACT*delta.y));
		else z = (1.0f+ZOOM_FACT*delta.y);
	}
	else if (direction == 1)
	{
		if (delta.x<0)
			z = (1.0f/(1.0f-ZOOM_FACT*delta.x));
		else z = (1.0f+ZOOM_FACT*delta.x);

	}
	else if (direction == 2)
	{
		if (delta.y<0)
			z = (1.0f/(1.0f-ZOOM_FACT*delta.y));
		else z = (1.0f+ZOOM_FACT*delta.y);
	}

	if (mod->ip)
		z = GetCOREInterface()->SnapPercent(z);

	mod->tempDir = direction;
	mod->tempAmount = z;


	mod->TransferSelectionStart();
	mod->ScalePoints(hWnd, z,direction);
	mod->TransferSelectionEnd(FALSE,FALSE);

	if (mod->update &&mod->ip ) mod->ip->RedrawViews(mod->ip->GetTime());
	UpdateWindow(hWnd);
	break;
				 }

case MOUSE_ABORT:

	theHold.Cancel();
	theHold.SuperCancel();
	if (mod->fnGetConstantUpdate())
		mod->NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);
	if (mod->ip) mod->ip->RedrawViews(mod->ip->GetTime());
	mod->InvalidateView();
	break;
	}
	return 1;
}

int FreeFormMode::subproc(HWND hWnd, int msg, int point, int flags, IPoint2 m)
{

	static float tempXScale = 0.0f;
	static float tempYScale = 0.0f;

	static float xLength = 0.0f;
	static float yLength = 0.0f;

	static float angle = 0.0f;

	static float tempCenterX = 0.0f;
	static float tempCenterY = 0.0f;

	static float tempXLength = 0.0f;
	static float tempYLength = 0.0f;


	static float xAltLength = 0.0f;
	static float yAltLength = 0.0f;

	static float xScreenPivot = 0.0f;
	static float yScreenPivot = 0.0f;


	switch (msg) {
case MOUSE_POINT:
	if (point==0) 
	{
		mod->tempVert = Point3(0.0f,0.0f,0.0f);
		dragging = TRUE;
		if (mod->freeFormSubMode == ID_TOOL_MOVE)
		{
			//VSNAP
			mod->BuildSnapBuffer();
			mod->PlugControllers();

			theHold.SuperBegin();
			theHold.Begin();
			mod->HoldPoints();
			om = m;
		}
		else if (mod->freeFormSubMode == ID_TOOL_ROTATE)
		{
			mod->PlugControllers();

			mod->inRotation = TRUE;
			theHold.SuperBegin();
			theHold.Begin();


			om = m;
			mod->center.x =  mod->freeFormPivotScreenSpace.x;
			mod->center.y =  mod->freeFormPivotScreenSpace.y;

			mod->tempCenter.x = mod->center.x;
			mod->tempCenter.y = mod->center.y;
			mod->centeron = TRUE;
			mod->origSelCenter = mod->selCenter;
		}
		else if (mod->freeFormSubMode == ID_TOOL_SCALE)
		{
			mod->PlugControllers();

			theHold.SuperBegin();
			theHold.Begin();
			theHold.Put(new UnwrapPivotRestore(mod));
			float cx = mod->freeFormCornersScreenSpace[mod->scaleCornerOpposite].x;
			float cy = mod->freeFormCornersScreenSpace[mod->scaleCornerOpposite].y;
			float tx = mod->freeFormCornersScreenSpace[mod->scaleCorner].x;
			float ty = mod->freeFormCornersScreenSpace[mod->scaleCorner].y;
			tempXLength = tx -cx;
			tempYLength = ty -cy;

			xAltLength = tx-mod->freeFormPivotScreenSpace.x;
			yAltLength = ty-mod->freeFormPivotScreenSpace.y;

			xScreenPivot = mod->freeFormPivotScreenSpace.x;
			yScreenPivot = mod->freeFormPivotScreenSpace.y;


			mod->center.x =  cx;
			mod->center.y =  cy;
			tempCenterX = cx;
			tempCenterY = cy;
			om = m;
			mod->tempCenter.x = mod->center.x;
			mod->tempCenter.y = mod->center.y;
			Point3 originalPt = mod->selCenter + mod->freeFormPivotOffset;
		}
		else if (mod->freeFormSubMode == ID_TOOL_MOVEPIVOT)
		{
			theHold.Begin();
			theHold.Put(new UnwrapPivotRestore(mod));
			om = m;
			mod->origSelCenter = mod->selCenter;
		}


	} 
	else 
	{
		mod->inRotation = FALSE;
		dragging = FALSE;

		if (mod->freeFormSubMode == ID_TOOL_MOVE)
		{

			TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap2.MoveSelected"));
			macroRecorder->FunctionCall(mstr, 1, 0,
				mr_point3,&mod->tempVert);

			if (mod->tempVert == Point3(0.0f,0.0f,0.0f))
			{
				theHold.Cancel();
				theHold.SuperCancel();
			}
			else
			{
				theHold.Accept(_T(GetString(IDS_PW_MOVE_UVW)));
				theHold.SuperAccept(_T(GetString(IDS_PW_MOVE_UVW)));
			}
		}
		else if (mod->freeFormSubMode == ID_TOOL_SCALE)
		{
			TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap2.ScaleSelectedXY"));
			macroRecorder->FunctionCall(mstr, 3, 0,
				mr_float,tempXScale,
				mr_float,tempYScale,
				mr_point3,&mod->axisCenter);

			theHold.Accept(_T(GetString(IDS_PW_SCALE_UVW)));
			theHold.SuperAccept(_T(GetString(IDS_PW_SCALE_UVW)));
		}
		else if (mod->freeFormSubMode == ID_TOOL_ROTATE)
		{
			TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap2.RotateSelected"));
			macroRecorder->FunctionCall(mstr, 2, 0,
				mr_float,angle,
				mr_point3,&mod->axisCenter);

			if (angle == 0.0f)
			{
				theHold.Cancel();
				theHold.SuperCancel();
			}
			else
			{

				theHold.Accept(_T(GetString(IDS_PW_ROTATE_UVW)));
				theHold.SuperAccept(_T(GetString(IDS_PW_ROTATE_UVW)));
			}
			//recompute pivot point
			Box3 bounds;
			bounds.Init();
			for (int ldID = 0; ldID < mod->GetMeshTopoDataCount(); ldID++)
			{
				MeshTopoData *ld = mod->GetMeshTopoData(ldID);
				if (ld==NULL)
				{
					DbgAssert(0);
				}
				else
				{
					int vselCount = ld->GetNumberTVVerts();

					int i1,i2;
					mod->GetUVWIndices(i1,i2);
					for (int i = 0; i < vselCount; i++)
					{
						if (ld->GetTVVertSelected(i))
						{
							//get bounds
							Point3 p = Point3(0.0f,0.0f,0.0f);
							p[i1] = ld->GetTVVert(i)[i1];
							p[i2] = ld->GetTVVert(i)[i2];
							bounds += p;
						}
					}
				}
			}

			Point3 originalPt = (mod->selCenter+mod->freeFormPivotOffset);
			mod->freeFormPivotOffset = originalPt - bounds.Center();

		}
		else if (mod->freeFormSubMode == ID_TOOL_MOVEPIVOT)
		{

			TSTR mstr = mod->GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap.setPivotOffset"));
			macroRecorder->FunctionCall(mstr, 1, 0,
				mr_point3,&mod->freeFormPivotOffset);

			theHold.Accept(_T(GetString(IDS_PW_PIVOTRESTORE)));
		}
		mod->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
		if (mod->ip) mod->ip->RedrawViews(mod->ip->GetTime());
		mod->InvalidateView();
	}
	break;

case MOUSE_MOVE: {

	if (mod->freeFormSubMode == ID_TOOL_MOVE)
	{
		theHold.Restore();

		float xzoom, yzoom;
		int width, height;
		IPoint2 delta = m-om;
		if (flags&MOUSE_SHIFT && mod->move==0) {
			if (abs(delta.x) > abs(delta.y)) delta.y = 0;
			else delta.x = 0;
		} else if (mod->move==1) {
			delta.y = 0;
		} else if (mod->move==2) {
			delta.x = 0;
		}
		mod->ComputeZooms(hWnd,xzoom,yzoom,width,height);
		Point2 mv;
		mv.x = delta.x/xzoom;
		mv.y = -delta.y/yzoom;
		//check if moving points or gizmo

		mod->tempVert.x = mv.x;
		mod->tempVert.y = mv.y;
		mod->tempVert.z = 0.0f;

		//convert our sub selection type to vertex selection
		mod->TransferSelectionStart();

		mod->MovePoints(mv);

		//convert our sub selection type to current selection
		mod->TransferSelectionEnd(FALSE,FALSE);


	}

	else if (mod->freeFormSubMode == ID_TOOL_ROTATE)
	{
		theHold.Restore();
		Point3 vecA, vecB;
		Point3 a,b,center;
		a.x = (float) om.x;
		a.y = (float) om.y;
		a.z = 0.f;

		b.x = (float) m.x;
		b.y = (float) m.y;
		b.z = 0.0f;
		center.x = mod->freeFormPivotScreenSpace.x;
		center.y = mod->freeFormPivotScreenSpace.y;
		center.z = 0.0f;

		vecA = Normalize(a - center);
		vecB = Normalize(b - center);
		Point3 cross = CrossProd(vecA,vecB);
		float dot = DotProd(vecA,vecB);
		if (dot >= 1.0f)
		{
			angle = 0.0f;
		}
		else
		{
			if (cross.z < 0.0f)
				angle = acos(DotProd(vecA,vecB));
			else angle = -acos(DotProd(vecA,vecB));
		}

		if (flags&MOUSE_ALT )
		{
			angle = floor(angle * 180.0f/PI) * PI/180.0f;
		}
		else if (flags&MOUSE_CTRL )
		{
			int iangle = (int) floor(angle * 180.0f/PI);
			int addOffset = iangle % 5;
			angle = (float)(iangle - addOffset) * PI/180.0f;
		}

		int i1,i2;
		mod->GetUVWIndices(i1,i2);
		if ((i1==0) && (i2==2)) angle *= -1.0f;
		//convert our sub selection type to vertex selection
		mod->TransferSelectionStart();

		mod->RotatePoints(hWnd,angle);
		//convert our sub selection type to current selection
		mod->TransferSelectionEnd(FALSE,FALSE);

	}
	else if (mod->freeFormSubMode == ID_TOOL_SCALE)
	{
		theHold.Restore();
		IPoint2 delta = om-m;
		int direction = 0;
		float xScale = 1.0f, yScale = 1.0f;

		if (flags&MOUSE_ALT )
		{
			xLength = xAltLength;
			yLength = yAltLength;
		}
		else
		{
			xLength = tempXLength;
			yLength = tempYLength;
		}

		if (flags&MOUSE_SHIFT ){
			if (abs(delta.x) > abs(delta.y)) 
			{
				delta.y = 0;
				direction = 1;
			}
			else 
			{	
				delta.x = 0;
				direction = 2;
			}
		}


		if (direction == 0)
		{
			if (xLength != 0.0f)
				xScale = 1.0f- (delta.x/xLength);
			else xScale = 0.0f;
			if (yLength != 0.0f)
				yScale = 1.0f- (delta.y/yLength);
			else yScale = 0.0f;
		}
		else if (direction == 1)
		{
			if (xLength != 0.0f)					
				xScale = 1.0f- (delta.x/xLength);
			else xScale = 0.0f;

		}
		else if (direction == 2)
		{
			if (yLength != 0.0f)
				yScale = 1.0f- (delta.y/yLength);
			else yScale = 0.0f;
		}
		if (flags&MOUSE_CTRL )
		{
			if (yScale > xScale)
				xScale = yScale;
			else yScale = xScale;
		}

		if (flags&MOUSE_ALT )
		{
			mod->center.x =  xScreenPivot;
			mod->center.y =  yScreenPivot;
		}
		else
		{
			mod->center.x =  tempCenterX;
			mod->center.y =  tempCenterY;
		}


		if (mod->ip)
		{
			xScale = GetCOREInterface()->SnapPercent(xScale);
			yScale = GetCOREInterface()->SnapPercent(yScale);
		}

		tempXScale = xScale;
		tempYScale = yScale;




		//convert our sub selection type to vertex selection
		mod->TransferSelectionStart();

		mod->ScalePointsXY(hWnd, xScale,yScale);

		//convert our sub selection type to current selection
		mod->TransferSelectionEnd(FALSE,FALSE);

		mod->freeFormPivotOffset.x *= xScale;
		mod->freeFormPivotOffset.y *= yScale;
	}
	else if (mod->freeFormSubMode == ID_TOOL_MOVEPIVOT)
	{
		theHold.Restore();
		IPoint2 delta = om-m;
		if (flags&MOUSE_SHIFT ) 
		{
			if (abs(delta.x) > abs(delta.y)) 
				delta.y = 0;
			else delta.x = 0;
		}
		float xzoom, yzoom;
		int width, height;				

		mod->ComputeZooms(hWnd,xzoom,yzoom,width,height);
		Point3 mv(0.0f,0.0f,0.0f);
		int i1, i2;
		mod->GetUVWIndices(i1,i2);
		mv[i1] = -delta.x/xzoom;
		mv[i2] = delta.y/yzoom;
		//				mv.z = 0.0f;
		mod->freeFormPivotOffset += mv;

	}

	if (mod->update && mod->ip) mod->ip->RedrawViews(mod->ip->GetTime());
	UpdateWindow(hWnd);
	break;		
	}


case MOUSE_ABORT:

	mod->inRotation = FALSE;
	dragging = FALSE;
	theHold.Cancel();
	if (mod->freeFormSubMode != ID_TOOL_MOVEPIVOT)
		theHold.SuperCancel();

	if (mod->fnGetConstantUpdate())
		mod->NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);

	if (mod->ip) mod->ip->RedrawViews(mod->ip->GetTime());
	mod->InvalidateView();
	break;
	}

	return 1;
}



int WeldMode::subproc(HWND hWnd, int msg, int point, int flags, IPoint2 m)
{

	switch (msg) 
	{
	case MOUSE_POINT:
		{
			if (point==0) {
				theHold.Begin();
				mod->HoldPointsAndFaces();
				om = m;
				mod->tWeldHit = -1;
				mod->tWeldHitLD = NULL;
			} else {
				if (mod->WeldPoints(hWnd,m))
				{
					theHold.Accept(_T(GetString(IDS_PW_WELD_UVW)));
				}
				else{
					theHold.Accept(_T(GetString(IDS_PW_MOVE_UVW)));
				}

				if (mod->ip) mod->ip->RedrawViews(mod->ip->GetTime());
			}
			break;
		}

	case MOUSE_MOVE: 
		{
			
			Tab<TVHitData> hits;
			Rect rect;
			rect.left = m.x-2;
			rect.right = m.x+2;
			rect.top = m.y-2;
			rect.bottom = m.y+2;
			mod->tWeldHit = -1;
			mod->tWeldHitLD = NULL;

			BOOL sel = TRUE;

			if (mod->HitTest(rect,hits,FALSE) )
			{
				

				MeshTopoData *selID = NULL;
				for (int i =0; i < hits.Count(); i++)
				{
					MeshTopoData *ld = mod->GetMeshTopoData(hits[i].mLocalDataID);
					int index = hits[i].mID;
					if (mod->fnGetTVSubMode() == TVVERTMODE) 
					{
						if (ld->GetTVVertSelected(index))//mod->vsel[hits[i]])
							selID = ld;

					}
					else if (mod->fnGetTVSubMode() == TVEDGEMODE) 
					{
						if (ld->GetTVEdgeSelected(index))//mod->esel[hits[i]])
						{
							selID = ld;
						}
					}
				}

				for (int i =0; i < hits.Count(); i++)
				{
					MeshTopoData *ld = mod->GetMeshTopoData(hits[i].mLocalDataID);
					int index = hits[i].mID;
					if (ld == selID)
					{

						if (mod->fnGetTVSubMode() == TVVERTMODE) 
						{
							if (!ld->GetTVVertSelected(index))//mod->vsel[hits[i]])
								sel = FALSE;
						}
						else if (mod->fnGetTVSubMode() == TVEDGEMODE) 
						{
							if (!ld->GetTVEdgeSelected(index))//mod->esel[hits[i]])
							{
								int edgeCount = ld->GetTVEdgeNumberTVFaces(index);//mod->TVMaps.ePtrList[hits[i]]->faceList.Count();
								if ( edgeCount ==1)
								{
									sel = FALSE;
									mod->tWeldHit = index;//hits[i];
									mod->tWeldHitLD = ld;
								}
							}
						}
					}
				}
			}
			if (!sel)
				SetCursor(mod->weldCurHit);
			else SetCursor(mod->weldCur);

//			else SetCursor(mod->weldCur);


			theHold.Restore();

			float xzoom, yzoom;
			int width, height;
			IPoint2 delta = m-om;
			if (flags&MOUSE_SHIFT && mod->move==0) {
				if (abs(delta.x) > abs(delta.y)) delta.y = 0;
				else delta.x = 0;
			} else if (mod->move==1) {
				delta.y = 0;
			} else if (mod->move==2) {
				delta.x = 0;
			}
			mod->ComputeZooms(hWnd,xzoom,yzoom,width,height);
			Point2 mv;
			mv.x = delta.x/xzoom;
			mv.y = -delta.y/yzoom;

			mod->TransferSelectionStart();
			mod->MovePoints(mv);
			mod->TransferSelectionEnd(FALSE,FALSE);

			if (mod->update && mod->ip) 
				mod->ip->RedrawViews(mod->ip->GetTime());
			UpdateWindow(hWnd);
			break;		
		}

	case MOUSE_ABORT:	
		{
			theHold.Cancel();
//			theHold.SuperCancel();
			if (mod->fnGetConstantUpdate())
				mod->NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);
			if (mod->ip) mod->ip->RedrawViews(mod->ip->GetTime());
			break;
		}
	}

	return 1;
}


int PanMode::proc(HWND hWnd, int msg, int point, int flags, IPoint2 m)
{

	switch (msg) {
case MOUSE_POINT:
	if (point==0) {
		om = m;
		oxscroll = mod->xscroll;
		oyscroll = mod->yscroll;
	}
	break;

case MOUSE_MOVE: {
	//watje tile
	mod->tileValid = FALSE;

	IPoint2 delta = m-om;
	mod->xscroll = oxscroll + float(delta.x);
	mod->yscroll = oyscroll + float(delta.y);
	mod->InvalidateView();
	SetCursor(GetPanCursor());
	break;
				 }

case MOUSE_ABORT:
	//watje tile
	mod->tileValid = FALSE;

	mod->xscroll = oxscroll;
	mod->yscroll = oyscroll;
	mod->InvalidateView();
	break;

case MOUSE_FREEMOVE:
	SetCursor(GetPanCursor());
	break;		
	}
	return 1;
}

int ZoomMode::proc(HWND hWnd, int msg, int point, int flags, IPoint2 m)
{
	switch (msg) {
case MOUSE_POINT:
	if (point==0) {
		om = m;
		ozoom = mod->zoom;
		oxscroll = mod->xscroll;
		oyscroll = mod->yscroll;


	}
	break;

case MOUSE_MOVE: {

	//watje tile
	mod->tileValid = FALSE;

	Rect rect;
	GetClientRect(hWnd,&rect);	
	int centerX = (rect.w()-1)/2-om.x;
	int centerY = (rect.h()-1)/2-om.y;

	//			oxscroll = oxscroll + centerX;
	//			oyscroll = oyscroll + centerY;


	IPoint2 delta = om-m;
	float z;
	if (delta.y<0)
		z = (1.0f/(1.0f-ZOOM_FACT*delta.y));
	else z = (1.0f+ZOOM_FACT*delta.y);
	mod->zoom = ozoom * z;
	mod->xscroll = (oxscroll + centerX)*z;
	mod->yscroll = (oyscroll + centerY)*z;


	mod->xscroll -= centerX;
	mod->yscroll -= centerY;

	mod->InvalidateView();
	SetCursor(mod->zoomCur);

	//			SetCursor(LoadCursor(NULL, IDC_ARROW));
	break;
				 }

case MOUSE_ABORT:
	//watje tile
	mod->tileValid = FALSE;

	mod->zoom = ozoom;
	mod->xscroll = oxscroll;
	mod->yscroll = oyscroll;
	mod->InvalidateView();
	break;

case MOUSE_FREEMOVE:
	SetCursor(mod->zoomCur);

	//			SetCursor(LoadCursor(NULL, IDC_ARROW));
	break;		
	}
	return 1;
}

int ZoomRegMode::proc(HWND hWnd, int msg, int point, int flags, IPoint2 m)
{
	switch (msg) {
case MOUSE_POINT:
	if (point==0) {
		lm = om = m;
		XORDottedRect(hWnd,om,lm);
	} else {

		if (om!=m)
		{
			//watje tile
			mod->tileValid = FALSE;

			Rect rect;
			GetClientRect(hWnd,&rect);
			IPoint2 mcent = (om+m)/2;
			IPoint2 scent = rect.GetCenter();
			IPoint2 delta = m-om;
			float rat1, rat2;
			if ((delta.x  != 0) && (delta.y != 0))
			{
				rat1 = float(rect.w()-1)/float(fabs((double)delta.x));
				rat2 = float(rect.h()-1)/float(fabs((double)delta.y));
				float rat = rat1<rat2?rat1:rat2;
				mod->zoom *= rat;
				delta = scent - mcent;
				mod->xscroll += delta.x;
				mod->yscroll += delta.y;
				mod->xscroll *= rat;
				mod->yscroll *= rat;
			}
		}
		mod->InvalidateView();
	}
	break;

case MOUSE_MOVE:


	XORDottedRect(hWnd,om,lm);
	XORDottedRect(hWnd,om,m);
	lm = m;
	SetCursor(mod->zoomRegionCur);
	break;

case MOUSE_ABORT:
	InvalidateRect(hWnd,NULL,FALSE);
	break;

case MOUSE_FREEMOVE:
	SetCursor(mod->zoomRegionCur);
	break;		
	}
	return 1;
}

int RightMouseMode::proc(HWND hWnd, int msg, int point, int flags, IPoint2 m)
{
	switch (msg) {
case MOUSE_POINT:
case MOUSE_PROPCLICK:

	int ct = 0;
	for (int ldID = 0; ldID < mod->GetMeshTopoDataCount(); ldID++)
	{
		ct += mod->GetMeshTopoData(ldID)->GetTVVertSelection().NumberSet();
	}

	if ((mod->mode == ID_SKETCHMODE) && (mod->sketchSelMode == SKETCH_SELPICK) && (ct > 0))
	{

		mod->sketchSelMode = SKETCH_DRAWMODE;
		if (mod->sketchType == SKETCH_LINE)
			GetCOREInterface()->ReplacePrompt( GetString(IDS_PW_SKETCHPROMPT_LINE));
		else if (mod->sketchType == SKETCH_FREEFORM)
		{
			GetCOREInterface()->ReplacePrompt( GetString(IDS_PW_SKETCHPROMPT_FREEFORM));
		}
		else if (mod->sketchType == SKETCH_BOX)
		{
			GetCOREInterface()->ReplacePrompt( GetString(IDS_PW_SKETCHPROMPT_BOX));
		}
		else if (mod->sketchType == SKETCH_CIRCLE)
		{
			GetCOREInterface()->ReplacePrompt( GetString(IDS_PW_SKETCHPROMPT_CIRCLE));
		}

	}
	//check if in pan zoom or soom region mode
	else if ( (mod->mode == ID_UNWRAP_PAN) ||
		(mod->mode == ID_TOOL_PAN) ||
		(mod->mode == ID_UNWRAP_ZOOM) ||
		(mod->mode == ID_TOOL_ZOOM) ||
		(mod->mode == ID_UNWRAP_ZOOMREGION) ||
		(mod->mode == ID_TOOL_ZOOMREG) ||
		(mod->mode == ID_UNWRAP_WELD) ||
		(mod->mode == ID_TOOL_WELD) ||
		(mod->mode == ID_SKETCHMODE) ||
		(mod->mode == ID_PAINTSELECTMODE)
		)
	{
		if (!( (mod->oldMode == ID_UNWRAP_PAN) ||
			(mod->oldMode == ID_TOOL_PAN) ||
			(mod->oldMode == ID_UNWRAP_ZOOM) ||
			(mod->oldMode == ID_TOOL_ZOOM) ||
			(mod->oldMode == ID_UNWRAP_ZOOMREGION) ||
			(mod->oldMode == ID_TOOL_ZOOMREG) ||
			(mod->oldMode == ID_UNWRAP_WELD) ||
			(mod->oldMode == ID_TOOL_WELD)  ||
			(mod->oldMode == ID_SKETCHMODE) ||
			(mod->oldMode == ID_PAINTSELECTMODE)
			))
		{
			mod->SetMode(mod->oldMode);
		}
		else 
		{
			IMenuContext *pContext = GetCOREInterface()->GetMenuManager()->GetContext(kIUVWUnwrapQuad);
			DbgAssert(pContext);
			DbgAssert(pContext->GetType() == kMenuContextQuadMenu);
			IQuadMenuContext *pQMContext = (IQuadMenuContext *)pContext;
			int curIndex = pQMContext->GetCurrentMenuIndex();
			IQuadMenu *pMenu = pQMContext->GetMenu( curIndex );
			DbgAssert(pMenu);
			pMenu->TrackMenu(hWnd, pQMContext->GetShowAllQuads(curIndex));
			//					return TRUE;
			//					mod->TrackRBMenu(hWnd, m.x, m.y);
		}

	}
	else{ 
		IMenuContext *pContext = GetCOREInterface()->GetMenuManager()->GetContext(kIUVWUnwrapQuad);
		DbgAssert(pContext);
		DbgAssert(pContext->GetType() == kMenuContextQuadMenu);
		IQuadMenuContext *pQMContext = (IQuadMenuContext *)pContext;
		int curIndex = pQMContext->GetCurrentMenuIndex();
		IQuadMenu *pMenu = pQMContext->GetMenu( curIndex );
		DbgAssert(pMenu);
		pMenu->TrackMenu(hWnd, pQMContext->GetShowAllQuads(curIndex));
		//				return TRUE;

		//				mod->TrackRBMenu(hWnd, m.x, m.y);
	}

	break;
	}
	return 1;
}

void MiddleMouseMode::ResetInitialParams()
{
	oxscroll = mod->xscroll;
	oyscroll = mod->yscroll;
	ozoom = mod->zoom;
	reset = TRUE;
}

int MiddleMouseMode::proc(HWND hWnd, int msg, int point, int flags, IPoint2 m)
{
	static int modeType = 0;
	switch (msg) {

case MOUSE_POINT:
	if (point==0) {

		inDrag = TRUE;
		BOOL ctrl = flags & MOUSE_CTRL;
		BOOL alt = flags & MOUSE_ALT;

		if (ctrl && alt)
		{
			modeType = ID_TOOL_ZOOM;
			ozoom = mod->zoom;
		}
		else modeType = ID_TOOL_PAN;

		om = m;
		oxscroll = mod->xscroll;
		oyscroll = mod->yscroll;
	}
	//tile
	else
	{
		inDrag = FALSE;
		mod->tileValid = FALSE;
		mod->InvalidateView();

	}
	break;

case MOUSE_MOVE: {
	if (reset)
	{
		om = m;
	}
	reset = FALSE;
	//watje tile
	mod->tileValid = FALSE;

	if (modeType == ID_TOOL_PAN)
	{
		IPoint2 delta = m-om;
		mod->xscroll = oxscroll + float(delta.x);
		mod->yscroll = oyscroll + float(delta.y);
		mod->InvalidateView();
		SetCursor(GetPanCursor());
	}
	else if (modeType == ID_TOOL_ZOOM)
	{
		IPoint2 delta = om-m;
		float z;
		if (delta.y<0)
			z = (1.0f/(1.0f-ZOOM_FACT*delta.y));
		else z = (1.0f+ZOOM_FACT*delta.y);
		mod->zoom = ozoom * z;
		mod->xscroll = oxscroll*z;
		mod->yscroll = oyscroll*z;
		mod->InvalidateView();
		SetCursor(mod->zoomCur);
	}
	break;
				 }

case MOUSE_ABORT:
	//watje tile
	mod->tileValid = FALSE;
	inDrag = FALSE;

	mod->xscroll = oxscroll;
	mod->yscroll = oyscroll;
	if (modeType == ID_TOOL_ZOOM)
		mod->zoom = ozoom;
	mod->InvalidateView();
	break;



	}
	return 1;
}




static INT_PTR CALLBACK PropDlgProc(
									HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static IColorSwatch *iSelColor, *iLineColor, *iOpenEdgeColor, *iHandleColor, *iGizmoColor, *iSharedColor, *iBackgroundColor;
	static ISpinnerControl *spinW, *spinH;
	static ISpinnerControl *spinThreshold ;

	static ISpinnerControl *spinTileLimit ;
	static ISpinnerControl *spinTileContrast ;

	static ISpinnerControl *spinLimitSoftSel;
	static ISpinnerControl *spinHitSize;
	static ISpinnerControl *spinTickSize;
	//new
	static IColorSwatch *iGridColor;
	static ISpinnerControl *spinGridSize;
	static ISpinnerControl *spinGridStr;
	static BOOL uiDestroyed = FALSE;

	UnwrapMod *mod = DLGetWindowLongPtr<UnwrapMod*>(hWnd);

	static COLORREF selColor,lineColor,openEdgeColor, handleColor,sharedColor,backgroundColor,freeFormColor,gridColor;

	static BOOL update = FALSE;
	static BOOL showVerts = FALSE;
	static BOOL midPixelSnap = FALSE;
	static BOOL useBitmapRes = FALSE;
	static BOOL fnGetTile = FALSE;
	static BOOL fnGetDisplayOpenEdges = FALSE;

	static BOOL fnGetThickOpenEdges = FALSE;
	static BOOL fnGetViewportOpenEdges = FALSE;

	static BOOL fnGetDisplayHiddenEdges = FALSE;
	static BOOL fnGetShowShared = FALSE;
	static BOOL fnGetBrightCenterTile = FALSE;
	static BOOL fnGetBlendToBack = FALSE;
	static BOOL fnGetGridVisible = FALSE;
	static BOOL fnGetLimitSoftSel = FALSE;

	static int rendW = 256;
	static int rendH = 256;
	static int fnGetHitSize = 3;

	static int fnGetTickSize = 2;
	static int fnGetLimitSoftSelRange = 8;
	static int fnGetFillMode = 0;
	static int fnGetTileLimit = 2;

	static float fnGetTileContrast = 0.0f;
	static float weldThreshold = 0.05f;

	static float fnGetGridSize = .2f;
	static float fnGetGridStr = .2f;
	static BOOL fnShowAlpha = FALSE;

	BOOL updateUI = FALSE;

	switch (msg) {
case WM_INITDIALOG: {
	uiDestroyed = FALSE;
	mod = (UnwrapMod*)lParam;
	DLSetWindowLongPtr(hWnd, lParam);

	::SetWindowContextHelpId(hWnd, idh_unwrap_options);


	mod->optionsDialogActive = TRUE;
	mod->hOptionshWnd = hWnd;

	iSelColor = GetIColorSwatch(GetDlgItem(hWnd,IDC_UNWRAP_SELCOLOR), 
		mod->selColor, _T(GetString(IDS_PW_LINECOLOR)));			
	iLineColor = GetIColorSwatch(GetDlgItem(hWnd,IDC_UNWRAP_LINECOLOR), 
		mod->lineColor, _T(GetString(IDS_PW_LINECOLOR)));						


	CheckDlgButton(hWnd,IDC_UNWRAP_CONSTANTUPDATE,mod->update);


	CheckDlgButton(hWnd,IDC_UNWRAP_SELECT_VERTS,mod->showVerts);
	CheckDlgButton(hWnd,IDC_UNWRAP_MIDPIXEL_SNAP,mod->midPixelSnap);

	CheckDlgButton(hWnd,IDC_UNWRAP_USEBITMAPRES,!mod->useBitmapRes);
	Texmap *activeMap = mod->GetActiveMap();
	if (activeMap && (activeMap->ClassID() != Class_ID(BMTEX_CLASS_ID,0) ) )
		EnableWindow(GetDlgItem(hWnd,IDC_UNWRAP_USEBITMAPRES),TRUE);

	spinThreshold = SetupFloatSpinner(
		hWnd,IDC_UNWRAP_WELDTHRESHSPIN,IDC_UNWRAP_WELDTHRESH,
		0.0f,10.0f,mod->weldThreshold);			
	spinW = SetupIntSpinner(
		hWnd,IDC_UNWRAP_WIDTHSPIN,IDC_UNWRAP_WIDTH,
		2,2048,mod->rendW);			
	spinH = SetupIntSpinner(
		hWnd,IDC_UNWRAP_HEIGHTSPIN,IDC_UNWRAP_HEIGHT,
		2,2048,mod->rendH);			

	CheckDlgButton(hWnd,IDC_UNWRAP_TILEMAP,mod->fnGetTile());

	spinTileLimit = SetupIntSpinner(
		hWnd,IDC_UNWRAP_TILELIMITSPIN,IDC_UNWRAP_TILELIMIT,
		0,50,mod->fnGetTileLimit());			
	spinTileContrast = SetupFloatSpinner(
		hWnd,IDC_UNWRAP_TILECONTRASTSPIN,IDC_UNWRAP_TILECONTRAST,
		0.0f,1.0f,mod->fnGetTileContrast());

	CheckDlgButton(hWnd,IDC_UNWRAP_LIMITSOFTSEL,mod->fnGetLimitSoftSel());
	spinLimitSoftSel = SetupIntSpinner(
		hWnd,IDC_UNWRAP_LIMITSOFTSELRANGESPIN,IDC_UNWRAP_LIMITSOFTSELRANGE,
		0,100,mod->fnGetLimitSoftSelRange());



	HWND hFill = GetDlgItem(hWnd,IDC_FILL_COMBO);
	SendMessage(hFill, CB_RESETCONTENT, 0, 0);

	SendMessage(hFill, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR) GetString(IDS_PW_FILL_NOFILL));
	SendMessage(hFill, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR) GetString(IDS_PW_FILL_SOLID));
	SendMessage(hFill, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR) GetString(IDS_PW_FILL_BDIAG));
	SendMessage(hFill, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR) GetString(IDS_PW_FILL_CROSS));
	SendMessage(hFill, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR) GetString(IDS_PW_FILL_DIAGCROSS));
	SendMessage(hFill, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR) GetString(IDS_PW_FILL_FDIAG));
	SendMessage(hFill, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR) GetString(IDS_PW_FILL_HORIZONTAL));
	SendMessage(hFill, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR) GetString(IDS_PW_FILL_VERTICAL));

	SendMessage(hFill, CB_SETCURSEL, mod->fnGetFillMode()-1, 0L);

	iOpenEdgeColor = GetIColorSwatch(GetDlgItem(hWnd,IDC_UNWRAP_OPENEDGECOLOR), 
		mod->openEdgeColor, _T(GetString(IDS_PW_OPENEDGECOLOR)));
	CheckDlgButton(hWnd,IDC_DISPLAYOPENEDGES_CHECK,mod->fnGetDisplayOpenEdges());

	CheckDlgButton(hWnd,IDC_THICKOPENEDGES_CHECK,mod->fnGetThickOpenEdges());
	CheckDlgButton(hWnd,IDC_VIEWSEAMSCHECK,mod->fnGetViewportOpenEdges());

	CheckDlgButton(hWnd,IDC_UNWRAP_DISPLAYHIDDENEDGES,mod->fnGetDisplayHiddenEdges());
	if (!mod->fnIsMesh())
		EnableWindow(GetDlgItem(hWnd,IDC_UNWRAP_DISPLAYHIDDENEDGES),FALSE);

	iHandleColor = GetIColorSwatch(GetDlgItem(hWnd,IDC_UNWRAP_HANDLECOLOR), 
		mod->handleColor, _T(GetString(IDS_PW_HANDLECOLOR)));

	iGizmoColor = GetIColorSwatch(GetDlgItem(hWnd,IDC_UNWRAP_GIZMOCOLOR), 
		mod->freeFormColor, _T(GetString(IDS_PW_HANDLECOLOR)));

	spinHitSize = SetupIntSpinner(
		hWnd,IDC_UNWRAP_HITSIZESPIN,IDC_UNWRAP_HITSIZE,
		1,10,mod->fnGetHitSize());

	iSharedColor = GetIColorSwatch(GetDlgItem(hWnd,IDC_UNWRAP_SHARECOLOR), 
		mod->sharedColor, _T(GetString(IDS_PW_HANDLECOLOR)));
	CheckDlgButton(hWnd,IDC_SHOWSHARED_CHECK,mod->fnGetShowShared());

	iBackgroundColor = GetIColorSwatch(GetDlgItem(hWnd,IDC_UNWRAP_BACKGROUNDCOLOR), 
		mod->backgroundColor, _T(GetString(IDS_PW_BACKGROUNDCOLOR)));

	CheckDlgButton(hWnd,IDC_UNWRAP_AFFECTCENTERTILE,mod->fnGetBrightCenterTile());
	CheckDlgButton(hWnd,IDC_UNWRAP_BLENDTOBACK,mod->fnGetBlendToBack());

	spinTickSize = SetupIntSpinner(
		hWnd,IDC_UNWRAP_TICKSIZESPIN,IDC_UNWRAP_TICKSIZE,
		1,10,mod->fnGetTickSize());

	//new
	CheckDlgButton(hWnd,IDC_SHOWGRID_CHECK,mod->fnGetGridVisible());
	spinGridSize = SetupFloatSpinner(
		hWnd,IDC_UNWRAP_GRIDSIZESPIN,IDC_UNWRAP_GRIDSIZE,
		0.00001f,1.0f,mod->fnGetGridSize());
	spinGridStr = SetupFloatSpinner(
		hWnd,IDC_UNWRAP_GRIDSTRSPIN,IDC_UNWRAP_GRIDSTR,
		0.0f,0.5f,mod->fnGetGridStr());
	iGridColor = GetIColorSwatch(GetDlgItem(hWnd,IDC_UNWRAP_GRIDCOLOR), 
		mod->gridColor, _T(GetString(IDS_PW_OPENEDGECOLOR)));



	selColor = mod->selColor;
	lineColor = mod->lineColor;
	openEdgeColor = mod->openEdgeColor;
	handleColor = mod->handleColor;
	sharedColor = mod->sharedColor;
	backgroundColor = mod->backgroundColor;
	freeFormColor = mod->freeFormColor;
	gridColor = mod->gridColor;

	update = mod->update;
	showVerts = mod->showVerts;
	midPixelSnap = mod->midPixelSnap;
	useBitmapRes = mod->useBitmapRes;
	fnGetTile = mod->fnGetTile();
	fnGetDisplayOpenEdges = mod->fnGetDisplayOpenEdges();

	fnGetThickOpenEdges = mod->fnGetThickOpenEdges();
	fnGetViewportOpenEdges = mod->fnGetViewportOpenEdges();


	fnGetDisplayHiddenEdges = mod->fnGetDisplayHiddenEdges();
	fnGetShowShared = mod->fnGetShowShared();
	fnGetBrightCenterTile = mod->fnGetBrightCenterTile();
	fnGetBlendToBack = mod->fnGetBlendToBack();
	fnGetGridVisible = mod->fnGetGridVisible();
	fnGetLimitSoftSel = mod->fnGetLimitSoftSel();

	rendW = mod->rendW;
	rendH = mod->rendH;
	fnGetHitSize = mod->fnGetHitSize();

	fnGetTickSize = mod->fnGetTickSize();
	fnGetLimitSoftSelRange = mod->fnGetLimitSoftSelRange();
	fnGetFillMode = mod->fnGetFillMode();
	fnGetTileLimit = mod->fnGetTileLimit();

	fnGetTileContrast = mod->fnGetTileContrast();
	weldThreshold = mod->weldThreshold;

	fnGetGridSize = mod->fnGetGridSize();
	fnGetGridStr = mod->fnGetGridStr();
	fnShowAlpha = mod->GetShowImageAlpha();

	CheckDlgButton(hWnd,IDC_UNWRAP_GRID_SNAP,mod->GetGridSnap());
	CheckDlgButton(hWnd,IDC_UNWRAP_VERTEX_SNAP,mod->GetVertexSnap());
	CheckDlgButton(hWnd,IDC_UNWRAP_EDGE_SNAP,mod->GetEdgeSnap());

	CheckDlgButton(hWnd,IDC_UNWRAP_SHOWIMAGEALPHA,mod->GetShowImageAlpha());

	break;
					}

case WM_SYSCOMMAND:
	if ((wParam & 0xfff0) == SC_CONTEXTHELP) 
	{
		DoHelp(HELP_CONTEXT, idh_unwrap_options); 
	}
	return FALSE;
	break;
case WM_CUSTEDIT_ENTER:	
case CC_SPINNER_BUTTONUP:
case CC_COLOR_CHANGE:
	{
		updateUI = TRUE;
		break;
	}
case WM_DESTROY:
	{
		mod->optionsDialogActive = FALSE;
		mod->hOptionshWnd = NULL;
		updateUI = FALSE;
		if (!uiDestroyed)
		{
			ReleaseIColorSwatch(iSelColor);
			ReleaseIColorSwatch(iLineColor);
			ReleaseIColorSwatch(iOpenEdgeColor);
			ReleaseIColorSwatch(iHandleColor);
			ReleaseIColorSwatch(iGizmoColor);
			ReleaseIColorSwatch(iSharedColor);
			ReleaseIColorSwatch(iBackgroundColor);

			ReleaseISpinner(spinThreshold);
			ReleaseISpinner(spinW);
			ReleaseISpinner(spinH);

			ReleaseISpinner(spinTileLimit);
			ReleaseISpinner(spinTileContrast);

			ReleaseISpinner(spinLimitSoftSel);
			ReleaseISpinner(spinHitSize);

			//new
			ReleaseISpinner(spinGridSize);
			ReleaseISpinner(spinGridStr);
			ReleaseIColorSwatch(iGridColor);

		}
	}
case WM_COMMAND:
	switch (LOWORD(wParam)) 
	{
	case IDC_FILL_COMBO:
		if ( HIWORD(wParam) == CBN_SELCHANGE ) 
			updateUI = TRUE;
		break;

	case IDC_DISPLAYOPENEDGES_CHECK:
	case IDC_THICKOPENEDGES_CHECK:
	case IDC_VIEWSEAMSCHECK:
	case IDC_SHOWGRID_CHECK:
	case IDC_UNWRAP_USEBITMAPRES:
	case IDC_UNWRAP_TILEMAP:
	case IDC_UNWRAP_CONSTANTUPDATE:
	case IDC_UNWRAP_AFFECTCENTERTILE:
	case IDC_UNWRAP_SELECT_VERTS:
	case IDC_UNWRAP_DISPLAYHIDDENEDGES:
	case IDC_UNWRAP_BLENDTOBACK:
	case IDC_UNWRAP_LIMITSOFTSEL:
	case IDC_UNWRAP_SHOWIMAGEALPHA:
		updateUI = TRUE;
		break;
	case IDOK:
		{
			mod->hOptionshWnd = NULL;
			mod->lineColor = iLineColor->GetColor();
			mod->selColor  = iSelColor->GetColor();					
			mod->update = IsDlgButtonChecked(hWnd,IDC_UNWRAP_CONSTANTUPDATE);
			mod->showVerts = IsDlgButtonChecked(hWnd,IDC_UNWRAP_SELECT_VERTS);
			mod->midPixelSnap = IsDlgButtonChecked(hWnd,IDC_UNWRAP_MIDPIXEL_SNAP);
			//watje 5-3-99
			BOOL oldRes = mod->useBitmapRes;

			mod->SetGridSnap(IsDlgButtonChecked(hWnd,IDC_UNWRAP_GRID_SNAP));
			mod->SetEdgeSnap(IsDlgButtonChecked(hWnd,IDC_UNWRAP_EDGE_SNAP));
			mod->SetVertexSnap(IsDlgButtonChecked(hWnd,IDC_UNWRAP_VERTEX_SNAP));


			mod->SetShowImageAlpha(IsDlgButtonChecked(hWnd,IDC_UNWRAP_SHOWIMAGEALPHA));


			mod->useBitmapRes = !IsDlgButtonChecked(hWnd,IDC_UNWRAP_USEBITMAPRES);
			mod->weldThreshold = spinThreshold->GetFVal();
			mod->rendW = spinW->GetIVal();
			mod->rendH = spinH->GetIVal();
			//watje 5-3-99
			if (mod->rendW!=mod->iw ||
				mod->rendH!=mod->ih || oldRes!=mod->useBitmapRes) {
					mod->SetupImage();
			}
			mod->fnSetTileLimit(spinTileLimit->GetIVal());
			mod->fnSetTileContrast(spinTileContrast->GetFVal());

			mod->fnSetTile(IsDlgButtonChecked(hWnd,IDC_UNWRAP_TILEMAP));


			mod->fnSetLimitSoftSel(IsDlgButtonChecked(hWnd,IDC_UNWRAP_LIMITSOFTSEL));
			mod->fnSetLimitSoftSelRange(spinLimitSoftSel->GetIVal());

			HWND hFill = GetDlgItem(hWnd,IDC_FILL_COMBO);
			mod->fnSetFillMode(SendMessage(hFill, CB_GETCURSEL, 0L, 0L)+1);

			mod->fnSetDisplayOpenEdges(IsDlgButtonChecked(hWnd,IDC_DISPLAYOPENEDGES_CHECK));
			mod->fnSetThickOpenEdges(IsDlgButtonChecked(hWnd,IDC_THICKOPENEDGES_CHECK));
			mod->fnSetViewportOpenEdges(IsDlgButtonChecked(hWnd,IDC_VIEWSEAMSCHECK));

			mod->fnSetDisplayHiddenEdges(IsDlgButtonChecked(hWnd,IDC_UNWRAP_DISPLAYHIDDENEDGES));
			mod->openEdgeColor = iOpenEdgeColor->GetColor();
			mod->handleColor = iHandleColor->GetColor();
			mod->freeFormColor = iGizmoColor->GetColor();
			mod->sharedColor = iSharedColor->GetColor();

			mod->backgroundColor = iBackgroundColor->GetColor();

			mod->fnSetHitSize(spinHitSize->GetIVal());
			mod->fnSetTickSize(spinTickSize->GetIVal());


			mod->fnSetShowShared(IsDlgButtonChecked(hWnd,IDC_SHOWSHARED_CHECK));
			mod->fnSetBrightCenterTile(IsDlgButtonChecked(hWnd,IDC_UNWRAP_AFFECTCENTERTILE));
			mod->fnSetBlendToBack(IsDlgButtonChecked(hWnd,IDC_UNWRAP_BLENDTOBACK));


			if (mod->iBuf) mod->iBuf->SetBkColor(mod->backgroundColor);
			if (mod->iTileBuf) mod->iTileBuf->SetBkColor(mod->backgroundColor);


			//new			
			mod->fnSetGridVisible(IsDlgButtonChecked(hWnd,IDC_SHOWGRID_CHECK));
			mod->fnSetGridSize(spinGridSize->GetFVal());
			mod->fnSetGridStr(spinGridStr->GetFVal());
			mod->gridColor = iGridColor->GetColor();

			mod->tileValid = FALSE;
			mod->RebuildDistCache();
			mod->InvalidateView();

			ReleaseIColorSwatch(iSelColor);
			ReleaseIColorSwatch(iLineColor);
			ReleaseIColorSwatch(iOpenEdgeColor);
			ReleaseIColorSwatch(iHandleColor);
			ReleaseIColorSwatch(iGizmoColor);
			ReleaseIColorSwatch(iSharedColor);
			ReleaseIColorSwatch(iBackgroundColor);

			ReleaseISpinner(spinThreshold);
			ReleaseISpinner(spinW);
			ReleaseISpinner(spinH);

			ReleaseISpinner(spinTileLimit);
			ReleaseISpinner(spinTileContrast);

			ReleaseISpinner(spinLimitSoftSel);
			ReleaseISpinner(spinHitSize);

			//new
			ReleaseISpinner(spinGridSize);
			ReleaseISpinner(spinGridStr);
			ReleaseIColorSwatch(iGridColor);

			mod->optionsDialogActive = FALSE;
			updateUI = FALSE;

			uiDestroyed = TRUE;
			mod->MoveScriptUI();

			EndDialog(hWnd,0);

			//fall through to release controls'
			break;
		}
	case IDCANCEL:

		mod->hOptionshWnd = NULL;
		ReleaseIColorSwatch(iSelColor);
		ReleaseIColorSwatch(iLineColor);
		ReleaseIColorSwatch(iOpenEdgeColor);
		ReleaseIColorSwatch(iHandleColor);
		ReleaseIColorSwatch(iGizmoColor);
		ReleaseIColorSwatch(iSharedColor);
		ReleaseIColorSwatch(iBackgroundColor);

		ReleaseISpinner(spinThreshold);
		ReleaseISpinner(spinW);
		ReleaseISpinner(spinH);

		ReleaseISpinner(spinTileLimit);
		ReleaseISpinner(spinTileContrast);

		ReleaseISpinner(spinLimitSoftSel);
		ReleaseISpinner(spinHitSize);

		//new
		ReleaseISpinner(spinGridSize);
		ReleaseISpinner(spinGridStr);
		ReleaseIColorSwatch(iGridColor);


		mod->selColor = selColor;
		mod->lineColor = lineColor;
		mod->openEdgeColor = openEdgeColor;
		mod->handleColor = handleColor;
		mod->sharedColor = sharedColor;
		mod->backgroundColor = backgroundColor;
		mod->freeFormColor = freeFormColor;
		mod->gridColor = gridColor;

		mod->update = update;
		mod->showVerts = showVerts;
		mod->midPixelSnap = midPixelSnap;
		mod->useBitmapRes = useBitmapRes;
		mod->fnSetTile(fnGetTile);

		mod->fnSetDisplayOpenEdges(fnGetDisplayOpenEdges);

		mod->fnSetThickOpenEdges(fnGetThickOpenEdges);
		mod->fnSetViewportOpenEdges(fnGetViewportOpenEdges);

		mod->fnSetDisplayHiddenEdges(fnGetDisplayHiddenEdges);
		mod->fnSetShowShared(fnGetShowShared);
		mod->fnSetBrightCenterTile(fnGetBrightCenterTile);
		mod->fnSetBlendToBack(fnGetBlendToBack);
		mod->fnSetGridVisible(fnGetGridVisible);
		mod->fnSetLimitSoftSel(fnGetLimitSoftSel);
		

		mod->rendW = rendW;
		mod->rendH = rendH;
		mod->fnSetHitSize(fnGetHitSize);

		mod->fnSetTickSize(fnGetTickSize);
		mod->fnSetLimitSoftSelRange(fnGetLimitSoftSelRange);
		mod->fnSetFillMode(fnGetFillMode);
		mod->fnSetTileLimit(fnGetTileLimit);

		mod->fnSetTileContrast(fnGetTileContrast);
		mod->weldThreshold = weldThreshold;

		mod->fnSetGridSize(fnGetGridSize);
		mod->fnSetGridStr(fnGetGridStr);
		mod->SetShowImageAlpha(fnShowAlpha);

		if (mod->iBuf) mod->iBuf->SetBkColor(mod->backgroundColor);
		if (mod->iTileBuf) mod->iTileBuf->SetBkColor(mod->backgroundColor);

		mod->optionsDialogActive = FALSE;
		updateUI = FALSE;

		mod->tileValid = FALSE;
		mod->RebuildDistCache();
		mod->InvalidateView();

		uiDestroyed = TRUE;

		EndDialog(hWnd,0);
		break;

	case IDC_UNWRAP_DEFAULTS:
		mod->fnLoadDefaults();

		mod->lineColor = RGB(255,255,255);
		mod->selColor  = RGB(255,0,0);
		mod->openEdgeColor = RGB(0,255,0);
		mod->handleColor = RGB(255,255,0);
		mod->freeFormColor = RGB(255,100,50);
		mod->sharedColor = RGB(0,0,255);

		mod->backgroundColor = RGB(60,60,60);

		selColor = mod->selColor;
		lineColor = mod->lineColor;
		openEdgeColor = mod->openEdgeColor;
		handleColor = mod->handleColor;
		sharedColor = mod->sharedColor;
		backgroundColor = mod->backgroundColor;
		freeFormColor = mod->freeFormColor;
		gridColor = mod->gridColor;

		update = mod->update;
		showVerts = mod->showVerts;
		midPixelSnap = mod->midPixelSnap;
		useBitmapRes = mod->useBitmapRes;
		fnGetTile = mod->fnGetTile();
		fnGetDisplayOpenEdges = mod->fnGetDisplayOpenEdges();

		fnGetThickOpenEdges = mod->fnGetThickOpenEdges();
		fnGetViewportOpenEdges = mod->fnGetViewportOpenEdges();


		fnGetDisplayHiddenEdges = mod->fnGetDisplayHiddenEdges();
		fnGetShowShared = mod->fnGetShowShared();
		fnGetBrightCenterTile = mod->fnGetBrightCenterTile();
		fnGetBlendToBack = mod->fnGetBlendToBack();
		fnGetGridVisible = mod->fnGetGridVisible();
		fnGetLimitSoftSel = mod->fnGetLimitSoftSel();

		rendW = mod->rendW;
		rendH = mod->rendH;
		fnGetHitSize = mod->fnGetHitSize();

		fnGetTickSize = mod->fnGetTickSize();
		fnGetLimitSoftSelRange = mod->fnGetLimitSoftSelRange();
		fnGetFillMode = mod->fnGetFillMode();
		fnGetTileLimit = mod->fnGetTileLimit();

		fnGetTileContrast = mod->fnGetTileContrast();
		weldThreshold = mod->weldThreshold;

		fnGetGridSize = mod->fnGetGridSize();
		fnGetGridStr = mod->fnGetGridStr();


		iSelColor->SetColor(mod->selColor);
		iLineColor->SetColor(mod->lineColor);


		CheckDlgButton(hWnd,IDC_UNWRAP_CONSTANTUPDATE,mod->update);


		CheckDlgButton(hWnd,IDC_UNWRAP_SELECT_VERTS,mod->showVerts);
		CheckDlgButton(hWnd,IDC_UNWRAP_MIDPIXEL_SNAP,mod->midPixelSnap);

		CheckDlgButton(hWnd,IDC_UNWRAP_USEBITMAPRES,!mod->useBitmapRes);
		Texmap *activeMap = mod->GetActiveMap();
		if (activeMap && (activeMap->ClassID() != Class_ID(BMTEX_CLASS_ID,0) ) )
			EnableWindow(GetDlgItem(hWnd,IDC_UNWRAP_USEBITMAPRES),TRUE);

		spinThreshold->SetValue(mod->weldThreshold,FALSE);			
		spinW->SetValue(mod->rendW,FALSE);			
		spinH->SetValue(mod->rendH,FALSE);			

		CheckDlgButton(hWnd,IDC_UNWRAP_TILEMAP,mod->fnGetTile());

		spinTileLimit->SetValue(mod->fnGetTileLimit(),FALSE);			
		spinTileContrast->SetValue(mod->fnGetTileContrast(),FALSE);

		CheckDlgButton(hWnd,IDC_UNWRAP_LIMITSOFTSEL,mod->fnGetLimitSoftSel());
		spinLimitSoftSel->SetValue(mod->fnGetLimitSoftSelRange(),FALSE);



		HWND hFill = GetDlgItem(hWnd,IDC_FILL_COMBO);
		SendMessage(hFill, CB_RESETCONTENT, 0, 0);

		SendMessage(hFill, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR) GetString(IDS_PW_FILL_NOFILL));
		SendMessage(hFill, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR) GetString(IDS_PW_FILL_SOLID));
		SendMessage(hFill, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR) GetString(IDS_PW_FILL_BDIAG));
		SendMessage(hFill, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR) GetString(IDS_PW_FILL_CROSS));
		SendMessage(hFill, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR) GetString(IDS_PW_FILL_DIAGCROSS));
		SendMessage(hFill, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR) GetString(IDS_PW_FILL_FDIAG));
		SendMessage(hFill, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR) GetString(IDS_PW_FILL_HORIZONTAL));
		SendMessage(hFill, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR) GetString(IDS_PW_FILL_VERTICAL));

		SendMessage(hFill, CB_SETCURSEL, mod->fnGetFillMode()-1, 0L);

		iOpenEdgeColor->SetColor(mod->openEdgeColor);
		CheckDlgButton(hWnd,IDC_DISPLAYOPENEDGES_CHECK,mod->fnGetDisplayOpenEdges());

		CheckDlgButton(hWnd,IDC_THICKOPENEDGES_CHECK,mod->fnGetThickOpenEdges());
		CheckDlgButton(hWnd,IDC_VIEWSEAMSCHECK,mod->fnGetViewportOpenEdges());

		CheckDlgButton(hWnd,IDC_UNWRAP_DISPLAYHIDDENEDGES,mod->fnGetDisplayHiddenEdges());

		iHandleColor->SetColor(mod->handleColor);

		iGizmoColor->SetColor(mod->freeFormColor);

		spinHitSize->SetValue(mod->fnGetHitSize(),FALSE);

		iSharedColor->SetColor(mod->sharedColor);
		CheckDlgButton(hWnd,IDC_SHOWSHARED_CHECK,mod->fnGetShowShared());

		iBackgroundColor->SetColor(mod->backgroundColor);

		CheckDlgButton(hWnd,IDC_UNWRAP_AFFECTCENTERTILE,mod->fnGetBrightCenterTile());
		CheckDlgButton(hWnd,IDC_UNWRAP_BLENDTOBACK,mod->fnGetBlendToBack());

		spinTickSize->SetValue(mod->fnGetTickSize(),FALSE);

		//new
		CheckDlgButton(hWnd,IDC_SHOWGRID_CHECK,mod->fnGetGridVisible());
		spinGridSize->SetValue(mod->fnGetGridSize(),FALSE);
		spinGridStr->SetValue(mod->fnGetGridStr(),FALSE);
		iGridColor->SetColor(mod->gridColor);


		mod->SetGridSnap(TRUE);
		mod->SetGridSnap(TRUE);
		mod->SetGridSnap(TRUE);

		CheckDlgButton(hWnd,IDC_UNWRAP_GRID_SNAP,mod->GetGridSnap());
		CheckDlgButton(hWnd,IDC_UNWRAP_VERTEX_SNAP,mod->GetVertexSnap());
		CheckDlgButton(hWnd,IDC_UNWRAP_EDGE_SNAP,mod->GetEdgeSnap());

		mod->SetShowImageAlpha(FALSE);
		CheckDlgButton(hWnd,IDC_UNWRAP_SHOWIMAGEALPHA,mod->GetShowImageAlpha());

		break;
	}
	break;

default:
	return FALSE;
	} 
	if ((updateUI) && mod->optionsDialogActive)
	{
		mod->lineColor = iLineColor->GetColor();
		mod->selColor  = iSelColor->GetColor();					
		mod->update = IsDlgButtonChecked(hWnd,IDC_UNWRAP_CONSTANTUPDATE);
		mod->showVerts = IsDlgButtonChecked(hWnd,IDC_UNWRAP_SELECT_VERTS);
		mod->midPixelSnap = IsDlgButtonChecked(hWnd,IDC_UNWRAP_MIDPIXEL_SNAP);
		//watje 5-3-99
		BOOL oldRes = mod->useBitmapRes;

		mod->useBitmapRes = !IsDlgButtonChecked(hWnd,IDC_UNWRAP_USEBITMAPRES);
		mod->weldThreshold = spinThreshold->GetFVal();
		mod->rendW = spinW->GetIVal();
		mod->rendH = spinH->GetIVal();
		//watje 5-3-99
		if (mod->rendW!=mod->iw ||
			mod->rendH!=mod->ih || oldRes!=mod->useBitmapRes) {
				mod->SetupImage();
		}
		mod->fnSetTileLimit(spinTileLimit->GetIVal());
		mod->fnSetTileContrast(spinTileContrast->GetFVal());

		mod->fnSetTile(IsDlgButtonChecked(hWnd,IDC_UNWRAP_TILEMAP));


		mod->fnSetLimitSoftSel(IsDlgButtonChecked(hWnd,IDC_UNWRAP_LIMITSOFTSEL));
		mod->fnSetLimitSoftSelRange(spinLimitSoftSel->GetIVal());

		HWND hFill = GetDlgItem(hWnd,IDC_FILL_COMBO);
		mod->fnSetFillMode(SendMessage(hFill, CB_GETCURSEL, 0L, 0L)+1);

		mod->fnSetDisplayOpenEdges(IsDlgButtonChecked(hWnd,IDC_DISPLAYOPENEDGES_CHECK));

		mod->fnSetThickOpenEdges(IsDlgButtonChecked(hWnd,IDC_THICKOPENEDGES_CHECK));
		mod->fnSetViewportOpenEdges(IsDlgButtonChecked(hWnd,IDC_VIEWSEAMSCHECK));

		mod->fnSetDisplayHiddenEdges(IsDlgButtonChecked(hWnd,IDC_UNWRAP_DISPLAYHIDDENEDGES));
		mod->openEdgeColor = iOpenEdgeColor->GetColor();
		mod->handleColor = iHandleColor->GetColor();
		mod->freeFormColor = iGizmoColor->GetColor();
		mod->sharedColor = iSharedColor->GetColor();

		mod->backgroundColor = iBackgroundColor->GetColor();

		mod->fnSetHitSize(spinHitSize->GetIVal());
		mod->fnSetTickSize(spinTickSize->GetIVal());


		mod->fnSetShowShared(IsDlgButtonChecked(hWnd,IDC_SHOWSHARED_CHECK));
		mod->fnSetBrightCenterTile(IsDlgButtonChecked(hWnd,IDC_UNWRAP_AFFECTCENTERTILE));
		mod->fnSetBlendToBack(IsDlgButtonChecked(hWnd,IDC_UNWRAP_BLENDTOBACK));

		mod->SetShowImageAlpha(IsDlgButtonChecked(hWnd,IDC_UNWRAP_SHOWIMAGEALPHA));

		

		if (mod->iBuf) mod->iBuf->SetBkColor(mod->backgroundColor);
		if (mod->iTileBuf) mod->iTileBuf->SetBkColor(mod->backgroundColor);




		//new			
		mod->fnSetGridVisible(IsDlgButtonChecked(hWnd,IDC_SHOWGRID_CHECK));
		mod->fnSetGridSize(spinGridSize->GetFVal());
		mod->fnSetGridStr(spinGridStr->GetFVal());
		mod->gridColor = iGridColor->GetColor();

		mod->tileValid = FALSE;
		mod->RebuildDistCache();
		mod->InvalidateView();
		mod->MoveScriptUI();
	}
	return TRUE;
}

void UnwrapMod::PropDialog() 
{
	if (!optionsDialogActive)
	{
		CreateDialogParam(
			hInstance,
			MAKEINTRESOURCE(IDD_UNWRAP_PROP),
			hWnd,
			PropDlgProc,
			(LONG_PTR)this);
		/*		DialogBoxParam(
		hInstance,
		MAKEINTRESOURCE(IDD_UNWRAP_PROP),
		hWnd,
		PropDlgProc,
		(LONG_PTR)this);
		*/
	}
}


void UnwrapMod::UnwrapMatrixFromNormal(Point3& normal, Matrix3& mat)
{
	Point3 vx;
	vx.z = .0f;
	vx.x = -normal.y;
	vx.y = normal.x;	
	if ( vx.x == .0f && vx.y == .0f ) {
		vx.x = 1.0f;
	}
	mat.SetRow(0,vx);
	mat.SetRow(1,normal^vx);
	mat.SetRow(2,normal);
	mat.SetTrans(Point3(0,0,0));
	mat.NoScale();
}


//--- Named selection sets -----------------------------------------

int UnwrapMod::FindSet(TSTR &setName) 
{
	if  (ip && ip->GetSubObjectLevel() == 1) 
	{
		for (int i=0; i<namedVSel.Count(); i++) 
		{
			if (setName == *namedVSel[i]) return i;
		}
		return -1;
	}
	else if  (ip && ip->GetSubObjectLevel() == 2) 
	{
		for (int i=0; i<namedESel.Count(); i++) 
		{
			if (setName == *namedESel[i]) return i;
		}
		return -1;
	}
	else if  (ip && ip->GetSubObjectLevel() == 3) 
	{
		for (int i=0; i<namedSel.Count(); i++) 
		{
			if (setName == *namedSel[i]) return i;
		}
		return -1;
	}
	return -1;
}

DWORD UnwrapMod::AddSet(TSTR &setName) {
	DWORD id = 0;
	TSTR *name = new TSTR(setName);

	if  (ip && ip->GetSubObjectLevel() == 1) 
	{
		namedVSel.Append(1,&name);
		BOOL found = FALSE;
		while (!found) {
			found = TRUE;
			for (int i=0; i<idsV.Count(); i++) {
				if (idsV[i]!=id) continue;
				id++;
				found = FALSE;
				break;
			}

		}
		idsV.Append(1,&id);
		return id;
	}
	else if  (ip && ip->GetSubObjectLevel() == 2) 
	{
		namedESel.Append(1,&name);
		BOOL found = FALSE;
		while (!found) {
			found = TRUE;
			for (int i=0; i<idsE.Count(); i++) {
				if (idsE[i]!=id) continue;
				id++;
				found = FALSE;
				break;
			}

		}
		idsE.Append(1,&id);
		return id;
	}
	else if  (ip && ip->GetSubObjectLevel() == 3) 
	{
		namedSel.Append(1,&name);
		BOOL found = FALSE;
		while (!found) {
			found = TRUE;
			for (int i=0; i<ids.Count(); i++) {
				if (ids[i]!=id) continue;
				id++;
				found = FALSE;
				break;
			}

		}
		ids.Append(1,&id);
		return id;
	}
	return -1;
}

void UnwrapMod::RemoveSet(TSTR &setName) {
	int i = FindSet(setName);
	if (i<0) return;

	if  (ip && ip->GetSubObjectLevel() == 1) 
	{
		delete namedVSel[i];
		namedVSel.Delete(i,1);
		idsV.Delete(i,1);
	}
	else if  (ip && ip->GetSubObjectLevel() == 2) 
	{
		delete namedESel[i];
		namedESel.Delete(i,1);
		idsE.Delete(i,1);
	}
	else if  (ip && ip->GetSubObjectLevel() == 3) 
	{
		delete namedSel[i];
		namedSel.Delete(i,1);
		ids.Delete(i,1);
	}
}

void UnwrapMod::ClearSetNames() 
{

	if  (ip && ip->GetSubObjectLevel() == 1) 
	{
		for (int j=0; j<namedVSel.Count(); j++) 
		{
			delete namedVSel[j];
			namedVSel[j] = NULL;
		}
	}

	else if  (ip && ip->GetSubObjectLevel() == 2) 
	{
		for (int j=0; j<namedESel.Count(); j++) 
		{
			delete namedESel[j];
			namedESel[j] = NULL;
		}
	}

	else if  (ip && ip->GetSubObjectLevel() == 3) 
	{
		for (int j=0; j<namedSel.Count(); j++) 
		{
			delete namedSel[j];
			namedSel[j] = NULL;
		}
	}
}

void UnwrapMod::ActivateSubSelSet(TSTR &setName) {
	ModContextList mcList;
	INodeTab nodes;
	int index = FindSet (setName);	
	if (index<0 || !ip) return;

	ip->GetModContexts(mcList,nodes);
	for (int i = 0; i < mcList.Count(); i++) 
	{
		MeshTopoData *meshData = (MeshTopoData*)mcList[i]->localData;
		if (!meshData) continue;

		BitArray *set = NULL;

		if  (ip->GetSubObjectLevel() == 1) 
		{

			set = meshData->vselSet.GetSet(idsV[index]);
			if (set) 
			{

				meshData->SetGeomVertSelection(*set);//gvsel = *set;
				SyncTVToGeomSelection(meshData);
				RebuildDistCache();
				InvalidateView();
				UpdateWindow(hWnd);
			}
		}
		else if  (ip->GetSubObjectLevel() == 2) 
		{

			set = meshData->eselSet.GetSet(idsE[index]);
			if (set) 
			{
				meshData->SetGeomEdgeSelection(*set);//gesel = *set;
				SyncTVToGeomSelection(meshData);
				InvalidateView();
				UpdateWindow(hWnd);
			}
		}
		if  (ip->GetSubObjectLevel() == 3) 
		{

			set = meshData->fselSet.GetSet(ids[index]);
			if (set) 
			{
				meshData->SetFaceSelection(*set);//meshData->SetFaceSel (*set, this, ip->GetTime());
				fnSyncTVSelection();
				InvalidateView();
				UpdateWindow(hWnd);
			}
		}
	}

	nodes.DisposeTemporary();
	LocalDataChanged ();
	ip->RedrawViews(ip->GetTime());
}

void UnwrapMod::NewSetFromCurSel(TSTR &setName) 
{
	if (!ip) return;
	ModContextList mcList;
	INodeTab nodes;
	DWORD id = -1;
	int index = FindSet(setName);
	if (index<0) id = AddSet(setName);
	else id = ids[index];

	ip->GetModContexts(mcList,nodes);

	for (int i = 0; i < mcList.Count(); i++) {
		MeshTopoData *meshData = (MeshTopoData*)mcList[i]->localData;
		if (!meshData) continue;

		BitArray *set = NULL;
		if  (ip->GetSubObjectLevel() == 1) 
		{
			if (index>=0 && (set = meshData->vselSet.GetSet(id)) != NULL) 
			{
				*set = meshData->GetGeomVertSelection();//gvsel;
			} else meshData->vselSet.AppendSet(meshData->GetGeomVertSelection(),id);
		}
		else if  (ip->GetSubObjectLevel() == 2) 
		{
			if (index>=0 && (set = meshData->eselSet.GetSet(id)) != NULL) 
			{
				*set = meshData->GetGeomEdgeSelection();//gesel;
			} else meshData->eselSet.AppendSet(meshData->GetGeomEdgeSelection(),id);
		}
		else if  (ip->GetSubObjectLevel() == 3) 
		{
			if (index>=0 && (set = meshData->fselSet.GetSet(id)) != NULL) 
			{
				*set = meshData->GetFaceSelection();//meshData->faceSel;
			} else meshData->fselSet.AppendSet(meshData->GetFaceSelection(),id);
		}
	}

	nodes.DisposeTemporary();
}

void UnwrapMod::RemoveSubSelSet(TSTR &setName) {
	int index = FindSet (setName);
	if (index<0 || !ip) return;		

	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);

	DWORD id = ids[index];

	for (int i = 0; i < mcList.Count(); i++) 
	{
		MeshTopoData *meshData = (MeshTopoData*)mcList[i]->localData;
		if (!meshData) continue;		

		//				if (theHold.Holding()) theHold.Put(new DeleteSetRestore(&meshData->fselSet,id));
		if  (ip->GetSubObjectLevel() == 1) 
			meshData->vselSet.RemoveSet(id);
		else if  (ip->GetSubObjectLevel() == 2) 
			meshData->eselSet.RemoveSet(id);
		else if  (ip->GetSubObjectLevel() == 3) 
			meshData->fselSet.RemoveSet(id);
	}

	//	if (theHold.Holding()) theHold.Put(new DeleteSetNameRestore(&(namedSel[nsl]),this,&(ids[nsl]),id));
	RemoveSet (setName);
	ip->ClearCurNamedSelSet();
	nodes.DisposeTemporary();
}

void UnwrapMod::SetupNamedSelDropDown() {
	if (!ip) return;		

	ip->ClearSubObjectNamedSelSets();
	if  (ip->GetSubObjectLevel() ==1) 
	{
		for (int i=0; i<namedVSel.Count(); i++)
		{
			ip->AppendSubObjectNamedSelSet(*namedVSel[i]);
		}
	}
	if  (ip->GetSubObjectLevel() ==2) 
	{
		for (int i=0; i<namedESel.Count(); i++)
		{
			ip->AppendSubObjectNamedSelSet(*namedESel[i]);
		}
	}	
	if  (ip->GetSubObjectLevel() ==3) 
	{
		for (int i=0; i<namedSel.Count(); i++)
		{
			ip->AppendSubObjectNamedSelSet(*namedSel[i]);
		}
	}

	/*
	else if  (ip->GetSubObjectLevel() == 2) 
	ip->AppendSubObjectNamedSelSet(*namedESel[i]);
	else if  (ip->GetSubObjectLevel() == 3) 
	ip->AppendSubObjectNamedSelSet(*namedSel[i]);

	}
	*/
}

int UnwrapMod::NumNamedSelSets() 
{
	if  (ip && ip->GetSubObjectLevel() ==1) 
		return namedVSel.Count();
	else if  (ip && ip->GetSubObjectLevel() ==2) 
		return namedESel.Count();
	else if  (ip && ip->GetSubObjectLevel() ==3) 
		return namedSel.Count();
	return -1;
}

TSTR UnwrapMod::GetNamedSelSetName(int i) 
{
	if  (ip && ip->GetSubObjectLevel() ==1) 
		return *namedVSel[i];
	if  (ip && ip->GetSubObjectLevel() ==2) 
		return *namedESel[i];
	if  (ip && ip->GetSubObjectLevel() ==3) 
		return *namedSel[i];
	return TSTR(" ");
}


void UnwrapMod::SetNamedSelSetName(int i,TSTR &newName) 
{
	if  (ip && ip->GetSubObjectLevel() ==1)
		*namedVSel[i] = newName;
	if  (ip && ip->GetSubObjectLevel() ==2)
		*namedESel[i] = newName;
	if  (ip && ip->GetSubObjectLevel() ==3)
		*namedSel[i] = newName;
}

void UnwrapMod::NewSetByOperator(TSTR &newName,Tab<int> &sets,int op) {
	ModContextList mcList;
	INodeTab nodes;

	if (!ip) return;		
	DWORD id = AddSet(newName);
	//	if (theHold.Holding()) theHold.Put(new AppendSetNameRestore(this,&namedSel,&ids));

	BOOL delSet = TRUE;
	ip->GetModContexts(mcList,nodes);
	for (int i = 0; i < mcList.Count(); i++) {
		MeshTopoData *meshData = (MeshTopoData*)mcList[i]->localData;
		if (!meshData) continue;

		BitArray bits;
		GenericNamedSelSetList *setList;

		setList = &meshData->fselSet; break;			


		bits = (*setList)[sets[0]];

		for (int i=1; i<sets.Count(); i++) {
			switch (op) {
case NEWSET_MERGE:
	bits |= (*setList)[sets[i]];
	break;

case NEWSET_INTERSECTION:
	bits &= (*setList)[sets[i]];
	break;

case NEWSET_SUBTRACT:
	bits &= ~((*setList)[sets[i]]);
	break;
			}
		}
		if (bits.NumberSet()) delSet = FALSE;

		setList->AppendSet(bits,id);
		//		if (theHold.Holding()) theHold.Put(new AppendSetRestore(setList));
	}
	if (delSet) RemoveSubSelSet(newName);
}

void UnwrapMod::LocalDataChanged() {
	NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	if (ip && editMod==this) {
		//	SetNumSelLabel();
		ip->ClearCurNamedSelSet();
	}
}

void UnwrapMod::SetNumSelLabel() {	
	TSTR buf;
	int num = 0, which;

	if (!hParams || !ip) return;

	ModContextList mcList;
	INodeTab nodes;

	ip->GetModContexts(mcList,nodes);
	for (int i = 0; i < mcList.Count(); i++) {
		MeshTopoData *meshData = (MeshTopoData*)mcList[i]->localData;
		if (!meshData) continue;

		BitArray fsel = meshData->GetFaceSel();
		num += fsel.NumberSet();
		if (fsel.NumberSet() == 1) {
			for (which=0; which < fsel.GetSize(); which++) 
				if (fsel[which]) 
					break;
		}
	}

}

int UnwrapMod::NumSubObjTypes() 
{ 
	//return based on where
	return subObjCount;
}

ISubObjType *UnwrapMod::GetSubObjType(int i) 
{	
	static bool initialized = false;
	if(!initialized)
	{
		initialized = true;
		SOT_SelFace.SetName(GetString(IDS_PW_SELECTFACE));
		SOT_SelVerts.SetName(GetString(IDS_PW_SELECTVERTS));
		SOT_SelEdges.SetName(GetString(IDS_PW_SELECTEDGES));
		SOT_SelGizmo.SetName(GetString(IDS_PW_SELECTGIZMO));
		//		SOT_FaceMap.SetName(GetString(IDS_PW_FACEMAP));
		//		SOT_Planar.SetName(GetString(IDS_PW_PLANAR));
	}

	switch(i)
	{
	case 0:
		return &SOT_SelVerts;

	case 1:
		return &SOT_SelEdges;
	case 2:
		return &SOT_SelFace;
	case 3:
		return &SOT_SelGizmo;
	}

	return NULL;
}



//Pelt
void UnwrapMod::HitGeomEdgeData(MeshTopoData *ld, Tab<UVWHitData> &hitEdges,GraphicsWindow *gw,  HitRegion hr)
{
	hitEdges.ZeroCount();

	gw->setHitRegion(&hr);	
	gw->setRndLimits(( GW_PICK) & ~GW_ILLUM);

	for (int i = 0; i < ld->GetNumberGeomEdges()/*TVMaps.gePtrList.Count()*/; i++)
	{
		if (!(ld->GetGeomEdgeHidden(i)))
		{
			int va = ld->GetGeomEdgeVec(i,0);//TVMaps.gePtrList[i]->avec;
			int vb = ld->GetGeomEdgeVec(i,1);// TVMaps.gePtrList[i]->bvec;

			if (ld->GetPatch())
			{
				int a,b;
				a = ld->GetGeomEdgeVert(i,0);//TVMaps.gePtrList[i]->a;
				b = ld->GetGeomEdgeVert(i,1);//TVMaps.gePtrList[i]->b;

				Point3 avec,bvec;

				int faceIndex = ld->GetGeomEdgeConnectedFace(i,0);
				int deg = ld->GetFaceDegree(faceIndex);
				PatchMesh *patch = ld->GetPatch();
				for (int k = 0; k < deg; k++)
				{
					int testa = k;
					int testb = (k+1)%deg;
					int pa = ld->GetPatch()->patches[faceIndex].v[testa];
					int pb = ld->GetPatch()->patches[faceIndex].v[testb];
					if ((pa == a) && (pb == b))
					{
						int va = patch->patches[faceIndex].vec[testa*2];
						int vb = patch->patches[faceIndex].vec[testa*2+1];
						avec = patch->vecs[va].p;
						bvec = patch->vecs[vb].p;
					}
					else if ((pb == a) && (pa == b))
					{
						int va = patch->patches[faceIndex].vec[testa*2];
						int vb = patch->patches[faceIndex].vec[testa*2+1];
						avec = patch->vecs[va].p;
						bvec = patch->vecs[vb].p;
					}
				}

//				avec = ld->GetGeomVert(vb);//TVMaps.geomPoints[vb]; <- is that right?
//				bvec = ld->GetGeomVert(va);//TVMaps.geomPoints[va];
				Point3 pa,pb;
				pa = ld->GetGeomVert(a);//TVMaps.geomPoints[a];
				pb = ld->GetGeomVert(b);//TVMaps.geomPoints[b];

				Spline3D sp;
				SplineKnot ka(KTYPE_BEZIER_CORNER,LTYPE_CURVE,pa,avec,avec);
				SplineKnot kb(KTYPE_BEZIER_CORNER,LTYPE_CURVE,pb,bvec,bvec);
				sp.NewSpline();
				sp.AddKnot(ka);
				sp.AddKnot(kb);
				sp.SetClosed(0);
				sp.InvalidateGeomCache();
				Point3 ip1,ip2;
				Point3 plist[3];
				//										Draw3dEdge(gw,size, plist[0], plist[1], c);
				gw->clearHitCode();
				for (int k = 0; k < 8; k++)
				{
					float per = k/7.0f;
					ip1 = sp.InterpBezier3D(0, per);
					if ( k > 0)
					{
						plist[0] = ip1;
						plist[1] = ip2;
						gw->segment(plist,1);
					}
					ip2 = ip1;
				}
				if (gw->checkHitCode()) 
				{
					UVWHitData d;
					d.index = i;
					d.dist = gw->getHitDistance();
					hitEdges.Append(1,&d,500);
				}

			}
			else
			{

				Point3 plist[3];
				int a,b;
				a = ld->GetGeomEdgeVert(i,0);//TVMaps.gePtrList[i]->a;
				b = ld->GetGeomEdgeVert(i,1);//TVMaps.gePtrList[i]->b;

				plist[0] = ld->GetGeomVert(a);//TVMaps.geomPoints[a];
				plist[1] = ld->GetGeomVert(b);//TVMaps.geomPoints[b];

				gw->clearHitCode();
				gw->segment(plist,1);
				if (gw->checkHitCode()) 
				{
					UVWHitData d;
					d.index = i;
					d.dist = gw->getHitDistance();
					hitEdges.Append(1,&d,500);
				}
			}
		}
	}


}


void UnwrapMod::HitGeomVertData(MeshTopoData *ld, Tab<UVWHitData> &hitVerts,GraphicsWindow *gw,  HitRegion hr)
{
	hitVerts.ZeroCount();

	gw->setHitRegion(&hr);	
	gw->setRndLimits(( GW_PICK) & ~GW_ILLUM);

	for (int i = 0; i < ld->GetNumberGeomVerts(); i++)
	{
		Point3 p  = ld->GetGeomVert(i);//TVMaps.geomPoints[i];

		gw->clearHitCode();

		gw->marker(&p,POINT_MRKR);
		if (gw->checkHitCode()) 
		{
			UVWHitData d;
			d.index = i;
			d.dist = gw->getHitDistance();
			hitVerts.Append(1,&d,500);
		}
	}
}


void UnwrapMod::Move(
					 TimeValue t, Matrix3& partm, Matrix3& tmAxis, 
					 Point3& val, BOOL localOrigin) 
{	

	assert(tmControl);	
	

	if (fnGetMapMode() != SPLINEMAP)
	{
		SetXFormPacket pckt(val,Matrix3(1)/*partm*/,tmAxis);
		tmControl->SetValue(t,&pckt,TRUE,CTRL_RELATIVE);	
	}
	else if (fnGetMapMode() == SPLINEMAP)
	{		
		mSplineMap.MoveSelectedCrossSections(val);
		macroRecorder->FunctionCall(_T("$.modifiers[#unwrap_uvw].unwrap6.splineMap_moveSelectedCrossSection"), 1, 0,
												mr_point3, &val);

		if (fnGetConstantUpdate())
			fnSplineMap();

		NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
		if (ip) ip->RedrawViews(ip->GetTime());
		InvalidateView();
	}

	if (fnGetConstantUpdate())
	{
		if ((fnGetMapMode() == PLANARMAP) || (fnGetMapMode() == CYLINDRICALMAP) || (fnGetMapMode() == SPHERICALMAP) || (fnGetMapMode() == BOXMAP))
		{
			ApplyGizmo();
		}
	}

}


void UnwrapMod::Rotate(
					   TimeValue t, Matrix3& partm, Matrix3& tmAxis, 
					   Quat& val, BOOL localOrigin) 
{
	if (fnGetMapMode() != SPLINEMAP)
	{
		assert(tmControl);	
		SetXFormPacket pckt(val,localOrigin,Matrix3(1),tmAxis);
		tmControl->SetValue(t,&pckt,TRUE,CTRL_RELATIVE);	
	}
	else if (fnGetMapMode() == SPLINEMAP)
	{




		Tab<int> selSplines;
		Tab<int> selCrossSections;
		mSplineMap.GetSelectedCrossSections(selSplines,selCrossSections);

		if (selSplines.Count() > 0)
		{
			Quat localQuat = val;

			mSplineMap.RotateSelectedCrossSections(localQuat);

			macroRecorder->FunctionCall(_T("$.modifiers[#unwrap_uvw].unwrap6.splineMap_rotateSelectedCrossSection"), 1, 0,
				mr_quat, &localQuat);

			if (fnGetConstantUpdate())
				fnSplineMap();

			NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
			if (ip) ip->RedrawViews(ip->GetTime());
			InvalidateView();
		}

	}

	if (fnGetConstantUpdate())
	{
		if ((fnGetMapMode() == PLANARMAP) || (fnGetMapMode() == CYLINDRICALMAP) || (fnGetMapMode() == SPHERICALMAP) || (fnGetMapMode() == BOXMAP))
			ApplyGizmo();
	}
}

void UnwrapMod::Scale(
					  TimeValue t, Matrix3& partm, Matrix3& tmAxis, 
					  Point3& val, BOOL localOrigin) 
{

	assert(tmControl);
	if (fnGetMapMode() != SPLINEMAP)
	{
		SetXFormPacket pckt(val,localOrigin,Matrix3(1),tmAxis);
		tmControl->SetValue(t,&pckt,TRUE,CTRL_RELATIVE);	
	}
	else if (fnGetMapMode() == SPLINEMAP)
	{
		Matrix3 scaleTM = tmAxis;

		Tab<int> selSplines;
		Tab<int> selCrossSections;
		mSplineMap.GetSelectedCrossSections(selSplines,selCrossSections);

		if (selSplines.Count() > 0)
		{	
			SplineCrossSection *selCrossSection = mSplineMap.GetCrossSection(selSplines[0],selCrossSections[0]);


			Matrix3 crossSectionTM = selCrossSection->mTM;
			crossSectionTM.NoScale();
			crossSectionTM.NoTrans();

			Matrix3 toWorldSpace =  crossSectionTM;


			Point3 xScale(1.0f,0.0f,0.0f);
			Point3 yScale(0.0f,1.0f,0.0f);
			xScale = xScale * toWorldSpace;
			yScale = yScale * toWorldSpace;
			xScale = VectorTransform(xScale,Inverse(scaleTM)) * val;
			yScale = VectorTransform(yScale,Inverse(scaleTM)) * val;


			float xs = Length(xScale);
			float xy = Length(yScale);


			Point2 scale(xs,xy);
			if (scale.x < 0.0f)
				scale.x *= -1.0f;
			if (scale.y < 0.0f)
				scale.y *= -1.0f;
			mSplineMap.ScaleSelectedCrossSections(scale);
			TSTR emitString;
			emitString.printf("$.modifiers[#unwrap_uvw].unwrap6.splineMap_scaleSelectedCrossSection [%f,%f]",scale.x,scale.y);
			macroRecorder->ScriptString(emitString);

			if (fnGetConstantUpdate())
				fnSplineMap();

			NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
			if (ip) ip->RedrawViews(ip->GetTime());
			InvalidateView();
		}
	}
	if (fnGetConstantUpdate())
	{
		if ((fnGetMapMode() == PLANARMAP) || (fnGetMapMode() == CYLINDRICALMAP) || (fnGetMapMode() == SPHERICALMAP) || (fnGetMapMode() == BOXMAP))
			ApplyGizmo();
	}
}

void UnwrapMod::TransformHoldingStart(TimeValue t)
{
	if (fnGetMapMode() == SPLINEMAP)
	{
		//hold our cross section data
		mSplineMap.HoldData();
	}
}

void UnwrapMod::TransformHoldingFinish(TimeValue t)
{
	if (!fnGetConstantUpdate())
	{
		if (fnGetMapMode() == SPLINEMAP)
		{
			fnSplineMap();
		}
		else if ((fnGetMapMode() == PLANARMAP) || (fnGetMapMode() == CYLINDRICALMAP) || (fnGetMapMode() == SPHERICALMAP) || (fnGetMapMode() == BOXMAP))
			ApplyGizmo();
	}
	Matrix3 macroTM = *fnGetGizmoTM();
	TSTR mstr = GetMacroStr(_T("modifiers[#unwrap_uvw].unwrap5.setGizmoTM"));
	macroRecorder->FunctionCall(mstr, 1, 0,	mr_matrix3,&macroTM);						

}

void UnwrapMod::EnableMapButtons(BOOL enable)
{
	ICustButton *iButton = GetICustButton(GetDlgItem(hMapParams, IDC_UNWRAP_PELT2));
	iButton->Enable(enable);
	ReleaseICustButton(iButton);

	iButton = GetICustButton(GetDlgItem(hMapParams, IDC_UNWRAP_PLANAR));
	iButton->Enable(enable);
	ReleaseICustButton(iButton);

	iButton = GetICustButton(GetDlgItem(hMapParams, IDC_UNWRAP_CYLINDRICAL));
	iButton->Enable(enable);
	ReleaseICustButton(iButton);

	iButton = GetICustButton(GetDlgItem(hMapParams, IDC_UNWRAP_SPHERICAL));
	iButton->Enable(enable);
	ReleaseICustButton(iButton);

	iButton = GetICustButton(GetDlgItem(hMapParams, IDC_UNWRAP_BOX));
	iButton->Enable(enable);
	ReleaseICustButton(iButton);

	iButton = GetICustButton(GetDlgItem(hMapParams, IDC_UNWRAP_SPLINE));
	iButton->Enable(enable);
	ReleaseICustButton(iButton);

	if (mapMapMode != NOMAP)
		enable = FALSE;

	iButton = GetICustButton(GetDlgItem(hMapParams, IDC_QMAP));
	iButton->Enable(enable);
	ReleaseICustButton(iButton);

	HWND hControlHWND = GetDlgItem(hMapParams,IDC_PREVIEW_CHECK);
	EnableWindow(hControlHWND,enable);

	hControlHWND = GetDlgItem(hMapParams,IDC_RADIO1);
	EnableWindow(hControlHWND,enable);
	hControlHWND = GetDlgItem(hMapParams,IDC_RADIO2);
	EnableWindow(hControlHWND,enable);
	hControlHWND = GetDlgItem(hMapParams,IDC_RADIO3);
	EnableWindow(hControlHWND,enable);
	hControlHWND = GetDlgItem(hMapParams,IDC_RADIO4);
	EnableWindow(hControlHWND,enable);


	hControlHWND = GetDlgItem(hMapParams,IDC_NORMALIZEMAP_CHECK2);
	EnableWindow(hControlHWND,enable);

}

void UnwrapMod::EnableAlignButtons(BOOL enable)
{
	if (ip == NULL) return;
	ICustButton *iButton = GetICustButton(GetDlgItem(hMapParams, IDC_UNWRAP_ALIGNX));
	if (iButton) {
		iButton->Enable(enable);
		ReleaseICustButton(iButton);
	}

	iButton = GetICustButton(GetDlgItem(hMapParams, IDC_UNWRAP_ALIGNY));
	if (iButton) {
		iButton->Enable(enable);
		ReleaseICustButton(iButton);
	}

	iButton = GetICustButton(GetDlgItem(hMapParams, IDC_UNWRAP_ALIGNZ));
	if (iButton) {
		iButton->Enable(enable);
		ReleaseICustButton(iButton);
	}

	iButton = GetICustButton(GetDlgItem(hMapParams, IDC_UNWRAP_ALIGNNORMAL));
	if (iButton) {
		iButton->Enable(enable);
		ReleaseICustButton(iButton);
	}

	iButton = GetICustButton(GetDlgItem(hMapParams, IDC_UNWRAP_FITMAP));
	if (iButton) {
		iButton->Enable(enable);
		ReleaseICustButton(iButton);
	}

	iButton = GetICustButton(GetDlgItem(hMapParams, IDC_UNWRAP_ALIGNTOVIEW));
	if (iButton) {
		iButton->Enable(enable);
		ReleaseICustButton(iButton);
	}

	iButton = GetICustButton(GetDlgItem(hMapParams, IDC_UNWRAP_CENTER));
	if (iButton) {
		iButton->Enable(enable);
		ReleaseICustButton(iButton);
	}

	iButton = GetICustButton(GetDlgItem(hMapParams, IDC_UNWRAP_RESET));
	if (iButton) {
		iButton->Enable(enable);
		ReleaseICustButton(iButton);
	}
}



void UnwrapMod::EnableFaceSelectionUI(BOOL enable)
{
	if (ip == NULL) return;

	iPlanarThreshold->Enable(enable);
	iMatID->Enable(enable);
	iSG->Enable(enable);


	HWND hControlHWND = GetDlgItem(hSelRollup,IDC_PLANARANGLE_CHECK);
	EnableWindow(hControlHWND,enable);

	ICustButton *iTempButton = GetICustButton(GetDlgItem(hSelRollup, IDC_UNWRAP_SELECTSG));
	iTempButton->Enable(enable);
	ReleaseICustButton(iTempButton);

	iTempButton = GetICustButton(GetDlgItem(hSelRollup, IDC_UNWRAP_SELECTMATID));
	iTempButton->Enable(enable);
	ReleaseICustButton(iTempButton);

	iTempButton = GetICustButton(GetDlgItem(hSelRollup, IDC_UNWRAP_EXPANDTOSEAMS));
	iTempButton->Enable(enable);
	ReleaseICustButton(iTempButton);
}

void UnwrapMod::EnableEdgeSelectionUI(BOOL enable)
{
	ICustButton *iTempButton = GetICustButton(GetDlgItem(hSelRollup, IDC_UNWRAP_RING));
	iTempButton->Enable(enable);
	ReleaseICustButton(iTempButton);

	iTempButton = GetICustButton(GetDlgItem(hSelRollup, IDC_UNWRAP_LOOP));
	iTempButton->Enable(enable);
	ReleaseICustButton(iTempButton);

	iTempButton = GetICustButton(GetDlgItem(hSelRollup, IDC_UNWRAP_SEAMPOINTTOPOINT));
	iTempButton->Enable(enable);
	ReleaseICustButton(iTempButton);


	iTempButton = GetICustButton(GetDlgItem(hSelRollup, IDC_UNWRAP_EDGESELTOSEAMS));
	iTempButton->Enable(enable);
	ReleaseICustButton(iTempButton);

	iTempButton = GetICustButton(GetDlgItem(hSelRollup, IDC_UNWRAP_SEAMSTOEDGESEL));
	iTempButton->Enable(enable);
	ReleaseICustButton(iTempButton);
}

void UnwrapMod::EnableSubSelectionUI(BOOL enable)
{
	ICustButton *iTempButton = GetICustButton(GetDlgItem(hSelRollup, IDC_UNWRAP_SELECTEXPAND));
	iTempButton->Enable(enable);
	ReleaseICustButton(iTempButton);

	iTempButton = GetICustButton(GetDlgItem(hSelRollup, IDC_UNWRAP_SELECTCONTRACT));
	iTempButton->Enable(enable);
	ReleaseICustButton(iTempButton);

	HWND hControlHWND = GetDlgItem(hSelRollup,IDC_IGNOREBACKFACING_CHECK);
	EnableWindow(hControlHWND,enable);

	hControlHWND = GetDlgItem(hSelRollup,IDC_SELECTELEMENT_CHECK);
	EnableWindow(hControlHWND,enable);


}

void UnwrapMod::fnSetWindowXOffset(int offset)
{
	xWindowOffset = offset;

	WINDOWPLACEMENT floaterPos;
	floaterPos.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement(hWnd,&floaterPos);

	if (floaterPos.showCmd == SW_MAXIMIZE)
	{	
		Rect rect;
		GetWindowRect(hWnd,&rect);

		SetWindowPos(hWnd,NULL,rect.left,rect.top,maximizeWidth-2-xWindowOffset,maximizeHeight-yWindowOffset,SWP_NOZORDER );
		SizeDlg();
		MoveScriptUI();
	}

}
void UnwrapMod::fnSetWindowYOffset(int offset)
{
	yWindowOffset = offset;
	WINDOWPLACEMENT floaterPos;
	floaterPos.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement(hWnd,&floaterPos);

	if (floaterPos.showCmd == SW_MAXIMIZE)
	{	
		Rect rect;
		GetWindowRect(hWnd,&rect);
		SetWindowPos(hWnd,NULL,rect.left,rect.top,maximizeWidth-2-xWindowOffset,maximizeHeight-yWindowOffset,SWP_NOZORDER );
		SizeDlg();
		MoveScriptUI();
	}

}

void UnwrapMod::SetCheckerMapChannel()
{
	Mtl *checkMat = GetCheckerMap();
	if (checkMat)
	{
		Texmap *checkMap = NULL;
		checkMap = (Texmap *) checkMat->GetSubTexmap(1);
		if (checkMap)
		{
			StdUVGen *uvGen = (StdUVGen*)checkMap->GetTheUVGen();
			uvGen->SetMapChannel(channel);
		}

	}
}


BOOL UnwrapMod::GetShowImageAlpha()
{
	BOOL show; 
	pblock->GetValue(unwrap_showimagealpha,0,show,FOREVER);
	return show;
}

void UnwrapMod::SetShowImageAlpha(BOOL show)
{
	pblock->SetValue(unwrap_showimagealpha,0,show);
	SetupImage();
}




int UnwrapMod::GetQMapAlign()
{
	int align = 0;
	pblock->GetValue(unwrap_qmap_align,0,align,FOREVER);
	return align;
}

void UnwrapMod::SetQMapAlign(int align)
{	
	if (theHold.Holding())
		theHold.Put(new UnwrapMapAttributesRestore(this));
	pblock->SetValue(unwrap_qmap_align,0,align);

	HWND hWnd = hMapParams;
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
}

BOOL UnwrapMod::GetQMapPreview()
{
	BOOL preview = TRUE;
	pblock->GetValue(unwrap_qmap_preview,0,preview,FOREVER);
	return preview;
}
void UnwrapMod::SetQMapPreview(BOOL preview)
{
	if (theHold.Holding())
		theHold.Put(new UnwrapMapAttributesRestore(this));
	pblock->SetValue(unwrap_qmap_preview,0,preview);
	
	CheckDlgButton(hMapParams,IDC_PREVIEW_CHECK,preview);
}

int UnwrapMod::GetMeshTopoDataCount()
{

	return mMeshTopoData.Count();
}

MeshTopoData *UnwrapMod::GetMeshTopoData(int index)
{
	if ((index < 0) || (index >= mMeshTopoData.Count()))
	{
		DbgAssert(0);
		return NULL;
	}
	return mMeshTopoData[index];
}

INode *UnwrapMod::GetMeshTopoDataNode(int index)
{
	if ((index < 0) || (index >= mMeshTopoData.Count()))
	{
		DbgAssert(0);
		return NULL;
	}
	return mMeshTopoData.GetNode(index);
}


int UnwrapMod::ControllerAdd()
{
	int controlIndex = -1;
	for (int i = 0; i < mUVWControls.cont.Count(); i++)
	{
		if (mUVWControls.cont[i] == NULL)
		{
			controlIndex = i;
			i = mUVWControls.cont.Count(); 
		}
	}
	if (controlIndex == -1)
	{
		controlIndex =  mUVWControls.cont.Count();
		mUVWControls.cont.SetCount(controlIndex+1);
	}
	mUVWControls.cont[controlIndex] = NULL;
	ReplaceReference(controlIndex+11+100,NewDefaultPoint3Controller());
	return controlIndex;
}
int UnwrapMod::ControllerCount()
{
	return mUVWControls.cont.Count();
}

Control* UnwrapMod::Controller(int index)
{
	if ((index < 0) || (index >= mUVWControls.cont.Count()))
	{
		DbgAssert(0);
		return NULL;
	}
	return  mUVWControls.cont[index];
}

void UnwrapMod::BuildFilterSelectedFacesData()
{

	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		mMeshTopoData[ldID]->BuilFilterSelectedFaces(filterSelectedFaces);
	}
}

MeshTopoData *UnwrapMod::GetMeshTopoData(INode *node)
{
	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
		if (mMeshTopoData.GetNode(ldID) == node)
			return mMeshTopoData[ldID];
	}
	return NULL;
}

void UnwrapMod::CleanUpGeoSelection(MeshTopoData *ld)
{
//	for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
	{
//		MeshTopoData *ld = mMeshTopoData[ldID];
		BOOL hasSelection = ld->HasIncomingFaceSelection();

		if (hasSelection)
		{
			int numFaces = ld->GetNumberFaces();
			BitArray fsel = ld->GetIncomingFaceSelection();
		//loop through the face
			for (int i = 0; i < numFaces; i++)
			{
				BOOL faceSelected = ld->GetFaceSelected(i);
				BOOL faceDead = ld->GetFaceDead(i);
				if (faceDead && faceSelected)
					ld->SetFaceSelected(i,FALSE);

				int degree = ld->GetFaceDegree(i);
				for (int j = 0; j < degree; j++)
				{
					int index = ld->GetFaceGeomVert(i,j);
					if (ld->GetGeomVertSelected(index) && faceDead)
						ld->SetGeomVertSelected(index,FALSE);

					if (ld->GetFaceCurvedMaping(i) && ld->GetFaceHasVectors(i))
					{
						int index = ld->GetFaceGeomHandle(i,j*2);
						if (ld->GetGeomVertSelected(index) && faceDead)
							ld->SetGeomVertSelected(index,FALSE);
						index = ld->GetFaceGeomHandle(i,j*2+1);
						if (ld->GetGeomVertSelected(index) && faceDead)
							ld->SetGeomVertSelected(index,FALSE);
						if (ld->GetFaceHasInteriors(i))
						{
							index = ld->GetFaceGeomInterior(i,j);
							if (ld->GetGeomVertSelected(index) && faceDead)
								ld->SetGeomVertSelected(index,FALSE);
						}
					}

				}
			}
		}
	}
}

TSTR    UnwrapMod::GetMacroStr(TCHAR *macroStr)
{
	TSTR macroCommand;
	if (mMeshTopoData.Count() < 2)
	{
		macroCommand.printf("$.%s",macroStr);
		return macroCommand;
	}
	else
	{
		for (int ldID = 0; ldID < mMeshTopoData.Count(); ldID++)
		{
			INode *node = mMeshTopoData.GetNode(0);
			if (node)
			{
				macroCommand.printf("$%s.%s",node->GetName(),macroStr);
				return macroCommand;
			}
		}
	}

	macroCommand.printf("$.%s",macroStr);
	return macroCommand;
}

void UnwrapMod::GetFaceSelectionFromMesh(ObjectState *os, ModContext &mc, TimeValue t)
{
	TriObject *tobj = (TriObject*)os->obj;
	MeshTopoData *d  = (MeshTopoData*)mc.localData;
	if (d)
	{
		d->SetFaceSel(tobj->GetMesh().faceSel);
	}
}

void UnwrapMod::GetFaceSelectionFromPatch(ObjectState *os, ModContext &mc, TimeValue t)
{
	PatchObject *pobj = (PatchObject*)os->obj;
	MeshTopoData *d  = (MeshTopoData*)mc.localData;
	if (d) 
	{
		d->SetFaceSel(pobj->patch.patchSel);
	}
}

void UnwrapMod::GetFaceSelectionFromMNMesh(ObjectState *os, ModContext &mc, TimeValue t)
{
	PolyObject *tobj = (PolyObject*)os->obj;

	MeshTopoData *d  = (MeshTopoData*)mc.localData;
	if (d) 
	{
		BitArray s;
		tobj->GetMesh().getFaceSel(s);
		d->SetFaceSel(s);
	}

}


void UnwrapMod::HoldCancelBuffer()
{
	mCancelBuffer.SetCount(GetMeshTopoDataCount());
	for (int ldID = 0; ldID < GetMeshTopoDataCount(); ldID++)
	{
		MeshTopoData *ld = GetMeshTopoData(ldID);
		mCancelBuffer[ldID] = new TVertAndTFaceRestore(this,ld,FALSE);
	}
}
void UnwrapMod::RestoreCancelBuffer()
{
	theHold.Begin();
	HoldPointsAndFaces();
	theHold.Accept(GetString(IDS_PW_CANCEL));					

	for (int ldID = 0; ldID < mCancelBuffer.Count(); ldID++)
	{		
		mCancelBuffer[ldID]->Restore(FALSE);
	}
}

void UnwrapMod::FreeCancelBuffer()
{
	for (int ldID = 0; ldID < mCancelBuffer.Count(); ldID++)
	{		
		delete mCancelBuffer[ldID];
	}
	mCancelBuffer.SetCount(0);
}


void UnwrapMod::PlugControllers()
{
	TransferSelectionStart();
	for (int ldID = 0; ldID <GetMeshTopoDataCount(); ldID++)
	{
		MeshTopoData *ld = GetMeshTopoData(ldID);
		if (ld!=NULL)
			ld->PlugControllers(this);
	}
	TransferSelectionEnd(FALSE,FALSE);
}

//we need to override the default bounding box which is the object or subobject selection
//flowing up the stack
void UnwrapNodeDisplayCallback::AddNodeCallbackBox(TimeValue t, INode *node, ViewExp *vpt, Box3& box,Object *pObj)
{
	if (mod && mod->ip && (mod->ip->GetSubObjectLevel() > 0))
	{
		if (mod->ip->GetExtendedDisplayMode()& EXT_DISP_ZOOMSEL_EXT)
		{
			Interface *ip = GetCOREInterface();
			//if we are in a mapping mode use the map gizmo bounds
			if ( (ip->GetSubObjectLevel() == 3) && (mod->fnGetMapMode() != NOMAP) )
			{
				box = mod->gizmoBounds;
			}
			//otherwise just use the selection set bounds
			else if (mod->AnyThingSelected())
				box = mBounds;
		}

	}
}


