/**********************************************************************
 *<
	FILE: collapse.cpp

	DESCRIPTION:  A collapse utility

	CREATED BY: Rolf Berteig

	HISTORY: created 11/20/96

 *>	Copyright (c) 1996, All Rights Reserved.
 **********************************************************************/
#include "util.h"
#include "utilapi.h"
#include "mnmath.h"
#include "modstack.h"
#include "ICustAttribCollapseManager.h"
#include "ICustAttribContainer.h"

#ifndef NO_UTILITY_COLLAPSE	// russom - 10/16/01

#define COLLAPSE_CLASS_ID		Class_ID(0xb338aad8,0x13c75c33)
#define COLLAPSE_CNAME			GetString(IDS_RB_COLLAPSE)

class CollapseUtil : public UtilityObj, public MeshOpProgress {
	public:
		IUtil *iu;
		Interface *ip;
		HWND hPanel;
		int collapseTo, outputType, boolType;
		BOOL dobool, canceled;
		int total;

		CollapseUtil();
		void BeginEditParams(Interface *ip,IUtil *iu);
		void EndEditParams(Interface *ip,IUtil *iu);
		void DeleteThis() {}
		void SelectionSetChanged(Interface *ip,IUtil *iu);

		void Init(HWND hWnd);
		void Destroy(HWND hWnd);
		void SetStates(HWND hWnd);

		void DoCollapse();

		// From MeshOpProgress
		void Init(int total);		
		BOOL Progress(int p);
	};
static CollapseUtil theCollapseUtil;

class CollapseUtilClassDesc:public ClassDesc {
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading = FALSE) {return &theCollapseUtil;}
	const TCHAR *	ClassName() {return COLLAPSE_CNAME;}
	SClass_ID		SuperClassID() {return UTILITY_CLASS_ID;}
	Class_ID		ClassID() {return COLLAPSE_CLASS_ID;}
	const TCHAR* 	Category() {return _T("");}
	};

static CollapseUtilClassDesc collapseUtilDesc;
ClassDesc* GetCollapseUtilDesc() {return &collapseUtilDesc;}


static INT_PTR CALLBACK CollapseUtilDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	switch (msg) {
		case WM_INITDIALOG:
			theCollapseUtil.Init(hWnd);
			break;

		case WM_DESTROY:
			theCollapseUtil.Destroy(hWnd);
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDOK:
					theCollapseUtil.iu->CloseUtility();
					break;
					
				case IDC_C_MULTIPLE:
				case IDC_C_SINGLE:
					theCollapseUtil.collapseTo = LOWORD(wParam);
					theCollapseUtil.SetStates(hWnd);
					break;

				case IDC_C_BOOL:
					theCollapseUtil.dobool = IsDlgButtonChecked(hWnd,IDC_C_BOOL);
					theCollapseUtil.SetStates(hWnd);
					break;

				case IDC_C_UNION:
				case IDC_C_SUBTRACTION:
				case IDC_C_INTERSECTION:
					theCollapseUtil.boolType = LOWORD(wParam);
					theCollapseUtil.SetStates(hWnd);
					break;

				case IDC_C_STACKRESULT:
				case IDC_C_MESH:
					theCollapseUtil.outputType = LOWORD(wParam);
					theCollapseUtil.SetStates(hWnd);					
					break;

				case IDC_C_COLLAPSE:
					theCollapseUtil.DoCollapse();
					break;
				}
			break;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:
			theCollapseUtil.ip->RollupMouseMessage(hWnd,msg,wParam,lParam); 
			break;

		default:
			return FALSE;
		}
	return TRUE;
	}	


CollapseUtil::CollapseUtil()
	{	
	collapseTo = IDC_C_SINGLE;
	outputType = IDC_C_MESH;	
	dobool     = FALSE;
	boolType   = IDC_C_UNION;
	}

void CollapseUtil::BeginEditParams(Interface *ip,IUtil *iu) 
	{
	this->iu = iu;
	this->ip = ip;
	hPanel = ip->AddRollupPage(
		hInstance,
		MAKEINTRESOURCE(IDD_COLLAPSE_PANEL),
		CollapseUtilDlgProc,
		GetString(IDS_RB_COLLAPSE),
		0);
	}
	
void CollapseUtil::EndEditParams(Interface *ip,IUtil *iu) 
	{
	this->iu = NULL;
	this->ip = NULL;
	ip->DeleteRollupPage(hPanel);
	hPanel = NULL;
	}

void CollapseUtil::Init(HWND hWnd)
	{
	hPanel = hWnd;
	CheckDlgButton(hWnd,collapseTo,TRUE);
	CheckDlgButton(hWnd,outputType,TRUE);
	CheckDlgButton(hWnd,IDC_C_BOOL,dobool);
	CheckDlgButton(hWnd,boolType,TRUE);
	SelectionSetChanged(ip,iu);	
	}

void CollapseUtil::SetStates(HWND hWnd)
	{
	if (ip->GetSelNodeCount()) {
		EnableWindow(GetDlgItem(hPanel,IDC_C_COLLAPSE),TRUE);
	} else {
		EnableWindow(GetDlgItem(hPanel,IDC_C_COLLAPSE),FALSE);
		}

	if (theCollapseUtil.outputType==IDC_C_STACKRESULT) {
		EnableWindow(GetDlgItem(hWnd,IDC_C_BOOL),FALSE);
		EnableWindow(GetDlgItem(hWnd,IDC_C_MULTIPLE),FALSE);
		EnableWindow(GetDlgItem(hWnd,IDC_C_SINGLE),FALSE);
		EnableWindow(GetDlgItem(hWnd,IDC_C_COLLPASETOLABEL),FALSE);
		EnableWindow(GetDlgItem(hWnd,IDC_C_UNION),FALSE);
		EnableWindow(GetDlgItem(hWnd,IDC_C_INTERSECTION),FALSE);
		EnableWindow(GetDlgItem(hWnd,IDC_C_SUBTRACTION),FALSE);
	} else {

		if (ip->GetSelNodeCount()<2) {
			collapseTo = IDC_C_SINGLE;
			CheckDlgButton(hWnd, IDC_C_SINGLE, TRUE);
			CheckDlgButton(hWnd, IDC_C_MULTIPLE, FALSE);
			EnableWindow(GetDlgItem(hWnd,IDC_C_MULTIPLE),FALSE);
		} else {
			EnableWindow(GetDlgItem(hWnd,IDC_C_MULTIPLE),TRUE);
			}
		
		EnableWindow(GetDlgItem(hWnd,IDC_C_SINGLE),TRUE);
		EnableWindow(GetDlgItem(hWnd,IDC_C_COLLPASETOLABEL),TRUE);

		if (collapseTo==IDC_C_SINGLE) {
			EnableWindow(GetDlgItem(hWnd,IDC_C_BOOL),TRUE);
			if (theCollapseUtil.dobool) {
				EnableWindow(GetDlgItem(hWnd,IDC_C_UNION),TRUE);
				EnableWindow(GetDlgItem(hWnd,IDC_C_INTERSECTION),TRUE);
				EnableWindow(GetDlgItem(hWnd,IDC_C_SUBTRACTION),TRUE);			
			} else {
				EnableWindow(GetDlgItem(hWnd,IDC_C_UNION),FALSE);
				EnableWindow(GetDlgItem(hWnd,IDC_C_INTERSECTION),FALSE);
				EnableWindow(GetDlgItem(hWnd,IDC_C_SUBTRACTION),FALSE);
				}
		} else {
			EnableWindow(GetDlgItem(hWnd,IDC_C_BOOL),FALSE);
			EnableWindow(GetDlgItem(hWnd,IDC_C_UNION),FALSE);
			EnableWindow(GetDlgItem(hWnd,IDC_C_INTERSECTION),FALSE);
			EnableWindow(GetDlgItem(hWnd,IDC_C_SUBTRACTION),FALSE);			
			}				
		}
	}

void CollapseUtil::Destroy(HWND hWnd)
	{		
	hPanel = NULL;
	}

void CollapseUtil::SelectionSetChanged(Interface *ip,IUtil *iu)
	{
	SetStates(hPanel);
	if (ip->GetSelNodeCount()==1) {
		SetDlgItemText(hPanel,IDC_SEL_NAME,ip->GetSelNode(0)->GetName());
	} else if (ip->GetSelNodeCount()) {
		SetDlgItemText(hPanel,IDC_SEL_NAME,GetString(IDS_RB_MULTISEL));
	} else {
		SetDlgItemText(hPanel,IDC_SEL_NAME,GetString(IDS_RB_NONESEL));
		}	
	}

static ULONG _stdcall Function(void *foo) {return 0;}

class CollapseRestore : public RestoreObj {
public:		   				
	BOOL mOnlyForRedo;
	static BOOL mMode;
	CollapseRestore(BOOL onlyForRedo = FALSE) { mOnlyForRedo = onlyForRedo; }	
	void Restore(int isUndo) {
		if (mOnlyForRedo) {
			mMode = GetCOREInterface7()->InManipMode();
			if (mMode) {
				GetCOREInterface7()->EndManipulateMode();
			}
			GetCOREInterface7()->SuspendEditing();
		}
		else {
			if (mMode) {
				mMode = FALSE;
				GetCOREInterface7()->StartManipulateMode();
			}
			GetCOREInterface7()->ResumeEditing();
		}
	}
	void Redo() {
		if (!mOnlyForRedo) {
			mMode = GetCOREInterface7()->InManipMode();
			if (mMode) {
				GetCOREInterface7()->EndManipulateMode();
			}
			GetCOREInterface7()->SuspendEditing();
		}
		else {
			if (mMode) {
				mMode = FALSE;
				GetCOREInterface7()->StartManipulateMode();
			}
			GetCOREInterface7()->ResumeEditing();
		}
	}

	TSTR Description() {return TSTR(_T("Restore Manipulate State"));}
};

BOOL CollapseRestore::mMode = FALSE;

void CollapseUtil::DoCollapse()
	{
	INode *node;
	INode *tnode;
	TriObject *tobj = NULL;
	Tab<INode*> delNode;
	INodeTab flash;

	ICustAttribCollapseManager * iCM = ICustAttribCollapseManager::GetICustAttribCollapseManager();

	Matrix3 tm1, tm2;
	int type;	

	theHold.Begin();
	if (theHold.Holding()) 
		theHold.Put(new CollapseRestore);
	canceled = FALSE;

	switch (boolType) {
		default:
		case IDC_C_UNION: type = MESHBOOL_UNION; break;
		case IDC_C_INTERSECTION: type = MESHBOOL_INTERSECTION; break;
		case IDC_C_SUBTRACTION: type = MESHBOOL_DIFFERENCE; break;
		}

	for (int i=0; i<ip->GetSelNodeCount(); i++) {
		// Get a selected node
		node = ip->GetSelNode(i);
		flash.Append(1,&node,10);

		// Get the node's object (exclude WSMs)
		Object *oldObj = node->GetObjectRef();
		
		// Check for NULL
		if (!oldObj) continue;

		// Skip bones
		if (oldObj->ClassID()==Class_ID(BONE_CLASS_ID,0)) continue;		

		// RB 6/14/99: Skip system nodes too
		Control *tmCont = node->GetTMController();
		if (tmCont && GetMasterController(tmCont)) continue;


		// NH 14 April 04: Added support for the maintaining CAs on stack collapse
		// LAM - 7/22/05 - DoCollapse does not affect WSMs, So we shouldn't enumerate them....
		bool ignoreBaseObjectCAs = false;
		if(iCM && iCM->GetCustAttribSurviveCollapseState())
		{
			NotifyCollapseMaintainCustAttribEnumProc2 PreNCEP(true,node);
			EnumGeomPipeline(&PreNCEP,oldObj);
		}
		else
		{
			NotifyCollapseEnumProc PreNCEP(true,node);
			EnumGeomPipeline(&PreNCEP,oldObj);
		}

		ObjectState os = oldObj->Eval(ip->GetTime());

		HoldSuspend hs; 
		Object *obj = (Object*)os.obj->CollapseObject();
		if(obj == os.obj)
		{
			// if we are cloning the base object, the clone will take care of the CAs
			Object *theBaseObject = oldObj->FindBaseObject();
			if (obj == theBaseObject)
				ignoreBaseObjectCAs = true;

				obj = (Object*)CloneRefHierarchy(obj);
		}
		hs.Resume();
		
		if (outputType==IDC_C_STACKRESULT) {
			obj->SetSubSelState(0); // Reset the selection level to object level (in case it happens to have an edit mesh modifier
			// Make the result of the stack the new object
			oldObj->SetAFlag(A_LOCK_TARGET);
			node->SetObjectRef(obj);		
			GetCOREInterface7()->InvalidateObCache(node);
			node->NotifyDependents(FOREVER, 0, REFMSG_SUBANIM_STRUCTURE_CHANGED);

			// NS: 4/6/00 Notify all mods and objs in the pipleine, that they have been collapsed
			// NH 14 April 04: Added support for the maintaining CAs on stack collapse
			if(iCM && iCM->GetCustAttribSurviveCollapseState())
			{
				NotifyCollapseMaintainCustAttribEnumProc2 PostNCEP(false,node,ignoreBaseObjectCAs,obj);
				EnumGeomPipeline(&PostNCEP,oldObj);
			}
			else
			{
				NotifyCollapseEnumProc PostNCEP(false,node,obj);
				EnumGeomPipeline(&PostNCEP,oldObj);
			}

			oldObj->ClearAFlag(A_LOCK_TARGET);
			oldObj->MaybeAutoDelete();
		} else {
			if (collapseTo==IDC_C_SINGLE) {
				if (os.obj->CanConvertToType(triObjectClassID)) {
					// Convert to a TriObject
					hs.Suspend();
					TriObject *ntobj = (TriObject*)obj->ConvertToType(ip->GetTime(),triObjectClassID);
					hs.Resume();
					if (ntobj != obj)
						ignoreBaseObjectCAs = false;
					oldObj->SetAFlag(A_LOCK_TARGET);
					
					if (!tobj) {
						// First one
						tobj = ntobj;
						tnode = node;
						node->SetObjectRef(tobj);		
						GetCOREInterface7()->InvalidateObCache(node);
						node->NotifyDependents(FOREVER, 0, REFMSG_SUBANIM_STRUCTURE_CHANGED);
						tm1 = node->GetObjTMBeforeWSM(ip->GetTime());
					} else
					if (dobool) {
						Mesh mesh;
						tm2 = node->GetObjTMBeforeWSM(ip->GetTime());
						
						TSTR title(GetString(IDS_RB_BOOLEAN));
						title = title + TSTR(node->GetName());
						ip->ProgressStart(title, TRUE, Function, 0);

						// Steve Anderson, March 1998: switch to new Boolean.
						MNMesh op1 (tobj->GetMesh());
						MNMesh op2 (ntobj->GetMesh());
						op1.Transform (tm1);
						op2.Transform (tm2);
						op1.PrepForBoolean ();
						op2.PrepForBoolean ();
						MNMesh out;
						out.MakeBoolean (op1, op2, type, this);

// RB 2/19/99: This seems to fix the bug where operands get scattered around. Don't know why we'd want to just fix it in Viz.
//#ifdef DESIGN_VER
						out.Transform(Inverse(tm1));
//#endif
						mesh.FreeAll ();
						out.OutToTri (mesh);
						/*
						BOOL res = CalcBoolOp(
							mesh, 
							tobj->mesh, 
							ntobj->mesh, 
							type, this, &tm1, &tm2);
							*/
						
						ip->ProgressEnd();
						if (canceled) {
							oldObj->ClearAFlag(A_LOCK_TARGET);
							oldObj->MaybeAutoDelete();
							if (obj!=ntobj) {
								hs.Suspend();
								obj->MaybeAutoDelete();
								hs.Resume();
								}
							break;
							}

						tobj->GetMesh() = mesh;
						delNode.Append(1,&node);
					} else {
						Mesh mesh;
						tm2 = node->GetObjTMBeforeWSM(ip->GetTime());
						CombineMeshes(
							mesh, tobj->GetMesh(), ntobj->GetMesh(),
							&tm1, &tm2);
						tobj->GetMesh() = mesh;
						delNode.Append(1,&node);
						}
						
					// NS: 4/6/00 Notify all mods and objs in the pipleine, that they have been collapsed
					// NH 14 April 04: Added support for the maintaining CAs on stack collapse
					if(iCM && iCM->GetCustAttribSurviveCollapseState())
					{
						NotifyCollapseMaintainCustAttribEnumProc2 PostNCEP(false,node,ignoreBaseObjectCAs,tobj);
						EnumGeomPipeline(&PostNCEP,oldObj);
					}
					else
					{
						NotifyCollapseEnumProc PostNCEP(false,node,tobj);
						EnumGeomPipeline(&PostNCEP,oldObj);
					}
					oldObj->ClearAFlag(A_LOCK_TARGET);
					oldObj->MaybeAutoDelete();

					if (obj!=ntobj) {
						hs.Suspend();
						obj->MaybeAutoDelete();
						hs.Resume();
					}
				} else {
					// Can't convert it.
					hs.Suspend();
					obj->MaybeAutoDelete();
					hs.Resume();
					}
			} else {
				if (os.obj->CanConvertToType(triObjectClassID)) {
					// Convert it to a TriObject and make that the new object
					tobj = (TriObject*)obj->ConvertToType(ip->GetTime(),triObjectClassID);
					if (tobj != obj)
						ignoreBaseObjectCAs = false;
					oldObj->SetAFlag(A_LOCK_TARGET);
					node->SetObjectRef(tobj);		
					GetCOREInterface7()->InvalidateObCache(node);
					node->NotifyDependents(FOREVER, 0, REFMSG_SUBANIM_STRUCTURE_CHANGED);

					// NS: 4/6/00 Notify all mods and objs in the pipleine, that they have been collapsed
					// NH 14 April 04: Added support for the maintaining CAs on stack collapse
					if(iCM && iCM->GetCustAttribSurviveCollapseState())
					{
						NotifyCollapseMaintainCustAttribEnumProc2 PostNCEP(false,node,ignoreBaseObjectCAs,tobj);
						EnumGeomPipeline(&PostNCEP,oldObj);
					}
					else
					{
						NotifyCollapseEnumProc PostNCEP(false,node,tobj);
						EnumGeomPipeline(&PostNCEP,oldObj);
					}
					oldObj->ClearAFlag(A_LOCK_TARGET);
					oldObj->MaybeAutoDelete();
					
					if (obj!=tobj) obj->MaybeAutoDelete();
					}
				}
			}
		}

		if (canceled)
			theHold.Cancel();
		else {
	// Now delete node's that were collpased
	for (int i=0; i<delNode.Count(); i++) {
		ip->DeleteNode(delNode[i],FALSE);
		}

			if (theHold.Holding())
				theHold.Put(new CollapseRestore (TRUE));
			theHold.Accept(GetString(IDS_COLLAPSE));

	ip->RedrawViews(ip->GetTime());
			}
	}

void CollapseUtil::Init(int total)
	{
	this->total = total;
	canceled = FALSE;
	}

BOOL CollapseUtil::Progress(int p)
	{
	int pct = total?(p*100)/total:100;
	ip->ProgressUpdate(pct);
	if (ip->GetCancel()) {
		ip->SetCancel(FALSE);
		canceled = TRUE;
		return FALSE;
	} else {
		return TRUE;
		}
	}

#endif	// NO_UTILITY_COLLAPSE
