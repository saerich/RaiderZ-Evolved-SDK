/**********************************************************************
 *<
	FILE: layerctrl.cpp

	DESCRIPTION:  Implements Layer Controllers

	CREATED BY: Michael Zyracki

	HISTORY: created 12/08/2005

 *>	Copyright (c)Autodesk 2005, All Rights Reserved.
 **********************************************************************/
#include "ctrl.h"
#include "decomp.h"
#include "iparamb2.h"
#include "istdplug.h"
#include "notify.h"
#include "listctrl.h"
#include "layerctrl.h"
#include "tvnode.h"
#include "CS\BipedApi.h"
#include "ILoadSaveAnimation.h"
#include "INodeMonitor.h"
#include "3dsmaxport.h"
#include "maxIcon.h"
#include "macrorec.h"
#include "MaxMixer.h"

extern HINSTANCE hInstance;

#define FLOATLAYER_CNAME		GetString(IDS_FLOATLAYER)
#define POINT3LAYER_CNAME	GetString(IDS_POINT3LAYER)
#define POSLAYER_CNAME		GetString(IDS_POSITIONLAYER)
#define ROTLAYER_CNAME		GetString(IDS_ROTATIONLAYER)
#define SCALELAYER_CNAME		GetString(IDS_SCALELAYER)
#define POINT4LAYER_CNAME	GetString(IDS_POINT4LAYER)


#define IPOS_CONTROL_CLASS_ID Class_ID(0x118f7e02,0xffee238a)
#define ISCALE_CONTROL_CLASS_ID     Class_ID(0x118f7c01,0xfeee238b)

static void GetSelectedNodes(Tab<INode *> &nodeTab)
{
	nodeTab.SetCount(GetCOREInterface()->GetSelNodeCount());
	for(int i=0;i<GetCOREInterface()->GetSelNodeCount();++i)
	{
		INode *node = GetCOREInterface()->GetSelNode(i);
		nodeTab[i] = node;
	}

}

//this is used to find the slave parent of the subanim reference
// private namespace
namespace
{
class GetNodesProc: public DependentEnumProc 
{
public:
   GetNodesProc(){}
   virtual int proc(ReferenceMaker *rmaker); 

   INodeTab nodes; //really only should be one.
};


int GetNodesProc::proc(ReferenceMaker *rmaker) 
{ 
   if (rmaker->SuperClassID()==BASENODE_CLASS_ID)    
   {
	   INode *node = static_cast<INode *>(rmaker);
	   nodes.Append(1,&node);
			return DEP_ENUM_SKIP;
   }
   return DEP_ENUM_CONTINUE;
}
}

static void PositionWindowNearCursor(HWND hwnd)
{
    RECT rect;
    POINT cur;
    GetWindowRect(hwnd,&rect);
    GetCursorPos(&cur);
	int scrwid = GetScreenWidth();
	int scrhgt = GetScreenHeight();
    int winwid = rect.right - rect.left;
    int winhgt = rect.bottom - rect.top;
    cur.x -= winwid/3; // 1/3 of the window should be to the left of the cursor
    cur.y -= winhgt/3; // 1/3 of the window should be above the cursor
    if (cur.x + winwid > scrwid - 10) cur.x = scrwid - winwid - 10; // check too far right 
	if (cur.x < 10) cur.x = 10; // check too far left
    if (cur.y + winhgt> scrhgt - 50) cur.y = scrhgt - 50 - winhgt; // check if too low
	if (cur.y < 10) cur.y = 10; // check for too high s45
    MoveWindow(hwnd, cur.x, cur.y, rect.right - rect.left, rect.bottom - rect.top, TRUE);
}

static void UnRegisterLayerCtrlWindow(HWND hWnd);
MasterLayerControlManager * GetMLCMan();

enum { kName=0,kLayerCtrlWeight,kLayerCtrlAverage,kX,kY,kZ,kW,kX_order,kY_order,kZ_order,kBlendEulerAsQuat };		// parameter IDs

static bool IsLayerControl(Animatable *anim)
{
	if(anim != NULL && 
		(anim->ClassID() == (FLOATLAYER_CONTROL_CLASS_ID) ||
		anim->ClassID() == (POINT3LAYER_CONTROL_CLASS_ID) ||
		anim->ClassID() == (POSLAYER_CONTROL_CLASS_ID) ||
		anim->ClassID() == (ROTLAYER_CONTROL_CLASS_ID) ||
		anim->ClassID() == (SCALELAYER_CONTROL_CLASS_ID) ||
		anim->ClassID() == (POINT4LAYER_CONTROL_CLASS_ID)))
	{
		return true;
	}
	return false;

}

static bool LayerActiveOnlyFilter(Animatable* anim, Animatable* parent, int subAnimIndex, Animatable* grandParent, INode* node)
{
	if(IsLayerControl(parent)==true&&GetMLCMan())
	{
		MasterLayerControlManager *man = GetMLCMan();
		if(man->GetFilterActiveOnlyTrackView()==FALSE)
		{
			LayerControl *lC = static_cast<LayerControl *>(parent);
			if (subAnimIndex == (lC)->GetLayerActive() || subAnimIndex >= ((lC)->GetLayerCount()))
				return true;
		}
		else
		{
			if(subAnimIndex==0)
				return true;
		}
		return false;
	}
	return true;
}

static bool LayerNoWeightsFilter(Animatable* anim, Animatable* parent, int subAnimIndex, Animatable* grandParent, INode* node)
{
	if(IsLayerControl(parent)&&GetMLCMan())
	{

		MasterLayerControlManager *man = GetMLCMan();
		if(man->GetFilterActiveOnlyTrackView()==FALSE)
		{

			LayerControl *lC = static_cast<LayerControl*>(parent);
			if (subAnimIndex >= lC->GetLayerCount())
				return false;
		}
		else
		{
			if(subAnimIndex>0)
				return false;
		}
	}
	return true;
}

void LayerCtrlNotifyStartup(void *param, NotifyInfo *info)
{
	ITrackBar* tb = GetCOREInterface()->GetTrackBar();
	if (tb != NULL)
	{
		ITrackBarFilterManager* filterManager = (ITrackBarFilterManager*)tb->GetInterface(TRACKBAR_FILTER_MANAGER_INTERFACE);
		if (filterManager != NULL)
		{
			filterManager->RegisterFilter(LayerActiveOnlyFilter, NULL, GetString(IDS_LAYER_ACTIVE_ONLY_FILTER),Class_ID(0x1adf422d, 0x38145d5f),true);
			filterManager->RegisterFilter(LayerNoWeightsFilter, NULL, GetString(IDS_LAYER_NO_WEIGHTS_FILTER), Class_ID(0x500513af, 0x7f6267e8));
		}
	}
}






static void GetSubAnims(Animatable *anim,Tab<Animatable *>&anims)
{
	if(anim!=NULL)
	{
		anims.Append(1,&anim);
		for(int i=0;i<anim->NumSubs();++i)
		{
			Animatable *subAnim = anim->SubAnim(i);
			if(subAnim)
			{
				GetSubAnims(subAnim,anims);
			}
		}
	}
}

static void GetKeyTimes(Animatable *anim,Tab<TimeValue> &keyTimes,Interval &range,bool getSubTimes)
{
	Tab<Animatable*> anims;
	if(getSubTimes)
		GetSubAnims(anim,anims);
	else
		anims.Append(1,&anim);
	anims.Shrink();
	Tab<TimeValue> tempTimes;
	TimeValue t;
	for(int i=0;i<anims.Count();++i)
	{
		if(anims[i]&&anims[i]->NumKeys()>0)
		{
			anims[i]->GetKeyTimes(tempTimes,range,0);
			int k=0;
			for(int j=0;j<keyTimes.Count();++j)
			{
				bool go =true;
				while(go)
				{
					if(k>=tempTimes.Count())
					{
						go = false;
						j = keyTimes.Count();
					}
					else if( tempTimes[k]> keyTimes[j])
					{
						go = false; //exit out and up j
					}
					else
					{
						if(tempTimes[k]<keyTimes[j])
						{
							t = tempTimes[k];
							keyTimes.Append(1,&t);
						}
						++k;
					}
				}
			}
			for(k;k<tempTimes.Count();++k)
			{
				t = tempTimes[k];
				keyTimes.Append(1,&t);
			}
		
		}
	}
}

//recursive deletes all child keys too!
static void DeleteAllKeys(Animatable *anim)
{

	if(anim->NumKeys()>0)
		anim->DeleteKeys(TRACK_DOALL);
    for(int i =0;i<anim->NumSubs();++i)
	{
        Animatable *subAnim = anim->SubAnim(i);
		if(subAnim)
			DeleteAllKeys(subAnim);

	}

}


class LayerCtrlTrackBarFilterRegister
{
public:
	LayerCtrlTrackBarFilterRegister()
	{
		RegisterNotification(LayerCtrlNotifyStartup, NULL, NOTIFY_SYSTEM_STARTUP);
	}
};
LayerCtrlTrackBarFilterRegister theLayerCtrlTrackBarFilterRegister;



/**********************************************************************************

Popup Dialogs:
  A)  EnableAnimLayersDlg
  B)  AddLayerDlg

**********************************************************************************/

struct EnableAnimFilterItems
{
	EnableAnimFilterItems():all(FALSE), pos(TRUE), rot(TRUE), scale(TRUE), IK(FALSE),
		object(FALSE),CA(FALSE),mod(FALSE),mat(FALSE),other(FALSE){}

	BOOL all;
	BOOL pos;
	BOOL rot;
	BOOL scale;
	BOOL IK;
	BOOL object;
	BOOL CA;
	BOOL mod;
	BOOL mat;
	BOOL other;

};

static EnableAnimFilterItems sEnableAnimFilters;

INT_PTR CALLBACK enableAnimFiltersProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	 switch (msg)  
	 {
	 case WM_INITDIALOG:
		 {
		 PositionWindowNearCursor(hDlg);
		 CheckDlgButton(hDlg, IDC_ENABLE_ANIM_POSITION,sEnableAnimFilters.pos);
		 CheckDlgButton(hDlg, IDC_ENABLE_ANIM_ROTATION,sEnableAnimFilters.rot);
		 CheckDlgButton(hDlg, IDC_ENABLE_ANIM_SCALE,sEnableAnimFilters.scale);
		 CheckDlgButton(hDlg, IDC_ENABLE_ANIM_IK_PARAMS,sEnableAnimFilters.IK);
		 CheckDlgButton(hDlg, IDC_ENABLE_ANIM_OBJECTPARAMS,sEnableAnimFilters.object);
		 CheckDlgButton(hDlg, IDC_ENABLE_ANIM_CUSTOMATTRIBUTES,sEnableAnimFilters.CA);
		 CheckDlgButton(hDlg, IDC_ENABLE_ANIM_MODIFIERS,sEnableAnimFilters.mod);
		 CheckDlgButton(hDlg, IDC_ENABLE_ANIM_MATERIALS,sEnableAnimFilters.mat);
		 CheckDlgButton(hDlg, IDC_ENABLE_ANIM_OTHER,sEnableAnimFilters.other);
		 CheckDlgButton(hDlg, IDC_ENABLE_ANIM_ALL,sEnableAnimFilters.all);
		 if(sEnableAnimFilters.all)
		 {
			EnableWindow(GetDlgItem(hDlg,IDC_ENABLE_ANIM_POSITION),FALSE);
			EnableWindow(GetDlgItem(hDlg,IDC_ENABLE_ANIM_ROTATION),FALSE);
			EnableWindow(GetDlgItem(hDlg,IDC_ENABLE_ANIM_SCALE),FALSE);
			EnableWindow(GetDlgItem(hDlg,IDC_ENABLE_ANIM_IK_PARAMS),FALSE);
			EnableWindow(GetDlgItem(hDlg,IDC_ENABLE_ANIM_OBJECTPARAMS),FALSE);
			EnableWindow(GetDlgItem(hDlg,IDC_ENABLE_ANIM_CUSTOMATTRIBUTES),FALSE);
			EnableWindow(GetDlgItem(hDlg,IDC_ENABLE_ANIM_MODIFIERS),FALSE);
			EnableWindow(GetDlgItem(hDlg,IDC_ENABLE_ANIM_MATERIALS),FALSE);
			EnableWindow(GetDlgItem(hDlg,IDC_ENABLE_ANIM_OTHER),FALSE);
		 }

	 }
	 break;
	 case WM_COMMAND:
		 switch (LOWORD(wParam))
		 {
		 case IDC_ENABLE_ANIM_ALL:
			 if (IsDlgButtonChecked(hDlg,IDC_ENABLE_ANIM_ALL))
			 {
				EnableWindow(GetDlgItem(hDlg,IDC_ENABLE_ANIM_POSITION),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_ENABLE_ANIM_ROTATION),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_ENABLE_ANIM_SCALE),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_ENABLE_ANIM_IK_PARAMS),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_ENABLE_ANIM_OBJECTPARAMS),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_ENABLE_ANIM_CUSTOMATTRIBUTES),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_ENABLE_ANIM_MODIFIERS),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_ENABLE_ANIM_MATERIALS),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_ENABLE_ANIM_OTHER),FALSE);
			 }
			 else
			 {
				EnableWindow(GetDlgItem(hDlg,IDC_ENABLE_ANIM_POSITION),TRUE);
				EnableWindow(GetDlgItem(hDlg,IDC_ENABLE_ANIM_ROTATION),TRUE);
				EnableWindow(GetDlgItem(hDlg,IDC_ENABLE_ANIM_SCALE),TRUE);
				EnableWindow(GetDlgItem(hDlg,IDC_ENABLE_ANIM_IK_PARAMS),TRUE);
				EnableWindow(GetDlgItem(hDlg,IDC_ENABLE_ANIM_OBJECTPARAMS),TRUE);
				EnableWindow(GetDlgItem(hDlg,IDC_ENABLE_ANIM_CUSTOMATTRIBUTES),TRUE);
				EnableWindow(GetDlgItem(hDlg,IDC_ENABLE_ANIM_MODIFIERS),TRUE);
				EnableWindow(GetDlgItem(hDlg,IDC_ENABLE_ANIM_MATERIALS),TRUE);
				EnableWindow(GetDlgItem(hDlg,IDC_ENABLE_ANIM_OTHER),TRUE);
			 }
			 break;
		 case IDOK:
			 if (IsDlgButtonChecked(hDlg,IDC_ENABLE_ANIM_ALL))
				 sEnableAnimFilters.all = TRUE;
			 else
				 sEnableAnimFilters.all = FALSE;
			 if (IsDlgButtonChecked(hDlg,IDC_ENABLE_ANIM_POSITION))
				 sEnableAnimFilters.pos= TRUE;
			 else
				 sEnableAnimFilters.pos= FALSE;
			 if (IsDlgButtonChecked(hDlg,IDC_ENABLE_ANIM_ROTATION))
				 sEnableAnimFilters.rot= TRUE;
			 else
				 sEnableAnimFilters.rot = FALSE;
			 if (IsDlgButtonChecked(hDlg,IDC_ENABLE_ANIM_SCALE))
				 sEnableAnimFilters.scale = TRUE;
			 else
				 sEnableAnimFilters.scale = FALSE;
			 if (IsDlgButtonChecked(hDlg,IDC_ENABLE_ANIM_IK_PARAMS))
				 sEnableAnimFilters.IK = TRUE;
			 else
				 sEnableAnimFilters.IK= FALSE;
			 if (IsDlgButtonChecked(hDlg,IDC_ENABLE_ANIM_OBJECTPARAMS))
				 sEnableAnimFilters.object= TRUE;
			 else
				 sEnableAnimFilters.object = FALSE;
			 if (IsDlgButtonChecked(hDlg,IDC_ENABLE_ANIM_CUSTOMATTRIBUTES))
				 sEnableAnimFilters.CA= TRUE;
			 else
				 sEnableAnimFilters.CA = FALSE;
			 if (IsDlgButtonChecked(hDlg,IDC_ENABLE_ANIM_MODIFIERS))
				 sEnableAnimFilters.mod = TRUE;
			 else
				 sEnableAnimFilters.mod = FALSE;
			 if (IsDlgButtonChecked(hDlg,IDC_ENABLE_ANIM_MATERIALS))
				 sEnableAnimFilters.mat = TRUE;
			 else
				 sEnableAnimFilters.mat = FALSE;
			 if (IsDlgButtonChecked(hDlg,IDC_ENABLE_ANIM_OTHER))
				 sEnableAnimFilters.other= TRUE;
			 else
				 sEnableAnimFilters.other = FALSE;

			 EndDialog(hDlg,1);
			 break;
		 case IDCANCEL:
			 EndDialog(hDlg,0);
			break;
		 }
		 break;
	 case WM_CLOSE:
		 EndDialog(hDlg, 0);
	 default:
		 return FALSE;
	 }
	 return TRUE;
}


struct AddLayerInfo
{
	BOOL duplicateActive;
	Tab<TSTR*> layerNames;
	int numLayerNames; //not including the new edited layer name
	int selectedLayer;
	AddLayerInfo() :selectedLayer(0),duplicateActive(TRUE),numLayerNames(0){};
	~AddLayerInfo();
};
AddLayerInfo::~AddLayerInfo()
{
	for(int i=0; i < layerNames.Count(); ++i)
	{
		if(layerNames[i])
		{
			delete layerNames[i];
			layerNames[i] = NULL;
		}
	}
}
          

INT_PTR CALLBACK addLayerProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	AddLayerInfo *info = DLGetWindowLongPtr<AddLayerInfo*>(hDlg);
	
	switch (msg) {
    case WM_INITDIALOG:
    {
		info = (AddLayerInfo*)lParam;
        DLSetWindowLongPtr(hDlg, lParam);
 	    PositionWindowNearCursor(hDlg);

		SendDlgItemMessage(hDlg,IDC_LAYERS,CB_RESETCONTENT,0,0);
		for (int i=0; i<info->layerNames.Count(); i++)
		{
			SendDlgItemMessage(hDlg,IDC_LAYERS,CB_ADDSTRING,0,
         (LPARAM)info->layerNames[i]->data());
		}
		SendDlgItemMessage(hDlg,IDC_LAYERS,CB_SETCURSEL,info->selectedLayer,0);
		HWND comboCtl = GetDlgItem(hDlg,IDC_LAYERS);
		Rect rect;
		GetClientRectP(comboCtl, &rect);
		SetWindowPos(comboCtl, NULL, 0, 0, rect.w()-1, GetListHieght(comboCtl),
					SWP_NOZORDER|SWP_NOMOVE|SWP_NOREDRAW|SWP_NOACTIVATE);
		SendMessage(comboCtl, CB_SETEDITSEL, 0, MAKELPARAM(0,0)); // this sets the edited text selection to be null

		if(info->duplicateActive)
			 CheckRadioButton(hDlg, IDC_DUPLICATE, IDC_DEFAULT, IDC_DUPLICATE);
		else
			 CheckRadioButton(hDlg, IDC_DUPLICATE, IDC_DEFAULT, IDC_DEFAULT);
       
	 }
	 break;
	 case WM_COMMAND:
		 switch (LOWORD(wParam))
		 {
		 case IDC_LAYERS:
			 switch( HIWORD(wParam) )
			 {
				 case CBN_DROPDOWN:
			     {
					 // open up the combo box
					SendDlgItemMessage(hDlg,IDC_LAYERS,CB_RESETCONTENT,0,0);
					for (int i=0; i<info->layerNames.Count(); i++)
					{
						SendDlgItemMessage(hDlg,IDC_LAYERS,CB_ADDSTRING,0,
					 (LPARAM)info->layerNames[i]->data());
					}
					SendDlgItemMessage(hDlg,IDC_LAYERS,CB_SETCURSEL,(WPARAM)info->selectedLayer,0);
					HWND comboCtl = GetDlgItem(hDlg,IDC_LAYERS);
					Rect rect;
					GetClientRectP(comboCtl, &rect);
					SetWindowPos(comboCtl, NULL, 0, 0, rect.w()-1, GetListHieght(comboCtl),
								SWP_NOZORDER|SWP_NOMOVE|SWP_NOREDRAW|SWP_NOACTIVATE);
					SendMessage(comboCtl, CB_SETEDITSEL, 0, MAKELPARAM(0,0)); // this sets the edited text selection to be null

					return TRUE;
				  }

			   case CBN_CLOSEUP:  // close up the combo box
					SendMessage((HWND)lParam, CB_SETEDITSEL, 0, MAKELPARAM(0,0));
					return TRUE;

			   case CBN_SELCHANGE: { // select a new item from the combo box
					info->selectedLayer =  SendMessage((HWND)lParam, CB_GETCURSEL, 0,0);
				    SendMessage((HWND)lParam, CB_SETEDITSEL, 0, MAKELPARAM(0,0));                            
					SendDlgItemMessage(hDlg,IDC_LAYERS,CB_RESETCONTENT,0,0);
					for (int i=0; i<info->layerNames.Count(); i++)
					{
						SendDlgItemMessage(hDlg,IDC_LAYERS,CB_ADDSTRING,0,
					 (LPARAM)info->layerNames[i]->data());
					}
					SendDlgItemMessage(hDlg,IDC_LAYERS,CB_SETCURSEL,(WPARAM)info->selectedLayer,0);
					HWND comboCtl = GetDlgItem(hDlg,IDC_LAYERS);
					Rect rect;
					GetClientRectP(comboCtl, &rect);
					SetWindowPos(comboCtl, NULL, 0, 0, rect.w()-1, GetListHieght(comboCtl),
								SWP_NOZORDER|SWP_NOMOVE|SWP_NOREDRAW|SWP_NOACTIVATE);
					SendMessage(comboCtl, CB_SETEDITSEL, 0, MAKELPARAM(0,0)); // this sets the edited text selection to be null

					SetFocus(hDlg); // return focus to dialog, not selected item
					} return TRUE;

			   case CBN_EDITUPDATE: {
					TCHAR bufstr[256];
					SendMessage((HWND)lParam, WM_GETTEXT, 256, (LPARAM)bufstr);
					TSTR *newName = new TSTR(bufstr);

					int index = info->layerNames.Count()-1;
					delete info->layerNames[index];
					info->layerNames[index] = newName;
					}
					return TRUE;

			   case CBN_SETFOCUS:
					DisableAccelerators(); // MAX call
					return TRUE;

			   case CBN_KILLFOCUS:
					EnableAccelerators(); // MAX call
					return TRUE;
			  }
			 break;
		 case IDOK: {
					if(IsDlgButtonChecked(hDlg,IDC_DUPLICATE))
						info->duplicateActive = TRUE;
					else info->duplicateActive = FALSE;
					EndDialog(hDlg,1);
					}
			 break;
		 case IDCANCEL:
					 EndDialog(hDlg,0);
			break;
		 }
		 break;
	 case WM_CLOSE:
		 EndDialog(hDlg, 0);
	 default:
		 return FALSE;
	 }
	 return TRUE;
}


#define LAYERDLG_CONTREF	0
#define LAYERDLG_CLASS_ID	0x833ba62a
class LayerControlDlg : public ReferenceMaker, public TimeChangeCallback {
	public:
		IObjParam *ip;
		LayerControl *cont;
		HWND hWnd;
		BOOL valid; 

		LayerControlDlg(IObjParam *i,LayerControl *c);
		~LayerControlDlg();

		Class_ID ClassID() {return Class_ID(LAYERDLG_CLASS_ID,0);}
		SClass_ID SuperClassID() {return REF_MAKER_CLASS_ID;}

		void StartEdit(int a);
		void EndEdit(int a);
		void SetButtonStates();
		void Init(HWND hParent);
		void Reset(IObjParam *i,LayerControl *c);
		void Invalidate();
		void Update();
		void SetupUI();
		void SetupList();
		void UpdateList();

		void TimeChanged(TimeValue t) {Invalidate(); Update();}
		
		virtual HWND CreateWin(HWND hParent)=0;
		virtual void MouseMessage(UINT message,WPARAM wParam,LPARAM lParam) {};	
		virtual void MaybeCloseWindow() {}

		void WMCommand(int id, int notify, HWND hCtrl);
		
		RefResult NotifyRefChanged(Interval, RefTargetHandle, PartID&, RefMessage);
		int NumRefs();
		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rtarg);
	};

class LayerControlMotionDlg : public LayerControlDlg {
	public:
		
		LayerControlMotionDlg(IObjParam *i,LayerControl *c)
			: LayerControlDlg(i,c) {}
					
		HWND CreateWin(HWND hParent);
		void MouseMessage(UINT message,WPARAM wParam,LPARAM lParam) 
			{ip->RollupMouseMessage(hWnd,message,wParam,lParam);}
	};

class LayerControlTrackDlg : public LayerControlDlg {
	public:
		
		LayerControlTrackDlg(IObjParam *i,LayerControl *c)
			: LayerControlDlg(i,c) {}
					
		HWND CreateWin(HWND hParent);
		void MouseMessage(UINT message,WPARAM wParam,LPARAM lParam) {};
		void MaybeCloseWindow();
	};

class NotifySelectionRestore : public RestoreObj{

public:
	NotifySelectionRestore(){};
	void Restore(int isUndo){GetMLCMan()->NotifySelectionChanged();}
	void Redo(){GetMLCMan()->NotifySelectionChanged();}
};



class MasterLayerControlRestore : public RestoreObj {
	public:
		MasterLayerControl *mLC;
		Tab<LayerControl*> rlayers,ulayers;	//Pointer to all of the layers controllers that have this layer
		Tab<ReferenceTarget *> rmonitorNodes,umonitorNodes; //INodeMonitor reference

		MasterLayerControlRestore(MasterLayerControl *c) 
		{
			mLC   = c;
			ulayers = mLC->layers;
			umonitorNodes = mLC->monitorNodes;
		}   		
		void Restore(int isUndo) 
		{	   
			rlayers = mLC->layers;
			rmonitorNodes = mLC->monitorNodes;

			mLC->layers = ulayers;
			mLC->monitorNodes = umonitorNodes;
			mLC->NotifyDependents(FOREVER,0,REFMSG_CHANGE);			
			mLC->NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);
		}
		void Redo() 
		{			
			mLC->layers = rlayers;
			mLC->monitorNodes = rmonitorNodes;

			mLC->NotifyDependents(FOREVER,0,REFMSG_CHANGE);
			mLC->NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);
		}				
};


class LayerControlRestore : public RestoreObj 
{
	public:
		LayerControl *cont;
		Tab<int> umLCs,rmLCs; //the masters this guy belongs to.
		Tab<Control*> uconts,rconts;	//actually controls that make it up
		int uactive,ractive;

		LayerControlRestore(LayerControl *c) 
		{
			cont   = c;
			uconts   = c->conts;
			umLCs = cont->mLCs;
			uactive = cont->active;
		}   		
		void Restore(int isUndo) 
		{
			rconts = cont->conts;
			rmLCs = cont->mLCs;
			ractive = cont->active;

			cont->conts = uconts;
			cont->mLCs = umLCs;
			cont->active = uactive;
			cont->NotifyDependents(FOREVER,0,REFMSG_CHANGE);			
			cont->NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);
		}
		void Redo() 
		{			
			cont->conts  = rconts;
			cont->mLCs = rmLCs;
			cont->active = ractive;
			cont->NotifyDependents(FOREVER,0,REFMSG_CHANGE);
			cont->NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);
		}				
};


// The restore object handles stopping and starting editing
// durring an undo. One of these is placed before and after
// a copy copy/paste/delete operation.
class RemoveLayerItemRestore : public RestoreObj {
	public:   		
		LayerControl *cont;
		BOOL parity;

		RemoveLayerItemRestore(LayerControl *c, BOOL p)
			{cont=c; parity=p;}
		void Restore(int isUndo) {
			if (cont->dlg) {
				if (parity) cont->dlg->EndEdit(cont->active);
				else cont->dlg->StartEdit(cont->active);
				}
			}
		void Redo() {
			if (cont->dlg) {
				if (!parity) cont->dlg->EndEdit(cont->active);
				else cont->dlg->StartEdit(cont->active);
				}
			}
		TSTR Description() {return TSTR(_T("Layer control remove item"));}
	};



// This restore object doesn't do anything except keep the clipboard
// item from being deleted.
class CopyRestore : public RestoreObj, public ReferenceMaker {	
	public:	
		Control *cont;
		CopyRestore(Control *c) {
			cont = NULL;
			theHold.Suspend();
			ReplaceReference(0,c);
			theHold.Resume();
			}
		~CopyRestore() {
			theHold.Suspend();
			DeleteReference(0);
			theHold.Resume();
			}
		void Restore(int isUndo) {}
		void Redo() {}
		TSTR Description() {return TSTR(_T("Layer Control Copy"));}
		int NumRefs() {return 1;}
		RefTargetHandle GetReference(int i) {return cont;}
		void SetReference(int i, RefTargetHandle rtarg) {cont=(Control*)rtarg;}
		RefResult NotifyRefChanged( Interval changeInt,RefTargetHandle hTarget, 
		   PartID& partID, RefMessage message) {
			if (message==REFMSG_TARGET_DELETED && hTarget==cont) {
				cont=NULL;
				}
			return REF_SUCCEED;
			}
		BOOL CanTransferReference(int i) { return FALSE; }
};


// CAL-06/06/02: TODO: this should really go to core\decomp.cpp, and it should be optimized.
//		For now, it's defined locally in individual files in the ctrl project.
static void comp_affine( const AffineParts &ap, Matrix3 &mat )
{
	Matrix3 tm;
	
	mat.IdentityMatrix();
	mat.SetTrans( ap.t );

	if ( ap.f != 1.0f ) {				// has f component
		tm.SetScale( Point3( ap.f, ap.f, ap.f ) );
		mat = tm * mat;
	}

	if ( !ap.q.IsIdentity() ) {			// has q rotation component
		ap.q.MakeMatrix( tm );
		mat = tm * mat;
	}
	
	if ( ap.k.x != 1.0f || ap.k.y != 1.0f || ap.k.z != 1.0f ) {		// has k scale component
		tm.SetScale( ap.k );
		if ( !ap.u.IsIdentity() ) {			// has u rotation component
			Matrix3 utm;
			ap.u.MakeMatrix( utm );
			mat = Inverse( utm ) * tm * utm * mat;
		} else {
			mat = tm * mat;
		}
	}
}
/**********************************************************

	LayerOutputControl
	This class handles outputing indivdual float controllers for the layer controls.

************************************************************/

//like expose output control but different!
class LayerOutputControl : public Control, public IUnReplaceableControl
{
public:

	LayerOutputControl(): paramID(-1),lC(NULL){};
	LayerOutputControl(LayerControl *l,int id):paramID(id),lC(l){};
	~LayerOutputControl(){DeleteAllRefsFromMe();};

	int IsKeyable(){return 0;}
	BOOL IsReplaceable(){return FALSE;}
	BOOL CanApplyEaseMultCurves() {return FALSE;}
	BOOL IsAnimated() {return TRUE;}
	BOOL CanInstanceController() {return FALSE;} //in theory only for tranform but you never know
	RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
		PartID& partID,  RefMessage message){return REF_SUCCEED;}
	BOOL IsLeaf() {return TRUE;}
	void CommitValue(TimeValue t) { }
	void RestoreValue(TimeValue t) { }
	void DeleteThis();
	BOOL CanCopyAnim(){return FALSE;}
	void* GetInterface(ULONG id) { if (id ==  I_UNREPLACEABLECTL) return (IUnReplaceableControl*)this; else return Control::GetInterface(id); }


	//ID
	int paramID;
	LayerControl *lC;

	void Copy(Control *from);
	void GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method=CTRL_ABSOLUTE);	
	void SetValue(TimeValue t, void *val, int commit=1, GetSetMethod method=CTRL_ABSOLUTE);

	//from IUnReplaceableControl
	Control * GetReplacementClone();

	//From Animatable
	Class_ID ClassID() {return  LAYEROUTPUT_CONTROL_CLASS_ID;}		
	SClass_ID SuperClassID() { return CTRL_FLOAT_CLASS_ID; }
	void GetClassName(TSTR& s) {s = GetString(IDS_LAYEROUTPUT_CLASS_NAME);}

	RefTargetHandle Clone(RemapDir& remap);
};

void LayerOutputControl::Copy(Control *from)
{
	return;
}

void LayerOutputControl::DeleteThis()
{
	delete this;
}

void LayerOutputControl::GetValue(TimeValue t, void *v, Interval &valid, GetSetMethod method)
{

	float * ourV = static_cast<float *>(v);
	if(method == CTRL_ABSOLUTE)
		*ourV = 0.0f;

	if(lC==NULL)
		return;
	
	if ((lC->SuperClassID() == CTRL_FLOAT_CLASS_ID) )
	{
		float what;
		lC->GetOutputValue(t,&what,valid);
		*ourV += what;
	}
	else if ((lC->SuperClassID() == CTRL_POSITION_CLASS_ID) )
	{
		Point3 val;
		lC->GetOutputValue(t,&val,valid);
		*ourV = *ourV+val[paramID];
	}
	else if ((lC->SuperClassID() == CTRL_POINT3_CLASS_ID) )
	{
		Point3 val;
		lC->GetOutputValue(t,&val,valid);
		*ourV = *ourV +val[paramID];
	}
	else if ((lC->SuperClassID() == CTRL_ROTATION_CLASS_ID) )
	{
		Point3 p;
		lC->GetOutputValue(t,&p,valid);
		*ourV = *ourV +p[paramID];
	}
	else if ((lC->SuperClassID() == CTRL_SCALE_CLASS_ID) )
	{
		ScaleValue val;
		lC->GetOutputValue(t,&val,valid);
		*ourV = *ourV+val.s[paramID];
	}
	else if ((lC->SuperClassID() == CTRL_POINT4_CLASS_ID) )
	{
		Point4 val;
		lC->GetOutputValue(t,&val,valid);
		*ourV = *ourV+val[paramID];
	}
}

void LayerOutputControl::SetValue(TimeValue t, void *val, int commit, GetSetMethod method)
{
}

Control * LayerOutputControl::GetReplacementClone()
{
	Control *control = NewDefaultFloatController();
	if(control)
	{
		// set key per frame
		Interval range =GetCOREInterface()->GetAnimRange();
		TimeValue tpf = GetTicksPerFrame();	
		SuspendAnimate();
		AnimateOn();
		float v;
		for(TimeValue t= range.Start(); t<=range.End();t+=tpf)
		{
			GetValue(t,&v,Interval(t,t));
			control->SetValue(t,&v,1,CTRL_ABSOLUTE);
		}
	
		ResumeAnimate();
	}
	return control;

}

RefTargetHandle LayerOutputControl::Clone(RemapDir& remap)
{
	LayerOutputControl *p = new LayerOutputControl();

	p->lC=lC;
	p->paramID =paramID;
	return p;
}


class LayerOutputControlClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 0; }
	void *			Create(BOOL loading) { return new LayerOutputControl(); }
	const TCHAR *	ClassName() { return GetString(IDS_LAYEROUTPUT_CLASS_NAME); }
	SClass_ID		SuperClassID() { return CTRL_FLOAT_CLASS_ID; }
	Class_ID		ClassID() { return LAYEROUTPUT_CONTROL_CLASS_ID; }
	const TCHAR* 	Category() { return _T("");  }
	};

static LayerOutputControlClassDesc lOLayerCD;
ClassDesc* GetLayerOutputDesc() {return &lOLayerCD;}

/**********************************************************

	class LayerControl

************************************************************/



class LayerIndexValidator : public FPValidator
{
public:
// validate val for the given param in function in interface
	virtual bool Validate(FPInterface* fpi, FunctionID fid, int paramNum, FPValue& val, TSTR& msg);
};

bool LayerIndexValidator::Validate(FPInterface* fpi, FunctionID fid, int paramNum, FPValue& val, TSTR& msg)
{
	int lowermargin=0;
	int highmargin = ((ILayerControl*)fpi)->GetLayerCount();
	switch (fid)
	{
	case ILayerControl::copyLayer:
	case ILayerControl::deleteLayer:
	case ILayerControl::collapseLayer:
		lowermargin = 1;
		break;
	case ILayerControl::pasteLayer:
		lowermargin = 1;
		highmargin+=1;
		break;
	}
	if(val.i < lowermargin)
	{
		msg.printf(GetString(IDS_MZ_LOWER_INDEX_ERROR), lowermargin+1);
		return false;
	}
	if( val.i >= ((ILayerControl*)fpi)->GetLayerCount())
	{
		msg.printf(GetString(IDS_MZ_UPPER_INDEX_ERROR),highmargin);
		return false;
	}

	return true;
}


static LayerIndexValidator layerValidator;

#define PBLOCK_INIT_REF_ID 0



class LayerControlPLCB : public PostLoadCallback 
{
public:
	LayerControl *lC;
	int howMany;
	LayerControlPLCB(LayerControl* e, int hMany)
	{ 
		lC = e;
		howMany = hMany;
	}
	void proc(ILoad *iload)
	{
		if(lC->pblock==NULL)
			return;
		//set up all of the pblock params..
		int count =0;
		for(int i = kX;i< (kX+howMany);++i)
		{
			LayerOutputControl * control = (LayerOutputControl*) lC->pblock->GetController(i-kX +1,0);
			if(control)
			{
				control->lC = lC;
				control->paramID = count++;

			}
			else //from an older version ..we need to create the paramter.
			{

				lC->CreateController(i);
			}
		}
		delete this;
	}
};


//we do this here since we need the floatLayerCD for the pblock2desc
class FloatLayerControl : public LayerControl
{
	public:
		FloatLayerControl(BOOL loading) : LayerControl(loading) {Init();} 
		FloatLayerControl() {Init();}

		void GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method=CTRL_ABSOLUTE);
		void GetOutputValue(TimeValue t, void *val, Interval &valid);
		void SetValue(TimeValue t, void *val, int commit, GetSetMethod method);
		LayerControl *DerivedClone();
		void Init();

		Class_ID ClassID() { return (FLOATLAYER_CONTROL_CLASS_ID); }  
		SClass_ID SuperClassID() { return CTRL_FLOAT_CLASS_ID; }  
		void GetClassName(TSTR& s) {s = FLOATLAYER_CNAME;}
};

class RotationLayerControl : public LayerControl
{
	public:
		RotationLayerControl(BOOL loading) : LayerControl(loading) {Init();} 
		RotationLayerControl() {Init();}

		void GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method=CTRL_ABSOLUTE);
		void GetOutputValue(TimeValue t, void *val, Interval &valid);
		BOOL GetValueAsEuler(TimeValue t, void *val, Interval &valid, GetSetMethod method,BOOL output);
		BOOL GetValueAsEuler(TimeValue t, Interval &valid, BOOL output,Point3 &currentVal);
		
		void SetValue(TimeValue t, void *val, int commit, GetSetMethod method);
		LayerControl *DerivedClone();
		void Init();

		int GetEulerXAxisOrder();
		void SetEulerXAxisOrder(int id);

		int GetEulerYAxisOrder();
		void SetEulerYAxisOrder(int id);

		int GetEulerZAxisOrder();
		void SetEulerZAxisOrder(int id);

		BOOL GetBlendEulerAsQuat();
		void SetBlendEulerAsQuat(BOOL val);

		Class_ID ClassID() { return (ROTLAYER_CONTROL_CLASS_ID); }  
		SClass_ID SuperClassID() { return CTRL_ROTATION_CLASS_ID; }  
		void GetClassName(TSTR& s) {s = ROTLAYER_CNAME;}

		//overriden by RotationLayerControl since every time we change conts we need to make sure to see if we can blend the conts
		//3 floats as a euler or as a quat.

		BOOL AssignController(Animatable *control, int subAnim);
		void AddLayer(Control *control,int newMLC);
		void DeleteLayerInternal(int active);
		void CopyLayerInternal(int active);
		void PasteLayerInternal(int active);
		void CollapseLayerInternal(int index);

		void CheckOkayToBlendAsEulers();
		//not used right now.. if we can blend it, we can collapse it as euler (though maybe not as per key frame)BOOL CheckOkayToCollapseAsEuler();
		BOOL okayToBlendEulers;  //this is set everytime a controller is added or deleted or assigned. It is then used by GetValue.
};

class FloatLayerClassDesc : public ClassDesc2 
{
	public:
	int 			IsPublic() {return 0;}
	const TCHAR *	ClassName() {return FLOATLAYER_CNAME;}
    SClass_ID		SuperClassID() {return CTRL_FLOAT_CLASS_ID;}
	Class_ID		ClassID() {return (FLOATLAYER_CONTROL_CLASS_ID);}
	const TCHAR* 	Category() {return _T("");}
	void *	Create(BOOL loading) { 
		return new FloatLayerControl(loading);	}

	const TCHAR*	InternalName() { return _T("FloatList"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }					// returns owning module handle
};

static FloatLayerClassDesc floatLayerCD;


// per instance list controller block
static ParamBlockDesc2 layer_paramblk (0, _T("Parameters"),  0, &floatLayerCD, P_AUTO_CONSTRUCT, PBLOCK_INIT_REF_ID, 
	// params
		
	kLayerCtrlAverage, _T("average"), TYPE_BOOL, 0, IDS_AF_LIST_AVERAGE, 
		p_default, 		FALSE, 
		end, 

	kX, 	_T("exposedValue"), TYPE_FLOAT, 	P_ANIMATABLE|P_TRANSIENT, 	IDS_VALUE, 
		end,

	end
	);



static FPInterfaceDesc layerControlInterface (
#ifndef NO_ANIM_LAYERS
	LAYER_CONTROLLER_INTERFACE, _T("layer"), 0, &floatLayerCD, FP_MIXIN,
#else
	LAYER_CONTROLLER_INTERFACE, _T("layer"), 0, &floatLayerCD, FP_MIXIN + FP_TEST_INTERFACE,
#endif
		ILayerControl::getNumLayers,		_T("getCount"),		0, TYPE_INT,	0,	0,
		ILayerControl::setLayerActive,		_T("setLayerActive"),	0, TYPE_VOID,	0,	1,
			_T("listIndex"), 0, TYPE_INDEX, f_validator, &layerValidator,
		ILayerControl::getLayerActive,		_T("getLayerActive"),	0, TYPE_INDEX,	0,	0,
		ILayerControl::copyLayer,			_T("copyLayer"),			0, TYPE_VOID,	0,	1,
			_T("listIndex"), 0, TYPE_INDEX,f_validator, &layerValidator,
		ILayerControl::pasteLayer,		_T("pasteLayer"),		0, TYPE_VOID,	0,	1,
			_T("listIndex"), 0, TYPE_INDEX,f_validator, &layerValidator,
		ILayerControl::deleteLayer,		_T("deleteLayer"),		0, TYPE_VOID,	0,	1,
			_T("listIndex"), 0, TYPE_INDEX,f_validator, &layerValidator,
		ILayerControl::getLayerName,			_T("getLayerName"),		0, TYPE_TSTR_BV, 0,  1,
			_T("listIndex"), 0, TYPE_INDEX,f_validator, &layerValidator,
		ILayerControl::setLayerName,			_T("setLayerName"),		0, TYPE_VOID,   0,  2,
			_T("listIndex"), 0, TYPE_INDEX,f_validator, &layerValidator,
			_T("name"),		 0, TYPE_STRING,


	    ILayerControl::getSubCtrl,			_T("getSubCtrl"),		0, TYPE_CONTROL,	0,	1,
			_T("index"), 0, TYPE_INDEX,f_validator, &layerValidator,
		ILayerControl::getLayerWeight,	_T("getLayerWeight"),	0, TYPE_FLOAT,	0,	2,
			_T("index"), 0, TYPE_INDEX,f_validator, &layerValidator,
			_T("atTime"), 0, TYPE_TIMEVALUE,

		ILayerControl::setLayerWeight,			_T("setLayerWeight"),		0, TYPE_VOID, 0,  3,
			_T("listIndex"), 0, TYPE_INDEX,f_validator, &layerValidator,
			_T("atTime"), 0, TYPE_TIMEVALUE,
			_T("weight"),0,TYPE_FLOAT,

		ILayerControl::getLayerWeight,	_T("getLayerMute"),	0, TYPE_bool,	0,	1,
			_T("index"), 0, TYPE_INDEX,f_validator, &layerValidator,

		ILayerControl::setLayerWeight,			_T("setLayerWeight"),		0, TYPE_VOID, 0,  2,
			_T("listIndex"), 0, TYPE_INDEX,f_validator, &layerValidator,
			_T("mute"), 0, TYPE_bool,

		ILayerControl::collapseLayer,		_T("collapseLayer"),		0, TYPE_VOID,	0,	1,
			_T("listIndex"), 0, TYPE_INDEX,f_validator, &layerValidator,

		ILayerControl::disableLayer,  _T("disableLayer"), 0, TYPE_VOID, 0,0,

		properties,
		ILayerControl::count, FP_NO_FUNCTION, _T("count"), 0, TYPE_INT,
		ILayerControl::getLayerActive_prop, ILayerControl::setLayerActive_prop, _T("active"), 0, TYPE_INDEX,

		end
	);


FPInterfaceDesc* ILayerControl::GetDesc()
{
	return &layerControlInterface;
	//return NULL;
}





RefResult LayerControl::NotifyRefChanged( Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message )
{
	switch (message) {
		case REFMSG_WANT_SHOWPARAMLEVEL:// show the paramblock2 entry in trackview
			{
			BOOL *pb = (BOOL *)(partID);
			*pb = TRUE;
			return REF_STOP;
			}
		
		case REFMSG_CHANGE:

			layer_paramblk.InvalidateUI();
			if(pblock)
			{
				static bool inNotify = false;
				if(inNotify==false)
				{
					int stop = kZ;//due x,y and z output tracks in most cases, cept for point4's and floats.
				    if(ClassID()== POINT4LAYER_CONTROL_CLASS_ID)
					{
						stop = kZ +1;
					}
					else if(ClassID()==FLOATLAYER_CONTROL_CLASS_ID)
					{
						stop = kX;
					}

					inNotify = true;
					for(int i = kX;i<=stop;++i)
					{
			
						LayerOutputControl * control = (LayerOutputControl*) pblock->GetController(i-kX +1,0);
						if(control)
							control->NotifyDependents(FOREVER,PART_TM,REFMSG_CHANGE);
					}
					inNotify = false;
				}
			}
			break;
	}
	return REF_SUCCEED;
}





LayerControl::LayerControl(BOOL loading)
{
	copyClip = NULL;
	dlg = NULL;
	paramFlags = 0;
	active =0;
	pblock =NULL;
}

LayerControl::LayerControl(const LayerControl& ctrl)
{
	DeleteAllRefsFromMe();
	Resize(ctrl.conts.Count());	
	ReplaceReference(0,ctrl.pblock);
	ReplaceReference(1,ctrl.copyClip);
	int i=0;
	for (i=0; i<ctrl.conts.Count(); i++) {		
		ReplaceReference(i+2,ctrl.conts[i]);
		}

	active = ctrl.active;
	dlg = NULL;
	paramFlags = 0;
	copyClip = NULL;
	active = ctrl.active;
	int val;
	mLCs.ZeroCount();
	for(i=0;i<ctrl.mLCs.Count();++i)
	{
		val = ctrl.mLCs[i];
		mLCs.Append(1,&val);
	}

}

LayerControl::~LayerControl()
{
	DeleteAllRefs();
}

void LayerControl::CreateController(int i)
{
	Control *c = new LayerOutputControl(this,i-kX);
	pblock->SetController(i-kX +1, 0, c, FALSE);
}


void LayerControl::Resize(int c)
{
	int pc = conts.Count();	
	conts.SetCount(c);
	int i;
	for (i=pc; i<c; i++)
	{
		conts[i] = NULL;
	}
}

LayerControl& LayerControl::operator=(const LayerControl& ctrl)
{	
	Resize(ctrl.conts.Count());	
	ReplaceReference(0,ctrl.pblock);
	ReplaceReference(1,ctrl.copyClip);
	int i=0;
	for (i=0; i<ctrl.conts.Count(); i++) {		
		ReplaceReference(i+2,ctrl.conts[i]);
		}

	active = ctrl.active;
	dlg = NULL;
	paramFlags = 0;
	copyClip = NULL;
	active = ctrl.active;
	int val;
	mLCs.ZeroCount();
	for(i=0;i<ctrl.mLCs.Count();++i)
	{
		val = ctrl.mLCs[i];
		mLCs.Append(1,&val);
	}
	return *this;
}



Interval LayerControl::GetTimeRange(DWORD flags)
{
	Interval range = NEVER;
	for (int i=0; i<conts.Count(); i++) {
		if (!i) range = conts[i]->GetTimeRange(flags);
		else {
			Interval iv = conts[i]->GetTimeRange(flags);
			if (!iv.Empty()) {
				if (!range.Empty()) {
					range += iv.Start();
					range += iv.End();
				} else {
					range = iv;
					}
				}
			}
		}
	return range;
}


// node create Notify callback
static void NotifyPostClone(void *param, NotifyInfo *info)
{
	LayerControl *lC = (LayerControl*)param; // LAM - 2/13/03 - don't do anything with construction objects

	TrackViewPick res;
	INode *node = lC->GetNodeAndClientSubAnim(res);
	for(int i=0;i<lC->mLCs.Count();++i)
	{
		//finally add the cloned to the laye rg
		MasterLayerControl *mLC = GetMLCMan()->GetNthMLC(lC->mLCs[i]);
		mLC->AddNode(node);
	}
	UnRegisterNotification(NotifyPostClone,lC,NOTIFY_NODE_CREATED);
}

RefTargetHandle LayerControl::Clone(RemapDir& remap)
{
	LayerControl *ctrl = DerivedClone();

	ctrl->Resize(conts.Count());
	for (int i=0; i<conts.Count(); i++) {
		ctrl->ReplaceReference(i+2,remap.CloneRef(conts[i]));
		}
	BaseClone(this, ctrl, remap);
	for(int i=0;i<mLCs.Count();++i)
	{
		int where = mLCs[i];
		ctrl->mLCs.Append(1,&where);
		//finally add the cloned to the laye rg
		MasterLayerControl *mLC = GetMLCMan()->GetNthMLC(mLCs[i]);
		mLC->AppendLayerControl(ctrl);
	}
	ctrl->active = active;
	
	RegisterNotification(NotifyPostClone, (void*)ctrl, NOTIFY_NODE_CREATED);
	return ctrl;
}

void LayerControl::AddLayer(Control *newControl, int newMLC)
{

	if(dlg) dlg->EndEdit(active);

	LayerControlRestore *rest = new LayerControlRestore(this);
	LayerControlRestore *rest2 = new LayerControlRestore(this);

	if(theHold.Holding())
	{
		theHold.Put(new RemoveLayerItemRestore(this,0));
		theHold.Put(rest);
	}
	else
	{
		delete rest;
		rest = NULL;
	}

	active = conts.Count();
	Control* null_ptr = NULL;
	conts.Append(1, &null_ptr);
	mLCs.Append(1,&newMLC);
	ReplaceReference(2+conts.Count()-1,newControl);	
	if(theHold.Holding())
	{
		theHold.Put(rest2);
		theHold.Put(new RemoveLayerItemRestore(this,1));
	}
	else
	{
		delete rest2;
		rest2 = NULL;
	}
	NotifyDependents(FOREVER,0,REFMSG_CHANGE);
	NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);
	if(dlg)dlg->StartEdit(active);

	//theHold.Put(new RemoveLayerItemRestore(this,1));
//	DeleteReference(conts.Count()+1);
//	copyClip = NULL;	
//	if (active<0) active = 0;
}

void LayerControl::Copy(Control *from)
{
/*	Do nothing...
if (from->ClassID() != Class_ID(DUMMY_CONTROL_CLASS_ID,0))
	{
		TSTR name;
		mLC->AddLayer(name,this,from);
	}
*/
}

void LayerControl::CommitValue(TimeValue t)
{	
	if (!conts.Count()) return;
	assert(active>=0);
	conts[active]->CommitValue(t);
}

void LayerControl::RestoreValue(TimeValue t)
{
	if (!conts.Count()) return;
	assert(active>=0);
	conts[active]->RestoreValue(t);
}

int LayerControl::IsKeyable()
{
	if (!conts.Count()) return 0;
	assert(active>=0);
	// CAL-10/18/2002: conts[active] could be null while undo. (459225)
	return conts[active] && conts[active]->IsKeyable();
}


void LayerControl::SetValue(TimeValue t, void *val, int commit, GetSetMethod method)
{
	if (!conts.Count()) return;
	// Note: if method is CTRL_ABSOLUTE this will not do the right thing.
	// need to pass in: Inverse(beforeControllers) * val * Inverse(afterControllers)
	//
	// RB 11/28/2000: Actually to update the above comment... the transformation should be:
	// Inverse(afterControllers) * val * Inverse(beforeControllers)
	assert(active>=0);
	conts[active]->SetValue(t,val,commit,method);
}

void LayerControl::EnumIKParams(IKEnumCallback &callback)
{
	if (!conts.Count()) return;
	assert(active>=0);
	conts[active]->EnumIKParams(callback);
}

BOOL LayerControl::CompDeriv(TimeValue t,Matrix3& ptm,IKDeriv& derivs,DWORD flags)
{
	if (!conts.Count()) return FALSE;
	// Note: ptm is not correct if there are controllers before
	assert(active>=0);
	return conts[active]->CompDeriv(t,ptm,derivs,flags);
}

void LayerControl::MouseCycleCompleted(TimeValue t)
{
	if (!conts.Count()) return;
	assert(active>=0);
	conts[active]->MouseCycleCompleted(t);
}

// ambarish added this method on 11/28/2000
// the purpose is to ensure that the controller that doesn't want to inherit parent transform doesn't do so
BOOL LayerControl::InheritsParentTransform()
{
	if (!conts.Count()) return FALSE;
//	int activeControlNumber =  GetLayerActive();
	for (int i=0; i <= GetLayerActive(); ++i){ // loop through each item in the list up to (and including) the active item
		if (conts[i]->InheritsParentTransform() == FALSE){
			return FALSE;
		}
	}
	return TRUE;
}

void LayerControl::AddNewKey(TimeValue t,DWORD flags)
{
	for (int i=0; i<conts.Count(); i++) {
		conts[i]->AddNewKey(t,flags);
		}
}

void LayerControl::CloneSelectedKeys(BOOL offset)
{
	if (!conts.Count()) return;
	assert(active>=0);
	conts[active]->CloneSelectedKeys(offset);
}

void LayerControl::DeleteKeys(DWORD flags)
{
	if (!conts.Count()) return;
	assert(active>=0);
	conts[active]->DeleteKeys(flags);
}

void LayerControl::SelectKeys(TrackHitTab& sel, DWORD flags)
{
	if (!conts.Count()) return;
	assert(active>=0);
	conts[active]->SelectKeys(sel,flags);
}

BOOL LayerControl::IsKeySelected(int index)
{
	if (!conts.Count()) return FALSE;
	assert(active>=0);
	return conts[active]->IsKeySelected(index);
}

void LayerControl::CopyKeysFromTime(TimeValue src,TimeValue dst,DWORD flags)
{
	if (!conts.Count()) return;
	assert(active>=0);
	conts[active]->CopyKeysFromTime(src,dst,flags);
}

void LayerControl::DeleteKeyAtTime(TimeValue t)
{
	if (!conts.Count()) return;
	assert(active>=0);
	conts[active]->DeleteKeyAtTime(t);
}

BOOL LayerControl::IsKeyAtTime(TimeValue t,DWORD flags)
{
	if (!conts.Count()) return FALSE;
	assert(active>=0);
	return conts[active]? conts[active]->IsKeyAtTime(t,flags) : FALSE;
}

BOOL LayerControl::GetNextKeyTime(TimeValue t,DWORD flags,TimeValue &nt)
{
	if (!conts.Count()) return FALSE;
	assert(active>=0);
	return conts[active]->GetNextKeyTime(t,flags,nt);
}

int LayerControl::GetKeyTimes(Tab<TimeValue> &times,Interval range,DWORD flags)
{
	if (!conts.Count()) return 0;
	assert(active>=0);
	return conts[active]->GetKeyTimes(times,range,flags);
}

int LayerControl::GetKeySelState(BitArray &sel,Interval range,DWORD flags)
{
	if (!conts.Count()) return 0;
	assert(active>=0);
	return conts[active]->GetKeySelState(sel,range,flags);
}



void LayerControl::NotifyForeground(TimeValue t)
{ 
	if (!conts.Count()) return;
	assert(active>=0);
	conts[active]->NotifyForeground(t);
}



int LayerControl::NumSubs()
{
	MasterLayerControlManager *man = GetMLCMan();
	if(man->GetFilterActiveOnlyTrackView()==FALSE)
		return conts.Count()*2+1;
	else
		return 3;

}

Animatable* LayerControl::SubAnim(int i)
{

	MasterLayerControlManager *man = GetMLCMan();
	if(man->GetFilterActiveOnlyTrackView()==FALSE)
	{

		if (i<conts.Count())
			return conts[i];
		else if(i<2*conts.Count())
		{
			MasterLayerControl *mLC = GetMLCMan()->GetNthMLC(mLCs[i-conts.Count()]);
			if(mLC&&mLC->pblock)
				return mLC->GetWeightControl();
		}
		else if(i==2*conts.Count())
			return pblock;
	}
	else
	{
		if(i==0)
			return conts[active];
		else if(i==1)
		{
			MasterLayerControl *mLC = GetMLCMan()->GetNthMLC(mLCs[active]);
			if(mLC&&mLC->pblock)
				return mLC->GetWeightControl();
		}
		else if(i==2)
			return pblock;
	}
	return NULL;
}


int LayerControl::SubNumToRefNum(int subNum)
{
	MasterLayerControlManager *man = GetMLCMan();
	if(man->GetFilterActiveOnlyTrackView()==FALSE)
	{
		if (subNum<=conts.Count()) return subNum+2;
		else if(subNum==2*conts.Count())
			return 0;
	}
	else
	{
		if(subNum==2)
			return 0;
		if(subNum==0)
			return active +2;
	}
	return -1;
}


TSTR LayerControl::SubAnimName(int i)	
{
	TSTR name;
	

	if(i<0)
		return name;
	
	MasterLayerControlManager *man = GetMLCMan();
	if(man->GetFilterActiveOnlyTrackView()==FALSE)
	{
		if(i<conts.Count())
		{

			MasterLayerControl *mLC = GetMLCMan()->GetNthMLC(mLCs[i]);
			if (mLC&&mLC->GetName() && mLC->GetName().Length()>0) 
			{
				name = mLC->GetName();
			}
			else if (conts[i])
			{
				conts[i]->GetClassName(name);
			} 
		}
		else if(i<conts.Count()*2)
		{
			MasterLayerControl *mLC = GetMLCMan()->GetNthMLC(mLCs[i-conts.Count()]);
			if (mLC&&mLC->GetName() && mLC->GetName().Length()>0) 
			{
				name =  TSTR(GetString(IDS_AF_LIST_WEIGHT)) +TSTR(": ") +TSTR(mLC->GetName());
			}
			else if (conts[i])
			{
				TSTR cname; conts[i]->GetClassName(cname);
				name = TSTR(GetString(IDS_AF_LIST_WEIGHT)) +TSTR(": ")+ TSTR(cname);
			} 

		}
		else if(i==2*conts.Count())
			name = GetString(IDS_MZ_OUTPUT_TRACK);
	}
	else
	{
		if(i==0)
		{
			MasterLayerControl *mLC = GetMLCMan()->GetNthMLC(mLCs[active]);
			if(mLC&&mLC->GetName() &&mLC->GetName().Length()>0)
			{
				name = mLC->GetName();
			}
			else if(conts[active])
			{
				conts[active]->GetClassName(name);
			}
		}
		else if(i==1)
		{
			MasterLayerControl *mLC = GetMLCMan()->GetNthMLC(mLCs[active]);
			if(mLC&&mLC->GetName() &&mLC->GetName().Length()>0)
			{
				name = mLC->GetName();
				name += TSTR(TSTR(" ") + TSTR(GetString(IDS_AF_LIST_WEIGHT)));
			}
			else if(conts[active])
			{
				conts[active]->GetClassName(name);
				name += TSTR(TSTR(" ") + TSTR(GetString(IDS_AF_LIST_WEIGHT)));			}
		}
		else if(i==2)
			name = GetString(IDS_MZ_OUTPUT_TRACK);
	}
	return name;
}




TSTR LayerControl::ControllerName(int i)	
{
	TSTR name;
	

	if(i<0)
		return name;
	
	if(i<conts.Count())
	{

		MasterLayerControl *mLC = GetMLCMan()->GetNthMLC(mLCs[i]);
		if (mLC&&mLC->GetName() && mLC->GetName().Length()>0) 
		{
			name = mLC->GetName();
		}
		else if (conts[i])
		{
			conts[i]->GetClassName(name);
		} 
	}
	return name;
}



RefTargetHandle LayerControl::GetReference(int i)
{
	if(i==0)
		return pblock;
	if(i==1)
		return copyClip;
	if((i-2)<conts.Count())
		return conts[i-2];
	return NULL;
}





void LayerControl::SetReference(int i, RefTargetHandle rtarg)
{
	if (i == 0|| 
	 (rtarg && rtarg->ClassID() == Class_ID(PARAMETER_BLOCK2_CLASS_ID,0)))
	{
		//set the parameter block
		pblock = (IParamBlock2*)rtarg;
		return;
	}
	else if(i==1)
	{
		copyClip = static_cast<Control*>(rtarg);
	}
	else if((i-2)<conts.Count())
	{
		conts[i-2] = static_cast<Control*>(rtarg);	  
	}
	return;
}

ParamDimension* LayerControl::GetParamDimension(int i)
{
	ParamDimension *dim = defaultDim;
	NotifyDependents(FOREVER, (PartID)&dim, REFMSG_GET_CONTROL_DIM);
	return dim;
}

BOOL LayerControl::AssignController(Animatable *control,int subAnim)
{

	if(GetMLCMan()->GetFilterActiveOnlyTrackView()==FALSE)
	{
		if(subAnim<conts.Count())
		{
			ReplaceReference(SubNumToRefNum(subAnim),(RefTargetHandle)control);
			NotifyDependents(FOREVER,0,REFMSG_CHANGE);
			NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);
			return TRUE;
		}
		else if (subAnim<conts.Count()*2)
		{
			MasterLayerControl *mLC = GetMLCMan()->GetNthMLC(mLCs[subAnim-conts.Count()]); 	//#867594 MZ
			Control *c = GetControlInterface(control);
			if(c!=NULL&&mLC!=NULL) //mLC may be NULL when closing max or a file.. it may have been deleted already.
				mLC->SetWeightControl(c);
			return TRUE;
		}
	}
	else
	{
		if(subAnim==0)
		{
			
			ReplaceReference(SubNumToRefNum(subAnim),(RefTargetHandle)control);	
			NotifyDependents(FOREVER,0,REFMSG_CHANGE);
			NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);
			return TRUE;
		}
		else if(subAnim==1)
		{
			MasterLayerControl *mLC = GetMLCMan()->GetNthMLC(mLCs[active]);  //#867594 MZ
			Control *c = GetControlInterface(control);
			if(c!=NULL&&mLC!=NULL)  //mLC may be NULL when closing max or a file.. it may have been deleted already
				mLC->SetWeightControl(c);
			return TRUE;
		}
	}
	return FALSE;
}




#define LAYERCOUNT_CHUNK	0x01010
#define ADDDEFAULT_CHUNK	0x01020
#define MUTE_CHUNK			0x01030
#define OUTPUTMUTE_CHUNK	0x01035
#define MLCITEM_CHUNK		0x01040
#define LAYERACTIVE_CHUNK	0x01050
#define MONITORCOUNT_CHUNK	0x01060



IOResult LayerControl::Save(ISave *isave)
{
	ULONG nb;
	int count = conts.Count();
	isave->BeginChunk(LAYERCOUNT_CHUNK);
	isave->Write(&count,sizeof(int),&nb);			
	isave->EndChunk();

	isave->BeginChunk(LAYERACTIVE_CHUNK);
	isave->Write(&active,sizeof(active),&nb);			
	isave->EndChunk();


	int val;
	for (int i=0; i<mLCs.Count(); i++)
	{
		isave->BeginChunk(MLCITEM_CHUNK);
		val = mLCs[i];
		isave->Write(&val,sizeof(int),&nb);	
		isave->EndChunk();
	}

	return IO_OK;

}


IOResult LayerControl::Load(ILoad *iload)
{


	if(ClassID()== POINT3LAYER_CONTROL_CLASS_ID ||
	ClassID()== POSLAYER_CONTROL_CLASS_ID ||
	ClassID()== ROTLAYER_CONTROL_CLASS_ID ||
	ClassID()== SCALELAYER_CONTROL_CLASS_ID)
	{
		LayerControlPLCB* plcb = new  LayerControlPLCB(this,3);
		iload->RegisterPostLoadCallback(plcb);
	}
	else if(ClassID()== POINT4LAYER_CONTROL_CLASS_ID)
	{
		LayerControlPLCB* plcb = new  LayerControlPLCB(this,4);
		iload->RegisterPostLoadCallback(plcb);
	}
	else if(ClassID()==FLOATLAYER_CONTROL_CLASS_ID)
	{
		LayerControlPLCB* plcb = new  LayerControlPLCB(this,1);
		iload->RegisterPostLoadCallback(plcb);
	}


	ULONG nb;
	IOResult res = IO_OK;
	int ix=0;

	while (IO_OK==(res=iload->OpenChunk())) {
		switch (iload->CurChunkID()) {
			case LAYERCOUNT_CHUNK: {
				int count;
				res = iload->Read(&count,sizeof(count),&nb);
				Resize(count);
				break;
				}
			case LAYERACTIVE_CHUNK:
				res = iload->Read(&active,sizeof(active),&nb);				
				break;
			case MLCITEM_CHUNK: {
				int val;
				res = iload->Read(&val,sizeof(val),&nb);
				mLCs.Append(1,&val);
				break;
				}
			}
		iload->CloseChunk();
		if (res!=IO_OK)  return res;
		}
	return IO_OK;
}


void LayerControl::SetLayerActiveInternal(int a)
{
	if (dlg)
	{
		if (conts.Count()) {
			conts[active]->EndEditParams(
				dlg->ip,END_EDIT_REMOVEUI,conts[a]);
			}
		if (conts.Count()) {
			conts[a]->BeginEditParams(
				dlg->ip,paramFlags,conts[active]);
			}
	}

	if(theHold.Holding()&&active!=a)
	{
		theHold.Put(new NotifySelectionRestore());
		theHold.Put(new LayerControlRestore(this));
		theHold.Put(new NotifySelectionRestore());

	}
	active = a;
	NotifyDependents(FOREVER,0,REFMSG_CHANGE);

	MasterLayerControlManager *man = GetMLCMan();
	if(man->GetFilterActiveOnlyTrackView()==TRUE)
	{
		NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);
	}

}



int	 LayerControl::GetLayerCount()
{
	return conts.Count();
} 

void LayerControl::SetLayerActive(int index)
{
	TrackViewPick res;
	INode *node = GetNodeAndClientSubAnim(res);
	if(node==NULL)
		return;
	Tab<INode*> nodes; nodes.Append(1,&node);
	if(index<0||index>=mLCs.Count())
		return;
	GetMLCMan()->SetLayerActiveNodes(mLCs[index],nodes);

}

int  LayerControl::GetLayerActive()
{
	return active; 
}

void LayerControl::DeleteLayerUI(int index)
{

	if(index<1||index>=mLCs.Count())
		return;

	theHold.SuperBegin();
	theHold.Begin();
	DeleteLayer(index);
	theHold.Accept(GetString(IDS_DELETE_ANIM_LAYER));
	theHold.SuperAccept(GetString(IDS_DELETE_ANIM_LAYER));
}

void LayerControl::DeleteLayer(int index)
{
	//{mLC->DeleteLayer(index);}

	if(index<1||index>=mLCs.Count())
		return;

	TrackViewPick res;
	INode *node = GetNodeAndClientSubAnim(res);
	if(node==NULL)
		return;
	Tab<INode*> nodes; nodes.Append(1,&node);

	if(GetMLCMan()&&GetMLCMan()->ContinueDelete())
		GetMLCMan()->DeleteNthLayerNodes(mLCs[index],nodes);

}


void LayerControl::CollapseLayer(int index)
{
	//{mLC->DeleteLayer(index);}

	if(index<1||index>=mLCs.Count())
		return;

	TrackViewPick res;
	INode *node = GetNodeAndClientSubAnim(res);
	if(node==NULL)
		return;
	Tab<INode*> nodes; nodes.Append(1,&node);

	GetMLCMan()->CollapseLayerNodes(mLCs[index],nodes);

}



void LayerControl::CopyLayer(int index)
{
	//{mLC->CopyLayer(index);}
	if(index<0||index>=mLCs.Count())
		return;

	TrackViewPick res;
	INode *node = GetNodeAndClientSubAnim(res);
	if(node==NULL)
		return;
	Tab<INode*> nodes; nodes.Append(1,&node);

	GetMLCMan()->CopyLayerNodes(mLCs[index],nodes);
}

void LayerControl::PasteLayerUI(int index)
{
	if(index==0)
		return;

	theHold.SuperBegin();
	theHold.Begin();
	PasteLayer(index);
	theHold.Accept(IDS_PASTE_NEW_LAYER);
	theHold.SuperAccept(IDS_PASTE_NEW_LAYER);

}
void LayerControl::PasteLayer(int index)
{
	if(index==0)
		return;

	TrackViewPick res;
	INode *node = GetNodeAndClientSubAnim(res);
	if(node==NULL)
		return;
	Tab<INode*> nodes; nodes.Append(1,&node);

	GetMLCMan()->PasteLayerNodes(index,nodes);
}
void LayerControl::SetLayerName(int index, TSTR name)
{
	if(index<0||index>=mLCs.Count())
		return;
	MasterLayerControl *mLC = GetMLCMan()->GetNthMLC(mLCs[index]);
	//todo check name is shte same
	if(mLC)
	{
		mLC->SetName(name);
		
	}
}

TSTR LayerControl::GetLayerName(int index)
{
	TSTR name;
	if(index<0||index>=mLCs.Count())
		return name;
	MasterLayerControl *mLC = GetMLCMan()->GetNthMLC(mLCs[index]);
	if(mLC)
		return mLC->GetName();

	return name;

}

float LayerControl::GetLayerWeight(int index,TimeValue t)
{
	if(index<0||index>=mLCs.Count())
		return 0.0f;
	Interval valid = FOREVER;
	MasterLayerControl *mLC = GetMLCMan()->GetNthMLC(mLCs[index]);
	if(mLC)
		return mLC->GetWeight(t,valid);
	return 0.0f;
}//{return mLC->GetLayerWeight(index,t);}

void LayerControl::SetLayerWeight(int index,TimeValue t,float weight)
{
	if(index<0||index>=mLCs.Count())
		return;
	MasterLayerControl *mLC = GetMLCMan()->GetNthMLC(mLCs[index]);
	if(mLC)
        mLC->SetWeight(t,weight);

}


void LayerControl::DeleteLayerInternal(int a)
{
	if(a==0)
		return; //can't delete layer 0
	if (dlg) dlg->EndEdit(active);

	LayerControlRestore *rest = new LayerControlRestore(this);
	if(theHold.Holding())
		theHold.Put(new RemoveLayerItemRestore(this,0));

	DeleteReference(a+2);
	conts.Delete(a,1);
	mLCs.Delete(a,1);

	if (active > a) active--;
	if (active > conts.Count()-1) active = conts.Count()-1;

	if(theHold.Holding())
	{
		theHold.Put(rest);
	}
	else 
	{
		delete rest;
		rest = NULL;
	}
	
	if(theHold.Holding())
		theHold.Put(new RemoveLayerItemRestore(this,1));
	NotifyDependents(FOREVER,0,REFMSG_CHANGE);
	NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);
	if (dlg) 
		dlg->StartEdit(active);

}

void LayerControl::CopyLayerInternal(int a)
{
	//all we do is set up the copy and other stuff.

	ReplaceReference(1,conts[a]->Clone());	


}

void LayerControl::PasteLayerInternal(int a)
{
	if (!copyClip) return;

	if(a>conts.Count())
		return;


	if(a==-1||a>conts.Count())
	{
		a = conts.Count();
	}

	if (dlg) dlg->EndEdit(active);

	if(theHold.Holding())
	{
		theHold.Put(new RemoveLayerItemRestore(this,0));
		theHold.Put(new LayerControlRestore(this));
	}

	Control* null_ptr = NULL;
	ReplaceReference(a+2,copyClip);


	//may need to go after the undo/holds
	DeleteReference(1);
	copyClip = NULL;	

	NotifyDependents(FOREVER,0,REFMSG_CHANGE);
	NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);

	if(theHold.Holding())
		theHold.Put(new RemoveLayerItemRestore(this,1));

	if (dlg) dlg->StartEdit(active);

}


Control* LayerControl::GetSubCtrl(int in_index) const
{
	if (in_index < 0 || in_index >= conts.Count())
		return NULL;

	return conts[in_index];
}




template <class T, class F> BOOL CollapseSameControllerType(F* values,Interval range,Tab<TimeValue > &ontoKeyTimes,
															Tab<TimeValue> &fromKeyTimes,Control *onto,Control *from)
{

	Interval interval = FOREVER;

	T* ontoKeys = NULL;
	T* fromKeys = NULL;
	int ontoKeysCount = -1, fromKeysCount = -1;
	T k;
	F val;
	if(ontoKeyTimes.Count()>0)
	{
		ontoKeys = new T[ontoKeyTimes.Count()];
		ontoKeysCount = ontoKeyTimes.Count();
		IKeyControl *keyControl = GetKeyControlInterface(onto);
		for(int i=0;i<ontoKeyTimes.Count();++i)
		{
			keyControl->GetKey(i,&k);
			ontoKeys[i] = k;
			int index = (ontoKeyTimes[i]-range.Start())/GetTicksPerFrame();
			val = values[index];
			ontoKeys[i].val = val;
		}
	}
	
	if(fromKeyTimes.Count()>0)
	{
		fromKeys = new T[fromKeyTimes.Count()];
		fromKeysCount = fromKeyTimes.Count();
		IKeyControl *keyControl = GetKeyControlInterface(from);
		for(int i=0;i<fromKeyTimes.Count();++i)
		{
			keyControl->GetKey(i,&k);
			fromKeys[i] = k;
			int index = (fromKeyTimes[i]-range.Start())/GetTicksPerFrame();
			val = values[index];
			fromKeys[i].val = val;
		}
	}
	//now we have the keys with the right values! all that's left is to put
	//them onto the dude.
	IKeyControl *keyControl = GetKeyControlInterface(onto);
	keyControl->SetNumKeys(0);

	for(int i=0;i<ontoKeysCount;++i)
	{
		keyControl->AppendKey( &(ontoKeys[i]));
	}
	
	for(int i=0;i<fromKeysCount;++i)
	{
		keyControl->AppendKey( &(fromKeys[i]));
	}

	keyControl->SortKeys();

	if(ontoKeys)
	{
		delete []ontoKeys;
		ontoKeys = NULL;
	}
	if(fromKeys)
	{
		delete [] fromKeys;
		fromKeys = NULL;
	}
	return TRUE;
}


static BOOL GetKeyTimesAndRange(Control *onto, Control *from,Tab<TimeValue> &ontoKeyTimes,Tab<TimeValue> &fromKeyTimes, Interval &range)
{
	Interval interval = FOREVER;
	::GetKeyTimes(onto,ontoKeyTimes,interval,false);
	::GetKeyTimes(from,fromKeyTimes,interval,false);

	if(ontoKeyTimes.Count()>0)
	{
		range.SetStart(ontoKeyTimes[0]);
		range.SetEnd(ontoKeyTimes[ontoKeyTimes.Count()-1]);
		if(fromKeyTimes.Count()>0)
		{
			if(fromKeyTimes[0]<range.Start())
				range.SetStart(fromKeyTimes[0]);
			if(fromKeyTimes[fromKeyTimes.Count()-1]>range.End())
				range.SetEnd(fromKeyTimes[fromKeyTimes.Count()-1]);
		}
		return TRUE;
	}
	else if(fromKeyTimes.Count()>0)
	{
		range.SetStart(fromKeyTimes[0]);
		range.SetEnd(fromKeyTimes[fromKeyTimes.Count()-1]);
		return TRUE;

	}
	return FALSE; // no keys, do nothing!
}


BOOL LayerControl::CollapseXYZController(int index, Control *onto, Control *from)
{
	if(onto->GetXController()->IsKeyable()==FALSE||onto->GetXController()->ClassID()!=from->GetXController()->ClassID()
		||(onto->GetXController()->ClassID()!=Class_ID(LININTERP_FLOAT_CLASS_ID,0)
		&&onto->GetXController()->ClassID()!=Class_ID(HYBRIDINTERP_FLOAT_CLASS_ID,0)
		&&onto->GetXController()->ClassID()!=Class_ID(TCBINTERP_FLOAT_CLASS_ID,0)
		&&onto->GetXController()->ClassID()!=BOOLCNTRL_CLASS_ID))
		return FALSE;
	if(onto->GetYController()->IsKeyable()==FALSE||onto->GetYController()->ClassID()!=from->GetYController()->ClassID()
		||
		(onto->GetYController()->ClassID()!=Class_ID(LININTERP_FLOAT_CLASS_ID,0)
		&&onto->GetYController()->ClassID()!=Class_ID(HYBRIDINTERP_FLOAT_CLASS_ID,0)
		&&onto->GetYController()->ClassID()!=Class_ID(TCBINTERP_FLOAT_CLASS_ID,0)
		&&onto->GetYController()->ClassID()!=BOOLCNTRL_CLASS_ID))
		return FALSE;
	if(onto->GetZController()->IsKeyable()==FALSE||onto->GetZController()->ClassID()!=from->GetZController()->ClassID()
		||
		(onto->GetZController()->ClassID()!=Class_ID(LININTERP_FLOAT_CLASS_ID,0)
		&&onto->GetZController()->ClassID()!=Class_ID(HYBRIDINTERP_FLOAT_CLASS_ID,0)
		&&onto->GetZController()->ClassID()!=Class_ID(TCBINTERP_FLOAT_CLASS_ID,0)
		&&onto->GetZController()->ClassID()!=BOOLCNTRL_CLASS_ID))
		return FALSE;

	Control *subOnto[3], *subFrom[3];
	Tab<TimeValue> ontoKeyTimes[3],fromKeyTimes[3];
	Interval range[3];
	
	subOnto[0] = onto->GetXController(); subFrom[0] = from->GetXController();
	if(GetKeyTimesAndRange(subOnto[0], subFrom[0],ontoKeyTimes[0],fromKeyTimes[0], range[0])==FALSE)
	{
		if(fromKeyTimes[0].Count()>0)
			return FALSE;
		else
			return TRUE;
	}
	subOnto[1] = onto->GetYController(); subFrom[1] = from->GetYController();
	if(GetKeyTimesAndRange(subOnto[1], subFrom[1],ontoKeyTimes[1],fromKeyTimes[1], range[1])==FALSE)
	{
		if(fromKeyTimes[2].Count()>0)
			return FALSE;
		else
			return TRUE;
	}
	subOnto[2] = onto->GetZController(); subFrom[2] = from->GetZController();
	if(GetKeyTimesAndRange(subOnto[2], subFrom[2],ontoKeyTimes[2],fromKeyTimes[2], range[2])==FALSE)
	{
		if(fromKeyTimes[2].Count()>0)
			return FALSE;
		else
			return TRUE;
	}

	//need to get the 3 x y and z value floats from either the Point3 or ScaleValue array!
	int numKeys;
	float *fValues[3]; fValues[0] = NULL; fValues[1] = NULL;
	Interval bigRange(range[0]);
	if(range[1].Start()<bigRange.Start())
		bigRange.SetStart(range[1].Start());
	if(range[2].Start()<bigRange.Start())
		bigRange.SetStart(range[2].Start());
	if(range[1].End()>bigRange.End())
		bigRange.SetEnd(range[1].End());
	if(range[2].End()>bigRange.End())
		bigRange.SetEnd(range[2].End());
	numKeys = ((bigRange.End()-bigRange.Start())/GetTicksPerFrame()) + 1;
	if(numKeys<=1)
		return FALSE;
	
	fValues[0] = new float[numKeys];
	fValues[1] = new float[numKeys];
	fValues[2] = new float[numKeys];

	if(onto->SuperClassID()!= CTRL_SCALE_CLASS_ID)
	{
		Point3 *values = static_cast<Point3*>(GetControllerValues(index,bigRange,TRUE));
		for(int j=0;j<numKeys;++j)
		{
			for(int i=0;i<3;++i)
			{
				fValues[i][j] = values[j][i];
			}
		}
		
		if(values)
		{
			delete [] values;
			values = NULL;
		}
	}
	else
	{
		ScaleValue *values = static_cast<ScaleValue*>(GetControllerValues(index,bigRange));
		for(int j=0;j<numKeys;++j)
		{
			for(int i=0;i<3;++i)
			{
				fValues[i][j] = values[j].s[i];
			}
		}
		
		if(values)
		{
			delete [] values;
			values = NULL;
		}
	}
	BOOL what = FALSE;
	for(int i=0;i<3;++i)
	{

		if(subOnto[i]->ClassID()==Class_ID(LININTERP_FLOAT_CLASS_ID,0))
		{
			what = CollapseSameControllerType<ILinFloatKey,float>(fValues[i],bigRange,ontoKeyTimes[i],fromKeyTimes[i],subOnto[i],subFrom[i]);
			if(what==FALSE)
				break;
		}
		else if(subOnto[i]->ClassID()==Class_ID(HYBRIDINTERP_FLOAT_CLASS_ID,0))
		{
			what = CollapseSameControllerType<IBezFloatKey,float>(fValues[i],bigRange,ontoKeyTimes[i],fromKeyTimes[i],subOnto[i],subFrom[i]);
			if(what==FALSE)
				 break;
		}
		else if(subOnto[i]->ClassID()==Class_ID(TCBINTERP_FLOAT_CLASS_ID,0))
		{

			what = CollapseSameControllerType<ITCBFloatKey,float>(fValues[i],bigRange,ontoKeyTimes[i],fromKeyTimes[i],subOnto[i],subFrom[i]);
			if(what==FALSE)
				break;
		}
		else if(subOnto[i]->ClassID()==BOOLCNTRL_CLASS_ID) //boolean
		{
			what = CollapseSameControllerType<IBoolFloatKey,float>(fValues[i],bigRange,ontoKeyTimes[i],fromKeyTimes[i],subOnto[i],subFrom[i]);
			if(what==FALSE)
				break;
		}
	}
	

	if(fValues[0])
	{
		delete [] fValues[0];
		fValues[0] = NULL;
	}
	if(fValues[1])
	{
		delete [] fValues[1];
		fValues[1] = NULL;
	}
	if(fValues[1])
	{
		delete [] fValues[2];
		fValues[2] = NULL;
	}
	return what;
}
//this function will make sure that the the from control is collapsed onto the 
//onto controller and keep track of keyframe specific data
BOOL LayerControl::CollapseKeys(int index, Control *onto, Control *from)
{

	//just make sure they are the same 
	if(onto->ClassID()!= from->ClassID())
		return FALSE;
	//special case for these 3 guys......
	if( (onto->SuperClassID()==CTRL_ROTATION_CLASS_ID&& onto->ClassID()==Class_ID(EULER_CONTROL_CLASS_ID,0) )||
		(onto->SuperClassID()==CTRL_POSITION_CLASS_ID&& onto->ClassID()==IPOS_CONTROL_CLASS_ID) || 
		(onto->SuperClassID()==CTRL_SCALE_CLASS_ID&& onto->ClassID()==ISCALE_CONTROL_CLASS_ID))
	{
		if(onto->SuperClassID()==CTRL_ROTATION_CLASS_ID)
		{
			RotationLayerControl *rLC = static_cast<RotationLayerControl *>(this);
			if(rLC->GetBlendEulerAsQuat()==TRUE)
				return FALSE; //blending euler as quat.. must do key per frame.
		}
		return CollapseXYZController(index,onto,from);
	}

	Tab<TimeValue> ontoKeyTimes,fromKeyTimes;
	Interval range;

	BOOL what = FALSE;
	if(onto->SuperClassID()==CTRL_FLOAT_CLASS_ID)
	{

		if(GetKeyTimesAndRange(onto, from,ontoKeyTimes,fromKeyTimes, range)==FALSE)
		{
			if(fromKeyTimes.Count()>0)
				return FALSE;
			else
				return TRUE;

		}
		float *values = static_cast<float*>(GetControllerValues(index,range));

		if(onto->ClassID()==Class_ID(LININTERP_FLOAT_CLASS_ID,0))
		{
			 what = CollapseSameControllerType<ILinFloatKey,float>(values,range,ontoKeyTimes,fromKeyTimes,onto,from);
		}
		else if(onto->ClassID()==Class_ID(HYBRIDINTERP_FLOAT_CLASS_ID,0))
		{
			 what = CollapseSameControllerType<IBezFloatKey,float>(values,range,ontoKeyTimes,fromKeyTimes,onto,from);
		
		}
		else if(onto->ClassID()==Class_ID(TCBINTERP_FLOAT_CLASS_ID,0))
		{
			 what = CollapseSameControllerType<ITCBFloatKey,float>(values,range,ontoKeyTimes,fromKeyTimes,onto,from);
		}
		else if(onto->ClassID()==BOOLCNTRL_CLASS_ID) //boolean
		{
			 what = CollapseSameControllerType<IBoolFloatKey,float>(values,range,ontoKeyTimes,fromKeyTimes,onto,from);
		}

		if(values)
		{
			delete [] values;
			values = NULL;
		}
		return what;
	}
	else if(onto->SuperClassID()==CTRL_ROTATION_CLASS_ID)
	{
		if(GetKeyTimesAndRange(onto, from,ontoKeyTimes,fromKeyTimes, range)==FALSE)
		{
			if(fromKeyTimes.Count()>0)
				return FALSE;
			else
				return TRUE;

		}	Quat *values = static_cast<Quat*>(GetControllerValues(index,range));
		if(onto->ClassID()==Class_ID(LININTERP_ROTATION_CLASS_ID,0))
		{
			what =  CollapseSameControllerType<ILinRotKey,Quat>(values,range,ontoKeyTimes,fromKeyTimes,onto,from);
		//	OutputLinearRotationControl(realNumKeys,keyControl,samplesNode,range);
		}
		else if(onto->ClassID()==Class_ID(HYBRIDINTERP_ROTATION_CLASS_ID,0))
		{
			what =  CollapseSameControllerType<IBezQuatKey,Quat>(values,range,ontoKeyTimes,fromKeyTimes,onto,from);
		//	OutputBezierRotationControl(realNumKeys,keyControl,samplesNode,range);
		}
		else if(onto->ClassID()==Class_ID(TCBINTERP_ROTATION_CLASS_ID,0))
		{
			what =  CollapseSameControllerType<ITCBRotKey,Quat>(values,range,ontoKeyTimes,fromKeyTimes,onto,from);
		//	OutputTCBRotationControl(realNumKeys,keyControl,samplesNode,range);
		}
		if(values)
		{
			delete [] values;
			values = NULL;
		}
		return what;

	}
	else if(onto->SuperClassID()==CTRL_POSITION_CLASS_ID||onto->SuperClassID()==CTRL_POINT3_CLASS_ID)
	{	
		if(GetKeyTimesAndRange(onto, from,ontoKeyTimes,fromKeyTimes, range)==FALSE)
		{
			if(fromKeyTimes.Count()>0)
				return FALSE;
			else
				return TRUE;

		}		Point3 *values = static_cast<Point3*>(GetControllerValues(index,range));

		if(onto->ClassID()==Class_ID(LININTERP_POSITION_CLASS_ID,0))
		{
			what =  CollapseSameControllerType<ILinPoint3Key,Point3>(values,range,ontoKeyTimes,fromKeyTimes,onto,from);
		//	OutputLinearPositionControl(realNumKeys,keyControl,samplesNode,range);
		}
		else if(onto->ClassID()==Class_ID(HYBRIDINTERP_POSITION_CLASS_ID,0)||onto->ClassID()==Class_ID(HYBRIDINTERP_COLOR_CLASS_ID,0)||
			onto->ClassID()==Class_ID(HYBRIDINTERP_POINT3_CLASS_ID,0))
		{
			what =  CollapseSameControllerType<IBezPoint3Key,Point3>(values,range,ontoKeyTimes,fromKeyTimes,onto,from);
		//	OutputBezierPositionControl(realNumKeys,keyControl,samplesNode,range);
		}
		else if(onto->ClassID()==Class_ID(TCBINTERP_POSITION_CLASS_ID,0) ||
			onto->ClassID()==Class_ID(TCBINTERP_POINT3_CLASS_ID,0))
		{
			what =  CollapseSameControllerType<ITCBPoint3Key,Point3>(values,range,ontoKeyTimes,fromKeyTimes,onto,from);
		//	OutputTCBPositionControl(realNumKeys,keyControl,samplesNode,range);
		}
		if(values)
		{
			delete [] values;
			values = NULL;
		}
		return what;

	}
	else if(onto->SuperClassID()==CTRL_SCALE_CLASS_ID)
	{
		if(GetKeyTimesAndRange(onto, from,ontoKeyTimes,fromKeyTimes, range)==FALSE)
		{
			if(fromKeyTimes.Count()>0)
				return FALSE;
			else
				return TRUE;

		}		ScaleValue *values = static_cast<ScaleValue*>(GetControllerValues(index,range));

		if(onto->ClassID()==Class_ID(LININTERP_SCALE_CLASS_ID,0))
		{
			what =  CollapseSameControllerType<ILinScaleKey,ScaleValue>(values,range,ontoKeyTimes,fromKeyTimes,onto,from);
		//	OutputLinearScaleControl(realNumKeys,keyControl,samplesNode,range);
		}
		else if(onto->ClassID()==Class_ID(HYBRIDINTERP_SCALE_CLASS_ID,0))
		{
			what =  CollapseSameControllerType<IBezScaleKey,ScaleValue>(values,range,ontoKeyTimes,fromKeyTimes,onto,from);
		//	OutputBezierScaleControl(realNumKeys,keyControl,samplesNode,range);
		}
		else if(onto->ClassID()==Class_ID(TCBINTERP_SCALE_CLASS_ID,0))
		{
			what =  CollapseSameControllerType<ITCBScaleKey,ScaleValue>(values,range,ontoKeyTimes,fromKeyTimes,onto,from);
		//	OutputTCBScaleControl(realNumKeys,keyControl,samplesNode,range);
		}
		if(values)
		{
			delete [] values;
			values = NULL;
		}
		return what;

	}
	else if(onto->SuperClassID()==CTRL_POINT4_CLASS_ID)
	{
		if(GetKeyTimesAndRange(onto, from,ontoKeyTimes,fromKeyTimes, range)==FALSE)
		{
			if(fromKeyTimes.Count()>0)
				return FALSE;
			else
				return TRUE;

		}		Point4 *values = static_cast<Point4*>(GetControllerValues(index,range));

		if(onto->ClassID()==Class_ID( HYBRIDINTERP_POINT4_CLASS_ID,0) ||  onto->ClassID()==Class_ID(HYBRIDINTERP_FRGBA_CLASS_ID,0))
		{
			what =  CollapseSameControllerType<IBezPoint4Key,Point4>(values,range,ontoKeyTimes,fromKeyTimes,onto,from);
		//	OutputBezierPoint4Control(realNumKeys,keyControl,samplesNode,range);
		}
		else if(onto->ClassID()==Class_ID( TCBINTERP_POINT4_CLASS_ID,0))
		{
			what =  CollapseSameControllerType< ITCBPoint4Key,Point4>(values,range,ontoKeyTimes,fromKeyTimes,onto,from);
		}
		if(values)
		{
			delete [] values;
			values = NULL;
		}
		return what;

	}


	return FALSE;
}


void * LayerControl::GetControllerValues(int index, Interval range, BOOL getRotAsEuler)
{
	int numKeys = ((range.End()-range.Start())/GetTicksPerFrame()) + 1;
	Interval valid = FOREVER;
	if(SuperClassID()==CTRL_FLOAT_CLASS_ID)
	{
		//first get the values.
		float *values = new float[numKeys];
		TimeValue t = range.Start();
		for(int i=0;i<numKeys;++i)
		{
			GetValue(t,&(values[i]),valid);
			t+= GetTicksPerFrame();
		}
		if(index-1>=0)
		{
			active = index-1;
			t = range.Start();
			float value;
			for(int i=0;i<numKeys;++i)
			{
				GetValue(t,&value,valid);
				values[i] = values[i]-value;
				t+= GetTicksPerFrame();
			}
		}
	
		return values;
	}
	else if(SuperClassID()==CTRL_ROTATION_CLASS_ID)
	{
		if(getRotAsEuler)
		{
			//double check to make sure.
			//if(ClassID()==Class_ID(EULER_CONTROL_CLASS_ID,0))
			{
				RotationLayerControl *rLC = static_cast<RotationLayerControl*>(this);
				Point3 *values = new Point3[numKeys];
				TimeValue t = range.Start();
				for(int i=0;i<numKeys;++i)
				{
					rLC->GetValueAsEuler(t,valid,FALSE,values[i]);
					t+= GetTicksPerFrame();
				}
				if(index-1>=0)
				{
					active = index-1;
					t = range.Start();
					Point3 value;
					for(int i=0;i<numKeys;++i)
					{
						rLC->GetValueAsEuler(t,valid,FALSE,value);
						values[i] = values[i]-value;
						t+= GetTicksPerFrame();
					}
				}
				return values;
			}

		}
		Quat *values = new Quat[numKeys];
		TimeValue t = range.Start();
		for(int i=0;i<numKeys;++i)
		{
			GetValue(t,&(values[i]),valid);
			t+= GetTicksPerFrame();
		}
		if(index-1>=0)
		{
			active = index-1;
			t = range.Start();
			Quat value;
			for(int i=0;i<numKeys;++i)
			{
				GetValue(t,&value,valid);
				values[i] = values[i]/value;
				t+= GetTicksPerFrame();
			}
		}
		return values;

	}
	else if(SuperClassID()==CTRL_POSITION_CLASS_ID||SuperClassID()==CTRL_POINT3_CLASS_ID)
	{
		//first get the values.
		Point3 *values = new Point3[numKeys];
		TimeValue t = range.Start();
		for(int i=0;i<numKeys;++i)
		{
			GetValue(t,&(values[i]),valid);
			t+= GetTicksPerFrame();
		}
		if(index-1>=0)
		{
			active = index-1;
			t = range.Start();
			Point3 value;
			for(int i=0;i<numKeys;++i)
			{
				GetValue(t,&value,valid);
				values[i] = values[i]-value;
				t+= GetTicksPerFrame();
			}
		}
		return values;
		
	}
	else if(SuperClassID()==CTRL_SCALE_CLASS_ID)
	{
		//first get the values.
		ScaleValue *values = new ScaleValue[numKeys];
		TimeValue t = range.Start();
		for(int i=0;i<numKeys;++i)
		{
			GetValue(t,&(values[i]),valid);
			t+= GetTicksPerFrame();
		}
		if(index-1>=0)
		{
			active = index-1;
			t = range.Start();
			ScaleValue value;
			for(int i=0;i<numKeys;++i)
			{
				GetValue(t,&value,valid);
				values[i] = values[i]-value;
				t+= GetTicksPerFrame();
			}
		}
		return values;
		
	}
	else if(SuperClassID()==CTRL_POINT4_CLASS_ID)
	{
		//first get the values.
		Point4 *values = new Point4[numKeys];
		TimeValue t = range.Start();
		for(int i=0;i<numKeys;++i)
		{
			GetValue(t,&(values[i]),valid);
			t+= GetTicksPerFrame();
		}
		if(index-1>=0)
		{
			active = index-1;
			t = range.Start();
			Point4 value;
			for(int i=0;i<numKeys;++i)
			{
				GetValue(t,&value,valid);
				values[i] = values[i]-value;
				t+= GetTicksPerFrame();
			}
		}
		return values;
	}
	return NULL;
}


BOOL LayerControl::CollapseFramePerKey(int index,Control *onto,Interval range,bool ontoKeys)
{
	int numKeys = ((range.End()-range.Start())/GetTicksPerFrame()) + 1;

	TimeValue t;
	Interval valid = FOREVER;

	Tab<TimeValue> keyTimes;
	if(ontoKeys)
		::GetKeyTimes(onto,keyTimes,valid,true);
	if(onto->SuperClassID()==CTRL_FLOAT_CLASS_ID)
	{
		//first get the values.
		float *values = static_cast<float*> (GetControllerValues(index,range));

		if(keyTimes.Count()<0)
			DeleteAllKeys(onto);
		SuspendAnimate();
		AnimateOn();
		t = range.Start();
		//? suspend notifications here for speed?????
		if(keyTimes.Count()<=0)
		{
			for(int i=0;i<numKeys;++i)
			{
				onto->SetValue(t,&(values[i]));
				t+=GetTicksPerFrame();
			}
		}
		else
		{
			for(int i=0;i<keyTimes.Count();++i)
			{
				t = keyTimes[i];
				int which = (i-range.Start())/GetTicksPerFrame();
				onto->SetValue(t,&(values[which]));
			}
		}
		ResumeAnimate();
		delete [] values;
		values = NULL;
	}
	else if(onto->SuperClassID()==CTRL_ROTATION_CLASS_ID)
	{
		
		Quat *values = static_cast<Quat*> (GetControllerValues(index,range));

		if(keyTimes.Count()<0)
			DeleteAllKeys(onto);
		SuspendAnimate();
		AnimateOn();
		t = range.Start();
		//? suspend notifications here for speed?????
			if(keyTimes.Count()<=0)
		{
			for(int i=0;i<numKeys;++i)
			{
				onto->SetValue(t,&(values[i]));
				t+=GetTicksPerFrame();
			}
		}
		else
		{
			for(int i=0;i<keyTimes.Count();++i)
			{
				t = keyTimes[i];
				int which = (i-range.Start())/GetTicksPerFrame();
				onto->SetValue(t,&(values[which]));
			}
		}
		ResumeAnimate();
		delete [] values;
		values = NULL;
	}
	else if(onto->SuperClassID()==CTRL_POSITION_CLASS_ID||onto->SuperClassID()==CTRL_POINT3_CLASS_ID)
	{
		//first get the values.
		Point3 *values = static_cast<Point3*> (GetControllerValues(index,range));
		
		if(keyTimes.Count()<0)
			DeleteAllKeys(onto);
		SuspendAnimate();
		AnimateOn();
		t = range.Start();
		//? suspend notifications here for speed?????
		if(keyTimes.Count()<=0)
		{
			for(int i=0;i<numKeys;++i)
			{
				onto->SetValue(t,&(values[i]));
				t+=GetTicksPerFrame();
			}
		}
		else
		{
			for(int i=0;i<keyTimes.Count();++i)
			{
				t = keyTimes[i];
				int which = (i-range.Start())/GetTicksPerFrame();
				onto->SetValue(t,&(values[which]));
			}
		}
		ResumeAnimate();
		delete [] values;
		values = NULL;
	}
	else if(onto->SuperClassID()==CTRL_SCALE_CLASS_ID)
	{
		//first get the values.
		ScaleValue *values = static_cast<ScaleValue*> (GetControllerValues(index,range));
	
		if(keyTimes.Count()<0)
			DeleteAllKeys(onto);
		SuspendAnimate();
		AnimateOn();
		t = range.Start();
		//? suspend notifications here for speed?????
		if(keyTimes.Count()<=0)
		{
			for(int i=0;i<numKeys;++i)
			{
				onto->SetValue(t,&(values[i]));
				t+=GetTicksPerFrame();
			}
		}
		else
		{
			for(int i=0;i<keyTimes.Count();++i)
			{
				t = keyTimes[i];
				int which = (i-range.Start())/GetTicksPerFrame();
				onto->SetValue(t,&(values[which]));
			}
		}
		ResumeAnimate();
		delete [] values;
		values = NULL;
	}
	else if(onto->SuperClassID()==CTRL_POINT4_CLASS_ID)
	{
		//first get the values.
		Point4 *values = static_cast<Point4*> (GetControllerValues(index,range));
		if(keyTimes.Count()<0)
			DeleteAllKeys(onto);
		SuspendAnimate();
		AnimateOn();
		t = range.Start();
		//? suspend notifications here for speed?????
		if(keyTimes.Count()<=0)
		{
			for(int i=0;i<numKeys;++i)
			{
				onto->SetValue(t,&(values[i]));
				t+=GetTicksPerFrame();
			}
		}
		else
		{
			for(int i=0;i<keyTimes.Count();++i)
			{
				t = keyTimes[i];
				int which = (i-range.Start())/GetTicksPerFrame();
				onto->SetValue(t,&(values[which]));
			}
		}
		ResumeAnimate();
		delete [] values;
		values = NULL;
	}
	return TRUE;
}


//only called if layer is not muted..
void LayerControl::CollapseLayerInternal(int index)
{
	if(index<=0||index>=conts.Count())
		return;


	MasterLayerControlManager *man = GetMLCMan();
	//set the values on the previous one.. that's not muted!
	int i;
	for(i= index-1;i>-1;--i)
	{
		MasterLayerControl *mLC = man->GetNthMLC(mLCs[i]);
		if(mLC->mute==false)
			break;
	}
	if(i==-1)
		return; //can't collapse nobody else is active!

	//okay now we now that we will collapse index to i.
	//before that, we need to mute

	Control * onto = conts[i];

	Control *from = conts[index];

	//don't collapse onto a mixer controller
	if(onto==NULL||from==NULL)
		return;

	if(onto->GetInterface(I_MIXSLAVEINTERFACE))
	{
		DeleteLayerInternal(index); //now delete it!
		return;

	}

	bool keyPerFrame = man->collapseAllPerFrame;
	bool ontoKeys = false; //always false for key per frame.

	if(keyPerFrame==false)
	{
		if(onto->IsKeyable()==FALSE)
		{
			onto = man->GetKeyableController(onto);
			if(onto==NULL)
				return;
			keyPerFrame = true;
		}

		if(from->IsKeyable()==FALSE) 
		{
			keyPerFrame = true;
			
		}
	}
	
	//manually mute all of the ones greater than me
	bool justUpToActive = man->GetJustUpToActive();
	man->justUpToActive = true;
	BOOL wasActive = active;
	active = index;

	BOOL wasReallyPerKey = FALSE;

	if(onto->ClassID()==from->ClassID()&&keyPerFrame==false)
	{
		wasReallyPerKey = CollapseKeys(i,onto,from);
	}

	if(wasReallyPerKey==FALSE)
	{
		Interval range;
		if(man->collapsePerFrameActiveRange==true)
			range = GetCOREInterface()->GetAnimRange(); //change to man option!
		else
			range = man->collapseRange;

		CollapseFramePerKey(i,onto,range,ontoKeys);
	}


	active = wasActive;
	man->justUpToActive = justUpToActive;


	if(onto!=GetReference(i+2))
		SetReference(i+2,onto);

	DeleteLayerInternal(index); //now delete it!

}

void LayerControl::DisableLayerUI()
{
	theHold.Begin();
	DisableLayer();
	theHold.Accept(GetString(IDS_DISABLE_ANIM_LAYER));
}

void LayerControl::DisableLayer()
{
	TrackViewPick res;
	INode *node = GetNodeAndClientSubAnim(res);


	if(node)
	{
		if(theHold.Holding())
			theHold.Put(new NotifySelectionRestore());
		DisableLayer(res.client,res.subNum);
		Tab<INode *> nodeTab;	nodeTab.Append(1,&node);
		Tab<MasterLayerControlManager::LCInfo> existingLayers;
		GetMLCMan()->GetLayerControls(nodeTab,existingLayers);


		for(int i= mLCs.Count()-1; i>-1;--i)
		{
			MasterLayerControl * mLC = GetMLCMan()->GetNthMLC(mLCs[i]);
			if(mLC)
			{
				for(int k=0;k<existingLayers.Count();++k)
				{
					LayerControl *lC = existingLayers[k].lC;
					int f;
					for(f=0;f<lC->mLCs.Count();++f)
					{
						if(lC->mLCs[f]==mLCs[i])
						{
							//okay this master layer control still exists
							//on this node, no need to kill it from the dude.
							break;
						}
					}
					if(f==lC->mLCs.Count())
					{
						MasterLayerControlRestore *mLCRestore = new MasterLayerControlRestore(mLC);
						int j;
						for(j=mLC->monitorNodes.Count()-1;j>-1;--j)
						{
							if(mLC->monitorNodes[j])
							{
								INodeMonitor *nm = static_cast<INodeMonitor *>(mLC->monitorNodes[j]->GetInterface(IID_NODEMONITOR));
								if(nm->GetNode()==node)
									break;
							}
						}
						if(j>-1)
						{
							mLC->DeleteReference(1 +mLC->layers.Count()+j);
							mLC->monitorNodes.Delete(j,1);
						}
						if(theHold.Holding())
							theHold.Put(mLCRestore);
					}
				}
			}
		}

		if(theHold.Holding())
			theHold.Put(new NotifySelectionRestore());
	}

}

void LayerControl::SetLayerMute(int index, bool mute)
{
	if(index<0||index>=mLCs.Count())
		return;
	MasterLayerControl *mLC = GetMLCMan()->GetNthMLC(mLCs[index]);
	if(mLC)
		mLC->mute = mute;
}

bool LayerControl::GetLayerMute(int index)
{	
	if(index<0||index>=mLCs.Count())
		return false;
	MasterLayerControl *mLC = GetMLCMan()->GetNthMLC(mLCs[index]);
	if(mLC)
		return mLC->mute;

	return false;
}

class AssignControllerRestore : public RestoreObj
{
	BOOL parity;
	public:	
		AssignControllerRestore(BOOL p); 
		void Restore(int isUndo);
		void Redo();
		TSTR Description();
};
// Cribbed from tvops.cpp. We need to simply stop editing in
// the motion branch when undoing a controller assignment.
AssignControllerRestore::AssignControllerRestore(BOOL p) {parity = p;}
void AssignControllerRestore::Restore(int isUndo) 
{	if (parity) GetCOREInterface7()->SuspendEditing(1<<TASK_MODE_MOTION | 1<<TASK_MODE_HIERARCHY);
	else GetCOREInterface7()->ResumeEditing(1<<TASK_MODE_MOTION | 1<<TASK_MODE_HIERARCHY);
}
void AssignControllerRestore::Redo() 
{	if (!parity) GetCOREInterface7()->SuspendEditing(1<<TASK_MODE_MOTION | 1<<TASK_MODE_HIERARCHY);
	else GetCOREInterface7()->ResumeEditing(1<<TASK_MODE_MOTION | 1<<TASK_MODE_HIERARCHY);
}
TSTR AssignControllerRestore::Description() { return TSTR(_T("Assign Controller")); }

void LayerControl::DisableLayer(Animatable *client,int subNum)
{
	if(conts.Count()==0)
		return; //somehow screwed up

//no we leave it out there !	if (dlg) dlg->EndEdit(active);
	MasterLayerControlManager *man = GetMLCMan();


	//remove this layer from the mLC's it's in.  It's a dead boy.
	for(int i= mLCs.Count()-1; i>-1;--i)
	{
		MasterLayerControl *mLC = man->GetNthMLC(mLCs[i]);
		for(int j=0;j<mLC->layers.Count();++j)
		{
			LayerControl *what = mLC->layers[j];
			if(what ==this)
			{
				MasterLayerControlRestore *rest = NULL;
				if(theHold.Holding())
					rest = new MasterLayerControlRestore(mLC);

				mLC->DeleteReference(j+1);
				mLC->layers.Delete(j,1);
				if(rest)
					theHold.Put(rest);
				break;
			}
		}

	}


	Control *newClone = static_cast<Control *>(conts[0]->Clone());
	if (theHold.Holding())
		theHold.Put(new AssignControllerRestore(0));
	GetCOREInterface7()->SuspendEditing(1<<TASK_MODE_MOTION | 1<<TASK_MODE_HIERARCHY);
	client->AssignController(newClone,subNum);
	GetCOREInterface7()->ResumeEditing(1<<TASK_MODE_MOTION | 1<<TASK_MODE_HIERARCHY);
	if (theHold.Holding())
		theHold.Put(new AssignControllerRestore(1));
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);
	static_cast<ReferenceTarget*>(client)->NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);
}


INode * LayerControl::GetNodeAndClientSubAnim(TrackViewPick &res)
{

	GetNodesProc dep;
	DoEnumDependents (&dep);
	Tab<MasterLayerControlManager::LCInfo> existingLayers;
	Tab<INode*> oneAtTime;
	MasterLayerControlManager * man = GetMLCMan();

	for(int i =0;i<dep.nodes.Count();++i)
	{
		INode *node = dep.nodes[i];
		if(node)
		{
			existingLayers.ZeroCount();
			oneAtTime.ZeroCount();
			oneAtTime.Append(1,&node);
			man->GetLayerControls(oneAtTime, existingLayers);
			for(int i=0;i<existingLayers.Count();++i)
			{
				if(existingLayers[i].lC==this)
				{
					res.client = static_cast<ReferenceTarget *>( existingLayers[i].client);
					res.subNum = (existingLayers[i].subNum);
					return node;
				}
			}

		}
	}
	return NULL;
}

class LayerCtrlWindow {
	public:
		HWND hWnd;
		HWND hParent;
		Control *cont;
		LayerCtrlWindow() {assert(0);}
		LayerCtrlWindow(HWND hWnd,HWND hParent,Control *cont)
			{this->hWnd=hWnd; this->hParent=hParent; this->cont=cont;}
	};

static Tab<LayerCtrlWindow> layerCtrlWindows;

static void RegisterLayerCtrl(HWND hWnd, HWND hParent, Control *cont)
{
	LayerCtrlWindow rec(hWnd,hParent,cont);
	layerCtrlWindows.Append(1,&rec);
}

static void UnRegisterLayerCtrlWindow(HWND hWnd)
{	
	for (int i=0; i<layerCtrlWindows.Count(); i++) {
		if (hWnd==layerCtrlWindows[i].hWnd) {
			layerCtrlWindows.Delete(i,1);
			return;
			}
		}	
}

static HWND FindOpenLayerCtrl(HWND hParent,Control *cont)
{	
	for (int i=0; i<layerCtrlWindows.Count(); i++) {
		if (hParent == layerCtrlWindows[i].hParent &&
			cont    == layerCtrlWindows[i].cont) {
			return layerCtrlWindows[i].hWnd;
		}
	}
	return NULL;
}

void LayerControl::EditTrackParams(
		TimeValue t,
		ParamDimensionBase *dim,
		TCHAR *pname,
		HWND hParent,		
		IObjParam *ip,
		DWORD flags)
{
#ifndef NO_ANIM_LAYERS
	HWND hCur = FindOpenLayerCtrl(hParent,this);
	if (hCur)
	{
		SetForegroundWindow(hCur);
		return;
	}

	LayerControlTrackDlg *dlg = 
		new LayerControlTrackDlg(ip,this);
	dlg->Init(hParent);
	RegisterLayerCtrl(dlg->hWnd,hParent,this);
#endif
}

void LayerControl::BeginEditParams(IObjParam *ip, ULONG flags,Animatable *prev)
{
#ifndef NO_ANIM_LAYERS
	paramFlags = flags;
	dlg = new LayerControlMotionDlg(ip,this);
	dlg->Init(NULL);
#else
	// In Viz, even though we don't want to display the Layer controller UI,
	// we do want to expose the active sub-controller's one.
	if (conts.Count())
	{
		if (conts[active] != NULL)
			conts[active]->BeginEditParams(ip, flags, NULL);
	}
#endif
}

void LayerControl::EndEditParams(IObjParam *ip, ULONG flags,Animatable *next)
{
#ifndef NO_ANIM_LAYERS
	if (dlg)
	{
			ip->DeleteRollupPage(dlg->hWnd);
			dlg        = NULL;
		}
	paramFlags = 0;
#else
	// In Viz, even though we do not display the Layer controller UI,
	// we do want to close the active sub-controller's rollup page.
	if (conts.Count())
	{
		if (conts[active] != NULL)
			conts[active]->EndEditParams(ip, flags, NULL);
	}
#endif
}

float LayerControl::AverageWeight(float weight,TimeValue t,Interval &valid)
{
	BOOL average = FALSE;
	pblock->GetValue(kLayerCtrlAverage, 0, average, FOREVER);
	if (average)
	{
		float tempWeight = 0.0f;
		float totalWeight = 0.0f;;
		for (int i=0;i<conts.Count();i++)
		{
			tempWeight = GetMLCMan()->GetNthMLC(mLCs[i])->GetWeight(t,valid);
			totalWeight += tempWeight;
		}
		return totalWeight!=0.0f?(average?(weight/totalWeight):weight):0.0f;
	}
	return weight;
}
#define NUM_SUPPORTED_EULERS		6
static int eulerIDs[] = {
    IDS_EULERTYPE0,IDS_EULERTYPE1,IDS_EULERTYPE2,
    IDS_EULERTYPE3,IDS_EULERTYPE4,IDS_EULERTYPE5,
    IDS_EULERTYPE6,IDS_EULERTYPE7,IDS_EULERTYPE8};



//***************************************************************


//-------------------------------------------------------------------------
// UI stuff

LayerControlDlg::LayerControlDlg(IObjParam *i,LayerControl *c)
{
	cont = NULL;
	// CAL-9/23/2002: don't want to put a MakeRefRestore into the undo stack. (436460)
	theHold.Suspend();
	ReplaceReference(0,c);
	theHold.Resume();
	
	ip = i;
	valid = FALSE;
	hWnd  = NULL;
	GetCOREInterface()->RegisterTimeChangeCallback(this);
	
}

LayerControlDlg::~LayerControlDlg()
{
	UnRegisterLayerCtrlWindow(hWnd);
	if (cont->dlg==this) {
		if (cont->conts.Count()) {
			if(cont->conts[cont->active])
				cont->conts[cont->active]->EndEditParams(
				ip,END_EDIT_REMOVEUI,NULL);
			}
		}
	// LAM-8/21/2004: don't want to put a MakeRefRestore into the undo stack. (589682)
	theHold.Suspend();
	DeleteAllRefsFromMe();
	theHold.Resume();
	GetCOREInterface()->UnRegisterTimeChangeCallback(this);
}
		
void LayerControlDlg::Reset(IObjParam *i,LayerControl *c)
{
	if (cont->dlg==this) {
		if (cont->conts.Count()) {
			if(cont->conts[cont->active])
				cont->conts[cont->active]->EndEditParams(
				i,END_EDIT_REMOVEUI,NULL);
			}
		if (c->conts.Count()) {
			if(cont->conts[cont->active])
				c->conts[cont->active]->BeginEditParams(
					i,c->paramFlags,NULL);
			}
		}
	ReplaceReference(0,c);
	ip = i;
	Invalidate();
}

void LayerControlDlg::Init(HWND hParent)
{
	hWnd = CreateWin(hParent);
	SetupList();
	SetButtonStates();
	if (cont->dlg==this) 
	{
		if (cont->conts.Count())
		{
			if(cont->conts[cont->active])
				cont->conts[cont->active]->BeginEditParams(
				ip,cont->paramFlags,NULL);
		}
	}
}

void LayerControlDlg::Invalidate()
{
	valid = FALSE;
	Rect rect(IPoint2(0,0),IPoint2(10,10));
	InvalidateRect(hWnd,&rect,FALSE);
}

void LayerControlDlg::Update()
{
	if (!valid && hWnd)
	{
		// CAL-10/7/2002: Check if the weight count and list count are consistent. When undoing
		// list controller assignment they will go out of sync and it's time to close the window.
		if(cont->mLCs.Count()!= cont->conts.Count())
		{
			PostMessage(hWnd, WM_CLOSE, 0, 0);
			return;
		}
		SetupUI();
		valid = TRUE;		
	}
}

void LayerControlDlg::UpdateList()
{
	
	HWND hList = GetDlgItem(hWnd, IDC_CONTROLLER_WEIGHT_LIST);

	int i=0;
	for (i=0; i<cont->conts.Count(); i++)
	{
		TSTR name = cont->ControllerName(i);
		if (i==cont->active)
		{
			name = TSTR(_T("->")) + name;
		} else
		{
			name = TSTR(_T("  ")) + name;
		}

		if (ListView_GetItemCount(hList) <= i)
		{
			LV_ITEM item;
			item.mask = LVIF_TEXT;
			item.iItem = i;
			item.iSubItem = 0;
			item.pszText = name;
			item.cchTextMax =static_cast<int> (_tcslen(name));
			ListView_InsertItem(hList,&item);

			item.iSubItem = 1;
			name.printf(_T("%.1f"),cont->GetLayerWeight(i, GetCOREInterface()->GetTime())*100.0f);
			item.pszText = name;
			item.cchTextMax = static_cast<int>(_tcslen(name));
			ListView_SetItemText(hList, i, 1, name);
		}
		else
		{
			ListView_SetItemText(hList, i, 0, name);
			name.printf(_T("%.1f"), cont->GetLayerWeight(i, GetCOREInterface()->GetTime())*100.0f);
			ListView_SetItemText(hList, i, 1, name);
		}
	}
	int itemCt = ListView_GetItemCount(hList);	for (int x = itemCt; x > i; x--)
		ListView_DeleteItem(hList, x-1);
  }

void LayerControlDlg::SetupList()
{
	
	HWND hList = GetDlgItem(hWnd, IDC_CONTROLLER_WEIGHT_LIST);
	int sel = ListView_GetNextItem(hList, -1, LVNI_SELECTED);

	LV_COLUMN column;
	column.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;

	column.fmt = LVCFMT_LEFT;
	column.pszText = GetString(IDS_AF_LIST_ENTRY);
	column.cx = 90;
	ListView_InsertColumn(hList, 2, &column);

	column.pszText = GetString(IDS_AF_LIST_WEIGHT);
	column.cx = 50;
	ListView_InsertColumn(hList, 1, &column);

	UpdateList();
 }

void LayerControlDlg::SetupUI()
{
	UpdateList();
	SetButtonStates();
}

void LayerControlDlg::StartEdit(int a)
{
	if (cont->dlg)
	{
		if (cont->conts.Count())
		{
			cont->conts[a]->BeginEditParams(
				ip,cont->paramFlags,cont->conts[a]);
		}
	}
}

void LayerControlDlg::EndEdit(int a)
{
	if (cont->dlg) {
		if (cont->conts.Count())
		{			
			cont->conts[a]->EndEditParams(
				ip,END_EDIT_REMOVEUI,NULL);				
		}
	}
}

void LayerControlDlg::SetButtonStates()
{
	HWND hList = GetDlgItem(hWnd, IDC_CONTROLLER_WEIGHT_LIST);
	int sel = ListView_GetNextItem(hList, -1, LVNI_SELECTED); 

	ICustEdit *iName = GetICustEdit(GetDlgItem(hWnd,IDC_LIST_NAME));
	iName->WantReturn(TRUE);
	ISpinnerControl *iWeight = GetISpinner(GetDlgItem(hWnd,IDC_LIST_WEIGHTSPIN));
	iWeight->SetLimits(-10000.0f,10000.0f,FALSE);
	iWeight->SetScale(1.0f);
	iWeight->LinkToEdit(GetDlgItem(hWnd,IDC_LIST_WEIGHT),EDITTYPE_FLOAT);
	
	if (sel >= cont->conts.Count())
		sel = cont->conts.Count()-1;

	MasterLayerControl *mLC = NULL;
	if(sel!=LB_ERR)
		mLC = GetMLCMan()->GetNthMLC(cont->mLCs[sel]);

	HWND disableButton = GetDlgItem(hWnd, IDC_LAYER_DISABLE);
	if (mLC)
	{
		if(cont->GetLayerCount()==1)
		{
			EnableWindow(GetDlgItem(hWnd,IDC_LIST_DELETE),FALSE);
			if(sel!=0)
			{
				EnableWindow(GetDlgItem(hWnd,IDC_LIST_CUT),TRUE);
			}
			else
			{
				EnableWindow(GetDlgItem(hWnd,IDC_LIST_CUT),FALSE);
			}
			if(disableButton)
				EnableWindow(disableButton,TRUE);
		}
		else
		{
			if(sel!=0)
			{
				EnableWindow(GetDlgItem(hWnd,IDC_LIST_DELETE),TRUE);
				EnableWindow(GetDlgItem(hWnd,IDC_LIST_CUT),TRUE);
			}
			else
			{
				EnableWindow(GetDlgItem(hWnd,IDC_LIST_DELETE),FALSE);
				EnableWindow(GetDlgItem(hWnd,IDC_LIST_CUT),FALSE);
			}
			if(disableButton)
				EnableWindow(disableButton,FALSE);
		}

		EnableWindow(GetDlgItem(hWnd,IDC_LIST_SETACTIVE),TRUE);
		
		iName->Enable();
		iName->SetText(cont->GetLayerName(sel));
		
		float weight = cont->GetLayerWeight(sel,GetCOREInterface()->GetTime());
		iWeight->Enable();
		iWeight->SetValue(weight*100.0f, 0);

		iWeight->SetKeyBrackets(mLC->pblock->KeyFrameAtTime(kLayerCtrlWeight, GetCOREInterface()->GetTime(), sel));

		if (cont->copyClip)
		{
			EnableWindow(GetDlgItem(hWnd,IDC_LIST_PASTE),TRUE);
		}
		else
		{
			
		}

		}
	else
	{
		EnableWindow(GetDlgItem(hWnd,IDC_LIST_SETACTIVE),FALSE);
		EnableWindow(GetDlgItem(hWnd,IDC_LIST_DELETE),FALSE);
		EnableWindow(GetDlgItem(hWnd,IDC_LIST_CUT),FALSE);	
		EnableWindow(GetDlgItem(hWnd,IDC_LIST_PASTE),FALSE);

		if(cont->GetLayerCount()==1)
		{
			if(disableButton)
				EnableWindow(disableButton,TRUE);
		}
		else
		{
			if(disableButton)
				EnableWindow(disableButton,FALSE);
		}

		iName->SetText(_T(""));
		iName->Disable();
		iWeight->SetValue(0.0f, 0);
		iWeight->Disable();
		iWeight->SetKeyBrackets(FALSE);
	}
	
	ReleaseISpinner(iWeight);
	ReleaseICustEdit(iName);

	BOOL average = FALSE;
	cont->pblock->GetValue(kLayerCtrlAverage, 0, average, FOREVER);
	CheckDlgButton(hWnd, IDC_AVERAGE_WEIGHTS, average);

	if (cont->SuperClassID() == CTRL_ROTATION_CLASS_ID)
	{
		RotationLayerControl *rLC = static_cast<RotationLayerControl*>(cont);


		int val = rLC->GetEulerXAxisOrder();
		SendDlgItemMessage(hWnd, IDC_EULER_ORDER_X, CB_SETCURSEL,val, 0);
		val = rLC->GetEulerYAxisOrder();
		SendDlgItemMessage(hWnd, IDC_EULER_ORDER_Y, CB_SETCURSEL,val, 0);
		val = rLC->GetEulerZAxisOrder();
		SendDlgItemMessage(hWnd, IDC_EULER_ORDER_Z, CB_SETCURSEL,val, 0);

		BOOL what = rLC->GetBlendEulerAsQuat();
		if(rLC->okayToBlendEulers==FALSE)
		{
			what = FALSE;
			EnableWindow(GetDlgItem(hWnd,IDC_BLEND_EULER_AS_QUAT),FALSE);
		}
		else
			EnableWindow(GetDlgItem(hWnd,IDC_BLEND_EULER_AS_QUAT),TRUE);

		CheckDlgButton(hWnd, IDC_BLEND_EULER_AS_QUAT,what );
		
		if(what==TRUE)
		{
			EnableWindow(GetDlgItem(hWnd,IDC_EULER_ORDER_X),TRUE);
			EnableWindow(GetDlgItem(hWnd,IDC_EULER_ORDER_Y),TRUE);
			EnableWindow(GetDlgItem(hWnd,IDC_EULER_ORDER_Z),TRUE);
		}
		else
		{
			EnableWindow(GetDlgItem(hWnd,IDC_EULER_ORDER_X),FALSE);
			EnableWindow(GetDlgItem(hWnd,IDC_EULER_ORDER_Y),FALSE);
			EnableWindow(GetDlgItem(hWnd,IDC_EULER_ORDER_Z),FALSE);
		}
	}
}

void LayerControlDlg::WMCommand(int id, int notify, HWND hCtrl)
{
	HWND hList = GetDlgItem(hWnd, IDC_CONTROLLER_WEIGHT_LIST);
	int sel = ListView_GetNextItem(hList, -1, LVNI_SELECTED);
	if (sel >= cont->conts.Count()) 
		sel = - 1;
	switch (id) {
		case IDC_LIST_NAME:
		{
			if (sel>0)
			{
				TCHAR buf[256];
				ICustEdit *iName = GetICustEdit(GetDlgItem(hWnd,IDC_LIST_NAME));
				if (iName&&(iName->GotReturn())) {
					iName->GetText(buf,256);
					cont->SetLayerName(sel, buf);
					}
				UpdateList();
				ReleaseICustEdit(iName);
			}
			break;
		}

		case IDC_LIST_SETACTIVE:
		{			
			if (sel!=LB_ERR)
			{
				cont->SetLayerActive(sel);
			}
			break;
		}

		case IDC_LAYER_DISABLE:

			cont->DisableLayerUI();
			GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
			PostMessage(hWnd,WM_CLOSE,0,0);
			break;

		case IDC_LIST_DELETE:
		{
			if (sel!=LB_ERR) 
			{
				cont->DeleteLayerUI(sel);	
				ip->RedrawViews(ip->GetTime());
			}
			break;
		}

		case IDC_LIST_CUT:
		{
			if (sel!=LB_ERR)
			{
				cont->CopyLayer(sel);
				ip->RedrawViews(ip->GetTime());
			}
			break;
		}
		case IDC_LIST_PASTE:
		{
			if (sel!=LB_ERR)
			{
				cont->PasteLayerUI(sel);
			} 
			else
			{
				cont->PasteLayer(cont->conts.Count());
			}
			ip->RedrawViews(ip->GetTime());
			break;
		}
		case IDC_AVERAGE_WEIGHTS:
		{
			BOOL average = IsDlgButtonChecked(hWnd, IDC_AVERAGE_WEIGHTS);
			cont->pblock->SetValue(kLayerCtrlAverage, 0, average);
			cont->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
			GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
			break;
		}
		case IDC_EULER_ORDER_X:
            if (notify==CBN_SELCHANGE)
			{
                int res = SendDlgItemMessage(hWnd, IDC_EULER_ORDER_X,
                                             CB_GETCURSEL, 0, 0);
                if (res!=CB_ERR)
				{
					RotationLayerControl *rLC  = static_cast<RotationLayerControl*>(cont);
					rLC->SetEulerXAxisOrder(res);
					GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
				}
            }
            break; 
		case IDC_EULER_ORDER_Y:
            if (notify==CBN_SELCHANGE)
			{
                int res = SendDlgItemMessage(hWnd, IDC_EULER_ORDER_Y,
                                             CB_GETCURSEL, 0, 0);
                if (res!=CB_ERR)
				{
					RotationLayerControl *rLC  = static_cast<RotationLayerControl*>(cont);
					rLC->SetEulerYAxisOrder(res);
					GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
				}
            }
            break; 
		case IDC_EULER_ORDER_Z:
            if (notify==CBN_SELCHANGE)
			{
                int res = SendDlgItemMessage(hWnd, IDC_EULER_ORDER_Z,
                                             CB_GETCURSEL, 0, 0);
                if (res!=CB_ERR)
				{
					RotationLayerControl *rLC  = static_cast<RotationLayerControl*>(cont);
					rLC->SetEulerZAxisOrder(res);
					GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
				}
            }
            break; 
		case IDC_BLEND_EULER_AS_QUAT:
		{
			BOOL what = IsDlgButtonChecked(hWnd, IDC_BLEND_EULER_AS_QUAT);
			RotationLayerControl *rLC  = static_cast<RotationLayerControl*>(cont);
			rLC->SetBlendEulerAsQuat(what);
			if(what==TRUE)
			{
				EnableWindow(GetDlgItem(hWnd,IDC_EULER_ORDER_X),TRUE);
				EnableWindow(GetDlgItem(hWnd,IDC_EULER_ORDER_Y),TRUE);
				EnableWindow(GetDlgItem(hWnd,IDC_EULER_ORDER_Z),TRUE);
			}
			else
			{
				EnableWindow(GetDlgItem(hWnd,IDC_EULER_ORDER_X),FALSE);
				EnableWindow(GetDlgItem(hWnd,IDC_EULER_ORDER_Y),FALSE);
				EnableWindow(GetDlgItem(hWnd,IDC_EULER_ORDER_Z),FALSE);
			}
			break;
		}
			
	}
}


int LayerControlDlg::NumRefs() 
{
	return 1;
}

RefTargetHandle LayerControlDlg::GetReference(int i) 
{
	switch (i)
	{
		case LAYERDLG_CONTREF:
			return cont;
	
		default:
			return NULL;
	}
}

void LayerControlDlg::SetReference(int i, RefTargetHandle rtarg) 
{
	switch (i)
	{
		case LAYERDLG_CONTREF:
			cont=(LayerControl*)rtarg;
			break;
	}
}

RefResult LayerControlDlg::NotifyRefChanged(
		Interval iv, 
		RefTargetHandle rtarg, 
		PartID& partID, 
		RefMessage message)
	{
	switch (message) {
		case REFMSG_CHANGE:
			Invalidate();
			break;
				
		case REFMSG_REF_DELETED:
			MaybeCloseWindow();
			break;
		
		}
	return REF_SUCCEED;
}


static INT_PTR CALLBACK LayerControlDlgProc(
		HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
			// WIN64 Cleanup: Shuler
	LayerControlDlg *ld = DLGetWindowLongPtr<LayerControlDlg*>(hWnd);
			// WIN64 Cleanup: Shuler
	
	static BOOL undoHeldHere = FALSE; //AF -- this flag is used to trigger an undo begin in the even of a WM_CUSTEDIT_ENTER
	
	switch (message)
	{
		case WM_INITDIALOG:
		{ 			
			SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam);
					// WIN64 Cleanup: Shuler
			ld = (LayerControlDlg*)lParam;
			if (ld->cont->SuperClassID() == CTRL_ROTATION_CLASS_ID)
			{
				int val,i;
				SetDlgItemText(hWnd, IDC_AVERAGE_WEIGHTS, GetString(IDS_AF_LIST_POSE_TO_POSE));

				RotationLayerControl *rLC = static_cast<RotationLayerControl*>(ld->cont);
				SendDlgItemMessage(hWnd, IDC_EULER_ORDER_X, CB_RESETCONTENT, 0, 0);
				for (i=0; i<NUM_SUPPORTED_EULERS; i++) {
					SendDlgItemMessage(hWnd,IDC_EULER_ORDER_X, CB_ADDSTRING, 0,
						(LPARAM)GetString(eulerIDs[i]));
				}
				val = rLC->GetEulerXAxisOrder();
				SendDlgItemMessage(hWnd, IDC_EULER_ORDER_X, CB_SETCURSEL,val, 0);

				SendDlgItemMessage(hWnd, IDC_EULER_ORDER_Y, CB_RESETCONTENT, 0, 0);
				for (i=0; i<NUM_SUPPORTED_EULERS; i++) {
					SendDlgItemMessage(hWnd,IDC_EULER_ORDER_Y, CB_ADDSTRING, 0,
						(LPARAM)GetString(eulerIDs[i]));
				}
				val = rLC->GetEulerYAxisOrder();
				SendDlgItemMessage(hWnd, IDC_EULER_ORDER_Y, CB_SETCURSEL,val, 0);

				SendDlgItemMessage(hWnd, IDC_EULER_ORDER_Z, CB_RESETCONTENT, 0, 0);
				for (i=0; i<NUM_SUPPORTED_EULERS; i++) {
					SendDlgItemMessage(hWnd,IDC_EULER_ORDER_Z, CB_ADDSTRING, 0,
						(LPARAM)GetString(eulerIDs[i]));
				}
				val = rLC->GetEulerZAxisOrder();
				SendDlgItemMessage(hWnd, IDC_EULER_ORDER_Z, CB_SETCURSEL,val, 0);

				BOOL what = rLC->GetBlendEulerAsQuat();
				if(rLC->okayToBlendEulers==FALSE)
				{
					what = FALSE;
					EnableWindow(GetDlgItem(hWnd,IDC_BLEND_EULER_AS_QUAT),FALSE);
				}
				else
					EnableWindow(GetDlgItem(hWnd,IDC_BLEND_EULER_AS_QUAT),TRUE);

				CheckDlgButton(hWnd, IDC_BLEND_EULER_AS_QUAT,what );
				
				if(what==TRUE)
				{
					EnableWindow(GetDlgItem(hWnd,IDC_EULER_ORDER_X),TRUE);
					EnableWindow(GetDlgItem(hWnd,IDC_EULER_ORDER_Y),TRUE);
					EnableWindow(GetDlgItem(hWnd,IDC_EULER_ORDER_Z),TRUE);
				}
				else
				{
					EnableWindow(GetDlgItem(hWnd,IDC_EULER_ORDER_X),FALSE);
					EnableWindow(GetDlgItem(hWnd,IDC_EULER_ORDER_Y),FALSE);
					EnableWindow(GetDlgItem(hWnd,IDC_EULER_ORDER_Z),FALSE);
				}


			}
			break;
		}
		case WM_NOTIFY:
		{
			int idCtrl = (int)wParam;
			LPNMHDR pnmh = (LPNMHDR)lParam;

			switch( idCtrl )
			{
				case IDC_CONTROLLER_WEIGHT_LIST:
				{
					switch(pnmh->code)
					{
						case LVN_ITEMCHANGED:
							{
							if ( !(((NMLISTVIEW *)lParam)->uChanged & LVIF_STATE) )
								return 0;

							if ( (((NMLISTVIEW *)lParam)->uNewState & LVIS_SELECTED) || (((NMLISTVIEW *)lParam)->uOldState & LVIS_SELECTED) )
								ld->SetButtonStates();				
							}
							return 0;
						case NM_DBLCLK:
							if (((LPNMITEMACTIVATE)lParam)->iItem >= 0 && ((LPNMITEMACTIVATE)lParam)->iItem < ld->cont->conts.Count())
							{
								ld->cont->SetLayerActive(((LPNMITEMACTIVATE)lParam)->iItem);
							}
							return 1;
					}
				}
			}
			break;
		}

		case WM_COMMAND:
			ld->WMCommand(LOWORD(wParam),HIWORD(wParam),(HWND)lParam);						
			break;
		case CC_SPINNER_BUTTONDOWN:
			theHold.Begin();
			undoHeldHere = TRUE;
			break;
		case CC_SPINNER_CHANGE: {
			if (!undoHeldHere) {theHold.Begin();undoHeldHere = TRUE;}
			TimeValue t = GetCOREInterface()->GetTime();
			switch (LOWORD(wParam)) 
			{
				case IDC_LIST_WEIGHTSPIN: {
					HWND hList = GetDlgItem(hWnd, IDC_CONTROLLER_WEIGHT_LIST);
					int sel = ListView_GetNextItem(hList, -1, LVNI_SELECTED);
					ISpinnerControl* iWeight = GetISpinner(GetDlgItem(hWnd,IDC_LIST_WEIGHTSPIN));
					if (sel!=LB_ERR)
					{
						ld->cont->SetLayerWeight(sel,t,iWeight->GetFVal()/100.0f);
					}
					ReleaseISpinner(iWeight);
					break;
					}
				default:
					break;
			}
			ld->cont->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
			GetCOREInterface()->RedrawViews(t);
			break;
			}
		case WM_CUSTEDIT_ENTER:
		case CC_SPINNER_BUTTONUP:
			if (HIWORD(wParam) || message==WM_CUSTEDIT_ENTER)
				theHold.Accept(GetString(IDS_AF_LIST_WEIGHT_UNDO));
			else theHold.Cancel();
			undoHeldHere = FALSE;
			break;
		case WM_PAINT:
			ld->Update();
			return 0;			
		
		case WM_CLOSE:
			DestroyWindow(hWnd);			
			break;

		case WM_DESTROY:						
			delete ld;
			ld = NULL;
			break;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:			
			ld->MouseMessage(message,wParam,lParam);
			return FALSE;

		default:
			return 0;
		}
	return 1;
}


HWND LayerControlMotionDlg::CreateWin(HWND hParent)
{
	TSTR name;
	cont->GetClassName(name);
	if(cont->SuperClassID() != CTRL_ROTATION_CLASS_ID)
	{
		return ip->AddRollupPage( 
			hInstance, 
			MAKEINTRESOURCE(IDD_LAYERPARAMS_MOTION),
			LayerControlDlgProc,
			name,
			(LPARAM)this);		
	}
	else
	{
		return ip->AddRollupPage( 
			hInstance, 
			MAKEINTRESOURCE(IDD_ROTLAYERPARAMS_MOTION),
			LayerControlDlgProc,
			name,
			(LPARAM)this);


	}


}

HWND LayerControlTrackDlg::CreateWin(HWND hParent)
{
	if(cont->SuperClassID() != CTRL_ROTATION_CLASS_ID)
	{
		return CreateDialogParam(
		hInstance,
		MAKEINTRESOURCE(IDD_LAYERPARAMS_TRACK),
		hParent,
		LayerControlDlgProc,
		(LPARAM)this);
	}
	else
	{
		return CreateDialogParam(
		hInstance,
		MAKEINTRESOURCE(IDD_ROTLAYERPARAMS_TRACK),
		hParent,
		LayerControlDlgProc,
		(LPARAM)this);
	}


}

class CheckForNonListDlg : public DependentEnumProc {
	public:		
		BOOL non;
		ReferenceMaker *me;
		CheckForNonListDlg(ReferenceMaker *m) {non = FALSE;me = m;}
		int proc(ReferenceMaker *rmaker) {
			if (rmaker==me) return DEP_ENUM_CONTINUE;
			if (rmaker->SuperClassID()!=REF_MAKER_CLASS_ID &&
				rmaker->ClassID()!=Class_ID(LAYERDLG_CLASS_ID,0)) {
				non = TRUE;
				return DEP_ENUM_HALT;
				}
			return DEP_ENUM_SKIP; // just look at direct dependents
			}
	};

void LayerControlTrackDlg::MaybeCloseWindow()
{
	CheckForNonListDlg check(cont);
	cont->DoEnumDependents(&check);
	if (!check.non)
	{
		PostMessage(hWnd,WM_CLOSE,0,0);
	}
}




//-------------------------------------------------------------------------
// Float layer control
//		
		




void FloatLayerControl::GetValue(
		TimeValue t, void *val, Interval &valid, GetSetMethod method)
{
	float *v = (float*)val;
	float weight;

	if (method==CTRL_ABSOLUTE) {
		*v = 0.0f;
		}

	if (conts.Count() != mLCs.Count())
		return;

	float prevVal;
	int stop = conts.Count();
	if(GetMLCMan()->GetJustUpToActive()==true)
		stop = active +1;
	for (int i=0; i<stop; i++) 
	{
		MasterLayerControl *mLC = GetMLCMan()->GetNthMLC(mLCs[i]);
		if(mLC==NULL||mLC->mute==true)
			continue;
		weight = mLC->GetWeight(t,valid);
		if (weight != 0.0f)
		{
			prevVal = *v;
			if (conts[i]) conts[i]->GetValue(t,v,valid,CTRL_RELATIVE);
			*v = prevVal + ((*v) - prevVal)*AverageWeight(weight,t,valid);
		}
	}
}

void FloatLayerControl::GetOutputValue(
		TimeValue t, void *val, Interval &valid)
{
	float *v = (float*)val;
	float weight;

	*v = 0.0f;

	// CAL-10/18/2002: conts.Count() and pblock->Count(pblock->IndextoID(kLayerCtrlWeight)) might go out of sync while undo. (459225)
	
	if (conts.Count() != mLCs.Count())
		return;

	float prevVal;
	int stop = conts.Count();
	if(GetMLCMan()->GetJustUpToActive()==true)
		stop = active +1;
	for (int i=0; i<stop; i++) 
	{
		MasterLayerControl *mLC = GetMLCMan()->GetNthMLC(mLCs[i]);
		if(mLC==NULL||mLC->outputMute==true)
			continue;
		weight = mLC->GetWeight(t,valid);
		if (weight != 0.0f)
		{
			prevVal = *v;
			if (conts[i]) conts[i]->GetValue(t,v,valid,CTRL_RELATIVE);
			*v = prevVal + ((*v) - prevVal)*AverageWeight(weight,t,valid);
		}
	}
}

void FloatLayerControl::SetValue(
		TimeValue t, void *val, int commit, GetSetMethod method)
{
	if (!conts.Count()) return;	
	float weight;

	Interval valid = FOREVER;		
	if (method==CTRL_ABSOLUTE)
	{
		float v = *((float*)val);

		weight = GetMLCMan()->GetNthMLC(mLCs[active])->GetWeight(t,valid);

		v *= weight;
		float before = 0.0f, after = 0.0f;
		float prevVal;
		int i;
		for (i=0; i<active; i++) 
		{
			prevVal = before;
			if (conts[i]) conts[i]->GetValue(t,&before,valid,CTRL_RELATIVE);
			if(GetMLCMan()->GetNthMLC(mLCs[i]))
			{
			weight = GetMLCMan()->GetNthMLC(mLCs[i])->GetWeight(t,valid);
			before = prevVal + (before - prevVal)*AverageWeight(weight,t,valid);
			}
		//	conts[i]->GetValue(t,&before,valid,CTRL_RELATIVE);
		}
		for (i=active+1; i<conts.Count(); i++)
		{
			prevVal = after;
			if (conts[i]) conts[i]->GetValue(t,&after,valid,CTRL_RELATIVE);
			if(GetMLCMan()->GetNthMLC(mLCs[i]))
			{
			weight = GetMLCMan()->GetNthMLC(mLCs[i])->GetWeight(t,valid);
			after = prevVal + (after - prevVal)*AverageWeight(weight,t,valid);
			}
		//	conts[i]->GetValue(t,&after,valid,CTRL_RELATIVE);
		}
		v = -before + v + -after;
		assert(active>=0);
		if(conts[active]) conts[active]->SetValue(t,&v,commit,method);
	} 
	else 
	{
		assert(active>=0);
		if(GetMLCMan()->GetNthMLC(mLCs[active]))
		{
		weight = GetMLCMan()->GetNthMLC(mLCs[active])->GetWeight(t,valid);
		*((float*)val) *= weight;
		}
		if(conts[active])	conts[active]->SetValue(t,val,commit,method);
	}
}

LayerControl *FloatLayerControl::DerivedClone()
{
	return new FloatLayerControl;
}


ClassDesc* GetFloatLayerDesc() {return &floatLayerCD;}

void FloatLayerControl::Init()
{
	// make the paramblock
	floatLayerCD.MakeAutoParamBlocks(this);

	CreateController(kX);
}



//-------------------------------------------------------------------------
// Point3 layer control
//		
		
class Point3LayerControl : public LayerControl
{
	public:
		Point3LayerControl(BOOL loading) : LayerControl(loading) {Init();} 
		Point3LayerControl() {Init();}

		void GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method=CTRL_ABSOLUTE);
		void GetOutputValue(TimeValue t, void *val, Interval &valid);

		void SetValue(TimeValue t, void *val, int commit, GetSetMethod method);
		LayerControl *DerivedClone();
		void Init();

		Class_ID ClassID() { return (POINT3LAYER_CONTROL_CLASS_ID); }  
		SClass_ID SuperClassID() { return CTRL_POINT3_CLASS_ID; }  
		void GetClassName(TSTR& s) {s = POINT3LAYER_CNAME;}

		void SetUpPblockOnClone(LayerControl *newObj);

};

void Point3LayerControl::GetValue(
		TimeValue t, void *val, Interval &valid, GetSetMethod method)
{
	Point3 *v = (Point3*)val;
	float weight;

	if (method==CTRL_ABSOLUTE)
	{
		*v = Point3(0,0,0);
	}
	Point3 prevVal;
	int stop = conts.Count();
	if(GetMLCMan()->GetJustUpToActive()==true)
		stop = active +1;
	for (int i=0; i<stop; i++)
	{
		MasterLayerControl *mLC = GetMLCMan()->GetNthMLC(mLCs[i]);
		if(mLC==NULL||mLC->mute==true)
			continue;
		weight = mLC->GetWeight(t,valid);		
		if (weight != 0.0f) 
		{
			prevVal = *v;
			if (conts[i]) conts[i]->GetValue(t,v,valid,CTRL_RELATIVE);
			*v = prevVal + ((*v) - prevVal)*AverageWeight(weight,t,valid);
		}
	}
}


void Point3LayerControl::GetOutputValue(
		TimeValue t, void *val, Interval &valid)
{
	Point3 *v = (Point3*)val;
	float weight;

	*v = Point3(0,0,0);
	Point3 prevVal;
	int stop = conts.Count();
	if(GetMLCMan()->GetJustUpToActive()==true)
		stop = active +1;
	for (int i=0; i<stop; i++)
	{
		MasterLayerControl *mLC = GetMLCMan()->GetNthMLC(mLCs[i]);
		if(mLC==NULL||mLC->outputMute==true)
			continue;
		weight = mLC->GetWeight(t,valid);		
		if (weight != 0.0f) 
		{
			prevVal = *v;
			if (conts[i]) conts[i]->GetValue(t,v,valid,CTRL_RELATIVE);
			*v = prevVal + ((*v) - prevVal)*AverageWeight(weight,t,valid);
		}
	}
}
void Point3LayerControl::SetValue(
		TimeValue t, void *val, int commit, GetSetMethod method)
{
	if (!conts.Count()) return;	
	float weight;

	Interval valid = FOREVER;
	if (method==CTRL_ABSOLUTE)
	{
		Point3 v = *((Point3*)val);
		Point3 before(0,0,0), after(0,0,0);
		Point3 prevVal;
		int i;
		for (i=0; i<active; i++)
		{
			prevVal = before;
			if (conts[i]) conts[i]->GetValue(t,&before,valid,CTRL_RELATIVE);
			if(GetMLCMan()->GetNthMLC(mLCs[i]))
			{
			weight = GetMLCMan()->GetNthMLC(mLCs[i])->GetWeight(t,valid);
			before = prevVal + (before - prevVal)*AverageWeight(weight,t,valid);
			}
			//conts[i]->GetValue(t,&before,valid,CTRL_RELATIVE);
		}
		for (i=active+1; i<conts.Count(); i++)
		{
			prevVal = after;
			if (conts[i]) conts[i]->GetValue(t,&after,valid,CTRL_RELATIVE);
			if(GetMLCMan()->GetNthMLC(mLCs[i]))
			{
			weight = GetMLCMan()->GetNthMLC(mLCs[i])->GetWeight(t,valid);
			after = prevVal + (after - prevVal)*AverageWeight(weight,t,valid);
			}
			//conts[i]->GetValue(t,&after,valid,CTRL_RELATIVE);
		}
		v = -before + v + -after;
		assert(active>=0);
		if(conts[active])conts[active]->SetValue(t,&v,commit,method);
	} else
	{
		assert(active>=0);
		if(GetMLCMan()->GetNthMLC(mLCs[active]))
		{
		weight = GetMLCMan()->GetNthMLC(mLCs[active])->GetWeight(t,valid);
		*((Point3*)val) = *((Point3*)val) * AverageWeight(weight,t,valid);
		}
		if(conts[active])conts[active]->SetValue(t,val,commit,method);
	}
}

LayerControl *Point3LayerControl::DerivedClone()
{
	return new Point3LayerControl;
}

//keeps track of whether an FP interface desc has been added to the ClassDesc
static bool p3LayerInterfaceLoaded = false;

class Point3LayerClassDesc : public ClassDesc2
{
	public:
	int 			IsPublic() {return 0;}
	const TCHAR *	ClassName() {return POINT3LAYER_CNAME;}
    SClass_ID		SuperClassID() {return CTRL_POINT3_CLASS_ID;}
	Class_ID		ClassID() {return (POINT3LAYER_CONTROL_CLASS_ID);}
	const TCHAR* 	Category() {return _T("");}
	void *	Create(BOOL loading) { 	
		if (!p3LayerInterfaceLoaded)
			{
			AddInterface(&layerControlInterface);
			p3LayerInterfaceLoaded = true;
			}
		return new Point3LayerControl(loading);
		}
	
	const TCHAR*	InternalName() { return _T("Point3Layer"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }					// returns owning module handle
};

static Point3LayerClassDesc point3LayerCD;
ClassDesc* GetPoint3LayerDesc() {return &point3LayerCD;}

void Point3LayerControl::Init()
{
	// make the paramblock
	point3LayerCD.MakeAutoParamBlocks(this);

	for(int i = kX;i<= (kZ);++i)
	{
		CreateController(i);
	}

}





// CAL-10/30/2002: Add individual ParamBlockDesc2 for each list controller (463222)
//		When the pblock is cloned in LayerControl::Clone(), the owner of the new pblock isn't set to the
//		new list controller after ctrl->ReplaceReference() is called.
//		Normally when ReplaceReference() is called with pblock, the owner of the pblock will be set
//		by ParamBlock2::RefAdded() if the rmaker's ClassID and the ParamBlockDesc2->ClassDescriptor's
//		ClassID are the same. However, all list controllers use one ParamBlockDesc2, layer_paramblk,
//		which has a class descriptor, floatLayerCD. Therefore, the owner of the new pblock will remain
//		NULL if 'ctrl' is not a FloatLayerControl. In max, it is assumed that a ParamBlockDesc2 belongs
//		to a single ClassDesc. The list controllers share one ParamBlockDesc2 in many Classes and
//		unfortunately max's architecture isn't quite ready for that yet.
//		Fix it by adding individual ParamBlockDesc2 for each list controller and remove the call of
//		AddParamBlockDesc() from the ClassDesc2::Create() method of these classes.

// per instance list controller block
static ParamBlockDesc2 point3_layer_paramblk (0, _T("Parameters"),  0, &point3LayerCD, P_AUTO_CONSTRUCT, PBLOCK_INIT_REF_ID, 
	// params
	
	kLayerCtrlAverage, _T("average"), TYPE_BOOL, 0, IDS_AF_LIST_AVERAGE, 
		p_default, 		FALSE, 
		end, 

	kX, 	_T("exposedX"), TYPE_FLOAT, 	P_ANIMATABLE|P_TRANSIENT, 	IDS_RB_X, 
		end,
	kY, 	_T("exposedY"), 		TYPE_FLOAT, 	P_ANIMATABLE|P_TRANSIENT, 	IDS_RB_Y,  
		end,
	kZ, 	_T("exposedZ"), 		TYPE_FLOAT, 	P_ANIMATABLE|P_TRANSIENT, 	IDS_RB_Z, 
		end,
	end
);

//-------------------------------------------------------------------------
// Point4 layer control
//		
		
class Point4LayerControl : public LayerControl
{
	public:
		Point4LayerControl(BOOL loading) : LayerControl(loading) {Init();} 
		Point4LayerControl() {Init();}

		void GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method=CTRL_ABSOLUTE);
		void GetOutputValue(TimeValue t, void *val, Interval &valid);

		void SetValue(TimeValue t, void *val, int commit, GetSetMethod method);
		LayerControl *DerivedClone();
		void Init();

		Class_ID ClassID() { return (POINT4LAYER_CONTROL_CLASS_ID); }  
		SClass_ID SuperClassID() { return CTRL_POINT4_CLASS_ID; }  
		void GetClassName(TSTR& s) {s = POINT4LAYER_CNAME;}
};

void Point4LayerControl::GetValue(
		TimeValue t, void *val, Interval &valid, GetSetMethod method)
{
	Point4 *v = (Point4*)val;
	float weight;

	if (method==CTRL_ABSOLUTE)
	{
		*v = Point4(0,0,0,0);
	}
	Point4 prevVal;
	int stop = conts.Count();
	if(GetMLCMan()->GetJustUpToActive()==true)
		stop = active +1;
	for (int i=0; i<stop; i++)
	{
		MasterLayerControl *mLC = GetMLCMan()->GetNthMLC(mLCs[i]);
		if(mLC==NULL||mLC->mute==true)
			continue;
		weight = mLC->GetWeight(t,valid);
		if (weight != 0.0f) 
		{
			prevVal = *v;
			if (conts[i]) conts[i]->GetValue(t,v,valid,CTRL_RELATIVE);
			*v = prevVal + ((*v) - prevVal)*AverageWeight(weight,t,valid);
		}
	}
}


void Point4LayerControl::GetOutputValue(
		TimeValue t, void *val, Interval &valid)
{
	Point4 *v = (Point4*)val;
	float weight;

	*v = Point4(0,0,0,0);

	Point4 prevVal;
	int stop = conts.Count();
	if(GetMLCMan()->GetJustUpToActive()==true)
		stop = active +1;
	for (int i=0; i<stop; i++)
	{
		MasterLayerControl *mLC = GetMLCMan()->GetNthMLC(mLCs[i]);
		if(mLC==NULL||mLC->outputMute==true)
			continue;
		weight = mLC->GetWeight(t,valid);		
		if (weight != 0.0f) 
		{
			prevVal = *v;
			if (conts[i]) conts[i]->GetValue(t,v,valid,CTRL_RELATIVE);
			*v = prevVal + ((*v) - prevVal)*AverageWeight(weight,t,valid);
		}
	}
}

void Point4LayerControl::SetValue(
		TimeValue t, void *val, int commit, GetSetMethod method)
{
	if (!conts.Count()) return;	
	float weight;

	Interval valid = FOREVER;
	if (method==CTRL_ABSOLUTE)
	{
		Point4 v = *((Point4*)val);
		Point4 before(0,0,0,0), after(0,0,0,0);
		Point4 prevVal;
		int i;
		for (i=0; i<active; i++)
		{
			prevVal = before;
			if (conts[i]) conts[i]->GetValue(t,&before,valid,CTRL_RELATIVE);
			if(GetMLCMan()->GetNthMLC(mLCs[i]))
			{
				weight = GetMLCMan()->GetNthMLC(mLCs[i])->GetWeight(t,valid);
				before = prevVal + (before - prevVal)*AverageWeight(weight,t,valid);
			}
			//conts[i]->GetValue(t,&before,valid,CTRL_RELATIVE);
		}
		for (i=active+1; i<conts.Count(); i++)
		{
			prevVal = after;
			if (conts[i]) conts[i]->GetValue(t,&after,valid,CTRL_RELATIVE);
			if(GetMLCMan()->GetNthMLC(mLCs[i]))
			{
				weight = GetMLCMan()->GetNthMLC(mLCs[i])->GetWeight(t,valid);
				after = prevVal + (after - prevVal)*AverageWeight(weight,t,valid);
			}
			//conts[i]->GetValue(t,&after,valid,CTRL_RELATIVE);
		}
		v = -before + v + -after;
		assert(active>=0);
		if(conts[active])conts[active]->SetValue(t,&v,commit,method);
	}
	else
	{
		assert(active>=0);
		if(GetMLCMan()->GetNthMLC(mLCs[active]))
		{
			weight = GetMLCMan()->GetNthMLC(mLCs[active])->GetWeight(t,valid);
			*((Point4*)val) = *((Point4*)val) * AverageWeight(weight,t,valid);
		}
		if(conts[active])conts[active]->SetValue(t,val,commit,method);
	}
}

LayerControl *Point4LayerControl::DerivedClone()
{
	return new Point4LayerControl;
}

//keeps track of whether an FP interface desc has been added to the ClassDesc
static bool p4LayerInterfaceLoaded = false;

class Point4LayerClassDesc : public ClassDesc2
{
	public:
	int 			IsPublic() {return 0;}
	const TCHAR *	ClassName() {return POINT4LAYER_CNAME;}
    SClass_ID		SuperClassID() {return CTRL_POINT4_CLASS_ID;}
	Class_ID		ClassID() {return (POINT4LAYER_CONTROL_CLASS_ID);}
	const TCHAR* 	Category() {return _T("");}
	void *	Create(BOOL loading)
	{ 	
		if (!p4LayerInterfaceLoaded)
		{
			AddInterface(&layerControlInterface);
			p4LayerInterfaceLoaded = true;
		}
		return new Point4LayerControl(loading);
	}
	
	const TCHAR*	InternalName() { return _T("Point4Layer"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }					// returns owning module handle
};
static Point4LayerClassDesc point4LayerCD;
ClassDesc* GetPoint4LayerDesc() {return &point4LayerCD;}

void Point4LayerControl::Init()
{
	// make the paramblock
	point4LayerCD.MakeAutoParamBlocks(this);
	for(int i = kX;i<= (kZ);++i)
	{
		CreateController(i);
	}

}

// per instance list controller block
static ParamBlockDesc2 point4_layer_paramblk (0, _T("Parameters"),  0, &point4LayerCD, P_AUTO_CONSTRUCT, PBLOCK_INIT_REF_ID, 
	// params
	
	kLayerCtrlAverage, _T("average"), TYPE_BOOL, 0, IDS_AF_LIST_AVERAGE, 
		p_default, 		FALSE, 
		end, 

	kX, 	_T("exposedX"), TYPE_FLOAT, 	P_ANIMATABLE|P_TRANSIENT, 	IDS_RB_X, 
		end,
	kY, 	_T("exposedY"), 		TYPE_FLOAT, 	P_ANIMATABLE|P_TRANSIENT, 	IDS_RB_Y,  
		end,
	kZ, 	_T("exposedZ"), 		TYPE_FLOAT, 	P_ANIMATABLE|P_TRANSIENT, 	IDS_RB_Z, 
		end,
	kW, 	_T("exposedW"), 		TYPE_FLOAT, 	P_ANIMATABLE|P_TRANSIENT, 	IDS_RB_W, 
		end,
	end
	);


//-------------------------------------------------------------------------
// Position layer control
//		
		
class PositionLayerControl : public LayerControl
{
	public:
		PositionLayerControl(BOOL loading) : LayerControl(loading) {Init();} 
		PositionLayerControl() {Init();}

		void GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method=CTRL_ABSOLUTE);
		void GetOutputValue(TimeValue t, void *val, Interval &valid);

		void SetValue(TimeValue t, void *val, int commit, GetSetMethod method);
		LayerControl *DerivedClone();
		void Init();
		
		Class_ID ClassID() { return (POSLAYER_CONTROL_CLASS_ID); }  
		SClass_ID SuperClassID() { return CTRL_POSITION_CLASS_ID; }  
		void GetClassName(TSTR& s) {s = POSLAYER_CNAME;}
};



// CAL-08/16/02: Relative to identity matrix.
//		Good:	SetValue() can be done without the parent's transformation
//		Bad:	constraints will be affected by the parent's transformation if it's not identity matrix.
void PositionLayerControl::GetValue(
		TimeValue t, void *val, Interval &valid, GetSetMethod method)
{
	Matrix3 tm(1);
	float weight;

	int stop = conts.Count();
	if(GetMLCMan()->GetJustUpToActive()==true)
		stop = active+1;
	if (method==CTRL_ABSOLUTE)
	{
		Point3 prevPos;
		Matrix3 tm(1);

		for (int i=0; i<stop; i++)
		{
			MasterLayerControl *mLC = GetMLCMan()->GetNthMLC(mLCs[i]);
			if(mLC==NULL||mLC->mute==true)
				continue;
			weight = mLC->GetWeight(t,valid);
			if (weight != 0.0f)
			{
				prevPos = tm.GetTrans();
				if (conts[i]) conts[i]->GetValue(t,&tm,valid,CTRL_RELATIVE);
				tm.SetTrans(prevPos + (tm.GetTrans() - prevPos)*AverageWeight(weight,t,valid));
			}
		}
		*(Point3*)val = tm.GetTrans();
	} else 
	{
		Matrix3* tm = (Matrix3*)val;
		Point3 prevPos;
		for (int i=0; i<stop; i++)
		{
			if (conts[i])
			{
				MasterLayerControl *mLC = GetMLCMan()->GetNthMLC(mLCs[i]);
				if(mLC==NULL||mLC->mute==true)
					continue;
				weight = mLC->GetWeight(t,valid);
				if (weight != 0.0f)
				{
					prevPos = tm->GetTrans();
					conts[i]->GetValue(t,tm,valid,CTRL_RELATIVE);
					tm->SetTrans(prevPos + (tm->GetTrans() - prevPos)*AverageWeight(weight,t,valid));
				}
			}
		}
	}
}
	


void PositionLayerControl::GetOutputValue(
		TimeValue t, void *val, Interval &valid)
{
	Matrix3 tm(1);
	float weight;


	int stop = conts.Count();
	if(GetMLCMan()->GetJustUpToActive()==true)
		stop = active+1;
	for (int i=0; i<stop; i++)
	{
		MasterLayerControl *mLC = GetMLCMan()->GetNthMLC(mLCs[i]);
		if(mLC==NULL||mLC->outputMute==true)
			continue;
		weight = mLC->GetWeight(t,valid);

		if (conts[i]) {
			Matrix3 deltaTM(1);
			conts[i]->GetValue(t, &deltaTM, valid, CTRL_RELATIVE);
			deltaTM.SetTrans(deltaTM.GetTrans() * AverageWeight(weight,t,valid));
			tm = deltaTM * tm;
		}
	}

	*(Point3*)val = tm.GetTrans();
}



void PositionLayerControl::SetValue(
		TimeValue t, void *val, int commit, GetSetMethod method)
{
	if (!conts.Count()) return;	
	float weight;
	Point3 v = *((Point3*)val);

	Matrix3 before(1), after(1);
	Interval valid = FOREVER;
	Point3 prevPos;

	int i;
	for (i=0; i<active; i++)
	{
		prevPos = before.GetTrans();
		if (conts[i]) conts[i]->GetValue(t,&before,valid,CTRL_RELATIVE);
		if(GetMLCMan()->GetNthMLC(mLCs[i]))
		{
		weight = GetMLCMan()->GetNthMLC(mLCs[i])->GetWeight(t,valid);
		before.SetTrans(prevPos + (before.GetTrans() - prevPos)*AverageWeight(weight,t,valid));
		}
		//conts[i]->GetValue(t,&before,valid,CTRL_RELATIVE);
	}
	for (i=active+1; i<conts.Count(); i++)
	{
		prevPos = after.GetTrans();
		if (conts[i]) conts[i]->GetValue(t,&after,valid,CTRL_RELATIVE);
		if(GetMLCMan()->GetNthMLC(mLCs[i]))
		{
		weight = GetMLCMan()->GetNthMLC(mLCs[i])->GetWeight(t,valid);
		after.SetTrans(prevPos + (after.GetTrans() - prevPos)*AverageWeight(weight,t,valid));
		//conts[i]->GetValue(t,&after,valid,CTRL_RELATIVE);
	}
	}

	if (method==CTRL_ABSOLUTE)
	{
		//v = -before.GetTrans() + v + -after.GetTrans();
		// RB 11/28/2000:
		// This was incorrect. It should be:
		// Inverse(afterControllers) * val * Inverse(beforeControllers)

		//v = Inverse(before) * v * Inverse(after);
		v = Inverse(after) * v * Inverse(before);

		assert(active>=0);
		if(conts[active])conts[active]->SetValue(t,&v,commit,method);
	} else 
	{		
		assert(active>=0);

		v = VectorTransform(Inverse(before),v);
		v = VectorTransform(Inverse(after),v);

		if(conts[active])conts[active]->SetValue(t,&v,commit,method);
	}
}



LayerControl *PositionLayerControl::DerivedClone()
{
	return new PositionLayerControl;
}

//keeps track of whether an FP interface desc has been added to the ClassDesc
static bool posLayerInterfaceLoaded = false;

class PositionLayerClassDesc : public ClassDesc2
{
	public:
	int 			IsPublic() {return 0;}
	const TCHAR *	ClassName() {return POSLAYER_CNAME;}
    SClass_ID		SuperClassID() {return CTRL_POSITION_CLASS_ID;}
	Class_ID		ClassID() {return (POSLAYER_CONTROL_CLASS_ID);}
	const TCHAR* 	Category() {return _T("");}
	void *	Create(BOOL loading)
	{ 		
		if (!posLayerInterfaceLoaded)
		{
			AddInterface(&layerControlInterface);
			posLayerInterfaceLoaded = true;
		}
		return new PositionLayerControl(loading);
	}
	
	const TCHAR*	InternalName() { return _T("PositionLayer"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }					// returns owning module handle
};

static PositionLayerClassDesc posLayerCD;
ClassDesc* GetPositionLayerDesc() {return &posLayerCD;}

void PositionLayerControl::Init()
{
	// make the paramblock
	posLayerCD.MakeAutoParamBlocks(this);
	for(int i = kX;i<= (kZ);++i)
	{
		CreateController(i);
	}

}

// CAL-10/30/2002: Add individual ParamBlockDesc2 for each list controller - see point3_layer_paramblk.
// per instance list controller block
static ParamBlockDesc2 pos_layer_paramblk (0, _T("Parameters"),  0, &posLayerCD, P_AUTO_CONSTRUCT, PBLOCK_INIT_REF_ID, 
	// params
	kLayerCtrlAverage, _T("average"), TYPE_BOOL, 0, IDS_AF_LIST_AVERAGE, 
		p_default, 		FALSE, 
		end, 

	kX, 	_T("exposedX"), TYPE_FLOAT, 	P_ANIMATABLE|P_TRANSIENT, 	IDS_RB_X, 
		end,
	kY, 	_T("exposedY"), 		TYPE_FLOAT, 	P_ANIMATABLE|P_TRANSIENT, 	IDS_RB_Y,  
		end,
	kZ, 	_T("exposedZ"), 		TYPE_FLOAT, 	P_ANIMATABLE|P_TRANSIENT, 	IDS_RB_Z, 
		end,

	end
);

//-------------------------------------------------------------------------
// Rotation layer control
//		


static BOOL AreXYZKeyableFloat(Control *onto)
{
	if((onto->GetXController()->ClassID()!=Class_ID(LININTERP_FLOAT_CLASS_ID,0)
		&&onto->GetXController()->ClassID()!=Class_ID(HYBRIDINTERP_FLOAT_CLASS_ID,0)
		&&onto->GetXController()->ClassID()!=Class_ID(TCBINTERP_FLOAT_CLASS_ID,0)
		&&onto->GetXController()->ClassID()!=BOOLCNTRL_CLASS_ID))
	{
		return FALSE;
	}
	else if((onto->GetYController()->ClassID()!=Class_ID(LININTERP_FLOAT_CLASS_ID,0)
		&&onto->GetYController()->ClassID()!=Class_ID(HYBRIDINTERP_FLOAT_CLASS_ID,0)
		&&onto->GetYController()->ClassID()!=Class_ID(TCBINTERP_FLOAT_CLASS_ID,0)
		&&onto->GetYController()->ClassID()!=BOOLCNTRL_CLASS_ID))
	{
		return FALSE;
	}
	else if((onto->GetZController()->ClassID()!=Class_ID(LININTERP_FLOAT_CLASS_ID,0)
		&&onto->GetZController()->ClassID()!=Class_ID(HYBRIDINTERP_FLOAT_CLASS_ID,0)
		&&onto->GetZController()->ClassID()!=Class_ID(TCBINTERP_FLOAT_CLASS_ID,0)
		&&onto->GetZController()->ClassID()!=BOOLCNTRL_CLASS_ID))
	{
		return FALSE;
	}
	return TRUE;
}
/*
BOOL RotationLayerControl::CheckOkayToCollapseAsEuler()
{
	if(conts.Count()<=0)
		return FALSE;
	int eulerOrder;
	Class_ID xClassID,yClassID, zClassID;
	BOOL firstOne = TRUE;
	for(int i=0;i<conts.Count();++i)
	{
		if(conts[i]==NULL||conts[i]->ClassID()!=Class_ID(EULER_CONTROL_CLASS_ID,0))
		{
			return FALSE;
			break;
		}
		IEulerControl *eC = static_cast<IEulerControl*>(conts[i]->GetInterface(I_EULERCTRL));
		if(eC!=NULL&&AreXYZKeyableFloat(conts[i]))
		{
			if(firstOne)
			{
				xClassID = conts[i]->GetXController()->ClassID();
				yClassID = conts[i]->GetYController()->ClassID();
				zClassID = conts[i]->GetZController()->ClassID();
				eulerOrder = eC->GetOrder();			
				firstOne = FALSE;
			}
			else
			{
				if(conts[i]->GetXController()->ClassID()!= xClassID)
				{
					return FALSE;
					break;
				}
				if(conts[i]->GetYController()->ClassID()!= yClassID)
				{
					return FALSE;
					break;
				}
				if(conts[i]->GetZController()->ClassID()!= zClassID)
				{
					return FALSE;
					break;
				}
				if(eC->GetOrder()!=eulerOrder)
				{
					return FALSE;
					break;
				}
			}

		}
		else
		{
			return FALSE;
			break;
		}
	}
	return TRUE;
}
*/

void RotationLayerControl::CheckOkayToBlendAsEulers()
{
	okayToBlendEulers = TRUE;
	int eulerOrder = 0;
	BOOL firstOne = TRUE;
	for(int i=0; i<conts.Count(); ++i)
	{
		if(conts[i]==NULL||conts[i]->ClassID()!=Class_ID(EULER_CONTROL_CLASS_ID,0))
		{
			okayToBlendEulers= FALSE;
			break;
		}
		IEulerControl *eC = static_cast<IEulerControl*>(conts[i]->GetInterface(I_EULERCTRL));
		if(eC!=NULL)
		{
			if(firstOne)
			{
				eulerOrder = eC->GetOrder();			
				firstOne = FALSE;
			}
			else
			{

				if(eC->GetOrder() != eulerOrder)
				{
					okayToBlendEulers = FALSE;
					break;
				}
			}

		}
		else
		{
			okayToBlendEulers = FALSE;
			break;
		}
	}
}

class EulerRecheck : public RestoreObj
{
public:
	RotationLayerControl *rLC;
	EulerRecheck(RotationLayerControl *lC){rLC =lC;};
	void Restore(int isUndo){rLC->CheckOkayToBlendAsEulers();}
	void Redo(){rLC->CheckOkayToBlendAsEulers();}
};

BOOL RotationLayerControl::AssignController(Animatable *control,int subAnim)
{
	if(theHold.Holding())
		theHold.Put(new EulerRecheck(this)); //this is makes sure that we reset whether or not we can blend as an euler for rot layers.

	BOOL what =LayerControl::AssignController(control,subAnim);
	if(what==TRUE&&conts.Count()>1)
	{
		CheckOkayToBlendAsEulers();
	}
	return what;
}

void RotationLayerControl::AddLayer(Control *control,int newMLC)
{
	if(theHold.Holding())
		theHold.Put(new EulerRecheck(this)); //this is makes sure that we reset whether or not we can blend as an euler for rot layers.

	LayerControl::AddLayer(control,newMLC);
	CheckOkayToBlendAsEulers();
}
void RotationLayerControl::DeleteLayerInternal(int active)
{
	if(theHold.Holding())
		theHold.Put(new EulerRecheck(this)); //this is makes sure that we reset whether or not we can blend as an euler for rot layers.
	LayerControl::DeleteLayerInternal(active);
	CheckOkayToBlendAsEulers();
}
void RotationLayerControl::CopyLayerInternal(int active)
{
	if(theHold.Holding())
		theHold.Put(new EulerRecheck(this)); //this is makes sure that we reset whether or not we can blend as an euler for rot layers.
	LayerControl::CopyLayerInternal(active);
	CheckOkayToBlendAsEulers();
}
void RotationLayerControl::PasteLayerInternal(int active)
{
	if(theHold.Holding())
		theHold.Put(new EulerRecheck(this)); //this is makes sure that we reset whether or not we can blend as an euler for rot layers.
	LayerControl::PasteLayerInternal(active);
	CheckOkayToBlendAsEulers();
}
void RotationLayerControl::CollapseLayerInternal(int index)
{
	if(theHold.Holding())
		theHold.Put(new EulerRecheck(this)); //this is makes sure that we reset whether or not we can blend as an euler for rot layers.
	LayerControl::CollapseLayerInternal(index);
	CheckOkayToBlendAsEulers();
}

int RotationLayerControl::GetEulerXAxisOrder()
{
	int order;
	pblock->GetValue(kX_order,GetCOREInterface()->GetTime(),order,FOREVER);
	return --order;

}
void RotationLayerControl::SetEulerXAxisOrder(int id)
{
	++id;
	pblock->SetValue(kX_order,0,id);
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);

}

int RotationLayerControl::GetEulerYAxisOrder()
{
	int order;
	pblock->GetValue(kY_order,GetCOREInterface()->GetTime(),order,FOREVER);
	return --order;

}

void RotationLayerControl::SetEulerYAxisOrder(int id)
{
	++id;
	pblock->SetValue(kY_order,0,id);
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
}

int RotationLayerControl::GetEulerZAxisOrder()
{
	int order;
	pblock->GetValue(kZ_order,GetCOREInterface()->GetTime(),order,FOREVER);
	return --order;

}
void RotationLayerControl::SetEulerZAxisOrder(int id)
{
	++id;
	pblock->SetValue(kZ_order,0,id);
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
}



BOOL RotationLayerControl::GetBlendEulerAsQuat()
{
	BOOL what;
	pblock->GetValue(kBlendEulerAsQuat,GetCOREInterface()->GetTime(),what,FOREVER);
	return what;

}
void RotationLayerControl::SetBlendEulerAsQuat(BOOL val)
{
	pblock->SetValue(kBlendEulerAsQuat,0,val);
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
}

typedef int EAOrdering[3];
static EAOrdering orderings[] = {
   // With the Euler order type o,
   // orerdings[o][i] gives the rotation axis of the i-th angle.
   {0,1,2},
   {0,2,1},
   {1,2,0},
   {1,0,2},
   {2,0,1},
   {2,1,0},
   {0,1,0},
   {1,2,1},
   {2,0,2},
   };

BOOL RotationLayerControl::GetValueAsEuler(TimeValue t, Interval &valid, BOOL output,Point3 &currentVal)
{

	currentVal = Point3(0,0,0);

	if(conts.Count()<=1)
		return FALSE;

	IEulerControl *eC = static_cast<IEulerControl*>(conts[0]->GetInterface(I_EULERCTRL));

	if(eC==NULL)
		return FALSE;

	float weight;
	BOOL average = FALSE;
	pblock->GetValue(kLayerCtrlAverage, 0, average, FOREVER);


	int stop = conts.Count();
	if(GetMLCMan()->GetJustUpToActive()==true)
		stop = active +1;

	Point3 prevVal;
	
	int numOfBlends = 0;
	for (int i=0; i<stop; i++) 
	{
		MasterLayerControl *mLC = GetMLCMan()->GetNthMLC(mLCs[i]);
		if(output==FALSE)
		{
			if(mLC==NULL||mLC->mute==true)
				continue;
		}
		else
		{
			if(mLC==NULL||mLC->outputMute==true)
				continue;
		}
		weight = mLC->GetWeight(t,valid);			
		if (weight != 0.0f && conts[i])
		{
			prevVal = currentVal; 
			for(int j=0;j<3;++j)
			{
				if(conts[i]->GetXController())
					conts[i]->GetXController()->GetValue(t,&currentVal.x,valid,CTRL_ABSOLUTE);
				else
					return FALSE;
				if(conts[i]->GetYController())
					conts[i]->GetYController()->GetValue(t,&currentVal.y,valid,CTRL_ABSOLUTE);
				else
					return FALSE;
				if(conts[i]->GetZController())
					conts[i]->GetZController()->GetValue(t,&currentVal.z,valid,CTRL_ABSOLUTE);
				else
					return FALSE;

				currentVal = (prevVal + currentVal*weight);
				++numOfBlends;
			}
		}
	}
	if(average)
		currentVal = currentVal/ (float)numOfBlends;
	return TRUE;
}
BOOL RotationLayerControl::GetValueAsEuler(TimeValue t, void *val, Interval &valid, GetSetMethod method,BOOL output)
{

	Point3 currentVal;
	BOOL good = GetValueAsEuler(t,valid,output,currentVal);
	if(good==FALSE)
		return FALSE;
	
	IEulerControl *eC = static_cast<IEulerControl*>(conts[0]->GetInterface(I_EULERCTRL));
	if(eC==NULL)
		return FALSE;

	Matrix3 tm(1);
	Quat q;
	int eulerOrder = eC->GetOrder();

	for (int i=0; i<3; i++) {
	  switch (orderings[eulerOrder][i]) {
		 case 0: tm.RotateX(currentVal[i]); break;
		 case 1: tm.RotateY(currentVal[i]); break;
		 case 2: tm.RotateZ(currentVal[i]); break;
		 }
	  }

	if (method == CTRL_RELATIVE) {
	  Matrix3 *mat = (Matrix3*)val;    
	  *mat = tm * *mat;

	}
	else
	{
	  q = Quat(tm);
	  *((Quat*)val) = q;
	
	}

	return TRUE;
}

void RotationLayerControl::GetValue(
		TimeValue t, void *val, Interval &valid, GetSetMethod method)
{
	if(okayToBlendEulers&&GetBlendEulerAsQuat())
	{
		if(GetValueAsEuler(t,val,valid,method,FALSE)==TRUE)
			return;
	}

	float weight;
	BOOL average = FALSE;
	pblock->GetValue(kLayerCtrlAverage, 0, average, FOREVER);

	Matrix3 tm, localTM;
	if (method==CTRL_ABSOLUTE)
	{
		tm.IdentityMatrix();
	}
	else
	{
		tm = *((Matrix3*)val);
	}
	
	int stop = conts.Count();
	if(GetMLCMan()->GetJustUpToActive()==true)
		stop = active +1;

	if (!average)
	{ // Addative method
		for (int i=0; i<stop; i++) 
		{
			MasterLayerControl *mLC = GetMLCMan()->GetNthMLC(mLCs[i]);
			if(mLC==NULL||mLC->mute==true)
				continue;
			weight = mLC->GetWeight(t,valid);			
			if (weight != 0.0f && conts[i])
			{
				localTM = tm;
				conts[i]->GetValue(t,&localTM,valid,CTRL_RELATIVE);
				localTM = localTM * Inverse(tm);

				Quat weightedRot = Quat(localTM);
				weightedRot.Normalize();
				weightedRot.MakeClosest(IdentQuat());	// CAL-10/15/2002: find the smallest rotation
				weightedRot = Slerp(IdentQuat(), weightedRot, min(max(weight, 0.0f), 1.0f));
				weightedRot.Normalize();
				PreRotateMatrix(tm, weightedRot);
			}
		}
	}
	else
	{ // pose to pose blending
		Matrix3 initTM = tm;
		Quat lastRot = IdentQuat();
		for (int i=0; i<stop; i++)
		{
			MasterLayerControl *mLC = GetMLCMan()->GetNthMLC(mLCs[i]);
			if(mLC==NULL||mLC->mute==true)
				continue;
			weight = mLC->GetWeight(t,valid);	
			if (weight != 0.0f && conts[i])
			{
				localTM = tm;
				conts[i]->GetValue(t,&localTM,valid,CTRL_RELATIVE);
				localTM = localTM * Inverse(tm);

				Quat weightedRot = Quat(localTM);
				weightedRot.Normalize();
				weightedRot.MakeClosest(lastRot);
				weightedRot = Slerp(lastRot, weightedRot, min(max(weight, 0.0f), 1.0f));
				weightedRot.Normalize();

				lastRot = weightedRot;
				weightedRot = weightedRot * Quat(initTM);

				Point3 trans = tm.GetTrans();
				tm.SetRotate(weightedRot);
				tm.SetTrans(trans);
			}
		}
	}
	if (method==CTRL_ABSOLUTE)
	{
		*((Quat*)val) = Quat(tm);
	}
	else
	{
		*((Matrix3*)val) = tm;
	}
}


//helper function with these order functions
static float GetEulerValue(float val[3],int which, int order)
{
	switch(order)
	{
	case EULERTYPE_XYZ:
		if(which==0)
			return val[0];
		else if(which==1)
			return val[1];
		else if(which==2)
			return val[2];
		break;
	case EULERTYPE_XZY:
		if(which==0)
			return val[0];
		else if(which==1)
			return val[2];
		else if(which==2)
			return val[1];
		break;
	case EULERTYPE_YZX:
		if(which==0)
			return val[2];
		else if(which==1)
			return val[0];
		else if(which==2)
			return val[1];
		break;
	case EULERTYPE_YXZ:
		if(which==0)
			return val[1];
		else if(which==1)
			return val[0];
		else if(which==2)
			return val[2];
		break;
	case EULERTYPE_ZXY:
		if(which==0)
			return val[1];
		else if(which==1)
			return val[2];
		else if(which==2)
			return val[0];
		break;
	case EULERTYPE_ZYX:
		if(which==0)
			return val[2];
		else if(which==1)
			return val[1];
		else if(which==2)
			return val[0];
		break;
	}
	return 0.0f;
}

static void CalculateEulerFromOrders(Point3 &val,Quat &q,int xorder,int yorder,int zorder)
{
	float valx[3],valy[3],valz[3];//each of the values for the three ..note only calculate them if different as opt

	QuatToEuler(q, valx, xorder);
	val[0] = GetEulerValue(valx,0,xorder);
	if(yorder==xorder)
		val[1] = GetEulerValue(valx,1,xorder);
	else
	{
		QuatToEuler(q,valy,yorder);
		val[1] = GetEulerValue(valy,1,yorder);
	}
	//now handle z
	if(zorder==xorder)
		val[2] = GetEulerValue(valx,2,xorder);
	else if(zorder==yorder)
		val[2] = GetEulerValue(valy,2,yorder);
	else
	{
		QuatToEuler(q,valz,zorder);
		val[2] = GetEulerValue(valz,2,zorder);
	}
}

void RotationLayerControl::GetOutputValue(
		TimeValue t, void *val, Interval &valid)
{

	if(okayToBlendEulers&&GetBlendEulerAsQuat())
	{
		Point3 currentVal;
		Quat q;
		BOOL good = GetValueAsEuler(t,valid,TRUE,currentVal);
		if(good==TRUE)
		{
			Point3 *pval = static_cast<Point3*>(val);
			*pval = currentVal;
			return;
		}
	}

	
	float weight;
	BOOL average = FALSE;
	pblock->GetValue(kLayerCtrlAverage, 0, average, FOREVER);

	Matrix3 tm, localTM;
	tm.IdentityMatrix();
		
	int stop = conts.Count();
	if(GetMLCMan()->GetJustUpToActive()==true)
		stop = active +1;

	if (!average)
	{ // Addative method
		for (int i=0; i<stop; i++) 
		{
			MasterLayerControl *mLC = GetMLCMan()->GetNthMLC(mLCs[i]);
			if(mLC==NULL||mLC->outputMute==true)
				continue;
			weight = mLC->GetWeight(t,valid);			
			if (weight != 0.0f && conts[i])
			{
				localTM = tm;
				conts[i]->GetValue(t,&localTM,valid,CTRL_RELATIVE);
				localTM = localTM * Inverse(tm);

				Quat weightedRot = Quat(localTM);
				weightedRot.Normalize();
				weightedRot.MakeClosest(IdentQuat());	// CAL-10/15/2002: find the smallest rotation
				weightedRot = Slerp(IdentQuat(), weightedRot, min(max(weight, 0.0f), 1.0f));
				weightedRot.Normalize();
				PreRotateMatrix(tm, weightedRot);
			}
		}
	}
	else
	{ // pose to pose blending
		Matrix3 initTM = tm;
		Quat lastRot = IdentQuat();
		for (int i=0; i<stop; i++)
		{
			MasterLayerControl *mLC = GetMLCMan()->GetNthMLC(mLCs[i]);
			if(mLC==NULL||mLC->outputMute==true)
				continue;
			weight = mLC->GetWeight(t,valid);	
			if (weight != 0.0f && conts[i])
			{
				localTM = tm;
				conts[i]->GetValue(t,&localTM,valid,CTRL_RELATIVE);
				localTM = localTM * Inverse(tm);

				Quat weightedRot = Quat(localTM);
				weightedRot.Normalize();
				weightedRot.MakeClosest(lastRot);
				weightedRot = Slerp(lastRot, weightedRot, min(max(weight, 0.0f), 1.0f));
				weightedRot.Normalize();

				lastRot = weightedRot;
				weightedRot = weightedRot * Quat(initTM);

				Point3 trans = tm.GetTrans();
				tm.SetRotate(weightedRot);
				tm.SetTrans(trans);
			}
		}
	}
	Quat what = Quat(tm);
	Point3 *pval = static_cast<Point3*>(val);
	int xorder = this->GetEulerXAxisOrder();
	int yorder = this->GetEulerYAxisOrder();
	int zorder = this->GetEulerZAxisOrder();
	CalculateEulerFromOrders(*pval,what,xorder,yorder,zorder);
}



static bool IsGimbalAxis(const Point3& a)
{
	int i = a.MaxComponent();
	if (a[i] != 1.0f) return false;
	int j = (i + 1) % 3, k = (i + 2) % 3;
	if (a[j] != 0.0f ||
		a[k] != FLT_MIN)
		return false;
	return true;
}
void RotationLayerControl::SetValue(
		TimeValue t, void *val, int commit, GetSetMethod method)
{
	if (!conts.Count()) return;	
	AngAxis *aa = (AngAxis*)val;
	Interval valid = FOREVER;		

	bool gimbal_axis = false;
	if (GetCOREInterface()->GetRefCoordSys() == COORDS_GIMBAL
		&& method == CTRL_RELATIVE
		&& IsGimbalAxis(aa->axis))
		gimbal_axis = true;

	Matrix3 before(1);
	Matrix3 localTM;
	float weight;
	BOOL average = FALSE;
	pblock->GetValue(kLayerCtrlAverage, 0, average, FOREVER);

	if (!average)
	{ // Addative method
		if (!gimbal_axis)
			for (int i=0; i<active; i++)
			{
				localTM = before;
				if (conts[i]) conts[i]->GetValue(t,&localTM,valid,CTRL_RELATIVE);
				localTM = localTM * Inverse(before);
				if(GetMLCMan()->GetNthMLC(mLCs[i]))
				{
				weight = GetMLCMan()->GetNthMLC(mLCs[i])->GetWeight(t,valid);
				Quat weightedRot = Slerp(IdentQuat(), Quat(localTM), min(max(weight, 0.0f), 1.0f));
				PreRotateMatrix(before, weightedRot);
			}
			}

		if (method==CTRL_ABSOLUTE)
		{
			Quat v = *((Quat*)val);
			Matrix3 after(1);
			for (int i=active+1; i<conts.Count(); i++) 
			{
				localTM = after;
				if (conts[i]) conts[i]->GetValue(t,&localTM,valid,CTRL_RELATIVE);
				localTM = localTM * Inverse(after);
				if(GetMLCMan()->GetNthMLC(mLCs[i]))
				{
				weight = GetMLCMan()->GetNthMLC(mLCs[i])->GetWeight(t,valid);
				Quat weightedRot = Slerp(IdentQuat(), Quat(localTM), min(max(weight, 0.0f), 1.0f));
				PreRotateMatrix(after, weightedRot);
			}
			}

			v = Inverse(Quat(after)) * v * Inverse(Quat(before));		
			if(conts[active])conts[active]->SetValue(t,&v,commit,method);		
		}
		else
		{
			AngAxis na = *aa;
			if (!(conts[active]->ClassID()== Class_ID(LOOKAT_CONSTRAINT_CLASS_ID,0))
				&& !gimbal_axis)
				na.axis = VectorTransform(Inverse(before),na.axis);
			if(conts[active])conts[active]->SetValue(t,&na,commit,method);
		}
	}
	else 
	{  // pose to pose blending
		Matrix3 initTM = before;
		Quat lastRot = IdentQuat();
		if (!gimbal_axis)
			for (int i=0; i<active; i++)
			{
				localTM = before;
				if (conts[i]) conts[i]->GetValue(t,&localTM,valid,CTRL_RELATIVE);
				localTM = localTM * Inverse(before);
				if(GetMLCMan()->GetNthMLC(mLCs[i]))
				{
				weight = GetMLCMan()->GetNthMLC(mLCs[i])->GetWeight(t,valid);
				Quat weightedRot = Slerp(lastRot, Quat(localTM), min(max(weight, 0.0f), 1.0f));
				lastRot = weightedRot;
				weightedRot = weightedRot * Quat(initTM);
				Point3 trans = before.GetTrans();
				before.SetRotate(weightedRot);
				before.SetTrans(trans);
			}
			}
		if (method==CTRL_ABSOLUTE)
		{
			Quat v = *((Quat*)val);
			Matrix3 after(1);
			initTM = after;
			lastRot = IdentQuat();
			for (int i=active+1; i<conts.Count(); i++)
			{
				localTM = after;
				if (conts[i]) conts[i]->GetValue(t,&localTM,valid,CTRL_RELATIVE);
				localTM = localTM * Inverse(after);
				if(GetMLCMan()->GetNthMLC(mLCs[i]))
				{
				weight = GetMLCMan()->GetNthMLC(mLCs[i])->GetWeight(t,valid);
				Quat weightedRot = Slerp(lastRot, Quat(localTM), min(max(weight, 0.0f), 1.0f));
				lastRot = weightedRot;
				weightedRot = weightedRot * Quat(initTM);
				Point3 trans = after.GetTrans();
				after.SetRotate(weightedRot);
				after.SetTrans(trans);
			}
			}
			
			v = Inverse(Quat(after)) * v * Inverse(Quat(before));		
			
			assert(active>=0);
			if(conts[active])conts[active]->SetValue(t,&v,commit,method);		
		}
		else
		{
			AngAxis na = *aa;
			if (!(conts[active]->ClassID()== Class_ID(LOOKAT_CONSTRAINT_CLASS_ID,0))
				&& !gimbal_axis)
				na.axis = VectorTransform(Inverse(before),na.axis);
			assert(active>=0);

			if(conts[active])conts[active]->SetValue(t,&na,commit,method);
		}
	}
}

LayerControl *RotationLayerControl::DerivedClone()
{
	return new RotationLayerControl;
}

//keeps track of whether an FP interface desc has been added to the ClassDesc
static bool rotLayerInterfaceLoaded = false;

class RotationLayerClassDesc : public ClassDesc2 {
	public:
	int 			IsPublic() {return 0;}
	const TCHAR *	ClassName() {return ROTLAYER_CNAME;}
    SClass_ID		SuperClassID() {return CTRL_ROTATION_CLASS_ID;}
	Class_ID		ClassID() {return (ROTLAYER_CONTROL_CLASS_ID);}
	const TCHAR* 	Category() {return _T("");}
	void *	Create(BOOL loading) { 		
		if (!rotLayerInterfaceLoaded)
		{
			AddInterface(&layerControlInterface);
			rotLayerInterfaceLoaded = true;
		}
		return new RotationLayerControl(loading);
	}
	
	const TCHAR*	InternalName() { return _T("RotationLayer"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }					// returns owning module handle
	};
static RotationLayerClassDesc rotLayerCD;
ClassDesc* GetRotationLayerDesc() {return &rotLayerCD;}

void RotationLayerControl::Init()
{
	// make the paramblock
	rotLayerCD.MakeAutoParamBlocks(this);
	for(int i = kX;i<= (kZ);++i)
	{
		CreateController(i);
	}
	okayToBlendEulers = FALSE;
}

// CAL-10/30/2002: Add individual ParamBlockDesc2 for each list controller - see point3_layer_paramblk.
// per instance list controller block
static ParamBlockDesc2 rot_layer_paramblk (0, _T("Parameters"),  0, &rotLayerCD, P_AUTO_CONSTRUCT, PBLOCK_INIT_REF_ID, 
	// params

	kLayerCtrlAverage, _T("average"), TYPE_BOOL, 0, IDS_AF_LIST_AVERAGE, 
		p_default, 		FALSE, 
		end, 

	kX, 	_T("exposedX"), TYPE_ANGLE, 	P_ANIMATABLE|P_TRANSIENT, 	IDS_RB_X, 
		end,
	kY, 	_T("exposedY"), 		TYPE_ANGLE, 	P_ANIMATABLE|P_TRANSIENT, 	IDS_RB_Y,  
		end,
	kZ, 	_T("exposedZ"), 		TYPE_ANGLE, 	P_ANIMATABLE|P_TRANSIENT, 	IDS_RB_Z, 
		end,
	kX_order,	_T("eulerXOrder"),	TYPE_INT, 0, IDS_XORDER, 
		p_default, 		1,	
		p_range, 		1, NUM_SUPPORTED_EULERS, 
		end,
	kY_order,	_T("eulerYOrder"),	TYPE_INT, 0, IDS_YORDER, 
		p_default, 		3,	
		p_range, 		1, NUM_SUPPORTED_EULERS, 
		end,
	kZ_order,	_T("eulerZOrder"),	TYPE_INT, 0, IDS_ZORDER, 
		p_default, 		5,	
		p_range, 		1, NUM_SUPPORTED_EULERS, 
		end,
	kBlendEulerAsQuat, _T("BlendEulerAsQuat"), TYPE_BOOL, 0, IDS_BLEND_EULER_AS_QUAT,
		p_default, FALSE,
		end,

	end
	);

//-------------------------------------------------------------------------
// Scale layer control
//		
		
class ScaleLayerControl : public LayerControl {
	public:
		ScaleLayerControl(BOOL loading) : LayerControl(loading) {Init();} 
		ScaleLayerControl() {Init();}

		void GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method=CTRL_ABSOLUTE);
		void GetOutputValue(TimeValue t, void *val, Interval &valid);

		void SetValue(TimeValue t, void *val, int commit, GetSetMethod method);
		LayerControl *DerivedClone();
		void Init();

		Class_ID ClassID() { return (SCALELAYER_CONTROL_CLASS_ID); }  
		SClass_ID SuperClassID() { return CTRL_SCALE_CLASS_ID; }  
		void GetClassName(TSTR& s) {s = SCALELAYER_CNAME;}
	};



void ScaleLayerControl::GetValue(
		TimeValue t, void *val, Interval &valid, GetSetMethod method)
{
	float weight;
	AffineParts parts;

	if (method==CTRL_ABSOLUTE)
	{
		ScaleValue totalScale(Point3(1,1,1));
		ScaleValue tempScale(Point3(0,0,0));

		int stop = conts.Count();
		if(GetMLCMan()->GetJustUpToActive()==true)
			stop = active +1;
		for (int i=0; i<conts.Count(); i++)
		{
			MasterLayerControl *mLC = GetMLCMan()->GetNthMLC(mLCs[i]);
			if(mLC==NULL||mLC->mute==true)
				continue;
			weight = mLC->GetWeight(t,valid);	
			if (weight != 0.0f) {
				// CAL-06/06/02: why do we need to zero the scale here? doesn't make sense to me??
				// if (i==0) totalScale = Point3(0,0,0);
				if (conts[i]) conts[i]->GetValue(t,&tempScale,valid,CTRL_ABSOLUTE);
				totalScale = totalScale + tempScale*AverageWeight(weight,t,valid);
			}
		}
		(*(ScaleValue*)val) = totalScale;
	}
	else
	{
		Matrix3* tm = (Matrix3*)val;
		decomp_affine(*tm,&parts);
		Point3 tempVal = parts.k;
		for (int i=0; i<conts.Count(); i++)
		{
			MasterLayerControl *mLC = GetMLCMan()->GetNthMLC(mLCs[i]);
			if(mLC==NULL||mLC->mute==true)
				continue;
			weight = mLC->GetWeight(t,valid);	
			if (weight != 0.0f && conts[i]) {
				conts[i]->GetValue(t,val,valid,CTRL_RELATIVE);
				decomp_affine(*tm,&parts);
				tempVal = tempVal + (parts.k - tempVal)*AverageWeight(weight,t,valid);
				// CAL-06/06/02: Use comp_affine to reconstruct the matrix better.
				//		NOTE: SetRotate() will erase the scale set by SetScale().
				parts.k = tempVal;
				comp_affine(parts, *tm);
				// Quat rot = Quat(*tm);
				// Point3 trans =tm->GetTrans();
				// tm->SetScale(tempVal);
				// tm->SetRotate(rot);
				// tm->SetTrans(trans);
			}
		}
	}
}


void ScaleLayerControl::GetOutputValue(
		TimeValue t, void *val, Interval &valid)
{
	float weight;

	ScaleValue totalScale(Point3(1,1,1));
	ScaleValue tempScale(Point3(0,0,0));

	int stop = conts.Count();
	if(GetMLCMan()->GetJustUpToActive()==true)
		stop = active +1;
	for (int i=0; i<conts.Count(); i++)
	{
		MasterLayerControl *mLC = GetMLCMan()->GetNthMLC(mLCs[i]);
		if(mLC==NULL||mLC->outputMute==true)
			continue;
		weight = mLC->GetWeight(t,valid);	
		if (weight != 0.0f) {
			// CAL-06/06/02: why do we need to zero the scale here? doesn't make sense to me??
			// if (i==0) totalScale = Point3(0,0,0);
			if (conts[i]) conts[i]->GetValue(t,&tempScale,valid,CTRL_ABSOLUTE);
			totalScale = totalScale + tempScale*AverageWeight(weight,t,valid);
		}
	}
	(*(ScaleValue*)val) = totalScale;
	
}


void ScaleLayerControl::SetValue(
		TimeValue t, void *val, int commit, GetSetMethod method)
{
	if (!conts.Count()) return;	

	if (method==CTRL_ABSOLUTE)
	{
		ScaleValue v = *((Point3*)val);
		Matrix3 before(1), after(1);
		Interval valid = FOREVER;
		int i;
		for (i=0; i<active; i++)
		{
			if (conts[i]) conts[i]->GetValue(t,&before,valid,CTRL_RELATIVE);
		}
		for (i=active+1; i<conts.Count(); i++)
		{
			if (conts[i]) conts[i]->GetValue(t,&after,valid,CTRL_RELATIVE);
		}
		
		AffineParts bparts, aparts;
		decomp_affine(Inverse(before),&bparts);
		decomp_affine(Inverse(after),&aparts);
		v.q = Inverse(bparts.u) * v.q * Inverse(aparts.u);
		// CAL-06/06/02: scale should be computed by scaling (not subtracting) out others scales.
		//		NOTE: probably need to check divide-by-zero exception.
		// v.s = -bparts.k + v.s + -aparts.k;
		DbgAssert((bparts.k * aparts.k) != Point3::Origin);
		v.s = v.s / (bparts.k * aparts.k);
		assert(active>=0);
		conts[active]->SetValue(t,&v,commit,method);
	} 
	else
	{
		assert(active>=0);
		conts[active]->SetValue(t,val,commit,method);
	}
}

LayerControl *ScaleLayerControl::DerivedClone()
{
	return new ScaleLayerControl;
}


//keeps track of whether an FP interface desc has been added to the ClassDesc
static bool scaleLayerInterfaceLoaded = false;

class ScaleLayerClassDesc : public ClassDesc2 {
	public:
	int 			IsPublic() {return 0;}
	const TCHAR *	ClassName() {return SCALELAYER_CNAME;}
    SClass_ID		SuperClassID() {return CTRL_SCALE_CLASS_ID;}
	Class_ID		ClassID() {return (SCALELAYER_CONTROL_CLASS_ID);}
	const TCHAR* 	Category() {return _T("");}
	void *	Create(BOOL loading) 
	{ 		
		if (!scaleLayerInterfaceLoaded)
		{
			AddInterface(&layerControlInterface);
			scaleLayerInterfaceLoaded = true;
		}
		return new ScaleLayerControl(loading);
	}
	
	const TCHAR*	InternalName() { return _T("ScaleLayer"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }					// returns owning module handle
};
static ScaleLayerClassDesc scaleLayerCD;
ClassDesc* GetScaleLayerDesc() {return &scaleLayerCD;}

void ScaleLayerControl::Init()
{
	// make the paramblock
	scaleLayerCD.MakeAutoParamBlocks(this);
	for(int i = kX;i<= (kZ);++i)
	{
		CreateController(i);
	}
}

static ParamBlockDesc2 scale_layer_paramblk (0, _T("Parameters"),  0, &scaleLayerCD, P_AUTO_CONSTRUCT, PBLOCK_INIT_REF_ID, 
	// params
	kLayerCtrlAverage, _T("average"), TYPE_BOOL, 0, IDS_AF_LIST_AVERAGE, 
		p_default, 		FALSE, 
		end, 

	kX, 	_T("exposedX"), TYPE_FLOAT, 	P_ANIMATABLE|P_TRANSIENT, 	IDS_RB_X, 
		end,
	kY, 	_T("exposedY"), 		TYPE_FLOAT, 	P_ANIMATABLE|P_TRANSIENT, 	IDS_RB_Y,  
		end,
	kZ, 	_T("exposedZ"), 		TYPE_FLOAT, 	P_ANIMATABLE|P_TRANSIENT, 	IDS_RB_Z, 
		end,

	end
);



/**********************************************************

	MasterLayerControl

************************************************************/
class MasterLayerClassDesc : public ClassDesc2
{
	public:
	int 			IsPublic() {return 0;}
	const TCHAR *	ClassName() {return MASTERLAYER_CNAME;}
    SClass_ID		SuperClassID() {return CTRL_USERTYPE_CLASS_ID;}
	Class_ID		ClassID() {return MASTERLAYER_CONTROL_CLASS_ID;}
	const TCHAR* 	Category() {return _T("");}
	void *	Create(BOOL loading) 
	{ 	
		return new MasterLayerControl(loading);
	}
	
	const TCHAR*	InternalName() { return _T("MasterLayer"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }					// returns owning module handle
};

static MasterLayerClassDesc masterLayerCD;
ClassDesc* GetMasterLayerDesc() {return &masterLayerCD;}

void MasterLayerControl::Init()
{
	// make the paramblock
	masterLayerCD.MakeAutoParamBlocks(this);
}


// per instance list controller block
static ParamBlockDesc2 master_layer_paramblk (0, _T("Parameters"),  0, &masterLayerCD, P_AUTO_CONSTRUCT, PBLOCK_INIT_REF_ID, 
	// params
		kName,				_T("name"), TYPE_STRING,	0,	IDS_MZ_NAME,
		end,
	kLayerCtrlWeight, _T("weight"), TYPE_FLOAT,P_TV_SHOW_ALL+ P_ANIMATABLE,  IDS_AF_LIST_WEIGHT, 
		p_default, 		1.0f, 
		p_range, 		-10000.0f, 10000.0f, 
		p_dim,			stdPercentDim,
		//p_accessor,		&theLayerCtrlPBAccessor,
		end,
		
	end
);

MasterLayerControl::MasterLayerControl(BOOL loading)
{
	pblock = NULL;
	Init();
	paramFlags = 0;
	addDefault = true;
	mute = false;
	outputMute = false;

	pblock->SetController((ParamID)kLayerCtrlWeight,0, CreateInterpFloat());
}

MasterLayerControl::MasterLayerControl(const MasterLayerControl& ctrl)
{
	pblock = NULL;
	DeleteAllRefsFromMe();
	ReplaceReference(0,ctrl.pblock);
	Resize(ctrl.layers.Count());
	monitorNodes.SetCount(ctrl.monitorNodes.Count());
	int i=0;
	for (i=0; i<ctrl.monitorNodes.Count(); i++)
		monitorNodes[i] = NULL;
	for (i=0; i<ctrl.layers.Count(); i++) {		
		ReplaceReference(i+1,ctrl.layers[i]);
		}
	for (i=0; i<ctrl.monitorNodes.Count(); i++) {		
		ReplaceReference(i+1+ctrl.layers.Count(),ctrl.monitorNodes[i]);
		}
	
	paramFlags = 0;
	addDefault = ctrl.addDefault;
	mute = ctrl.mute;
	outputMute = ctrl.mute;

}

RefResult MasterLayerControl::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID,  RefMessage message) 
{
	
	switch (message) 
	{
	case REFMSG_WANT_SHOWPARAMLEVEL:// show the paramblock2 entry in trackview
			{
			BOOL *pb = (BOOL *)(partID);
			*pb = TRUE;
			return REF_STOP;
			}
	case REFMSG_KEY_SELECTION_CHANGED:
	case REFMSG_CHANGE:
	case REFMSG_RANGE_CHANGE:
	case REFMSG_TARGET_SELECTIONCHANGE:
	case REFMSG_BECOMING_ANIMATED:
	case REFMSG_NODE_LINK:
	case REFMSG_TARGET_DELETED:
	case REFMSG_SUBANIM_STRUCTURE_CHANGED:
	case REFMSG_MODIFIER_ADDED:
	case REFMSG_OBREF_CHANGE:
	case REFMSG_CONTROLREF_CHANGE:
	//since the MasterLayerControl owns the weight control (that's in our pblock) as a subanim that shows up in trackbar, etc.. we need
	//to propragate messages to dependents so such things as redrawing of the trackbar happens correctly
			//Defect #   MZ 797495
			//note that we modified this some more for  868689.  We would proprage REFMSG_CHANGE instead of just the message for any message we recieved which was wrong.
			//to fix 797495 it looks like we just needed to do this propragation for just these messages which are used by trackbar.
	if(pblock && (hTarget==pblock))
	{
		for(int i=0;i<layers.Count();++i)
		{
			LayerControl *lC = layers[i];
			if(lC)
			{
						lC->NotifyDependents(FOREVER,0,message);
			}
		}
			}
			if(message==REFMSG_CHANGE)
			master_layer_paramblk.InvalidateUI();
		break;
	}
	return REF_SUCCEED;
}



RefTargetHandle MasterLayerControl::GetReference(int i)
{
	if (i==0) return pblock;	//pblock
	else if (i<=layers.Count()) return layers[i-1];	//all of the layers that look back at this guy
	else if ( i<=(layers.Count()+monitorNodes.Count())) return monitorNodes[i-(layers.Count()+1)];
	return NULL;
}

int MasterLayerControl::NumSubs()
{
	return 1;
}


Animatable* MasterLayerControl::SubAnim(int i) 	
{
    if (i==0) return pblock; else return NULL;
}

TSTR MasterLayerControl::SubAnimName(int i) 
{
	switch (i) 
	{
		case 0: return GetName();
		default: return _T("");
	}
}

int MasterLayerControl::SubNumToRefNum(int subNum)
{
	if (subNum==0) return subNum;
	return -1;
}

void MasterLayerControl::SetReference(int i, RefTargetHandle rtarg)
{
	if ((i == 0)||  (rtarg && rtarg->ClassID() == Class_ID(PARAMETER_BLOCK2_CLASS_ID,0)))
	{
		//set the parameter block
		pblock = (IParamBlock2*)rtarg;
		return;
	}
	else if(i<=layers.Count())
	{
		layers[i-1] = static_cast<LayerControl*>(rtarg);	   
	}
	else if ( i<=(layers.Count()+monitorNodes.Count()))
	{
		monitorNodes[i-(layers.Count()+1)] = rtarg;;
	}
	return;
}

IOResult MasterLayerControl::Save(ISave *isave)
{
	ULONG nb;
	int count = layers.Count();
	isave->BeginChunk(LAYERCOUNT_CHUNK);
	isave->Write(&count,sizeof(int),&nb);			
	isave->EndChunk();

	int iAddDefault = (addDefault==true)? 1 :0; 
	isave->BeginChunk(ADDDEFAULT_CHUNK);
	isave->Write(&iAddDefault,sizeof(int),&nb);			
	isave->EndChunk();

	int iMute = (mute==true)? 1 :0; 
	isave->BeginChunk(MUTE_CHUNK);
	isave->Write(&iMute,sizeof(int),&nb);			
	isave->EndChunk();

	count = monitorNodes.Count();
	isave->BeginChunk( MONITORCOUNT_CHUNK);
	isave->Write(&count,sizeof(int),&nb);			
	isave->EndChunk();

	iMute = (outputMute==true)? 1 :0; 
	isave->BeginChunk(OUTPUTMUTE_CHUNK);
	isave->Write(&iMute,sizeof(int),&nb);			
	isave->EndChunk();

	return IO_OK;
}


IOResult MasterLayerControl::Load(ILoad *iload)
{
	ULONG nb;
	IOResult res = IO_OK;
	int ix=0;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch (iload->CurChunkID()) {
			case LAYERCOUNT_CHUNK: {
				int layerCount;
				res = iload->Read(&layerCount,sizeof(layerCount),&nb);
				Resize(layerCount);
				break;
				}
			case MONITORCOUNT_CHUNK: {
				int monitorCount;
				res = iload->Read(&monitorCount,sizeof(monitorCount),&nb);
				if(monitorCount>0)
				{
					monitorNodes.SetCount(monitorCount);
					for(int i =0;i<monitorCount;++i)
						monitorNodes[i]= NULL;
				}
				else
					monitorNodes.ZeroCount();
				break;
			}
			case ADDDEFAULT_CHUNK: {
				int iAddDefault;
				res = iload->Read(&iAddDefault,sizeof(iAddDefault),&nb);
				addDefault = (iAddDefault==1) ? true : false;
				break;
				}
			case MUTE_CHUNK: {
				int iMute;
				res = iload->Read(&iMute,sizeof(iMute),&nb);
				mute = (iMute==1) ? true : false;
				break;
				}
			case OUTPUTMUTE_CHUNK: {
				int iMute;
				res = iload->Read(&iMute,sizeof(iMute),&nb);
				outputMute = (iMute==1) ? true : false;
				break;
				}
			}
		iload->CloseChunk();
		if (res!=IO_OK)  return res;
		}
	return IO_OK;
}

Control *MasterLayerControl::GetDefaultControl(SClass_ID sclassID)
{
	Control *c = (Control*)GetDefaultController(sclassID)->Create();
	return static_cast<Control*>(c);
}





TSTR MasterLayerControl::GetName()
{
    TCHAR* n = NULL;
	pblock->GetValue(kName, 0, n, FOREVER);
    return TSTR(n);
}

void MasterLayerControl::SetName(TSTR &name)
{
    pblock->SetValue(kName,0,name.data());
	for(int j=0;j<layers.Count();++j)
	{
		LayerControl *lC = layers[j];
		if(lC)
		{
			lC->NotifyDependents(FOREVER,0,REFMSG_CHANGE);
			lC->NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);
		}
	}
}

void MasterLayerControl::NotifyLayers()
{
	for(int j=0;j<layers.Count();++j)
	{
		LayerControl *lC = layers[j];
		if(lC)
		{
			lC->NotifyDependents(FOREVER,0,REFMSG_CHANGE);
			lC->NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);
		}
	}
}

float MasterLayerControl::GetWeight(TimeValue t, Interval &valid)
{
	float val;
	pblock->GetValue(kLayerCtrlWeight, t, val, valid);
	return val;
}

void MasterLayerControl::SetWeight(TimeValue t, float weight)
{
	pblock->SetValue(kLayerCtrlWeight,t,weight);
	for(int j=0;j<layers.Count();++j)
	{
		LayerControl *lC = layers[j];
		if(lC)
		{
			lC->NotifyDependents(FOREVER,0,REFMSG_CHANGE);
		}
	}

}
Control *MasterLayerControl::GetWeightControl()
{
	return pblock->GetController((ParamID)kLayerCtrlWeight);

}


void MasterLayerControl::SetWeightControl(Control *weight)
{
	if(weight)
		pblock->SetController((ParamID)kLayerCtrlWeight,0, weight);
	for(int j=0;j<layers.Count();++j)
	{
		LayerControl *lC = layers[j];
		if(lC)
		{
			lC->NotifyDependents(FOREVER,0,REFMSG_CHANGE);
			lC->NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);
		}
	}
}

void MasterLayerControl::AppendLayerControl(LayerControl *lC)
{
	Resize(layers.Count()+1);
	ReplaceReference(layers.Count(),static_cast<RefTargetHandle>(lC));
}

MasterLayerControl::~MasterLayerControl()
{
	DeleteAllRefs();
}

void MasterLayerControl::Resize(int numLayers)
{
	int pNumLayers = layers.Count();
	layers.SetCount(numLayers);
	pblock->EnableNotifications(FALSE);
	for (int i=pNumLayers; i<numLayers; i++)
	{
		layers[i] = NULL;
	}
	pblock->EnableNotifications(TRUE);			// CAL-10/18/2002: enable it here
}




MasterLayerControl& MasterLayerControl::operator=(const MasterLayerControl& ctrl)
{	
	int i;
	RemapDir *remap = NewRemapDir();
	ReplaceReference(0, remap->CloneRef(ctrl.pblock));
	for (i=0; i<ctrl.layers.Count(); i++)
	{
		ReplaceReference(i+1,remap->CloneRef(ctrl.layers[i]));
	}
	remap->Backpatch();
	remap->DeleteThis();

	return *this;
}



void MasterLayerControl::NotifyForeground(TimeValue t)
{
	for(int i=0;i<layers.Count();++i)
		if(layers[i])
			layers[i]->NotifyForeground(t);
}



Control * MasterLayerControlManager::GetKeyableController(Control *control)
{
	if(controllerType== IAnimLayerControlManager::eBezier)
	{
		if(control->SuperClassID()==CTRL_ROTATION_CLASS_ID)
		{
			return static_cast<Control*>(GetCOREInterface()->CreateInstance(
						CTRL_ROTATION_CLASS_ID, Class_ID(EULER_CONTROL_CLASS_ID,0)));

		}
		else if(control->SuperClassID()==CTRL_POSITION_CLASS_ID)
		{
			return static_cast<Control*>(GetCOREInterface()->CreateInstance(
						CTRL_POSITION_CLASS_ID, IPOS_CONTROL_CLASS_ID));

		}
		else if(control->SuperClassID()==CTRL_SCALE_CLASS_ID)
		{
			return static_cast<Control*>(GetCOREInterface()->CreateInstance(
						CTRL_SCALE_CLASS_ID, ISCALE_CONTROL_CLASS_ID));
		}
		else if ((control->SuperClassID() == CTRL_FLOAT_CLASS_ID) )
		{
			return CreateInterpFloat();
		}
		else if((control->SuperClassID() ==CTRL_POINT4_CLASS_ID))
		{
			return CreateInterpPoint4();
		}
		else if ((control->SuperClassID() == CTRL_POINT3_CLASS_ID) )
		{
			return CreateInterpPoint3();
		}
	}
	else if(controllerType== IAnimLayerControlManager::eLinear)
	{


		if(control->SuperClassID()==CTRL_ROTATION_CLASS_ID)
		{
			return static_cast<Control*>(GetCOREInterface()->CreateInstance(
						CTRL_ROTATION_CLASS_ID, Class_ID(LININTERP_ROTATION_CLASS_ID,0)));

		}
		else if(control->SuperClassID()==CTRL_POSITION_CLASS_ID)
		{
			return static_cast<Control*>(GetCOREInterface()->CreateInstance(
						CTRL_POSITION_CLASS_ID, Class_ID(LININTERP_POSITION_CLASS_ID,0)));

		}
		else if(control->SuperClassID()==CTRL_SCALE_CLASS_ID)
		{
			return static_cast<Control*>(GetCOREInterface()->CreateInstance(
						CTRL_SCALE_CLASS_ID, Class_ID(LININTERP_SCALE_CLASS_ID,0)));
		}
		else if ((control->SuperClassID() == CTRL_FLOAT_CLASS_ID) )
		{
			return static_cast<Control*>(GetCOREInterface()->CreateInstance(
						CTRL_FLOAT_CLASS_ID,  Class_ID(LININTERP_FLOAT_CLASS_ID,0)));
			
		}
		else if((control->SuperClassID() ==CTRL_POINT4_CLASS_ID))
		{
			return static_cast<Control*>(GetCOREInterface()->CreateInstance(
						CTRL_POINT4_CLASS_ID, Class_ID(TCBINTERP_POINT3_CLASS_ID,0)));
		}
		else if ((control->SuperClassID() == CTRL_POINT3_CLASS_ID) )
		{
			return static_cast<Control*>(GetCOREInterface()->CreateInstance(
						CTRL_POINT3_CLASS_ID, Class_ID(TCBINTERP_POINT4_CLASS_ID,0)));
		}
	}

	//in case we fall down here.. just do this
	if ((control->SuperClassID() == CTRL_FLOAT_CLASS_ID) )
	{
		return NewDefaultFloatController();
	}
	else if ((control->SuperClassID() == CTRL_POSITION_CLASS_ID) )
	{
		return NewDefaultPositionController();
	}
	else if ((control->SuperClassID() == CTRL_POINT3_CLASS_ID) )
	{
		return NewDefaultPoint3Controller();
	}
	else if ((control->SuperClassID() == CTRL_ROTATION_CLASS_ID) )
	{
		return NewDefaultRotationController();
	}
	else if ((control->SuperClassID() == CTRL_SCALE_CLASS_ID) )
	{
		return NewDefaultScaleController();
	}
	else if((control->SuperClassID() ==CTRL_POINT4_CLASS_ID))
	{
		return NewDefaultPoint4Controller();
	}

	return NULL;
}

void MasterLayerControlManager::UpdateLayerControls()
{

	for(int i=0;i<GetNumMLCs();++i)
	{
		MasterLayerControl *mLC = GetNthMLC(i);
		if(mLC)
		{
			for(int j=0;j<mLC->layers.Count();++j)
			{
				LayerControl *lC = mLC->layers[j];
				if(lC)
				{
					lC->NotifyDependents(FOREVER,0,REFMSG_CHANGE);
					lC->NotifyDependents(FOREVER,0,REFMSG_CONTROLREF_CHANGE,TREE_VIEW_CLASS_ID,FALSE);	//we really only want to send this to the treeview, not others (like the mixer).

				}
			}
		}
	}
}

void MasterLayerControlManager::SetFilterActiveOnlyTrackView(bool val)
{
	if(val!=filterActive)
	{
		filterActive = val;
		UpdateLayerControls();
	}
}
bool MasterLayerControlManager::GetFilterActiveOnlyTrackView()
{
	return filterActive;
}

void MasterLayerControlManager::SetJustUpToActive(bool v)
{
	if(v!=justUpToActive)
	{
		justUpToActive = v;
	/*	getCfgMgr().setSection(_T("AnimationLayers"));
		if(v==true)
			getCfgMgr().putInt(_T("justUpToActive"), 1);
		else
			getCfgMgr().putInt(_T("justUpToActive"), 0);
			*/
		UpdateLayerControls();
	}
}

bool MasterLayerControlManager::GetJustUpToActive()
{
	return justUpToActive;

}




void MasterLayerControlManager::SetCollapseControllerType(IAnimLayerControlManager::ControllerType type)
{
	controllerType = type;
/*	getCfgMgr().setSection(_T("AnimationLayers"));
	if(type==IAnimLayerControlManager::eBezier)
		getCfgMgr().putInt(_T("controllerType"), 0);
	else if(type==IAnimLayerControlManager::eLinear)
		getCfgMgr().putInt(_T("controllerType"), 1);
	else if(type==IAnimLayerControlManager::eDefault)
		getCfgMgr().putInt(_T("controllerType"), 2);
*/

}
IAnimLayerControlManager::ControllerType MasterLayerControlManager::GetCollapseControllerType()
{
	return controllerType;
}
void MasterLayerControlManager::SetCollapsePerFrame(bool v)
{
	collapseAllPerFrame = v;
/*	getCfgMgr().setSection(_T("AnimationLayers"));
	if(v==true)
		getCfgMgr().putInt(_T("collapseAllPerFrame"), 1);
	else
		getCfgMgr().putInt(_T("collapseAllPerFrame"), 0);
*/

}
bool MasterLayerControlManager::GetCollapsePerFrame()
{
	return 	collapseAllPerFrame;
}
void MasterLayerControlManager::SetCollapsePerFrameActiveRange(bool v)
{

	collapsePerFrameActiveRange = v;
/*	getCfgMgr().setSection(_T("AnimationLayers"));
	if(v==true)
		getCfgMgr().putInt(_T("collapsePerFrameActiveRange"), 1);
	else
		getCfgMgr().putInt(_T("collapsePerFrameActiveRange"), 0);
*/

}
bool MasterLayerControlManager::GetCollapsePerFrameActiveRange()
{
	return collapsePerFrameActiveRange;
}
void MasterLayerControlManager::SetCollapseRange(Interval range)
{
	collapseRange =range;
/*	getCfgMgr().setSection(_T("AnimationLayers"));
	getCfgMgr().putInt(_T("rangeStart"), collapseRange.Start());
	getCfgMgr().putInt(_T("rangeEnd"), collapseRange.End());
*/
}
Interval MasterLayerControlManager::GetCollapseRange()
{
	return collapseRange;
}



TSTR  MasterLayerControlManager::GetLayerName(int index)
{
	MasterLayerControl *mLC = GetNthMLC(index);
	TSTR name;
	if(mLC)
	{
		name = mLC->GetName();
	}
	return name;
}


void MasterLayerControlManager::SetLayerName(int index, TSTR name)
{
	if(index==0)
		return;
	MasterLayerControl *mLC = GetNthMLC(index);
	if(mLC)
	{
		mLC->SetName(name);
	}
	NotifySelectionChanged();

}

bool  MasterLayerControlManager::GetLayerMute(int index)
{
	MasterLayerControl *mLC = GetNthMLC(index);
	bool mute = false;
	if(mLC)
	{
		mute = mLC->mute;
	}
	return mute;
}


void MasterLayerControlManager::SetLayerMute(int index, bool mute)
{
	MasterLayerControl *mLC = GetNthMLC(index);
	if(mLC)
	{
		mLC->mute = mute;
		mLC->NotifyLayers();
	}
	NotifySelectionChanged();

}


bool  MasterLayerControlManager::GetLayerOutputMute(int index)
{
	MasterLayerControl *mLC = GetNthMLC(index);
	bool mute = false;
	if(mLC)
	{
		mute = mLC->outputMute;
	}

	return mute;
}


void MasterLayerControlManager::SetLayerOutputMute(int index, bool mute)
{
	MasterLayerControl *mLC = GetNthMLC(index);
	if(mLC)
	{
		mLC->outputMute = mute;
	}
	NotifySelectionChanged();

}


float  MasterLayerControlManager::GetLayerWeight(int index,TimeValue t)
{
	MasterLayerControl *mLC = GetNthMLC(index);
	float weight = 0.0f;
	Interval valid = FOREVER;		
	if(mLC)
	{
		weight= mLC->GetWeight(t,valid);
	}

	return weight;
}



void  MasterLayerControlManager::SetLayerWeight(int index, TimeValue t, float weight)
{
	MasterLayerControl *mLC = GetNthMLC(index);
	if(mLC)
	{
		mLC->SetWeight(t,weight);
	}
	NotifySelectionChanged();

}

Control*  MasterLayerControlManager::GetLayerWeightControl(int index)
{
	MasterLayerControl *mLC = GetNthMLC(index);
	Control *c = NULL;
	if(mLC)
	{
		c = mLC->GetWeightControl();
	}
	return c;
}



bool  MasterLayerControlManager::SetLayerWeightControl(int index, Control *c)
{
	MasterLayerControl *mLC = GetNthMLC(index);
	if(mLC&&c&&c->SuperClassID()==CTRL_FLOAT_CLASS_ID)
	{
		mLC->SetWeightControl(c);
		NotifySelectionChanged();
		return true;
	}
	NotifySelectionChanged();
	return false;
}




void MasterLayerControlManager::DisableLayerNodes(Tab<INode *> &nodeTab)
{

	if(theHold.Holding())
		theHold.Put(new NotifySelectionRestore());
	Tab<MasterLayerControlManager::LCInfo> existingLayers;
	GetLayerControls(nodeTab, existingLayers);
	for(int i=0;i<existingLayers.Count();++i)
	{
		LayerControl *lC2 = existingLayers[i].lC;
		lC2->DisableLayer(existingLayers[i].client, existingLayers[i].subNum);
		for(int f =0;f< lC2->mLCs.Count();++f)
		{
			MasterLayerControl *mLC = GetMLCMan()->GetNthMLC(lC2->mLCs[f]);
			if(mLC)
			{
				int j;
				for(j=mLC->monitorNodes.Count()-1;j>-1;--j)
				{
					if(mLC->monitorNodes[j])
					{
						INodeMonitor *nm = static_cast<INodeMonitor *>(mLC->monitorNodes[j]->GetInterface(IID_NODEMONITOR));
						if(nm->GetNode()==existingLayers[i].node)
							break;
					}
				}
				if(j>-1)
				{
					MasterLayerControlRestore *mLCRestore = new MasterLayerControlRestore(mLC);
					mLC->DeleteReference(1 +mLC->layers.Count()+j);
					mLC->monitorNodes.Delete(j,1);
					if(theHold.Holding())
					{
						theHold.Put(mLCRestore);
					}
					else
					{
						delete mLCRestore;
						mLCRestore = NULL;
					}
				}
			}
		}
	}
	if(theHold.Holding())
		theHold.Put(new NotifySelectionRestore());
	NotifySelectionChanged();
}

void MasterLayerControl::AddNode(INode *node) //add nodes to the monitor list.
{
	int j;
	for(j=0;j<monitorNodes.Count();++j)
	{

		INodeMonitor *nm = static_cast<INodeMonitor *>(monitorNodes[j]->GetInterface(IID_NODEMONITOR));
		if(nm->GetNode()==node)
			break;
	}
	if(j==monitorNodes.Count())
	{
		ReferenceTarget* rnm = (ReferenceTarget*)CreateInstance(REF_TARGET_CLASS_ID,NODEMONITOR_CLASS_ID);
		DbgAssert (rnm);
		if (rnm)
		{
			INodeMonitor *nm = (INodeMonitor*)rnm->GetInterface(IID_NODEMONITOR);
			nm->SetNode(node);
		}
		ReferenceTarget* null_ptr = NULL;;
		monitorNodes.Append(1,&null_ptr);
		ReplaceReference(layers.Count() + monitorNodes.Count(), rnm);
	}
}




////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//				UIActions
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef NO_ANIM_LAYERS
// We do not want to expose Animation Layers UI in Viz

#define ID_ENABLE_ANIM_LAYERS  1
#define ID_ANIM_LAYER_PROPERTIES 2
#define ID_ADD_ANIM_LAYER  3
#define ID_DELETE_ANIM_LAYER  4
#define ID_COPY_ANIM_LAYER 5
#define ID_PASTE_ACTIVE_ANIM_LAYER  6
#define ID_PASTE_NEW_LAYER  7
#define ID_COLLAPSE_ANIM_LAYER  8
#define ID_DISABLE_ANIM_LAYER  9
#define ID_SELECT_NODES_FROM_ACTIVE 10

class MasterLayerControlManagerActionCallback : public ActionCallback
{
public:
		MasterLayerControlManagerActionCallback(){}
		BOOL	KeyboardShortcut(int id); 
		BOOL	ExecuteAction(int id) { return KeyboardShortcut(id); }
};


BOOL MasterLayerControlManagerActionCallback::KeyboardShortcut(int cmd)
{
	if(GetCOREInterface()->GetSelNodeCount()<1)
		return FALSE;
	IAnimLayerControlManager *manager =static_cast<IAnimLayerControlManager*>(GetCOREInterface(IANIMLAYERCONTROLMANAGER_INTERFACE ));

	MasterLayerControlManager *man = GetMLCMan();
	man->NotifySelectionChanged();

	Tab<INode *> nodeTab;
	GetSelectedNodes(nodeTab);

	switch(cmd)
	{
		case ID_ENABLE_ANIM_LAYERS:
			theHold.Begin();
			manager->EnableAnimLayersDlg(nodeTab);
			theHold.Accept(GetString(IDS_ENABLE_ANIM_LAYERS));
			return TRUE;			
		case ID_ANIM_LAYER_PROPERTIES:
			
			manager->AnimLayerPropertiesDlg(); 
			
			return TRUE;
		case ID_ADD_ANIM_LAYER:
			
				theHold.SuperBegin();
				theHold.Begin();
				manager->AddLayerDlg(nodeTab); 
				theHold.Accept(GetString(IDS_ADD_ANIM_LAYER));
				theHold.SuperAccept(GetString(IDS_ADD_ANIM_LAYER));
			
			return TRUE;
		
		case ID_DELETE_ANIM_LAYER: 
			if(man->activeLayers.Count()==1)
			{
				if(man->ContinueDelete())
				{
					theHold.SuperBegin();
					theHold.Begin();
					manager->DeleteLayerNodes(man->activeLayers[0],nodeTab);
					theHold.Accept(GetString(IDS_DELETE_ANIM_LAYER));
					theHold.SuperAccept(GetString(IDS_DELETE_ANIM_LAYER));			
				}
			}
	
			return TRUE;
		case ID_COPY_ANIM_LAYER:
			if(man->activeLayers.Count()==1)
			{
				man->CopyLayerNodes(man->activeLayers[0],nodeTab);
			} 
			return TRUE;
		case ID_COLLAPSE_ANIM_LAYER: 
			if(man->activeLayers.Count()==1)
			{
				theHold.SuperBegin();
				theHold.Begin();
				man->CollapseLayerNodes(man->activeLayers[0],nodeTab);
				theHold.Accept(GetString(IDS_COLLAPSE_ANIM_LAYER));
				theHold.SuperAccept(GetString(IDS_COLLAPSE_ANIM_LAYER));
			}
			return TRUE;

		case ID_PASTE_ACTIVE_ANIM_LAYER:
			if(man->activeLayers.Count()==1&&man->copyLayers)
			{
				theHold.SuperBegin();
				theHold.Begin();
				manager->PasteLayerNodes(man->activeLayers[0],nodeTab);
				theHold.Accept(IDS_PASTE_NEW_LAYER);
				theHold.SuperAccept(IDS_PASTE_NEW_LAYER);
			}
			return TRUE;
		case ID_PASTE_NEW_LAYER: 
			if(man->copyLayers)
			{
				theHold.SuperBegin();
				theHold.Begin();
				manager->PasteLayerNodes(-1,nodeTab);
				theHold.Accept(IDS_PASTE_NEW_LAYER);
				theHold.SuperAccept(IDS_PASTE_NEW_LAYER);
			}
			return TRUE;
		case ID_DISABLE_ANIM_LAYER:
			if(man->canDisable)
			{
				theHold.SuperBegin();
				theHold.Begin();
				manager->DisableLayerNodes(nodeTab);
				theHold.Accept(GetString(IDS_DISABLE_ANIM_LAYER));
				theHold.SuperAccept(GetString(IDS_DISABLE_ANIM_LAYER));

			}
			return TRUE;
		case ID_SELECT_NODES_FROM_ACTIVE: 
			if(man->activeLayers.Count()==1)
			{
				man->SelectNodesFromActive();
			}
			return TRUE;
	}

	return FALSE;
}

class MasterLayerControlManagerActionTable : public ActionTable
{
	public:
		MasterLayerControlManagerActionTable(ActionTableId id, ActionContextId contextId, TSTR& name, HACCEL hDefaults, int numIds, ActionDescription* pOps, HINSTANCE hInst);
		~MasterLayerControlManagerActionTable();
		BOOL IsChecked(int cmdId);
		BOOL IsEnabled(int cmdId);
		MaxIcon* GetIcon(int cmdId);
	private:
		static MaxIcon* mi1;
		static MaxIcon* mi2;
		static MaxIcon* mi3;
		static MaxIcon* mi4;
		static MaxIcon* mi5;
		static MaxIcon* mi6;
		static MaxIcon* mi7;
		static MaxIcon* mi8;
		static MaxIcon* mi9;
		static MaxIcon* mi10;
};
;
MaxIcon* MasterLayerControlManagerActionTable::mi1 = NULL;
MaxIcon* MasterLayerControlManagerActionTable::mi2 = NULL;
MaxIcon* MasterLayerControlManagerActionTable::mi3 = NULL;
MaxIcon* MasterLayerControlManagerActionTable::mi4 = NULL;
MaxIcon* MasterLayerControlManagerActionTable::mi5 = NULL;
MaxIcon* MasterLayerControlManagerActionTable::mi6 = NULL;
MaxIcon* MasterLayerControlManagerActionTable::mi7 = NULL;
MaxIcon* MasterLayerControlManagerActionTable::mi8 = NULL;
MaxIcon* MasterLayerControlManagerActionTable::mi9 = NULL;
MaxIcon* MasterLayerControlManagerActionTable::mi10 = NULL;

// Constructor
MasterLayerControlManagerActionTable::MasterLayerControlManagerActionTable(ActionTableId id, ActionContextId contextId, TSTR& name, HACCEL hDefaults, int numIds, ActionDescription* pOps, HINSTANCE hInst)
			: ActionTable(id,contextId,name,hDefaults,numIds,pOps,hInst)
{}

// Destructor
MasterLayerControlManagerActionTable::~MasterLayerControlManagerActionTable()
{
	if (mi1 != NULL) { delete mi1; mi1 = NULL; }
	if (mi2 != NULL) { delete mi2; mi2 = NULL; }
	if (mi3 != NULL) { delete mi3; mi3 = NULL; }
	if (mi4 != NULL) { delete mi4; mi4 = NULL; }
	if (mi5 != NULL) { delete mi5; mi5 = NULL; }
	if (mi6 != NULL) { delete mi6; mi6 = NULL; }
	if (mi7 != NULL) { delete mi7; mi7 = NULL; }
	if (mi8 != NULL) { delete mi8; mi8 = NULL; }
	if (mi9 != NULL) { delete mi9; mi9 = NULL; }
	if (mi10 != NULL) { delete mi10; mi10 = NULL; }
}

BOOL MasterLayerControlManagerActionTable::IsChecked(int cmdId)
{
	return 0;
}

BOOL MasterLayerControlManagerActionTable::IsEnabled(int cmdId)
{

	MasterLayerControlManager *man = GetMLCMan();
	switch(cmdId)
	{
		case ID_ENABLE_ANIM_LAYERS: return man->ShowEnableAnimLayersBtn();
		case ID_ANIM_LAYER_PROPERTIES: return man->ShowAnimLayerPropertiesBtn();
		case ID_ADD_ANIM_LAYER: return man->ShowAddAnimLayerBtn();
		case ID_DELETE_ANIM_LAYER: return man->ShowDeleteAnimLayerBtn();
		case ID_COPY_ANIM_LAYER: return man->ShowCopyAnimLayerBtn();
		case ID_PASTE_ACTIVE_ANIM_LAYER: return man->ShowPasteActiveAnimLayerBtn();
		case ID_PASTE_NEW_LAYER: return man->ShowPasteNewLayerBtn();
		case ID_COLLAPSE_ANIM_LAYER: return man->ShowCollapseAnimLayerBtn();
		case ID_DISABLE_ANIM_LAYER: return man->ShowDisableAnimLayerBtn();
		case ID_SELECT_NODES_FROM_ACTIVE: return man->ShowSelectNodesFromActive();
	
	}
	return 0;

	
}

MaxIcon* MasterLayerControlManagerActionTable::GetIcon(int cmdId)
{
	switch(cmdId)
	{
		case ID_ENABLE_ANIM_LAYERS:
			 if (!mi1) mi1 = new MaxBmpFileIcon("AnimLayerToolbar", 3); return mi1; 
		case ID_ANIM_LAYER_PROPERTIES:
			 if (!mi2) mi2 = new MaxBmpFileIcon("AnimLayerToolbar", 1); return mi2; 
		case ID_ADD_ANIM_LAYER: 
			 if (!mi3) mi3 = new MaxBmpFileIcon("AnimLayerToolbar", 2); return mi3; 
		case ID_DELETE_ANIM_LAYER:
			 if (!mi4) mi4 = new MaxBmpFileIcon("AnimLayerToolbar", 6); return mi4; 
		case ID_COPY_ANIM_LAYER:
			 if (!mi5) mi5 = new MaxBmpFileIcon("AnimLayerToolbar", 7); return mi5; 
		case ID_PASTE_ACTIVE_ANIM_LAYER:
			 if (!mi6) mi6 = new MaxBmpFileIcon("AnimLayerToolbar", 8); return mi6; 
		case ID_PASTE_NEW_LAYER: 
			 if (!mi7) mi7 = new MaxBmpFileIcon("AnimLayerToolbar", 9); return mi7; 
		case ID_COLLAPSE_ANIM_LAYER: 
			 if (!mi8) mi8 = new MaxBmpFileIcon("AnimLayerToolbar", 10); return mi8; 
		case ID_DISABLE_ANIM_LAYER: 
			 if (!mi9) mi9 = new MaxBmpFileIcon("AnimLayerToolbar", 11); return mi9; 
		case ID_SELECT_NODES_FROM_ACTIVE: 
			 if (!mi10) mi10 = new MaxBmpFileIcon("AnimLayerToolbar", 12); return mi10; 
		
	}
	return NULL;
}


const ActionTableId kActions = 0x7a6d7ced;
const ActionContextId kContext = 0x1e707dcf;

//use this in case we add more elements..
#define NumElements(array) (sizeof(array) / sizeof(array[0]))

static ActionDescription animLayerActions[] = {
	ID_ENABLE_ANIM_LAYERS,	IDS_ENABLE_ANIM_LAYERS,	IDS_ENABLE_ANIM_LAYERS,	IDS_ANIM_LAYERS,
	ID_ANIM_LAYER_PROPERTIES,		IDS_ANIM_LAYER_PROPERTIES,	IDS_ANIM_LAYER_PROPERTIES,	IDS_ANIM_LAYERS,
	ID_ADD_ANIM_LAYER,IDS_ADD_ANIM_LAYER,IDS_ADD_ANIM_LAYER,IDS_ANIM_LAYERS,
	ID_DELETE_ANIM_LAYER,		IDS_DELETE_ANIM_LAYER,	IDS_DELETE_ANIM_LAYER,	IDS_ANIM_LAYERS,
	ID_COPY_ANIM_LAYER,		IDS_COPY_ANIM_LAYER,	IDS_COPY_ANIM_LAYER,	IDS_ANIM_LAYERS,
	ID_PASTE_ACTIVE_ANIM_LAYER,	IDS_PASTE_ACTIVE_ANIM_LAYER,IDS_PASTE_ACTIVE_ANIM_LAYER,IDS_ANIM_LAYERS,
	ID_PASTE_NEW_LAYER,	IDS_PASTE_NEW_LAYER,IDS_PASTE_NEW_LAYER,	IDS_ANIM_LAYERS,
	ID_COLLAPSE_ANIM_LAYER,	IDS_COLLAPSE_ANIM_LAYER,IDS_COLLAPSE_ANIM_LAYER,	IDS_ANIM_LAYERS,
	ID_DISABLE_ANIM_LAYER,	IDS_DISABLE_ANIM_LAYER, IDS_DISABLE_ANIM_LAYER, IDS_ANIM_LAYERS ,
	ID_SELECT_NODES_FROM_ACTIVE,IDS_SELECT_NODES_FROM_ACTIVE, IDS_SELECT_NODES_FROM_ACTIVE, IDS_ANIM_LAYERS

	};

// Implements the Singleton Pattern
class LayerControlActionManager 
{
private:
	// Memory is owned by the Action System, thus deleted by the system
	static MasterLayerControlManagerActionTable*    mLCManActionTable;
	// Memory is owned by this class.
	static MasterLayerControlManagerActionCallback* mLCManActionCallback; 
protected:
	LayerControlActionManager();
	LayerControlActionManager(const LayerControlActionManager& rhs);
	~LayerControlActionManager();
	LayerControlActionManager& operator=(const LayerControlActionManager& rhs);
public:
	static LayerControlActionManager& GetInstance();
	static ActionTable*        GetActionTable();
	static void                SetUpActions();
};
// Static Initialization
MasterLayerControlManagerActionTable*    LayerControlActionManager::mLCManActionTable    = NULL; 
MasterLayerControlManagerActionCallback* LayerControlActionManager::mLCManActionCallback = NULL;
// Constructor
LayerControlActionManager::LayerControlActionManager()
{
	// Do Nothing
}
// Destructor
LayerControlActionManager::~LayerControlActionManager()
{
	if (mLCManActionCallback)
	{
		delete mLCManActionCallback;
		mLCManActionCallback = NULL;
	}
}
// Singleton Method
LayerControlActionManager& LayerControlActionManager::GetInstance()
{
	static LayerControlActionManager instance;
	return instance;
}

ActionTable* LayerControlActionManager::GetActionTable()
{
	if (!mLCManActionTable)
	{
		TSTR name = GetString(IDS_ANIM_LAYERS);
		int numOps = NumElements(animLayerActions);
		mLCManActionTable = new  MasterLayerControlManagerActionTable(kActions, kContext, name, NULL, numOps, animLayerActions, hInstance);
		GetCOREInterface()->GetActionManager()->RegisterActionContext(kContext, name.data());
	}
	return mLCManActionTable;
}

void LayerControlActionManager::SetUpActions()
{
	LayerControlActionManager::GetActionTable(); //just to make sure

	if(mLCManActionCallback==NULL)
	{
		mLCManActionCallback = new MasterLayerControlManagerActionCallback ();
		GetCOREInterface()->GetActionManager()->ActivateActionTable (mLCManActionCallback, kActions);
	}
}

#endif // NO_ANIM_LAYERS

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//				MasterLayerControlManager
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////



class MasterLayerControlManagerClassDesc:public ClassDesc2
{
	public:
	MasterLayerControlManagerClassDesc(){}
	int 			IsPublic() { return 0; }
	void *			Create(BOOL loading) { return GetMLCMan(); }
	const TCHAR *	ClassName() { return _T("MasterLayerControlManager");} // MASTERLAYERCONTROLMANAGER_CNAME; }
	SClass_ID		SuperClassID() { return CTRL_USERTYPE_CLASS_ID; }
	Class_ID		ClassID() { return MASTERLAYERCONTROLMANAGER_CLASS_ID; }
	const TCHAR* 	Category() { return _T("");  }
	const TCHAR*	InternalName() { return _T("MasterLayerControlManager"); }			// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle
	void ResetClassParams(BOOL fileReset){MasterLayerControlManager::LoadFromIni();}
#ifndef NO_ANIM_LAYERS
	// We do not want to expose Animation Layers UI in Viz
	int             NumActionTables() {return 1;}//in order to register or callback we need to manual set this up!
	ActionTable*  GetActionTable(int i) {return LayerControlActionManager::GetInstance().GetActionTable();}
#endif
};

static MasterLayerControlManagerClassDesc mLCManCD;
ClassDesc* GetMasterLayerControlManagerDesc() {return &mLCManCD;}


class MasterLayerControlAccessor : public PBAccessor
{ 
	public:
		TSTR GetLocalName(ReferenceMaker* owner, ParamID id, int tabIndex) {
			if(owner)
			{
				MasterLayerControlManager *manager = static_cast<MasterLayerControlManager*>(owner);
				TSTR name;
				if (id == MasterLayerControlManager::masterlayers)
				{
					MasterLayerControl *mLC = manager->GetNthMLC(tabIndex);
					if(mLC)
						return mLC->GetName();
				}
			}
			return TSTR(GetString(IDS_MASTERLAYER));
		}
};

static MasterLayerControlAccessor theMasterLayerControlAccessor;


static ParamBlockDesc2  MasterLayerControlManager_pblk(0, _T("MasterLayerControlManagerParams"), 0, &mLCManCD, P_AUTO_CONSTRUCT,0,
		
	MasterLayerControlManager::masterlayers,_T("AnimLayers"),TYPE_REFTARG_TAB,	0 ,
	P_COMPUTED_NAME+P_VARIABLE_SIZE+P_SUBANIM,	IDS_MASTERLAYER, 
		p_accessor,		&theMasterLayerControlAccessor,
		end,
	end
);



static MasterLayerControlManager *theMasterLayerControlManager = NULL;
MasterLayerControlManager * GetMLCMan()
{
	if(theMasterLayerControlManager)
	{
		if(theMasterLayerControlManager->pblock==NULL)
			mLCManCD.MakeAutoParamBlocks(theMasterLayerControlManager);	
		return theMasterLayerControlManager;
	}
	else
	{
		theMasterLayerControlManager = new MasterLayerControlManager;
	}
	return theMasterLayerControlManager; 
}


void AddMasterLayerControlManagerToScene()
{
	ITrackViewNode* tvRoot = GetCOREInterface()->GetTrackViewRootNode();
	if (tvRoot->FindItem(MASTERLAYERCONTROLMANAGER_CLASS_ID) < 0)
	{
		tvRoot->AddController(GetMLCMan(), _T("Anim Layer Control Manager"), MASTERLAYERCONTROLMANAGER_CLASS_ID);
	}
}




////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//				MasterLayerControlManager
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PostReset(void *param, NotifyInfo *info)
{
	AddMasterLayerControlManagerToScene();
	if(GetMLCMan()->pblock==NULL)
	{
		mLCManCD.MakeAutoParamBlocks(GetMLCMan());	
	}
}

void PreReset(void *param, NotifyInfo *info)
{
	GetMLCMan()->DeleteMe();
}

void ShutDown(void *param, NotifyInfo *info)
{
	// Must delete the toolbar BEFORE the DeleteMe call below
	if(MasterLayerControlManager::toolbarCtrl)
	{
		delete MasterLayerControlManager::toolbarCtrl;
		MasterLayerControlManager::toolbarCtrl = NULL;
	}
	GetMLCMan()->DeleteMe();
}

void PreMerge(void *param, NotifyInfo *info)
{
	//save out the clipAssociations
	MasterLayerControlManager *mmcP = static_cast<MasterLayerControlManager*>(param);
	mmcP->SaveExistingMLCs();
}

void PostMerge(void *param, NotifyInfo *info)
{
	MasterLayerControlManager *mmcP = static_cast<MasterLayerControlManager*>(param);
	mmcP->RevertExistingMLCs();
}

void NotifySelectionChanged(void *param, NotifyInfo *info)
{
	MasterLayerControlManager *mmcP = static_cast<MasterLayerControlManager*>(param);
	mmcP->NotifySelectionChanged();
}



AnimLayerToolbarControl *MasterLayerControlManager::toolbarCtrl = NULL;  //leaks need to figure out not to leak..

MasterLayerControlManager::MasterLayerControlManager() : pblock(NULL)
{

	theMasterLayerControlManager = this; //always singleton know excuses!

	RegisterNotification(&ShutDown, this, NOTIFY_SYSTEM_SHUTDOWN);
	RegisterNotification(PreReset, this, NOTIFY_SYSTEM_PRE_RESET);
	RegisterNotification(PreReset, this, NOTIFY_FILE_PRE_OPEN);
	RegisterNotification(PreReset, this, NOTIFY_SYSTEM_PRE_NEW);
	RegisterNotification(PostReset, this, NOTIFY_SYSTEM_POST_RESET);
	RegisterNotification(PostReset, this, NOTIFY_FILE_POST_OPEN);
	RegisterNotification(PostReset, this, NOTIFY_SYSTEM_POST_NEW);
	RegisterNotification(PreMerge,this,NOTIFY_FILE_PRE_MERGE);
	RegisterNotification(PostMerge,this,NOTIFY_FILE_POST_MERGE);

	mLCManCD.MakeAutoParamBlocks(this);	

	ITrackViewNode* tvRoot = GetCOREInterface()->GetTrackViewRootNode();
	if (tvRoot->FindItem(MASTERLAYERCONTROLMANAGER_CLASS_ID) < 0)
	{
		tvRoot->AddController(this, _T("Anim Layer Control Manager"), MASTERLAYERCONTROLMANAGER_CLASS_ID);
	}
	
	RegisterNotification(::NotifySelectionChanged, this, NOTIFY_SELECTIONSET_CHANGED);
	justUpToActive = false; //mute all greater than active!
	controllerType =  IAnimLayerControlManager::eDefault;
	collapseAllPerFrame = false;
	collapsePerFrameActiveRange = true;
	collapseRange = Interval(0,100.0f);
	askBeforeDeleting = true;
#ifndef NO_ANIM_LAYERS
	LayerControlActionManager::GetInstance().SetUpActions();
	
#endif
}

MasterLayerControlManager::~MasterLayerControlManager()
{
	UnRegisterNotification(&ShutDown, this, NOTIFY_SYSTEM_SHUTDOWN);
	UnRegisterNotification(PreReset, this, NOTIFY_SYSTEM_PRE_RESET);
	UnRegisterNotification(PreReset, this, NOTIFY_FILE_PRE_OPEN);
	UnRegisterNotification(PreReset, this, NOTIFY_SYSTEM_PRE_NEW);
	UnRegisterNotification(PostReset, this, NOTIFY_SYSTEM_POST_RESET);
	UnRegisterNotification(PostReset, this, NOTIFY_FILE_POST_OPEN);
	UnRegisterNotification(PostReset, this, NOTIFY_SYSTEM_POST_NEW);
	UnRegisterNotification(PreMerge,this,NOTIFY_FILE_PRE_MERGE);
	UnRegisterNotification(PostMerge,this,NOTIFY_FILE_POST_MERGE);
	UnRegisterNotification(::NotifySelectionChanged, this, NOTIFY_SELECTIONSET_CHANGED);

	DeleteAllRefs();
	theMasterLayerControlManager = NULL;
}
void MasterLayerControlManager::LoadFromIni()
{
	/*
 	int i;
	getCfgMgr().setSection(_T("AnimationLayers"));
    if(getCfgMgr().keyExists(_T("justUpToActive"))) {
      getCfgMgr().getInt(_T("justUptoActive"), &i);
	  if(i>0)
		  justUpToActive = true;
	  else
		  justUpToActive = false;
    }
    if(getCfgMgr().keyExists(_T("controllerType"))) {shor
      getCfgMgr().getInt(_T("controllerType"), &i);
	  if(i==0)
		  controllerType = IAnimLayerControlManager::eBezier;
	  else if(i==1)
		  controllerType = IAnimLayerControlManager::eLinear;
	  else if(i==2)
		  controllerType = IAnimLayerControlManager::eDefault;
    }
    if(getCfgMgr().keyExists(_T("collapseAllPerFrame"))) {
      getCfgMgr().getInt(_T("collapseAllPerFrame"), &i);
	  if(i>0)
		  collapseAllPerFrame = true;
	  else
		  collapseAllPerFrame = false;
    }
    if(getCfgMgr().keyExists(_T("collapsePerFrameActiveRange"))) {
      getCfgMgr().getInt(_T("collapsePerFrameActiveRange"), &i);
	  if(i>0)
		  collapsePerFrameActiveRange = true;
	  else
		  collapsePerFrameActiveRange = false;
    }
    if(getCfgMgr().keyExists(_T("collapseRangeStart"))) {
      getCfgMgr().getInt(_T("collapseRangeStart"), &i);
	  collapseRange.SetStart(i);
    }
  if(getCfgMgr().keyExists(_T("collapseRangeEnd"))) {
      getCfgMgr().getInt(_T("collapseRangeEnd"), &i);
	  collapseRange.SetEnd(i);
    }
*/

}
void MasterLayerControlManager::SetUpCustUI(ICustToolbar *toolbar,int id,HWND hWnd, HWND hParent)
{
	if(id==ANIMLAYERMGR_COMBOBOX_ID)
	{
		if(toolbarCtrl == NULL)
		{
			// this pointer must exist for the entire lifetime of 3dsmax. This gets deleted
			// when 3dsmax shuts down via a Notification callback.
			toolbarCtrl = new AnimLayerToolbarControl();
		}
		toolbarCtrl->SetHWND(hWnd);
		SendMessage(hWnd, WM_SETFONT, (WPARAM)GetCOREInterface()->GetAppHFont(), MAKELONG(0, 0));
		toolbarCtrl->SetParent(hParent);
		toolbarCtrl->Refresh();
	}
	else if(id==ANIMLAYERMGR_EDIT_ID)
	{
		if(toolbarCtrl)
		{
			toolbarCtrl->SetEditHWND(hWnd);
			SendMessage(hWnd, WM_SETFONT, (WPARAM)GetCOREInterface()->GetAppHFont(), MAKELONG(0, 0));
		}
	}
	else if(id==ANIMLAYERMGR_SPINNER_ID)
	{
		if(toolbarCtrl)
			toolbarCtrl->SetSpinHWND(hWnd);
		ISpinnerControl *spin = GetISpinner(hWnd);
		spin->SetLimits(-999999999,999999999,FALSE);
		spin->SetScale(0.5f);
		spin->LinkToEdit(AnimLayerToolbarControl::GetEditHWND(),EDITTYPE_FLOAT);
		ReleaseISpinner(spin);
		if(toolbarCtrl)
			toolbarCtrl->Refresh();

	}
}

TCHAR *MasterLayerControlManager::GetComboToolTip()
{
	//return the name of the active layer
	if(activeLayers.Count()==1)
	{
		TSTR name;
		name = GetLayerName(activeLayers[0]);
		static char what[256];
		_tcscpy(what,name.data());
		return what;
	}
	return NULL;
}

void MasterLayerControlManager::NotifySelectionChanged()
{
	activeLayers.ZeroCount();
	copyLayers = false;
	canDisable = false;

	currentLayers.ZeroCount();
	isLayerOnAll.ZeroCount();

	if(GetNumMLCs()>0) //only do all this if we have any layers.
	{
		Tab<INode *>nodeTab;
		GetSelectedNodes(nodeTab); //get all selected nodes.

		//based on the selected nodes we need to know 2 things
		//1) the active layers.
		//2) if there are any cut layers in the layer controls...

		GetNodesLayers(nodeTab,currentLayers,isLayerOnAll);

		bool anyLayerMoreThanOne = false;
		if(currentLayers.Count()>0)
		{
			Tab<MasterLayerControlManager::LCInfo> layerControls;
			GetLayerControls(nodeTab,layerControls);
			BitArray bArray;
			bArray.SetSize(GetNumMLCs());
			
			for(int i=0; i < layerControls.Count(); ++i)
			{
				LayerControl *lC = layerControls[i].lC;
				if(lC)
				{
					int active = lC->mLCs[lC->GetLayerActive()];
					bArray.Set(active);
					if(lC->copyClip!=NULL)
						copyLayers = true;
					if(lC->GetLayerCount()>1)
						anyLayerMoreThanOne = true;
				}
			}
			for(int i=0; i<GetNumMLCs(); ++i)
			{
				if(bArray[i])
				{
					activeLayers.Append(1,&i); // Why are you appending the address of i? Possibly use an MaxSDK::Util::Array instead
				}
			}
		}
		if(activeLayers.Count()==1 && anyLayerMoreThanOne==false)
		{
			canDisable = true;
		}
	}

	if(toolbarCtrl)
	{
		toolbarCtrl->Refresh();
	}
}

bool MasterLayerControlManager::ShowEnableAnimLayersBtn()
{
	return (GetCOREInterface()->GetSelNodeCount()>0);
}
bool MasterLayerControlManager::ShowAnimLayerPropertiesBtn()
{
	return true;
}
bool MasterLayerControlManager::ShowAddAnimLayerBtn()
{
	return (activeLayers.Count()>0);
}
bool MasterLayerControlManager::ShowDeleteAnimLayerBtn()
{
	return (activeLayers.Count()==1&&activeLayers[0]!=0);
}
bool MasterLayerControlManager::ShowCopyAnimLayerBtn()
{
	return (activeLayers.Count()==1&&copyLayers==false);
}
bool MasterLayerControlManager::ShowPasteActiveAnimLayerBtn()
{
	return (activeLayers.Count()==1&&copyLayers==true);
}
bool MasterLayerControlManager::ShowPasteNewLayerBtn()
{
	return (activeLayers.Count()>=1 && copyLayers==true);
}
bool MasterLayerControlManager::ShowCollapseAnimLayerBtn()
{
	return (activeLayers.Count()==1&&activeLayers[0]!=0);
}
bool MasterLayerControlManager::ShowDisableAnimLayerBtn()
{
	return (activeLayers.Count()==1&&canDisable==true);
}
void* MasterLayerControlManager::GetInterface(ULONG Ifaceid) 
{
	return Control::GetInterface(Ifaceid);
}


void MasterLayerControlManager::InvalidateUI()
{
	if(pblock)
        MasterLayerControlManager_pblk.InvalidateUI(pblock->LastNotifyParamID());
}

RefTargetHandle MasterLayerControlManager::Clone(RemapDir& remap) 
{
	MasterLayerControlManager* newstate = new MasterLayerControlManager();	
	if(pblock)
        newstate->ReplaceReference(0,remap.CloneRef(pblock));
	BaseClone(this,newstate,remap);
	return(newstate);
}




int MasterLayerControlManager::NumSubs()
{
	return 1;
}

int MasterLayerControlManager::NumRefs()
{
	return 1 + existingMasterLayerControls.Count(); //save a reference to the current clip assocations on save so we can reinsert it in after
												 //the pblock gets merged.
}

Animatable* MasterLayerControlManager::SubAnim(int i) 	
{
    if (i==0) return pblock; else return NULL;
}

TSTR MasterLayerControlManager::SubAnimName(int i) 
{
	switch (i) 
	{
		case 0: return GetString(IDS_ANIM_LAYERS);
		default: return _T("");
	}
}


int	MasterLayerControlManager::NumParamBlocks()
{
	return 1;
}
IParamBlock2 *MasterLayerControlManager::GetParamBlock(int i)
{
    if (i==0) return pblock;
	else return NULL;
}

IParamBlock2 *MasterLayerControlManager::GetParamBlockByID(BlockID id)
{ 
    if (pblock&&pblock->ID() == id) return pblock; else return NULL;
}

RefTargetHandle MasterLayerControlManager::GetReference(int i)
{
    if (i==0) return (RefTargetHandle)pblock;
	else if( i < (1 + existingMasterLayerControls.Count()))
		return  static_cast<RefTargetHandle> (existingMasterLayerControls[i-1]);
	else return (RefTargetHandle)NULL;

}
void MasterLayerControlManager::SetReference(int i, RefTargetHandle rtarg)
{
    if (i==0)
	{
		pblock = (IParamBlock2*)rtarg;
	}
	else if (i < (1+existingMasterLayerControls.Count()))
	{
		existingMasterLayerControls[i-1] = static_cast<MasterLayerControl*>(rtarg);
	}
}



RefResult MasterLayerControlManager::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID,  RefMessage message) 
{
	switch (message) 
	{
	case REFMSG_WANT_SHOWPARAMLEVEL:// show the paramblock2 entry in trackview
		{
			BOOL *pb = (BOOL *)(partID);
			*pb = TRUE;
			return REF_STOP;
			break; // It should never hit this line of code, but it's safer to put it in anyways
		}
	case REFMSG_CHANGE:
		{
			MasterLayerControlManager_pblk.InvalidateUI();
			break;
		}
	}
	return REF_SUCCEED;
}




MasterLayerControl* MasterLayerControlManager::GetNthMLC(int i)
{
	MasterLayerControl* result = NULL;
	
	if(pblock && i > -1 && i < GetNumMLCs())
	{
		ReferenceTarget* c = NULL;
	    pblock->GetValue(masterlayers,0,c,FOREVER,i);
		DbgAssert(c);
		result = (MasterLayerControl*)c;
	}
	return result;
}


int MasterLayerControlManager::GetNumMLCs()
{
	if(pblock)
        return pblock->Count(masterlayers);
	else
		return 0;
}

void MasterLayerControlManager::ResetLayerMLCs(int index)
{

	//we need to get all of the LayerContrl's in the scene we do that by getting all of the nodes with layers and then all of the nodes.
	Tab<INode *> nodes;
	GetAllNodes(nodes);

	//then get all of the layer controls
	Tab<LCInfo> layerControls;
	GetLayerControls(nodes,layerControls);
	LayerControlRestore* rest = NULL;
	for(int i=0; i<layerControls.Count(); ++i)
	{
		LayerControl* lC = layerControls[i].lC;
		if(lC)
		{
			if(theHold.Holding())
				rest = new LayerControlRestore(lC);
			for(int j=0; j<lC->mLCs.Count(); ++j)
			{
				if(lC->mLCs[j]>index)
				{
					lC->mLCs[j] = lC->mLCs[j]-1; //minus one if the layer is greater than the one deleted
				}
			}
			if(rest)
			{
				theHold.Put(rest);
			}
		}
	}
}


void MasterLayerControlManager::DeleteNthMLC(int index)
{
    if (pblock==NULL ||index < 0 || index >= GetNumMLCs())
	{
		return;
	}
	pblock->Delete(masterlayers,index,1);
}

void MasterLayerControlManager::AppendMLC(MasterLayerControl *mLC)
{
	if(pblock==NULL) //never should happen, but just in case
	{
        mLCManCD.MakeAutoParamBlocks(this);	
	}
	if(pblock)
	{
		ReferenceTarget *rA = (ReferenceTarget *) mLC;
		pblock->Append(masterlayers,1,&rA);
	}
}


void MasterLayerControlManager::GetNodesLayers(Tab<INode *> &nodeTab, Tab<int> &layers,Tab<bool> &layersOnAll)
{
	//go through of the layers(mlcs) and see which if any have nodes in them.
	MasterLayerControl* mLC = NULL;
	layers.ZeroCount();
	int foundIndex = 0;
	for(int i=0; i < GetNumMLCs(); ++i)
	{
		mLC = GetNthMLC(i);
		if(mLC)
		{
			bool isInAll = true;
			BOOL found = FALSE;

			for(int k=0; k<nodeTab.Count(); ++k)
			{
				for(int j=0; j < mLC->monitorNodes.Count(); ++j)
				{
					if(mLC->monitorNodes[j])
					{
						INodeMonitor* nm = static_cast<INodeMonitor *>(mLC->monitorNodes[j]->GetInterface(IID_NODEMONITOR));
						if(nm->GetNode()==nodeTab[k])
						{
							found=TRUE;
							foundIndex = j;
							break;
						}
					}
				}
				if(foundIndex == mLC->monitorNodes.Count())
				{
					isInAll=false;
					if(found==TRUE)
						break;
				}
			}
			if(found)
			{
				layers.Append(1,&i);
				layersOnAll.Append(1,&isInAll);
			}
		}
	}
}
void MasterLayerControlManager::SetLayerActiveNodes(int index, Tab<INode *> &nodeTab)
{

	Tab<MasterLayerControlManager::LCInfo> existingLayers;
	MasterLayerControl *mLC = GetNthMLC(index);
	LayerControl* lC1 = NULL;
	LayerControl* lC2 = NULL;
	if(mLC)
	{
		GetLayerControls(nodeTab, existingLayers);
		for(int i=0; i < mLC->layers.Count(); ++i)
		{
			lC1 = mLC->layers[i];
			if(lC1)
			{
				for(int j=0;j<existingLayers.Count();++j)
				{
					lC2 = existingLayers[j].lC;
					if(lC1==lC2)
					{
						for(int k=0; k < lC1->mLCs.Count(); ++k)
						{
							if(lC1->mLCs[k] ==index)
							{
								lC1->SetLayerActiveInternal(k);
								break;
							}
						}
						break;
					}
				}
			}
		}
	}
	NotifySelectionChanged();
}


void MasterLayerControlManager::InsertMLC(int location,MasterLayerControl *mLC)
{
	if(pblock==NULL) //never should happen, but just in case
        mLCManCD.MakeAutoParamBlocks(this);	
	if(pblock)
	{
		ReferenceTarget *rA = (ReferenceTarget *) mLC;
		pblock->Insert(masterlayers,location,1,&rA);
	}
	NotifySelectionChanged();
}

void MasterLayerControlManager::DeleteNthLayer(int index)
{
    if (pblock==NULL ||index < 0 || index >= GetNumMLCs())
	{
		return;
	}

	if(theHold.Holding())
		theHold.Put(new NotifySelectionRestore());

	MasterLayerControl* mLC = GetNthMLC(index);
	for(int i=0;i<mLC->layers.Count();++i)
	{
		LayerControl *lC  = mLC->layers[i];
		if(lC)
		{
			for(int j=lC->mLCs.Count()-1;j>-1;--j)
			{
				if(lC->mLCs[j]>index)
					lC->mLCs[j] = lC->mLCs[j]-1;
				else if(lC->mLCs[j]==index)
				{
					lC->DeleteLayerInternal(j);
					break;
				}
			}
		}
	}
	pblock->Delete(masterlayers,index,1);

	if(theHold.Holding())
		theHold.Put(new NotifySelectionRestore());

	NotifySelectionChanged();

}


static INT_PTR CALLBACK DeleteWarningProc(
		HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
	{
	switch (message) {
		case WM_INITDIALOG: {			
			CenterWindow(hWnd,GetParent(hWnd));			
			return FALSE;
			}
	
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDOK:
					if(IsDlgButtonChecked(hWnd, IDC_DONTSHOWAGAIN))
						GetMLCMan()->askBeforeDeleting = false;;
					EndDialog(hWnd,1);
					break;
				case IDCANCEL:
					EndDialog(hWnd,0);
					break;
				}
			break;
		
		default:
			return 0;
		}
	return 1;
	}



bool MasterLayerControlManager::ContinueDelete()
{
	if (askBeforeDeleting&&GetCOREInterface()->IsNetServer()==FALSE&&GetCOREInterface()->GetQuietMode()==FALSE)
	{
		int what = DialogBoxParam(hInstance,MAKEINTRESOURCE(IDD_ASKBEFOREDELETING),
		GetCOREInterface()->GetMAXHWnd(),DeleteWarningProc,0);
		if(what>0)
			return true;
		else 
			return false;

	}
	return true;
}

void MasterLayerControlManager::DeleteNthLayerNodes(int index, Tab<INode *> &nodeTab)
{
	if(index==0) //don't delete the base layer
		return;
	Tab<MasterLayerControlManager::LCInfo> existingLayers;
	MasterLayerControl* mLC = GetNthMLC(index);
	LayerControl* lC1 = NULL;
	LayerControl* lC2 = NULL;

	if(mLC)
	{
		if(theHold.Holding())
			theHold.Put(new NotifySelectionRestore());
		MasterLayerControlRestore *mLCRestore = new  MasterLayerControlRestore(mLC);
	
		GetLayerControls(nodeTab, existingLayers);
		for(int i=mLC->layers.Count()-1;i>-1;--i)
		{
			lC1 = mLC->layers[i];
			if(lC1)
			{
				for(int j=0;j<existingLayers.Count();++j)
				{
					lC2 = existingLayers[j].lC;
					if(lC1==lC2)
					{
						for(int k=0;k<lC1->mLCs.Count();++k)
						{
							if(lC1->mLCs[k] ==index)
							{
								lC1->DeleteLayerInternal(k);
								break;
							}
						}
						mLC->DeleteReference(i+1);
						mLC->layers.Delete(i,1);
						break;
					}
				}
			}
		}
	

		//Delete the monitor nodes!
		int j;
		for(int i=0;i<nodeTab.Count();++i)
		{
			INode *node = nodeTab[i];
			for(j=mLC->monitorNodes.Count()-1;j>-1;--j)
			{
				if(mLC->monitorNodes[j])
				{
					INodeMonitor *nm = static_cast<INodeMonitor *>(mLC->monitorNodes[j]->GetInterface(IID_NODEMONITOR));
					if(nm->GetNode()==node)
						break;
				}
			}
			if(j>-1)
			{
				mLC->DeleteReference(1 +mLC->layers.Count()+j);
				mLC->monitorNodes.Delete(j,1);
			}
			}
		if(theHold.Holding())
		{
			theHold.Put(mLCRestore);
		}
		else
		{
			delete mLCRestore;
			mLCRestore = NULL;
		}

		if(mLC->layers.Count()==0)
		{
			DeleteNthMLC(index);
			ResetLayerMLCs(index);
		}

		if(theHold.Holding())
			theHold.Put(new NotifySelectionRestore());

		NotifySelectionChanged();

	}
}



void MasterLayerControlManager::SetLayerActive(int index)
{
	if(pblock==NULL||index<0||index>=this->GetNumMLCs())
		return;

	MasterLayerControl * mLC = GetNthMLC(index);
	for(int i=0;i<mLC->layers.Count();++i)
	{
		LayerControl *lC  = mLC->layers[i];
		if(lC)
		{
			for(int j=0;j<lC->mLCs.Count();++j)
			{
				if(lC->mLCs[j]==index)
				{
					lC->SetLayerActiveInternal(j);
				}
			}
		}
	}
	NotifySelectionChanged();

}

int MasterLayerControlManager::GetLayerActive()
{
	return -1;//activeLayer;
}

int MasterLayerControlManager::GetNthMLC(TSTR &name)
{
	MasterLayerControl* mLC = NULL;
	for(int i=0;i<GetNumMLCs();++i)
	{
		mLC = GetNthMLC(i);
		if(mLC)
		{
			if(name==mLC->GetName())
			{
				return i;
			}
		}
	}
	return -1;
}

int MasterLayerControlManager::GetMLCIndex(TSTR &name)
{
	MasterLayerControl* mLC = NULL;
	for(int i=0;i<GetNumMLCs();++i)
	{
		mLC = GetNthMLC(i);
		if(mLC)
		{
			if(name==mLC->GetName())
			{
				return i;
			}
		}
	}
	return -1;
}

TSTR MasterLayerControlManager::MakeLayerNameUnique(TSTR &name)
{
	MasterLayerControl* mLC = NULL;
	for(int i=0;i<GetNumMLCs();++i)
	{
		mLC = GetNthMLC(i);


	}
	return name;
}

//actually don't clone but we save them as a reference
void MasterLayerControlManager::SaveExistingMLCs()
{
	for(int j =0;j<GetNumMLCs();++j)
	{
		MasterLayerControl* mLC =GetNthMLC(j);
		if(mLC)
		{
			MasterLayerControl* null_ptr = NULL;;
            existingMasterLayerControls.Append(1,&null_ptr);
			ReplaceReference(existingMasterLayerControls.Count(),static_cast<RefTargetHandle>(mLC));
		}
	}
}

void MasterLayerControlManager::RevertExistingMLCs()
{
	for(int i=0;i<existingMasterLayerControls.Count();++i)
	{
		MasterLayerControl *mLC = existingMasterLayerControls[i];
		//only reinsert if it doesn't exist...
		int j;
		MasterLayerControl *testMLC = NULL;
		for(j=0;j<GetNumMLCs();++j)
		{
			testMLC = GetNthMLC(j);
			if(testMLC!=NULL &&(testMLC->GetName()==mLC->GetName()))
			{
				break;
			}
		}
		//doesn't exist so append it and adjust layer indices..
		if(j==GetNumMLCs())
		{
			ReferenceTarget *rA = (ReferenceTarget *) mLC;
			pblock->Append(masterlayers,1,&rA);
			int newIndex = GetNumMLCs()-1;
			for(int k=0;k<mLC->layers.Count();++k)
			{
				LayerControl *lC = mLC->layers[k];
				if(lC)
				{
					for(int z =0;z<lC->mLCs.Count();++z)
					{
						if(lC->mLCs[z]==i) //i is old index!
						{
							lC->mLCs[z] = newIndex; 
							break; //only one layer per object!
						}
					}
				}
			}
		}
		else
		{
			//it exists so we need to adjust indices and add the layers to the existing layer!
			//first adjust indices
			for(int k=0;k<mLC->layers.Count();++k)
			{
				LayerControl *lC = mLC->layers[k];
				if(lC)
				{
					for(int z =0;z<lC->mLCs.Count();++z)
					{
						if(lC->mLCs[z]==i) //i is old index!
						{
							lC->mLCs[z] = j; //j is the new index
							break; //only one layer per object!
						}
					}
				
					//now we need to make sure that the existing mlc has the layers added to it
					GetNodesProc dep;
					lC->DoEnumDependents (&dep);
					if(dep.nodes.Count()>0&&dep.nodes[0])
					{
						testMLC->AddNode(dep.nodes[0]);
						testMLC->AppendLayerControl(lC);
					}
				}
			}
		}
		DeleteReference(1+i);
	}
	existingMasterLayerControls.ZeroCount();
	NotifySelectionChanged();

}








class MasterLayerControlManagerPostLoadCallback:public  PostLoadCallback
{
public:
	MasterLayerControlManager      *s;
	MasterLayerControlManagerPostLoadCallback(MasterLayerControlManager *r) {s=r;}
	void proc(ILoad *iload);
};

void MasterLayerControlManagerPostLoadCallback::proc(ILoad *iload)
{
	//no undo of this..
	theHold.Suspend();

	for(int j =s->GetNumMLCs()-1;j>-1;--j)
	{
		MasterLayerControl *mLC = s->GetNthMLC(j);
		if(mLC)
		{
			if(mLC->layers.Count()==0) //should be deleted
			{
				s->DeleteNthMLC(j);
				s->	ResetLayerMLCs(j);

			}
		}
	}
	theHold.Resume();
	delete this;
}




#define UPTOACTIVE_CHUNK	0x1001
#define ASKBEFOREDELETING_CHUNK	0x1002
#define CONTROLLERTYPE_CHUNK		0x1003
#define COLLAPSEALLPERFRAME_CHUNK	0x1004
#define COLLAPSEPERFRAMEACTIVERANGE_CHUNK	0x1005
#define COLLAPSERANGESTART_CHUNK	0x1006
#define COLLAPSERANGEEND_CHUNK	0x1007
IOResult MasterLayerControlManager::Load(ILoad *iload)
{

	MasterLayerControlManagerPostLoadCallback* ctrlcb = new MasterLayerControlManagerPostLoadCallback(this);
	iload->RegisterPostLoadCallback(ctrlcb);


	ULONG nb;
	IOResult res = IO_OK;
	int ix=0;
	int v;

	while (IO_OK==(res=iload->OpenChunk())) {
		switch (iload->CurChunkID()) {
			case UPTOACTIVE_CHUNK: {
				res = iload->Read(&v,sizeof(v),&nb);
				if(v==1)
					justUpToActive = true;
				else 
					justUpToActive = false;
				break;
				}
			case ASKBEFOREDELETING_CHUNK:
				res = iload->Read(&v,sizeof(v),&nb);
				if(v==1)
					askBeforeDeleting = true;
				else
					askBeforeDeleting = false;
				break;
			case CONTROLLERTYPE_CHUNK:
				res = iload->Read(&v,sizeof(v),&nb);
				controllerType = static_cast<IAnimLayerControlManager::ControllerType>(v);
				break;
			case COLLAPSEALLPERFRAME_CHUNK:
				res = iload->Read(&v,sizeof(v),&nb);
				if(v==1)
					collapseAllPerFrame = true;
				else
					collapseAllPerFrame = false;
				break;
			case COLLAPSEPERFRAMEACTIVERANGE_CHUNK:
				res = iload->Read(&v,sizeof(v),&nb);
				if(v==1)
					collapsePerFrameActiveRange = true;
				else
					collapsePerFrameActiveRange = false;
				break;
			case COLLAPSERANGESTART_CHUNK:
				res = iload->Read(&v,sizeof(v),&nb);
				collapseRange.SetStart(v);
				break;
			case COLLAPSERANGEEND_CHUNK:
				res = iload->Read(&v,sizeof(v),&nb);
				collapseRange.SetEnd(v);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK)  return res;
		}
	return IO_OK;
}


IOResult MasterLayerControlManager::Save(ISave *isave)
{
	ULONG nb;
	int v;

	isave->BeginChunk(UPTOACTIVE_CHUNK);
	if(justUpToActive==true)
		v=1;
	else
		v=0;
	isave->Write(&v,sizeof(v),&nb);			
	isave->EndChunk();

	isave->BeginChunk(ASKBEFOREDELETING_CHUNK);
	if(askBeforeDeleting==true)
		v=1;
	else
		v=0;
	isave->Write(&v,sizeof(v),&nb);			
	isave->EndChunk();

	isave->BeginChunk(CONTROLLERTYPE_CHUNK);
	v = static_cast<int>(controllerType);
	isave->Write(&v,sizeof(v),&nb);			
	isave->EndChunk();

	isave->BeginChunk(COLLAPSEALLPERFRAME_CHUNK);
	if(collapseAllPerFrame==true)
		v=1;
	else
		v=0;
	isave->Write(&v,sizeof(v),&nb);			
	isave->EndChunk();


	isave->BeginChunk(COLLAPSEPERFRAMEACTIVERANGE_CHUNK);
	if(collapsePerFrameActiveRange==true)
		v=1;
	else
		v=0;
	isave->Write(&v,sizeof(v),&nb);			
	isave->EndChunk();

	isave->BeginChunk(COLLAPSERANGESTART_CHUNK);
	v = collapseRange.Start();
	isave->Write(&v,sizeof(v),&nb);			
	isave->EndChunk();

	isave->BeginChunk(COLLAPSERANGEEND_CHUNK);
	v = collapseRange.End();
	isave->Write(&v,sizeof(v),&nb);			
	isave->EndChunk();

	return IO_OK;
}

void MasterLayerControlManager::DeleteMe()
{
	//need to do this backwards and reset the indices, otherwize could crash in wire param code since
	//the mLCs indices may be off and thus we won't correctly de-wire and boom. #867594 MZ.
	for(int j = GetNumMLCs()-1; j > -1 ; --j)
	{
		MasterLayerControl* mLC = GetNthMLC(j);
		if(mLC)
		{
			DeleteNthMLC(j);
		}
		ResetLayerMLCs(j);
	}
}

//The following class is used when we need to iterate over the anim list of a particular node in order to get the
//layer controls that exist on this node.
class LayerControlEnum : public AnimEnum {
public:	
	struct AnimData{ Animatable *anim;   Animatable *client; int subNum;};
	Tab<AnimData> animData;

	LayerControlEnum() : AnimEnum(SCOPE_ALL)
	{
	}
	int proc(Animatable *anim, Animatable *client, int subNum)
	{
		if(IsLayerControl(anim)==true)
		{
		AnimData data;
		data.anim = anim; data.client = client; data.subNum = subNum;
		animData.Append(1,&data);
		}
		return ANIM_ENUM_PROCEED;
	}
};


void MasterLayerControlManager::GetLayerControls(Tab<INode *> &incomingNodes, Tab<LCInfo> &existingLayers)
{

	Tab<INode*> layerNodes,nodes;
	GetAllNodes(layerNodes); //get the nodes that have layers.
	//put the union of the incomingNodes and the nodes with layers into nodes
	for(int i=0; i<incomingNodes.Count(); ++i)
	{
		for(int j=0;j<layerNodes.Count();++j)
		{
			INode* layerNode = layerNodes[j];
			if(incomingNodes[i]==layerNode)
			{
				nodes.Append(1,&layerNode); //node has layer and is selected!
				break;
			}
		}
	}

	//go through and get all of the
	existingLayers.ZeroCount();
	for(int i=0;i<nodes.Count();++i)
	{
		LayerControlEnum ctrlEnum;
		if(nodes[i])
			nodes[i]->EnumAnimTree(&ctrlEnum,NULL,0);
		for(int j=0; j<ctrlEnum.animData.Count(); ++j)
		{
			LayerControlEnum::AnimData data;
			data = ctrlEnum.animData[j];
			Animatable *anim = data.anim;
			LayerControl *lC = static_cast<LayerControl *>(anim);
			LCInfo info;
			info.node = nodes[i];
			info.lC = lC;
			info.client = data.client;
			info.subNum = data.subNum;
			existingLayers.Append(1,&info);
		}
	}
}



class StopLayerEnum : public AnimEnum {
public:	
	BOOL badChild;
	StopLayerEnum() : AnimEnum(SCOPE_ALL)
	{
		badChild = FALSE;
	}

	int proc(Animatable *anim, Animatable *client, int subNum)
	{
		if(IsLayerControl(anim))
		{
			badChild = TRUE;
			return ANIM_ENUM_ABORT;
		}
		return ANIM_ENUM_PROCEED;
	}
};

void MasterLayerControlManager::MakeLayerControl(TrackViewPick res,LayerControl *existingLayer,INode *node)
{

	//create the new layer
	LayerControl *lC = BuildLayerControl(res);
	//clone the existing, it becomes the base layer.
	Control *current = static_cast<Control *> (res.anim);
	Control *newClone = current; //dont clone, in case we have wires, expose tm, mixer, etc..//static_cast<Control *>(current->Clone());
	//set the base as the first layer.

	//now we need to sync up the layer and it's masters, if they exist, otherwis,make sure the layer has the same number of controls.
	int active =0;
	if(existingLayer==NULL) //no existing layer so we need to make a masterlayercontrol, set it up
	{
		MasterLayerControl *mLC = NULL;
		if(GetNumMLCs()==0) //no layers created yet, need to create the base layer.
		{
			mLC = static_cast<MasterLayerControl*>(GetCOREInterface()->CreateInstance(
			CTRL_USERTYPE_CLASS_ID,MASTERLAYER_CONTROL_CLASS_ID));
			TSTR baseName = GetString(IDS_BASE_LAYER);
			baseName =_T("Base Layer");
			mLC->SetName(baseName);
			mLC->addDefault = false;
			AppendMLC(mLC);
		}
		else
			mLC = GetNthMLC(0); //first is always Base!
		
		//set that layer to point back to that new mLC.
		int which = 0;
		if(theHold.Holding())
		{
			MasterLayerControlRestore *mLCRestore = new MasterLayerControlRestore(mLC);
			theHold.Put(mLCRestore); //need to have the restore before we add it
		}
		mLC->AppendLayerControl(lC); //make sure whatever lay
		mLC->AddNode(node);
		lC->AddLayer(newClone,which);
	}
	else
	{
		//need to add the LC to the mLC's it belongs too.
		MasterLayerControl* mLC = NULL;
		for(int j=0;j<existingLayer->mLCs.Count();++j)
		{
			int which = existingLayer->mLCs[j];
			mLC = GetNthMLC(which);
			if(mLC==NULL)
				return; //not good.

			if(theHold.Holding())
			{
				MasterLayerControlRestore *mLCRestore = new MasterLayerControlRestore(mLC);
				theHold.Put(mLCRestore); //need to restore before we add it so it happens last on undo
			}

			mLC->AppendLayerControl(lC);
			mLC->AddNode(node);

			//create the controllers...
			Control* newestClone = NULL;
			if(j==0 && which==0)
			{
				newestClone = newClone;
			}
			else
			{
				if(mLC->addDefault)
				{
					newestClone = mLC->GetDefaultControl(newClone->SuperClassID());
				}
				else
				{
					newestClone = static_cast<Control*>(newClone->Clone());
					DeleteAllKeys(static_cast<Animatable*>(newestClone));
				}
			}
			lC->AddLayer(newestClone,which);
			active = existingLayer->active;
		}
	}
	//do this last in order to not break constraints or other controllers that expect to be in the layer first,
	//before their parent is remoev
	
	if (theHold.Holding())
		theHold.Put(new AssignControllerRestore(0));
	GetCOREInterface7()->SuspendEditing(1<<TASK_MODE_MOTION | 1<<TASK_MODE_HIERARCHY);
	res.client->AssignController(lC,res.subNum);
	GetCOREInterface7()->ResumeEditing(1<<TASK_MODE_MOTION | 1<<TASK_MODE_HIERARCHY);
	if (theHold.Holding())
		theHold.Put(new AssignControllerRestore(1));

	lC->SetLayerActiveInternal(active);

}

BOOL MasterLayerControlManager::CanEnableAnimLayer(ReferenceTarget *anim, ReferenceTarget *client,int subNum,INode *node,XMLAnimTreeEntryList *list)
{
	if(anim==NULL||client==NULL||subNum<0||subNum>=client->NumSubs()||
		anim!=client->SubAnim(subNum)||node==NULL||list==NULL)
		return FALSE;

	if(GetControlInterface(anim)==NULL)
		return FALSE;
	if(anim->SuperClassID()==CTRL_MATRIX3_CLASS_ID)
		return FALSE;
	if(IsLayerControl(anim))
		return FALSE;

	//if isn't leaf????? not doing here

	MasterLayerControlManager* man = GetMLCMan();

	Tab<int> relatives;

	for(int j=0;j<list->Count();++j)
	{
		XMLAnimTreeEntry *entry = (*(list))[j];
		if(anim==entry->GetAnim())
		{
			//need to check 3 things..parent or child's aren't layers and it's not a pos, rot or scale leaf!
			if(entry->IsType(XMLAnimTreeEntry::ePos))
			{
				if(node->GetTMController() &&node->GetTMController()->GetPositionController()!=anim)
					return FALSE;
			}
			if(entry->IsType(XMLAnimTreeEntry::eRot))
			{
				if(node->GetTMController() &&node->GetTMController()->GetRotationController()!=anim)
					return FALSE;
			}
			if(entry->IsType(XMLAnimTreeEntry::eScale))
			{
				if(node->GetTMController() &&node->GetTMController()->GetScaleController()!=anim)
					return FALSE;
			}
			relatives.ZeroCount();
			list->FindChildren(j,relatives);
			for(int k=0;k<relatives.Count();++k)
			{
				XMLAnimTreeEntry *relative = (*(list))[k];
				if(relative->GetAnim()&&IsLayerControl(relative->GetAnim()))
					return FALSE;
			}
			relatives.ZeroCount();
			list->FindParents(j,relatives);
			for(int k=0;k<relatives.Count();++k)
			{
				XMLAnimTreeEntry *relative = (*(list))[k];
				if(relative->GetAnim()&&IsLayerControl(relative->GetAnim()))
					return FALSE;
			}
			break;
		}
	}

	return TRUE;
}




BOOL MasterLayerControlManager::CanEnableAnimLayer(ReferenceTarget *anim, ReferenceTarget *client, int subNum)
{

	if( anim == NULL        || 
		client == NULL      ||
		subNum < 0          ||
		subNum >= client->NumSubs() || 
		anim != client->SubAnim(subNum) )
	{
		return FALSE;
	}

	GetNodesProc dep;
	anim->DoEnumDependents (&dep);
	ILoadSaveAnimation *animation = static_cast<ILoadSaveAnimation*>(GetCOREInterface(ILOADSAVEANIMATION_INTERFACE));
	Tab<NodeAndAnims> nodeAnims;
	Tab<INode*> oneAtTime;

	for(int i =0; i < dep.nodes.Count(); ++i)
	{
		INode *node = dep.nodes[i];
		if(node)
		{
			oneAtTime.ZeroCount();
			oneAtTime.Append(1,&node);
		    animation->SetUpAnimsForSave(oneAtTime,FALSE,TRUE,FALSE,nodeAnims);
			XMLAnimTreeEntryList *list = nodeAnims[0].GetList();
			if(list==NULL)
				return FALSE;
			BOOL can  = CanEnableAnimLayer(anim,client,subNum,node,list);
			if(can==FALSE)
				return FALSE;
		}
	}
	return TRUE;
}



BOOL MasterLayerControlManager::EnableAnimLayer(ReferenceTarget *anim, ReferenceTarget *client, int subNum)
{

	if(CanEnableAnimLayer(anim,client,subNum)==FALSE)
		return FALSE;

	//okay now we have a good control. w need to see if there are existing layers
	//on that node already
	GetNodesProc dep;
	anim->DoEnumDependents (&dep);

	if(dep.nodes.Count()<=0)
		return FALSE;
	INode *node = dep.nodes[0];
	ILoadSaveAnimation *animation = static_cast<ILoadSaveAnimation*>(GetCOREInterface(ILOADSAVEANIMATION_INTERFACE));

	Tab<NodeAndAnims> nodeAnims;
	LayerControl * existingLayer = NULL;
	Tab<INode *> oneAtTime;

	oneAtTime.Append(1,&node);
    animation->SetUpAnimsForSave(oneAtTime,FALSE,TRUE,FALSE,nodeAnims);
	
	if(theHold.Holding())
		theHold.Put(new NotifySelectionRestore());	//go through and get all of the
	for(int i=0;i<nodeAnims.Count();++i)
	{
		for(int j=0;j<nodeAnims[i].GetList()->Count();++j)
		{
			XMLAnimTreeEntry *entry = (*(nodeAnims[i].GetList()))[j];
			Animatable *anim = entry->GetAnim();
				//if layer save it and continue
			if(IsLayerControl(anim)==true)
			{
				existingLayer = static_cast<LayerControl *>(anim);
				break; //okay we have an existing layer, use that
			}
		}
	}
	TrackViewPick res;
	res.anim = anim; res.client = client; res.subNum = subNum;
	MakeLayerControl(res,existingLayer,node);
	NotifySelectionChanged();

	if(theHold.Holding())
		theHold.Put(new NotifySelectionRestore());
	return TRUE;
}


LayerControl* MasterLayerControlManager::BuildLayerControl(TrackViewPick res)
{
	LayerControl *list=NULL;
	if ((res.anim->SuperClassID() == CTRL_FLOAT_CLASS_ID) )
	{
		list = static_cast<LayerControl*>(GetCOREInterface()->CreateInstance(
		CTRL_FLOAT_CLASS_ID,
		(FLOATLAYER_CONTROL_CLASS_ID)));
	}
	else if ((res.anim->SuperClassID() == CTRL_POSITION_CLASS_ID) )
	{
		list = static_cast<LayerControl*>(GetCOREInterface()->CreateInstance(
		CTRL_POSITION_CLASS_ID,
		(POSLAYER_CONTROL_CLASS_ID)));
	}
	else if ((res.anim->SuperClassID() == CTRL_POINT3_CLASS_ID) )
	{
		list = static_cast<LayerControl*>(GetCOREInterface()->CreateInstance(
		CTRL_POINT3_CLASS_ID,
		(POINT3LAYER_CONTROL_CLASS_ID)));
	}

	else if ((res.anim->SuperClassID() == CTRL_ROTATION_CLASS_ID) )
	{
		list = static_cast<LayerControl*>(GetCOREInterface()->CreateInstance(
		CTRL_ROTATION_CLASS_ID,
		(ROTLAYER_CONTROL_CLASS_ID)));
	}
	else if ((res.anim->SuperClassID() == CTRL_SCALE_CLASS_ID) )
	{
		list = static_cast<LayerControl*>(GetCOREInterface()->CreateInstance(
		CTRL_SCALE_CLASS_ID,
		(SCALELAYER_CONTROL_CLASS_ID)));
	}
	return list;
}      


void MasterLayerControlManager::GetActiveLayersNodes(Tab<INode *> &nodeTab,Tab<int> &layers)
{
	layers.ZeroCount();
	Tab<MasterLayerControlManager::LCInfo> layerControls;
	GetLayerControls(nodeTab,layerControls);
	BitArray bArray;
	bArray.SetSize(GetNumMLCs());
	
	for(int i=0;i<layerControls.Count();++i)
	{
		LayerControl *lC = layerControls[i].lC;
		if(lC)
		{
			int active = lC->mLCs[lC->GetLayerActive()];
			bArray.Set(active);
		}
	}
	for(int i=0;i<GetNumMLCs();++i)
	{
		if(bArray[i])
			layers.Append(1,&i);
	}
}

void MasterLayerControlManager::SelectNodesFromActive()
{
	if(activeLayers.Count()==1)
	{
		Tab<INode *> nodeTab;
		GetNodesActiveLayer(nodeTab);
		GetCOREInterface()->ClearNodeSelection(FALSE);
		GetCOREInterface()->SelectNodeTab(static_cast<INodeTab&>(nodeTab),TRUE,TRUE);

	}
}

bool MasterLayerControlManager::ShowSelectNodesFromActive()
{
	if(activeLayers.Count()==1)
		return TRUE;
	return FALSE;

}
void MasterLayerControlManager::GetNodesActiveLayer(Tab<INode *> &nodeTab)
{
	nodeTab.ZeroCount();
	MasterLayerControl* mLC = NULL;
	int newFlagBit =-1;
	for(int i=0;i<activeLayers.Count();++i)
	{
		mLC = GetNthMLC(activeLayers[i]);
		if(mLC)
		{
			for(int j=0;j<mLC->monitorNodes.Count();++j)
			{
				if(mLC->monitorNodes[j])
				{
					INodeMonitor *nm = static_cast<INodeMonitor *>(mLC->monitorNodes[j]->GetInterface(IID_NODEMONITOR));
					INode *newNode = nm->GetNode();
					if(newFlagBit!=-1)
					{
						if(newNode&&(!newNode->TestFlagBit(newFlagBit)))
					{
							newNode->SetFlagBit(newFlagBit);
							nodeTab.Append(1,&newNode);
						}
					}
					else if(newNode)
						{
						//first item...
						newFlagBit = Animatable::RequestFlagBit();
						Animatable::ClearFlagBitInAllAnimatables(newFlagBit);
						newNode->SetFlagBit(newFlagBit);
						nodeTab.Append(1,&newNode);
					}
						}
					}
				}
			}

	//release the flag
	if(newFlagBit!=-1)
	Animatable::ReleaseFlagBit(newFlagBit);

}
//get all of the nodes that have layers on them.
void MasterLayerControlManager::GetAllNodes(Tab<INode *>&nodeTab)
			{
	nodeTab.ZeroCount();
	MasterLayerControl* mLC = NULL;
	int newFlagBit =-1;
	for(int i=0;i<GetNumMLCs();++i)
	{
		mLC = GetNthMLC(i);
		if(mLC)
		{
			for(int j=0;j<mLC->monitorNodes.Count();++j)
			{
				if(mLC->monitorNodes[j])
				{
					INodeMonitor *nm = static_cast<INodeMonitor *>(mLC->monitorNodes[j]->GetInterface(IID_NODEMONITOR));
					INode *newNode = nm->GetNode();
	                                if(newFlagBit!=-1)
					{
						if(newNode&&(!newNode->TestFlagBit(newFlagBit)))
						{
							newNode->SetFlagBit(newFlagBit);
							nodeTab.Append(1,&newNode);
						}
					}
					else if(newNode)
					{
						//first item...
						newFlagBit = Animatable::RequestFlagBit();
						Animatable::ClearFlagBitInAllAnimatables(newFlagBit);
						newNode->SetFlagBit(newFlagBit);
				nodeTab.Append(1,&newNode);
			}
		}
	}

}
	}
	//release the flag
	if(newFlagBit!=-1)
	Animatable::ReleaseFlagBit(newFlagBit);
}

void MasterLayerControlManager::CopyLayerNodes(int index, Tab<INode *>&nodeTab)
{
	if(index<0)
		return;
	Tab<MasterLayerControlManager::LCInfo> existingLayers;
	MasterLayerControl *mLC = GetNthMLC(index);
	LayerControl *lC1,*lC2;
	if(mLC)
	{

		GetLayerControls(nodeTab, existingLayers);
		for(int i=mLC->layers.Count()-1;i>-1;--i)
		{
			lC1 = mLC->layers[i];
			if(lC1)
			{
				for(int j=0;j<existingLayers.Count();++j)
				{
					lC2 = existingLayers[j].lC;
					if(lC1==lC2)
					{
						for(int k=0;k<lC1->mLCs.Count();++k)
						{
							if(lC1->mLCs[k] ==index)
							{
								lC1->CopyLayerInternal(k);
								break;
							}
						}
						break;
					}
				}
			}
		}
	}
	NotifySelectionChanged();
}



void MasterLayerControlManager::PasteLayerNodes(int index, Tab<INode *>&nodeTab)
{

	if(theHold.Holding())
		theHold.Put(new NotifySelectionRestore());


	bool layerWasAdded = false;
	if(index==-1)
	{
		//this whole section of code will 
		int count = GetNumMLCs();
		TCHAR *layerName = GetString(IDS_ANIMLAYER);
		TCHAR digitPart[16];
		if(count<9)
			_stprintf(digitPart,"0%d",count);
		else
			_stprintf(digitPart,"%d",count);
	
		TSTR newName = TSTR(layerName) + TSTR(digitPart);

		IAnimLayerControlManager * IAnimMan = static_cast<IAnimLayerControlManager*>(GetCOREInterface(IANIMLAYERCONTROLMANAGER_INTERFACE ));
		IAnimMan->AddLayer(newName,nodeTab,false);
		layerWasAdded = true;
		index =  GetNumMLCs()-1;
	}


	Tab<MasterLayerControlManager::LCInfo> existingLayers;
	LayerControl* lC1 = NULL;
	GetLayerControls(nodeTab, existingLayers);


	for(int j=0;j<existingLayers.Count();++j)
	{
		lC1 = existingLayers[j].lC;
		if(lC1)
			lC1->PasteLayerInternal(index);
	}
	if(layerWasAdded)
	{
		RenameLayer(GetCOREInterface()->GetMAXHWnd(), index);
	}
	if(theHold.Holding())
		theHold.Put(new NotifySelectionRestore());

	NotifySelectionChanged();

}


void MasterLayerControlManager::CollapseLayerNodes(int index, Tab<INode *> &nodeTab)
{

	if(index<=0)
		return;
	Tab<MasterLayerControlManager::LCInfo> existingLayers;
	MasterLayerControl *mLC = GetNthMLC(index);
	LayerControl *lC1,*lC2;
	BOOL somethingCollapsed = FALSE;
	if(mLC&&mLC->mute==false)
	{
		if(theHold.Holding())
			theHold.Put(new NotifySelectionRestore());
		MasterLayerControlRestore *mLCRestore = new  MasterLayerControlRestore(mLC);

		GetLayerControls(nodeTab, existingLayers);
		for(int i=mLC->layers.Count()-1;i>-1;--i)
		{
			lC1 = mLC->layers[i];
			if(lC1)
			{
				for(int j=0;j<existingLayers.Count();++j)
				{
					lC2 = existingLayers[j].lC;
					if(lC1==lC2)
					{
						for(int k=0;k<lC1->mLCs.Count();++k)
						{
							if(lC1->mLCs[k] ==index)
							{
								somethingCollapsed = TRUE;
								lC1->CollapseLayerInternal(k);
								break;
							}
						}
						mLC->DeleteReference(i+1);
						mLC->layers.Delete(i,1);
						break;
					}
				}
			}
		}
		int j;
		for(int i=0;i<nodeTab.Count();++i)
		{
			INode *node = nodeTab[i];
			for(j=mLC->monitorNodes.Count()-1;j>-1;--j)
			{
				if(mLC->monitorNodes[j])
				{
					INodeMonitor *nm = static_cast<INodeMonitor *>(mLC->monitorNodes[j]->GetInterface(IID_NODEMONITOR));
					if(nm->GetNode()==node)
						break;
				}
			}
			if(j>-1)
			{
				mLC->DeleteReference(1 +mLC->layers.Count()+j);
				mLC->monitorNodes.Delete(j,1);
			}
		}
		if(theHold.Holding())
		{
			theHold.Put(mLCRestore);
		}
		else
		{
			delete mLCRestore;
			mLCRestore = NULL;
		}
		if(mLC->layers.Count()==0)
		{
			DeleteNthMLC(index);
			ResetLayerMLCs(index);

		}
		if(somethingCollapsed)
		{
			
			BroadcastNotification(NOTIFY_ANIM_LAYERS_DISABLED,(void*)&nodeTab);
		}
		if(theHold.Holding())
			theHold.Put(new NotifySelectionRestore());
		
	
	}
	NotifySelectionChanged();

}

/**********************************************************

	IAnimLayerControlManager_Imp

************************************************************/



class IAnimLayerControlManager_Imp : public IAnimLayerControlManager 
{

public:
        
    static IAnimLayerControlManager& GetInstance() { return _theInstance; }

	enum {kShowAnimLayersManagerToolbar,kEnableAnimLayersDlg , kEnableAnimLayers_FPS, kCanEnableAnimLayer,kEnableAnimLayer, kGetLayerCount,kGetNodesLayers,
	kSetLayerActive, kSetLayerActiveNodes, kGetActiveLayersNodes,kGetNodesActiveLayer,
	kAddLayer,kAddLayerDlg,kDeleteLayer, kDeleteLayerNodes, kCopyLayerNodes,
	kPasteLayerNodes,kSetLayerName,kGetLayerName, kGetLayerWeight,
	kSetLayerWeight,  kSetLayerMute, kGetLayerMute,kSetLayerOutputMute, kGetLayerOutputMute,kCollapseLayerNodes,kDisableLayerNodes,
	kSetFilterActiveOnlyTrackView, kGetFilterActiveOnlyTrackView, kSetJustUpToActive,
	kGetJustUpToActive,kAnimLayerPropertiesDlg,
	kSetCollapseControllerType,kGetCollapseControllerType,
	kSetCollapsePerFrame,kGetCollapsePerFrame,
	kSetCollapsePerFrameActiveRange, kGetCollapsePerFrameActiveRange,
	kSetCollapseRange,kGetCollapseRange,kEnum,kSetLayerWeightControl,
	kGetLayerWeightControl};


	BEGIN_FUNCTION_MAP		        
		VFN_1(kShowAnimLayersManagerToolbar,ShowAnimLayersManagerToolbar,TYPE_bool);
		FN_1(kEnableAnimLayersDlg,TYPE_INT, EnableAnimLayersDlg,TYPE_INODE_TAB_BR);
		FN_10(kEnableAnimLayers_FPS, TYPE_INT,EnableAnimLayers_FPS,TYPE_INODE_TAB_BR,
			TYPE_BOOL,TYPE_BOOL,TYPE_BOOL,TYPE_BOOL,TYPE_BOOL,
			TYPE_BOOL,TYPE_BOOL,TYPE_BOOL,TYPE_BOOL);
		
		FN_3(kCanEnableAnimLayer,TYPE_BOOL, CanEnableAnimLayer,TYPE_REFTARG,TYPE_REFTARG,TYPE_INDEX);
		FN_3(kEnableAnimLayer,TYPE_BOOL, EnableAnimLayer,TYPE_REFTARG,TYPE_REFTARG,TYPE_INDEX);


		FN_0(kGetLayerCount,TYPE_INT,GetLayerCount);
		FN_1(kGetNodesLayers, TYPE_INDEX_TAB_BV,GetNodesLayers_FPS, TYPE_INODE_TAB_BR);

		VFN_1(kSetLayerActive,  SetLayerActive,TYPE_INDEX);
		VFN_2(kSetLayerActiveNodes,  SetLayerActiveNodes,TYPE_INDEX,TYPE_INODE_TAB_BR);
	
		FN_1(kGetActiveLayersNodes, TYPE_INDEX_TAB_BV,GetActiveLayersNodes_FPS,TYPE_INODE_TAB_BR);

		VFN_1(kGetNodesActiveLayer,GetNodesActiveLayer,TYPE_INODE_TAB_BR);

		VFN_3(kAddLayer,AddLayer,TYPE_TSTR_BR,TYPE_INODE_TAB_BR,TYPE_bool);
		VFN_1(kAddLayerDlg,AddLayerDlg,TYPE_INODE_TAB_BR);

		VFN_1(kDeleteLayer,  DeleteLayer,TYPE_INDEX);
		VFN_2(kDeleteLayerNodes,  DeleteLayerNodes,TYPE_INDEX,TYPE_INODE_TAB_BR);

		VFN_2(kCopyLayerNodes,  CopyLayerNodes,TYPE_INDEX,TYPE_INODE_TAB_BR);

		VFN_2(kPasteLayerNodes, PasteLayerNodes,TYPE_INDEX,TYPE_INODE_TAB_BR);

		VFN_2(kSetLayerName,  SetLayerName,TYPE_INDEX,TYPE_TSTR_BV);
		FN_1(kGetLayerName,	 TYPE_TSTR_BV,	GetLayerName,		TYPE_INDEX	);
		FN_2(kGetLayerWeight,	 TYPE_FLOAT,GetLayerWeight,TYPE_INDEX,TYPE_TIMEVALUE);
		VFN_3(kSetLayerWeight,  SetLayerWeight,TYPE_INDEX,TYPE_TIMEVALUE,TYPE_FLOAT);
		FN_1(kGetLayerWeightControl,	 TYPE_CONTROL,GetLayerWeightControl,TYPE_INDEX);
		FN_2(kSetLayerWeightControl,  TYPE_bool,SetLayerWeightControl,TYPE_INDEX,TYPE_CONTROL);


		VFN_2(kSetLayerMute,  SetLayerMute,TYPE_INDEX,TYPE_bool);
		FN_1(kGetLayerMute,	 TYPE_bool,	GetLayerMute,		TYPE_INDEX	);
		VFN_2(kSetLayerOutputMute,  SetLayerOutputMute,TYPE_INDEX,TYPE_bool);
		FN_1(kGetLayerOutputMute,	 TYPE_bool,	GetLayerOutputMute,		TYPE_INDEX	);
		
		VFN_2(kCollapseLayerNodes,CollapseLayerNodes, TYPE_INDEX,TYPE_INODE_TAB_BR);
		VFN_1(kDisableLayerNodes,DisableLayerNodes, TYPE_INODE_TAB_BR);

		VFN_0(kAnimLayerPropertiesDlg,AnimLayerPropertiesDlg);

		VFN_1 (kSetCollapseControllerType,SetCollapseControllerType_FPS,TYPE_ENUM);
		FN_0 (kGetCollapseControllerType,TYPE_ENUM,GetCollapseControllerType_FPS);

		VFN_1 (kSetCollapsePerFrame,SetCollapsePerFrame,TYPE_bool);
		FN_0 (kGetCollapsePerFrame,TYPE_bool,GetCollapsePerFrame);

		VFN_1 (kSetCollapsePerFrameActiveRange,SetCollapsePerFrameActiveRange,TYPE_bool);
		FN_0 (kGetCollapsePerFrameActiveRange,TYPE_bool,GetCollapsePerFrameActiveRange);
	
		VFN_1 (kSetCollapseRange,SetCollapseRange,TYPE_INTERVAL);
		FN_0 (kGetCollapseRange,TYPE_INTERVAL,GetCollapseRange);


		PROP_FNS(kGetFilterActiveOnlyTrackView, GetFilterActiveOnlyTrackView, kSetFilterActiveOnlyTrackView, SetFilterActiveOnlyTrackView, TYPE_bool);
		PROP_FNS(kGetJustUpToActive, GetJustUpToActive, kSetJustUpToActive, SetJustUpToActive, TYPE_bool);




	END_FUNCTION_MAP

	void ShowAnimLayersManagerToolbar(bool show);
	int  EnableAnimLayersDlg(Tab<INode *> &nodeTab);
	int  EnableAnimLayers(Tab<INode *> &nodeTab,DWORD filter);
	int  EnableAnimLayers_FPS(Tab<INode *> &nodeTab, BOOL pos, BOOL rot, BOOL scale,BOOL iK,
		BOOL object, BOOL cA, BOOL mode, BOOL mat, BOOL other);

	BOOL CanEnableAnimLayer(ReferenceTarget *anim, ReferenceTarget *client, int subNum);
	BOOL EnableAnimLayer(ReferenceTarget *anim, ReferenceTarget *client, int subNum);

	int	 GetLayerCount();
	void GetNodesLayers(Tab<INode *> &nodeTab,Tab<int> &layers);
	Tab<int> GetNodesLayers_FPS(Tab<INode *>&nodeTab);

	void SetLayerActive(int index);
	void SetLayerActiveNodes(int index,Tab<INode *> &nodeTab);

	void GetActiveLayersNodes(Tab<INode *> &nodeTab,Tab<int> &layers);
	Tab<int> GetActiveLayersNodes_FPS(Tab<INode *> &nodeTab);
	void GetNodesActiveLayer(Tab<INode *> &nodeTab);

	void AddLayer(TSTR &name, Tab<INode *> &nodeTab, bool useActiveControllerType);
	void AddLayerDlg(Tab<INode *> &nodeTab);

	void DeleteLayer(int index);
	void DeleteLayerNodes(int index,Tab<INode *> &nodeTab);

	void CopyLayer(int index);
	void CopyLayerNodes(int index,Tab<INode *> &nodeTab);

	void PasteLayer(int index);
	void PasteLayerNodes(int index,Tab<INode *> &nodeTab);

	void SetLayerName(int index, TSTR name);
	TSTR GetLayerName(int index);

	float GetLayerWeight(int index,TimeValue t);
	void SetLayerWeight(int index, TimeValue t, float weight);

	Control* GetLayerWeightControl(int index);
	bool SetLayerWeightControl(int index, Control *c);

	bool GetLayerMute(int index);
	void SetLayerMute(int index, bool mute);

	bool GetLayerOutputMute(int index);
	void SetLayerOutputMute(int index, bool mute);

	void CollapseLayerNodes(int index,Tab<INode*> &nodeTab);
	void DisableLayerNodes(Tab<INode*> &nodeTab);

	void SetFilterActiveOnlyTrackView(bool val);
	bool GetFilterActiveOnlyTrackView();
	void SetJustUpToActive(bool v);
	bool GetJustUpToActive();
	void SetUpCustUI(ICustToolbar *toolbar,int id,HWND hWnd, HWND hParent);
	TCHAR *GetComboToolTip();

	void SetCollapseControllerType(IAnimLayerControlManager::ControllerType type);
	IAnimLayerControlManager::ControllerType GetCollapseControllerType();
	void SetCollapseControllerType_FPS(int);
	int GetCollapseControllerType_FPS();
	void SetCollapsePerFrame(bool v);
	bool GetCollapsePerFrame();
	void SetCollapsePerFrameActiveRange(bool v);
	bool GetCollapsePerFrameActiveRange();
	void SetCollapseRange(Interval range);
	Interval GetCollapseRange();
	void AnimLayerPropertiesDlg();


private:

	
    // Declares the constructor of this class. It is private so a single static instance of this class
    // may be instantiated.
     DECLARE_DESCRIPTOR(IAnimLayerControlManager_Imp );
    
    // The single instance of this class
    static IAnimLayerControlManager_Imp  _theInstance;


};

			
class LayerManagerIndexValidator : public FPValidator
{
public:
// validate val for the given param in function in interface
	virtual bool Validate(FPInterface* fpi, FunctionID fid, int paramNum, FPValue& val, TSTR& msg);
};

bool LayerManagerIndexValidator::Validate(FPInterface* fpi, FunctionID fid, int paramNum, FPValue& val, TSTR& msg)
{
	int lowermargin=0;
	int highmargin = ((IAnimLayerControlManager*)fpi)->GetLayerCount();
	switch (fid)
	{
	case IAnimLayerControlManager_Imp::kCopyLayerNodes:
	case IAnimLayerControlManager_Imp::kDeleteLayer:
	case IAnimLayerControlManager_Imp::kDeleteLayerNodes:
	case IAnimLayerControlManager_Imp::kCollapseLayerNodes:
		lowermargin = 1;

		break;
	case IAnimLayerControlManager_Imp::kPasteLayerNodes:
		lowermargin = 1;
		highmargin +=1;
		break;
	
	}
	if(val.i < lowermargin)
	{
		msg.printf(GetString(IDS_MZ_LOWER_INDEX_ERROR), lowermargin+1);
		return false;
	}
	if( val.i >= highmargin)
	{
		msg.printf(GetString(IDS_MZ_UPPER_INDEX_ERROR),((IAnimLayerControlManager*)fpi)->GetLayerCount());
		return false;
	}

	return true;
}


static LayerManagerIndexValidator layerManagerValidator;

//IAnimLayerControlManager
IAnimLayerControlManager_Imp IAnimLayerControlManager_Imp::_theInstance(
#ifndef NO_ANIM_LAYERS
  IANIMLAYERCONTROLMANAGER_INTERFACE, _T("AnimLayerManager"), 0, NULL, FP_CORE,
#else
  IANIMLAYERCONTROLMANAGER_INTERFACE, _T("AnimLayerManager"), 0, NULL, FP_CORE + FP_TEST_INTERFACE,
#endif
  IAnimLayerControlManager_Imp::kShowAnimLayersManagerToolbar,_T("showAnimLayersManagerToolbar"),0, TYPE_VOID, 0 ,1,
    _T("show"), 0, TYPE_bool,
  IAnimLayerControlManager_Imp::kEnableAnimLayersDlg, _T("enableLayersDlg"), 0, TYPE_INT, 0, 1,
  	_T("nodes"),0,TYPE_INODE_TAB_BR,
  IAnimLayerControlManager_Imp::kEnableAnimLayers_FPS, _T("enableLayers"), 0, TYPE_INT, 0, 10,
		_T("nodes"),0,TYPE_INODE_TAB_BR,
		_T("pos"),0,TYPE_BOOL, f_keyArgDefault, TRUE,
		_T("rot"),0,TYPE_BOOL, f_keyArgDefault, TRUE,
		_T("scale"),0,TYPE_BOOL, f_keyArgDefault, TRUE,
		_T("ik"),0,TYPE_BOOL, f_keyArgDefault, FALSE,
		_T("object"),0,TYPE_BOOL, f_keyArgDefault, FALSE,
		_T("customAtt"),0,TYPE_BOOL, f_keyArgDefault, FALSE,
		_T("mod"),0,TYPE_BOOL, f_keyArgDefault, FALSE,
		_T("mat"),0,TYPE_BOOL, f_keyArgDefault, FALSE,
		_T("other"),0,TYPE_BOOL, f_keyArgDefault, FALSE,
 IAnimLayerControlManager_Imp::kCanEnableAnimLayer, _T("canEnableLayer"), 0, TYPE_BOOL, 0, 3,
  	_T("anim"),0,TYPE_REFTARG,
  	_T("client"),0,TYPE_REFTARG,
  	_T("subNum"),0,TYPE_INDEX,
  IAnimLayerControlManager_Imp::kEnableAnimLayer, _T("enableLayer"), 0, TYPE_BOOL, 0, 3,
  	_T("anim"),0,TYPE_REFTARG,
  	_T("client"),0,TYPE_REFTARG,
  	_T("subNum"),0,TYPE_INDEX,
  IAnimLayerControlManager_Imp::kGetLayerCount, _T("getLayerCount"),0,TYPE_INDEX,0,0,
  IAnimLayerControlManager_Imp::kGetNodesLayers, _T("getNodesLayers"),0,TYPE_INDEX_TAB,0,1,
	   	_T("nodes"),0,TYPE_INODE_TAB_BR,
  IAnimLayerControlManager_Imp::kSetLayerActive,		_T("setLayerActive"),	0, TYPE_VOID,	0,	1,
		_T("listIndex"), 0, TYPE_INDEX,f_validator, &layerManagerValidator,
  IAnimLayerControlManager_Imp::kSetLayerActiveNodes,		_T("setLayerActiveNodes"),	0, TYPE_VOID,	0,	2,
		_T("listIndex"), 0, TYPE_INDEX,f_validator, &layerManagerValidator,
 		_T("nodes"),0,TYPE_INODE_TAB_BR,
  
  IAnimLayerControlManager_Imp::kGetActiveLayersNodes,		_T("getActiveLayersNodes"),	0, TYPE_INDEX_TAB,	0,	1,
   	   	_T("nodes"),0,TYPE_INODE_TAB_BR,

  IAnimLayerControlManager_Imp::kGetNodesActiveLayer, _T("getNodesActiveLayer"),0,TYPE_VOID,0,1,
   	   	_T("nodes"),0,TYPE_INODE_TAB_BR,

  IAnimLayerControlManager_Imp::kAddLayer,		_T("addLayer"),		0, TYPE_VOID,	0,	3,
			_T("name"), 0, TYPE_TSTR_BR,
			_T("nodes"),0,TYPE_INODE_TAB_BR,
			_T("useActiveControllerType"),0,TYPE_bool,
  IAnimLayerControlManager_Imp::kAddLayerDlg,		_T("addLayerDlg"),		0, TYPE_VOID,	0,	1,
			_T("nodes"),0,TYPE_INODE_TAB_BR,
   
  IAnimLayerControlManager_Imp::kDeleteLayer,		_T("deleteLayer"),		0, TYPE_VOID,	0,	1,
			_T("listIndex"), 0, TYPE_INDEX,f_validator, &layerManagerValidator,
  IAnimLayerControlManager_Imp::kDeleteLayerNodes,		_T("deleteLayerNodes"),		0, TYPE_VOID,	0,	2,
			_T("listIndex"), 0, TYPE_INDEX,f_validator, &layerManagerValidator,
			_T("nodes"),0,TYPE_INODE_TAB_BR,
 
  IAnimLayerControlManager_Imp::kCopyLayerNodes,		_T("copyLayerNodes"),			0, TYPE_VOID,	0,	2,
			_T("listIndex"), 0, TYPE_INDEX,f_validator, &layerManagerValidator,
			_T("nodes"),0,TYPE_INODE_TAB_BR,
  IAnimLayerControlManager_Imp::kPasteLayerNodes,		_T("pasteLayerNodes"),		0, TYPE_VOID,	0,	2,
			_T("listIndex"), 0, TYPE_INDEX,f_validator, &layerManagerValidator,
			_T("nodes"),0,TYPE_INODE_TAB_BR,


  IAnimLayerControlManager_Imp::kGetLayerName,			_T("getLayerName"),		0, TYPE_TSTR_BV, 0,  1,
			_T("listIndex"), 0, TYPE_INDEX,f_validator, &layerManagerValidator,

  IAnimLayerControlManager_Imp::kSetLayerName,			_T("setLayerName"),		0, TYPE_VOID,   0,  2,
			_T("listIndex"), 0, TYPE_INDEX,f_validator, &layerManagerValidator,
			_T("name"),		 0, TYPE_TSTR_BV,
 	
  IAnimLayerControlManager_Imp::kGetLayerWeight,		_T("getLayerWeight"),		0, TYPE_FLOAT, 0,  2,
			_T("listIndex"), 0, TYPE_INDEX,f_validator, &layerManagerValidator,
			_T("atTime"),0,TYPE_TIMEVALUE,
   IAnimLayerControlManager_Imp::kSetLayerWeight,			_T("setLayerWeight"),		0, TYPE_VOID,   0,  3,
			_T("listIndex"), 0, TYPE_INDEX,f_validator, &layerManagerValidator,
			_T("atTime"),0,TYPE_TIMEVALUE,
			_T("weight"),0,TYPE_FLOAT,

  IAnimLayerControlManager_Imp::kGetLayerWeightControl,		_T("getLayerWeightControl"),		0, TYPE_CONTROL, 0,  1,
			_T("listIndex"), 0, TYPE_INDEX,f_validator, &layerManagerValidator,
   IAnimLayerControlManager_Imp::kSetLayerWeight,			_T("setLayerWeightControl"),		0, TYPE_bool,   0,  2,
			_T("listIndex"), 0, TYPE_INDEX,f_validator, &layerManagerValidator,
			_T("control"),0,TYPE_CONTROL,

  IAnimLayerControlManager_Imp::kGetLayerMute,			_T("getLayerMute"),		0, TYPE_bool, 0,  1,
			_T("listIndex"), 0, TYPE_INDEX,f_validator, &layerManagerValidator,

  IAnimLayerControlManager_Imp::kSetLayerMute,			_T("setLayerMute"),		0, TYPE_VOID,   0,  2,
			_T("listIndex"), 0, TYPE_INDEX,f_validator, &layerManagerValidator,
			_T("mute"),		 0, TYPE_bool,


  IAnimLayerControlManager_Imp::kGetLayerOutputMute,			_T("getLayerOutputMute"),		0, TYPE_bool, 0,  1,
			_T("listIndex"), 0, TYPE_INDEX,f_validator, &layerManagerValidator,

  IAnimLayerControlManager_Imp::kSetLayerOutputMute,			_T("setLayerOutputMute"),		0, TYPE_VOID,   0,  2,
			_T("listIndex"), 0, TYPE_INDEX,f_validator, &layerManagerValidator,
			_T("mute"),		 0, TYPE_bool,

  IAnimLayerControlManager_Imp::kCollapseLayerNodes,		_T("collapseLayerNodes"),			0, TYPE_VOID,	0,	2,
			_T("listIndex"), 0, TYPE_INDEX,f_validator, &layerManagerValidator,
			_T("nodes"),0,TYPE_INODE_TAB_BR,
  IAnimLayerControlManager_Imp::kDisableLayerNodes,		_T("disableLayerNodes"),			0, TYPE_VOID,	0,	1,
			_T("nodes"),0,TYPE_INODE_TAB_BR,


  IAnimLayerControlManager_Imp::kAnimLayerPropertiesDlg, _T("animLayerPropertiesDlg"), 0, TYPE_VOID,0,0,

  IAnimLayerControlManager_Imp::kSetCollapseControllerType,_T("SetCollapseControllerType"), 0 , TYPE_VOID, 0 , 1,
		   _T("controllerType"), 0, TYPE_ENUM,IAnimLayerControlManager_Imp::kEnum,
  IAnimLayerControlManager_Imp::kGetCollapseControllerType,_T("GetCollapseControllerType"), 0, TYPE_ENUM,IAnimLayerControlManager_Imp::kEnum, 0,0,

  IAnimLayerControlManager_Imp::kSetCollapsePerFrame,_T("SetCollapsePerFrame"), 0, TYPE_VOID, 0 , 1,
		_T("keyable"), 0 ,TYPE_bool,
  IAnimLayerControlManager_Imp::kGetCollapsePerFrame,_T("GetCollapsePerFrame"), 0, TYPE_bool, 0 ,0,

  IAnimLayerControlManager_Imp::kSetCollapsePerFrameActiveRange,_T("SetCollapsePerFrameActiveRange"), 0, TYPE_VOID, 0, 1,
		_T("activeRange"),0,TYPE_bool,
  IAnimLayerControlManager_Imp::kGetCollapsePerFrameActiveRange,_T("GetCollapsePerFrameActiveRange"),0,TYPE_bool,0,0,

  IAnimLayerControlManager_Imp::kSetCollapseRange,_T("SetCollapseRange"),0, TYPE_VOID, 0 ,1,
		_T("range"), 0, TYPE_INTERVAL,
  IAnimLayerControlManager_Imp::kGetCollapseRange,_T("GetCollapseRange"), 0, TYPE_INTERVAL, 0 ,0,

  properties,
  IAnimLayerControlManager_Imp::kGetFilterActiveOnlyTrackView,IAnimLayerControlManager_Imp::kSetFilterActiveOnlyTrackView,_T("filterActiveOnly"),0,TYPE_bool,
  IAnimLayerControlManager_Imp::kGetJustUpToActive,IAnimLayerControlManager_Imp::kSetJustUpToActive,_T("justUpToActive"),0,TYPE_bool,


	enums,
	IAnimLayerControlManager_Imp::kEnum, 3,
	_T("Bezier"),			(int)IAnimLayerControlManager::eBezier,
	_T("Linear"),			(int)IAnimLayerControlManager::eLinear,
	_T("Default"),			(int)IAnimLayerControlManager::eDefault,
  end 
);

void IAnimLayerControlManager_Imp::ShowAnimLayersManagerToolbar(bool show)
{

	ICUIFrame *pCF = GetCUIFrameMgr()->GetICUIFrame(GetString(IDS_ANIMATION_LAYERS));
	if( pCF ) {

		int isDocked = pCF->GetCurPosition() & CUI_ALL_DOCK;
		pCF->Hide(!show);
		if(isDocked) {
			  GetCUIFrameMgr()->RecalcLayout(TRUE);
			  Sleep(250);
	   }
	}
	GetCUIFrameMgr()->SetMacroButtonStates(FALSE);
}




//uses sEnableAnimFilters struct
int  IAnimLayerControlManager_Imp::EnableAnimLayersDlg(Tab<INode *> &nodeTab)
{
	if (DialogBoxParam(hInstance,MAKEINTRESOURCE(IDD_ENABLE_ANIM_LAYERS),
		GetCOREInterface()->GetMAXHWnd(),enableAnimFiltersProc,0))
	{	
		DWORD flag =0;
		if(sEnableAnimFilters.pos||sEnableAnimFilters.all)
			flag |= static_cast<int>(IAnimLayerControlManager::ePos);
		if(sEnableAnimFilters.rot||sEnableAnimFilters.all)
			flag |= static_cast<int>(IAnimLayerControlManager::eRot);
		if(sEnableAnimFilters.scale||sEnableAnimFilters.all)
			flag |= static_cast<int>(IAnimLayerControlManager::eScale);
		if(sEnableAnimFilters.IK||sEnableAnimFilters.all)
			flag |= static_cast<int>(IAnimLayerControlManager::eIK);
		if(sEnableAnimFilters.object||sEnableAnimFilters.all)
			flag |= static_cast<int>(IAnimLayerControlManager::eObject);
		if(sEnableAnimFilters.CA||sEnableAnimFilters.all)
			flag |= static_cast<int>(IAnimLayerControlManager::eCA);
		if(sEnableAnimFilters.mod||sEnableAnimFilters.all)
			flag |= static_cast<int>(IAnimLayerControlManager::eMod);
		if(sEnableAnimFilters.mat||sEnableAnimFilters.all)
			flag |= static_cast<int>(IAnimLayerControlManager::eMat);
		if(sEnableAnimFilters.other||sEnableAnimFilters.all)
			flag |= static_cast<int>(IAnimLayerControlManager::eOther);
		return EnableAnimLayers(nodeTab,flag);
	}	

	return -1;
}

static void SetupSpin(HWND hDlg, int spinID, int editID, int val)
{
	ISpinnerControl* spin = NULL;

	spin = GetISpinner(GetDlgItem(hDlg, spinID));
	spin->SetLimits(-999999999,999999999, FALSE );
	spin->SetScale( 1.0f );
	spin->LinkToEdit( GetDlgItem(hDlg, editID), EDITTYPE_INT );
	spin->SetValue( val, FALSE );
	ReleaseISpinner( spin );
}

static int GetSpinVal(HWND hDlg, int spinID)
{
	ISpinnerControl *spin = GetISpinner(GetDlgItem(hDlg, spinID));
	int res = spin->GetIVal();
	ReleaseISpinner(spin);
	return res;
}

INT_PTR CALLBACK layerPropertiesProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	MasterLayerControlManager *man =  GetMLCMan();


	 switch (msg)  
	 {
	 case WM_INITDIALOG:
		 {
		  	 PositionWindowNearCursor(hDlg);

			 CheckDlgButton(hDlg, IDC_MUTE,man->justUpToActive);

			 if(man->collapseAllPerFrame==true)
				 CheckDlgButton(hDlg,IDC_PER_KEY,FALSE);
			 else
				 CheckDlgButton(hDlg,IDC_PER_KEY, TRUE);
		
			 if(man->controllerType== IAnimLayerControlManager::eBezier)
				 CheckDlgButton(hDlg,IDC_BEZIER,TRUE);//CheckRadioButton(hDlg,IDC_BEZIER, IDC_DEFAULT, IDC_BEZIER);
			 else if (man->controllerType== IAnimLayerControlManager::eLinear)
				 CheckDlgButton(hDlg,IDC_LINEAR,TRUE);//CheckRadioButton(hDlg,IDC_BEZIER,IDC_DEFAULT, IDC_LINEAR);
			 else
				 CheckDlgButton(hDlg,IDC_DEFAULT,TRUE);		 //CheckRadioButton(hDlg,IDC_BEZIER, IDC_DEFAULT, IDC_DEFAULT);

				
			 if(man->collapsePerFrameActiveRange==true)
 				 CheckRadioButton(hDlg,IDC_CURRENT, IDC_RANGE, IDC_CURRENT);
			 else
  				 CheckRadioButton(hDlg,IDC_CURRENT, IDC_RANGE, IDC_RANGE);

			 SetupSpin(hDlg,IDC_START_FRAME_SPIN,IDC_START_FRAME, man->collapseRange.Start());
			 SetupSpin(hDlg,IDC_END_FRAME_SPIN,IDC_END_FRAME, man->collapseRange.End());

	 }
	 break;
	 case WM_COMMAND:
		 switch (LOWORD(wParam))
		 {
		 
		 case IDOK:


			 if (IsDlgButtonChecked(hDlg,IDC_PER_KEY))
				man->collapseAllPerFrame = false;
			 else
 				man->collapseAllPerFrame = true;

			 if (IsDlgButtonChecked(hDlg,IDC_CURRENT))
				man->collapsePerFrameActiveRange = true;
			 else
				man->collapsePerFrameActiveRange = false;


			 if (IsDlgButtonChecked(hDlg,IDC_BEZIER))
				 man->controllerType = IAnimLayerControlManager::eBezier;
			 else if (IsDlgButtonChecked(hDlg,IDC_LINEAR))
				 man->controllerType = IAnimLayerControlManager::eLinear;
			 else 
				 man->controllerType = IAnimLayerControlManager::eDefault;

			 man->collapseRange.SetStart(GetSpinVal(hDlg,IDC_START_FRAME_SPIN));
			 man->collapseRange.SetEnd(GetSpinVal(hDlg,IDC_END_FRAME_SPIN));
 			 if (IsDlgButtonChecked(hDlg,IDC_MUTE))
				 man->SetJustUpToActive(true);
			 else
				 man->SetJustUpToActive(false);
 			 GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());



			 EndDialog(hDlg,1);
			 break;
		 case IDCANCEL:
			 EndDialog(hDlg,0);
			break;
		 }
		 break;
	 case WM_CLOSE:
		 EndDialog(hDlg, 0);
	 default:
		 return FALSE;
	 }
	 return TRUE;
}


void IAnimLayerControlManager_Imp::AnimLayerPropertiesDlg()
{

	DialogBoxParam(hInstance,MAKEINTRESOURCE(IDD_LAYER_PROPERTIES),
		GetCOREInterface()->GetMAXHWnd(),layerPropertiesProc,0);
		
}



void IAnimLayerControlManager_Imp::AddLayerDlg(Tab<INode *> &nodeTab)
{
	static AddLayerInfo addLayerInfo;
	addLayerInfo.numLayerNames = GetLayerCount()-1;
	if(addLayerInfo.numLayerNames>0)
	{
		for(int i=1;i<=addLayerInfo.numLayerNames;++i)
		{
			TSTR *newName = new TSTR(GetLayerName(i));
			addLayerInfo.layerNames.Append(1,&newName);
		}
	}


	TCHAR *layerName = GetString(IDS_ANIMLAYER);
	TCHAR digitPart[16];
	if(addLayerInfo.layerNames.Count()<9)
		_stprintf(digitPart,"0%d",addLayerInfo.layerNames.Count()+1);
	else
		_stprintf(digitPart,"%d",addLayerInfo.layerNames.Count()+1);

	TSTR *newName = new TSTR( TSTR(layerName) + TSTR(digitPart));
	addLayerInfo.layerNames.Append(1,&newName);

	addLayerInfo.selectedLayer = addLayerInfo.layerNames.Count()-1;
	if (DialogBoxParam(hInstance,MAKEINTRESOURCE(IDD_ADD_LAYER),
		GetCOREInterface()->GetMAXHWnd(),addLayerProc,(LPARAM)&addLayerInfo))
	{
		if(addLayerInfo.selectedLayer<0)
			addLayerInfo.selectedLayer = 0;
		if(addLayerInfo.selectedLayer<addLayerInfo.layerNames.Count())
		{
			TSTR *layerName = addLayerInfo.layerNames[addLayerInfo.selectedLayer];
			if(layerName)
			{
				bool b = (addLayerInfo.duplicateActive==TRUE) ? true :false;
				AddLayer(*layerName,nodeTab,b);		
			}
		}
	}
	for(int i=0;i<addLayerInfo.layerNames.Count();++i)
	{
		if(addLayerInfo.layerNames[i])
		{
			delete addLayerInfo.layerNames[i];
			addLayerInfo.layerNames[i] = NULL;
		}
	}
	addLayerInfo.layerNames.ZeroCount();
}


int IAnimLayerControlManager_Imp::EnableAnimLayers_FPS(Tab<INode *> &nodeTab, BOOL pos, BOOL rot, BOOL scale,BOOL iK,
		BOOL object, BOOL cA, BOOL mod, BOOL mat, BOOL other)
{
	DWORD flag =0;
	if(pos)
		flag |= static_cast<int>(IAnimLayerControlManager::ePos);
	if(rot)
		flag |= static_cast<int>(IAnimLayerControlManager::eRot);
	if(scale)
		flag |= static_cast<int>(IAnimLayerControlManager::eScale);
	if(iK)
		flag |= static_cast<int>(IAnimLayerControlManager::eIK);
	if(object)
		flag |= static_cast<int>(IAnimLayerControlManager::eObject);
	if(cA)
		flag |= static_cast<int>(IAnimLayerControlManager::eCA);
	if(mod)
		flag |= static_cast<int>(IAnimLayerControlManager::eMod);
	if(mat)
		flag |= static_cast<int>(IAnimLayerControlManager::eMat);
	if(other)
		flag |= static_cast<int>(IAnimLayerControlManager::eOther);

	return EnableAnimLayers(nodeTab,flag);
}


void IAnimLayerControlManager_Imp::SetUpCustUI(ICustToolbar *toolbar,int id,HWND hWnd, HWND hParent)
{
	MasterLayerControlManager * man = GetMLCMan();
	man->SetUpCustUI(toolbar,id,hWnd,hParent);
}

TCHAR * IAnimLayerControlManager_Imp::GetComboToolTip()
{
	MasterLayerControlManager * man = GetMLCMan();
	return man->GetComboToolTip();
}

bool MakeSureWeCanMakeLayer(XMLAnimTreeEntryList *list, int item, Tab<TrackViewPick> &newLayersToMake)
{
	Tab<int> parentIndices;
	list->FindParents(item,parentIndices);
	for(int i=0;i<parentIndices.Count();++i)
	{
		XMLAnimTreeEntry *parent = (*list)[parentIndices[i]];
		if(IsLayerControl(parent->GetAnim()))
			return false;
		for(int j=0;j<newLayersToMake.Count();++j)
		{
			if(newLayersToMake[j].anim==parent->GetAnim())
				return false;
		}
	}
	return true;
}

int IAnimLayerControlManager_Imp::EnableAnimLayers(Tab<INode *> &nodeTab,DWORD filter)
{
	Tab<NodeAndAnims> nodeAnims;
	Tab<TrackViewPick> newLayersToMake;
	Tab<LayerControl *>existingLayers;
	Tab<INode *> oneAtTime;
	
	if(theHold.Holding())
		theHold.Put(new NotifySelectionRestore());
	//go through and get all of the
	int numOfLayersMade = 0;;
	ILoadSaveAnimation *animation = static_cast<ILoadSaveAnimation*>(GetCOREInterface(ILOADSAVEANIMATION_INTERFACE));

	HCURSOR oldcur = GetCursor();
    HCURSOR waitcur = LoadCursor(NULL,IDC_WAIT);
    SetCursor(waitcur);

	for(int z=0;z<nodeTab.Count();++z)
	{
		nodeAnims.ZeroCount();
		newLayersToMake.ZeroCount();
		existingLayers.ZeroCount();
		oneAtTime.ZeroCount();
		INode *node = nodeTab[z];
		oneAtTime.Append(1,&node);
        animation->SetUpAnimsForSave(oneAtTime,FALSE,TRUE,FALSE,nodeAnims);
		for(int i=0;i<nodeAnims.Count();++i)
		{
			for(int j=0;j<nodeAnims[i].GetList()->Count();++j)
			{
				XMLAnimTreeEntryList *list = (nodeAnims[i].GetList());
				XMLAnimTreeEntry *entry = (*list)[j];
				Animatable *anim = entry->GetAnim();
					//if layer save it and continue
				if(IsLayerControl(anim)==true)
				{
					LayerControl *lC = static_cast<LayerControl *>(anim);
					existingLayers.Append(1,&lC);
					continue;
				}
				//if bipmaster control.. continue
				if(GetBipMasterInterface(anim))
				{
					continue;
				}
				if(GetControlInterface(anim)!=NULL)
				{
					TrackViewPick res;
					res.anim = static_cast<ReferenceTarget*>(anim);
					res.client = static_cast<ReferenceTarget*>(entry->GetClient());
					res.subNum = entry->GetSubNum();
					
					Control *control = static_cast<Control*>(GetControlInterface(anim));				
					SClass_ID superClass = res.anim->SuperClassID();

					//if parent is the transform controller..
					if(node->GetTMController()==res.client)
					{
						if(superClass == CTRL_POSITION_CLASS_ID && 
						(IAnimLayerControlManager::ePos &filter))
						{
							if(MakeSureWeCanMakeLayer(list,j,newLayersToMake)==true)
							newLayersToMake.Append(1,&res);
							continue;
						}
						else if (superClass == CTRL_ROTATION_CLASS_ID  && 
							(IAnimLayerControlManager::eRot &filter))
						{
							if(MakeSureWeCanMakeLayer(list,j,newLayersToMake)==true)
							newLayersToMake.Append(1,&res);
							continue;
						}
						else if (superClass == CTRL_SCALE_CLASS_ID && 
							(IAnimLayerControlManager::eScale &filter))
						{
							if(MakeSureWeCanMakeLayer(list,j,newLayersToMake)==true)
							newLayersToMake.Append(1,&res);
							continue;
						}
					}

					if( (IAnimLayerControlManager::eObject &filter)&&
					(entry->IsType(XMLAnimTreeEntry::eBaseObject)))
					{
						if(control->IsLeaf())
						{
							if(MakeSureWeCanMakeLayer(list,j,newLayersToMake)==true)
							newLayersToMake.Append(1,&res);
						}
						else if (superClass == CTRL_POSITION_CLASS_ID && 
						(IAnimLayerControlManager::ePos &filter))
						{
							if(MakeSureWeCanMakeLayer(list,j,newLayersToMake)==true)
							newLayersToMake.Append(1,&res);
							continue;
						}
						else if (superClass == CTRL_ROTATION_CLASS_ID  && 
							(IAnimLayerControlManager::eRot &filter))
						{
							if(MakeSureWeCanMakeLayer(list,j,newLayersToMake)==true)
							newLayersToMake.Append(1,&res);
							continue;
						}
						else if (superClass == CTRL_SCALE_CLASS_ID && 
							(IAnimLayerControlManager::eScale &filter))
						{
							if(MakeSureWeCanMakeLayer(list,j,newLayersToMake)==true)
							newLayersToMake.Append(1,&res);
							continue;
						}
					}
					else if( (IAnimLayerControlManager::eMod&filter)&&
					(entry->IsType(XMLAnimTreeEntry::eModObject)))
					{
						if(control->IsLeaf())
						{
							if(MakeSureWeCanMakeLayer(list,j,newLayersToMake)==true)
							newLayersToMake.Append(1,&res);
						}
						else if (superClass == CTRL_POSITION_CLASS_ID && 
						(IAnimLayerControlManager::ePos &filter))
						{
							if(MakeSureWeCanMakeLayer(list,j,newLayersToMake)==true)
							newLayersToMake.Append(1,&res);
							continue;
						}
						else if (superClass == CTRL_ROTATION_CLASS_ID  && 
							(IAnimLayerControlManager::eRot &filter))
						{
							if(MakeSureWeCanMakeLayer(list,j,newLayersToMake)==true)
							newLayersToMake.Append(1,&res);
							continue;
						}
						else if (superClass == CTRL_SCALE_CLASS_ID && 
							(IAnimLayerControlManager::eScale &filter))
						{
							if(MakeSureWeCanMakeLayer(list,j,newLayersToMake)==true)
							newLayersToMake.Append(1,&res);
							continue;
						}
					}
					else if( (IAnimLayerControlManager::eIK&filter)&&
					(entry->IsType(XMLAnimTreeEntry::eIK)))
					{
						if(control->IsLeaf())
						{
							if(MakeSureWeCanMakeLayer(list,j,newLayersToMake)==true)
							newLayersToMake.Append(1,&res);
						}
						else if (superClass == CTRL_POSITION_CLASS_ID && 
						(IAnimLayerControlManager::ePos &filter))
						{
							if(MakeSureWeCanMakeLayer(list,j,newLayersToMake)==true)
							newLayersToMake.Append(1,&res);
							continue;
						}
						else if (superClass == CTRL_ROTATION_CLASS_ID  && 
							(IAnimLayerControlManager::eRot &filter))
						{
							if(MakeSureWeCanMakeLayer(list,j,newLayersToMake)==true)
							newLayersToMake.Append(1,&res);
							continue;
						}
						else if (superClass == CTRL_SCALE_CLASS_ID && 
							(IAnimLayerControlManager::eScale &filter))
						{
							if(MakeSureWeCanMakeLayer(list,j,newLayersToMake)==true)
							newLayersToMake.Append(1,&res);
							continue;
						}
					}
					else if( (IAnimLayerControlManager::eCA&filter)&&
					(entry->IsType(XMLAnimTreeEntry::eCustomAttributes)))
					{
						if(control->IsLeaf())
						{
							if(MakeSureWeCanMakeLayer(list,j,newLayersToMake)==true)
							newLayersToMake.Append(1,&res);
						}
						else if (superClass == CTRL_POSITION_CLASS_ID && 
						(IAnimLayerControlManager::ePos &filter))
						{
							if(MakeSureWeCanMakeLayer(list,j,newLayersToMake)==true)
							newLayersToMake.Append(1,&res);
							continue;
						}
						else if (superClass == CTRL_ROTATION_CLASS_ID  && 
							(IAnimLayerControlManager::eRot &filter))
						{
							if(MakeSureWeCanMakeLayer(list,j,newLayersToMake)==true)
							newLayersToMake.Append(1,&res);
							continue;
						}
						else if (superClass == CTRL_SCALE_CLASS_ID && 
							(IAnimLayerControlManager::eScale &filter))
						{
							if(MakeSureWeCanMakeLayer(list,j,newLayersToMake)==true)
							newLayersToMake.Append(1,&res);
							continue;
						}
					}
					else if( (IAnimLayerControlManager::eMat&filter)&&
						(entry->IsType(XMLAnimTreeEntry::eMatParams)))
					{
						if(control->IsLeaf())
						{
							if(MakeSureWeCanMakeLayer(list,j,newLayersToMake)==true)
							newLayersToMake.Append(1,&res);
						}
						else if (superClass == CTRL_POSITION_CLASS_ID && 
						(IAnimLayerControlManager::ePos &filter))
						{
							if(MakeSureWeCanMakeLayer(list,j,newLayersToMake)==true)
							newLayersToMake.Append(1,&res);
							continue;
						}
						else if (superClass == CTRL_ROTATION_CLASS_ID  && 
							(IAnimLayerControlManager::eRot &filter))
						{
							if(MakeSureWeCanMakeLayer(list,j,newLayersToMake)==true)
							newLayersToMake.Append(1,&res);
							continue;
						}
						else if (superClass == CTRL_SCALE_CLASS_ID && 
							(IAnimLayerControlManager::eScale &filter))
						{
							if(MakeSureWeCanMakeLayer(list,j,newLayersToMake)==true)
							newLayersToMake.Append(1,&res);
							continue;
						}
					}
					else if( (IAnimLayerControlManager::eOther&filter))
					{
						if(control->IsLeaf())
						{
							if(MakeSureWeCanMakeLayer(list,j,newLayersToMake)==true)
							newLayersToMake.Append(1,&res);
						}
						else if (superClass == CTRL_POSITION_CLASS_ID && 
						(IAnimLayerControlManager::ePos &filter))
						{
							if(MakeSureWeCanMakeLayer(list,j,newLayersToMake)==true)
							newLayersToMake.Append(1,&res);
							continue;
						}
						else if (superClass == CTRL_ROTATION_CLASS_ID  && 
							(IAnimLayerControlManager::eRot &filter))
						{
							if(MakeSureWeCanMakeLayer(list,j,newLayersToMake)==true)
							newLayersToMake.Append(1,&res);
							continue;
						}
						else if (superClass == CTRL_SCALE_CLASS_ID && 
							(IAnimLayerControlManager::eScale &filter))
						{
							if(MakeSureWeCanMakeLayer(list,j,newLayersToMake)==true)
							newLayersToMake.Append(1,&res);
							continue;
						}
					}
					
				}
			}
		}
		if(newLayersToMake.Count()>0)
		{
			//okay newLayersToMake has the layersToMake.
			//existingLayers has the existing..
			//node is the node
			LayerControl *existingLayer = NULL;
			if(existingLayers.Count()>0)
				existingLayer = existingLayers[0];
			MasterLayerControlManager * man = GetMLCMan();
			for(int i=0;i<newLayersToMake.Count();++i)
			{
				TrackViewPick &res = newLayersToMake[i];
				//create the new layer
				man->MakeLayerControl(res,existingLayer,node);
				++numOfLayersMade;
			}
		}
	}

	if(numOfLayersMade>0)
	{
		BroadcastNotification(NOTIFY_ANIM_LAYERS_ENABLED,(void*)&nodeTab);
		MasterLayerControlManager * man = GetMLCMan();
		man->NotifySelectionChanged();
		if(theHold.Holding())
			theHold.Put(new NotifySelectionRestore());
		
	}

    SetCursor(oldcur);

	return numOfLayersMade;
}


static void ZeroOut(Control *control)
{
	SuspendAnimate();
	AnimateOff();
	switch(control->SuperClassID())
	{		
		case CTRL_POSITION_CLASS_ID:
		case CTRL_POINT3_CLASS_ID:
			{
				Point3 val = Point3(0,0,0);
				control->SetValue(0,&val);
				break;
			}
		case CTRL_POINT4_CLASS_ID:
			{
				Point4 val = Point4(0,0,0,0);
				control->SetValue(0,&val);
				break;
			}
		case CTRL_FLOAT_CLASS_ID:
			{
				float val = 0.0f;
				control->SetValue(0,&val);
				break;
			}
		case CTRL_SCALE_CLASS_ID:
			{
				ScaleValue val = ScaleValue(Point3(1,1,1));
				control->SetValue(0,&val);
				break;
			}
		case CTRL_ROTATION_CLASS_ID:
			{
				Quat val = Quat(0.0f,0.0f,0.0f,1.0f);
				control->SetValue(0,&val);
				break;
			}
		default:
			break;
	}
	
	ResumeAnimate();


}



void IAnimLayerControlManager_Imp::AddLayer(TSTR &name,Tab<INode *> &nodeTab,bool useActiveControllerType)
{
	Tab<MasterLayerControlManager::LCInfo> existingLayers;
	MasterLayerControlManager* man = GetMLCMan();
	MasterLayerControl *mLC = NULL;
	int index = man->GetNthMLC(name);
	INode* node = NULL;
	Control* newestClone = NULL;
	Tab<INode *> oneAtTime;
	
	
	if(theHold.Holding())
		theHold.Put(new NotifySelectionRestore());
	
	if(index==-1) //no layer exists so we create it and hook it!
	{
		mLC = static_cast<MasterLayerControl*>(GetCOREInterface()->CreateInstance(
		CTRL_USERTYPE_CLASS_ID,MASTERLAYER_CONTROL_CLASS_ID));
		MasterLayerControlRestore *mLCRestore = new MasterLayerControlRestore(mLC);
		mLC->SetName(name);
		man->AppendMLC(mLC);
		index = man->GetNumMLCs()-1;
		if(useActiveControllerType==false)
			mLC->addDefault =true;
		else
			mLC->addDefault = false;
		for(int z=0;z<nodeTab.Count();++z)
		{
			node = nodeTab[z];
			if (node==NULL) continue;
			oneAtTime.ZeroCount();
			existingLayers.ZeroCount();
			oneAtTime.Append(1,&node);
			man->GetLayerControls(oneAtTime, existingLayers);
			for(int i=0;i<existingLayers.Count();++i)
			{
				LayerControl *lC = existingLayers[i].lC;
				if(mLC->addDefault)
					newestClone = mLC->GetDefaultControl(lC->SuperClassID());
				else
				{
					newestClone = static_cast<Control*>(lC->conts[lC->active]->Clone());
					DeleteAllKeys(newestClone);
					ZeroOut(newestClone);
				}
				lC->AddLayer(newestClone,index);
				mLC->AppendLayerControl(lC);
				mLC->AddNode(node);
			}
		}
		if(theHold.Holding())
		{
			theHold.Put(mLCRestore);
		}
		else
		{
			delete mLCRestore;
			mLCRestore = NULL;
		}
	}
	else
	{
		//the layer exists.. all we need to do is to then add it to any nodes in the tab that don't
		mLC = man->GetNthMLC(index);
		MasterLayerControlRestore *mLCRestore = new MasterLayerControlRestore(mLC);
		if(useActiveControllerType==false)
			mLC->addDefault =true;
		else
			mLC->addDefault = false;

		//find the nodes that don't have it!
		int i;
		for(int z = 0;z<nodeTab.Count();++z)
		{
			node = nodeTab[z];
			if(node==NULL) continue;
			for(i=0;i<mLC->monitorNodes.Count();++i)
			{
				if(mLC->monitorNodes[i])
				{
					INodeMonitor *nm = static_cast<INodeMonitor *>(mLC->monitorNodes[i]->GetInterface(IID_NODEMONITOR));
					if(nm->GetNode()==node)
						break;
				}
			}
			if(i==mLC->monitorNodes.Count()) // not found we need to append the mLC!
			{
				oneAtTime.ZeroCount();
				existingLayers.ZeroCount();
				oneAtTime.Append(1,&node);
				man->GetLayerControls(oneAtTime, existingLayers);			
				for(int i=0;i<existingLayers.Count();++i)
				{
					LayerControl *lC = existingLayers[i].lC;
					if(mLC->addDefault)
						newestClone = mLC->GetDefaultControl(lC->SuperClassID());
					else
					{
						newestClone = static_cast<Control*>(lC->conts[lC->active]->Clone());
						DeleteAllKeys(newestClone);
						ZeroOut(newestClone);
					}
					lC->AddLayer(newestClone,index);
					mLC->AppendLayerControl(lC);
					mLC->AddNode(node);
				}
			}
		}
		if(theHold.Holding())
		{
			theHold.Put(mLCRestore);
		}
		else
		{
			delete mLCRestore;
			mLCRestore = NULL;
		}
	}

	if(theHold.Holding())
		theHold.Put(new NotifySelectionRestore());
	man->NotifySelectionChanged();
}




int	 IAnimLayerControlManager_Imp::GetLayerCount()
{
	MasterLayerControlManager * man = GetMLCMan();
	return man->GetNumMLCs();
}

Tab<int> IAnimLayerControlManager_Imp::GetNodesLayers_FPS(Tab<INode *>&nodeTab)
{
	Tab<int> layers;
	GetNodesLayers(nodeTab,layers);
	return layers;
}
void IAnimLayerControlManager_Imp::GetNodesLayers(Tab<INode *> &nodeTab, Tab<int> &layers)
{
	MasterLayerControlManager *man = GetMLCMan();
	Tab<bool> inAll;
	man->GetNodesLayers(nodeTab,layers,inAll);

}

Tab<int> IAnimLayerControlManager_Imp::GetActiveLayersNodes_FPS(Tab<INode *> &nodeTab)
{
	Tab<int> layers;
	GetActiveLayersNodes(nodeTab,layers);
	return layers;
}

void IAnimLayerControlManager_Imp::GetActiveLayersNodes(Tab<INode *> &nodeTab,Tab<int> &layers)
{
	MasterLayerControlManager *man = GetMLCMan();
	man->GetActiveLayersNodes(nodeTab,layers);
}

void IAnimLayerControlManager_Imp::GetNodesActiveLayer(Tab<INode *> &nodeTab)
{
	MasterLayerControlManager *man = GetMLCMan();
	man->GetNodesActiveLayer(nodeTab);
}
BOOL IAnimLayerControlManager_Imp::CanEnableAnimLayer(ReferenceTarget *anim, ReferenceTarget *client, int subNum)
{
   	MasterLayerControlManager * man = GetMLCMan();
	return man->CanEnableAnimLayer(anim,client,subNum);
}

BOOL IAnimLayerControlManager_Imp::EnableAnimLayer(ReferenceTarget *anim, ReferenceTarget *client, int subNum)
{
   	MasterLayerControlManager * man = GetMLCMan();
	return man->EnableAnimLayer(anim,client,subNum);
}

void IAnimLayerControlManager_Imp::SetLayerActive(int index)
{
   	MasterLayerControlManager * man = GetMLCMan();
	man->SetLayerActive(index);

}

void IAnimLayerControlManager_Imp::SetLayerActiveNodes(int index,Tab<INode *> &nodeTab)
{
   	MasterLayerControlManager * man = GetMLCMan();
	man->SetLayerActiveNodes(index,nodeTab);
}

void IAnimLayerControlManager_Imp::DeleteLayer(int index)
{

   	MasterLayerControlManager * man = GetMLCMan();
	if(index==0) //don't delete the base layer
		return;
	man->DeleteNthLayer(index);
}

void IAnimLayerControlManager_Imp::DeleteLayerNodes(int index,Tab<INode *> &nodeTab)
{
	MasterLayerControlManager *man = GetMLCMan();
	man->DeleteNthLayerNodes(index,nodeTab);
	
}


void IAnimLayerControlManager_Imp::CopyLayerNodes(int index,Tab<INode *> &nodeTab)
{
	MasterLayerControlManager *man = GetMLCMan();
	man->CopyLayerNodes(index,nodeTab);
}




void IAnimLayerControlManager_Imp::PasteLayerNodes(int index,Tab<INode *> &nodeTab)
{
	MasterLayerControlManager * man = GetMLCMan();
	man->PasteLayerNodes(index,nodeTab);
   
}



TSTR  IAnimLayerControlManager_Imp::GetLayerName(int index)
{
	MasterLayerControlManager * man = GetMLCMan();
	MasterLayerControl *mLC = man->GetNthMLC(index);
	TSTR name;
	if(mLC)
	{
		name = mLC->GetName();
	}
	return name;
}


void IAnimLayerControlManager_Imp::SetLayerName(int index, TSTR name)
{
	MasterLayerControlManager * man = GetMLCMan();
	man->SetLayerName(index,name);
}

bool  IAnimLayerControlManager_Imp::GetLayerMute(int index)
{
	MasterLayerControlManager * man = GetMLCMan();
	MasterLayerControl *mLC = man->GetNthMLC(index);
	bool mute = false;
	if(mLC)
	{
		mute = mLC->mute;
	}
	return mute;
}


void IAnimLayerControlManager_Imp::SetLayerMute(int index, bool mute)
{
	MasterLayerControlManager * man = GetMLCMan();
	man->SetLayerMute(index,mute);
}

bool  IAnimLayerControlManager_Imp::GetLayerOutputMute(int index)
{
	MasterLayerControlManager * man = GetMLCMan();
	MasterLayerControl *mLC = man->GetNthMLC(index);
	bool mute = false;
	if(mLC)
	{
		mute = mLC->outputMute;
	}
	return mute;
}


void IAnimLayerControlManager_Imp::SetLayerOutputMute(int index, bool mute)
{
	MasterLayerControlManager * man = GetMLCMan();
	man->SetLayerOutputMute(index,mute);
}


float  IAnimLayerControlManager_Imp::GetLayerWeight(int index,TimeValue t)
{
	MasterLayerControlManager * man = GetMLCMan();
	MasterLayerControl *mLC = man->GetNthMLC(index);
	float weight = 0.0f;
	Interval valid = FOREVER;		
	if(mLC)
	{
		weight= mLC->GetWeight(t,valid);
	}
	return weight;
}



void  IAnimLayerControlManager_Imp::SetLayerWeight(int index, TimeValue t, float weight)
{
	MasterLayerControlManager * man = GetMLCMan();
	man->SetLayerWeight(index,t,weight);

}


Control *  IAnimLayerControlManager_Imp::GetLayerWeightControl(int index)
{
	MasterLayerControlManager * man = GetMLCMan();
	return man->GetLayerWeightControl(index);
}



bool  IAnimLayerControlManager_Imp::SetLayerWeightControl(int index, Control *c)
{
	MasterLayerControlManager * man = GetMLCMan();
	return man->SetLayerWeightControl(index,c);
}


void IAnimLayerControlManager_Imp::CollapseLayerNodes(int index,Tab<INode*> &nodeTab)
{

	MasterLayerControlManager * man = GetMLCMan();
	man->CollapseLayerNodes(index,nodeTab);

}

void IAnimLayerControlManager_Imp::DisableLayerNodes(Tab<INode*> &nodeTab)
{

	MasterLayerControlManager * man = GetMLCMan();
	man->DisableLayerNodes(nodeTab);

}


void IAnimLayerControlManager_Imp::SetFilterActiveOnlyTrackView(bool val)
{
	MasterLayerControlManager * man = GetMLCMan();
	man->SetFilterActiveOnlyTrackView(val);
}
bool IAnimLayerControlManager_Imp::GetFilterActiveOnlyTrackView()
{
	MasterLayerControlManager * man = GetMLCMan();
	return man->GetFilterActiveOnlyTrackView();

}

void IAnimLayerControlManager_Imp::SetJustUpToActive(bool v)
{
	MasterLayerControlManager * man = GetMLCMan();
	man->SetJustUpToActive(v);
}



bool IAnimLayerControlManager_Imp::GetJustUpToActive()
{
	MasterLayerControlManager * man = GetMLCMan();
	return man->GetJustUpToActive();
}


void IAnimLayerControlManager_Imp::SetCollapseControllerType(IAnimLayerControlManager::ControllerType type)
{
	MasterLayerControlManager * man = GetMLCMan();
	man->SetCollapseControllerType(type);

}

void IAnimLayerControlManager_Imp::SetCollapseControllerType_FPS(int type)
{
	MasterLayerControlManager * man = GetMLCMan();
	man->SetCollapseControllerType(static_cast<IAnimLayerControlManager::ControllerType>(type));

}

IAnimLayerControlManager::ControllerType IAnimLayerControlManager_Imp::GetCollapseControllerType()
{
	MasterLayerControlManager * man = GetMLCMan();
	return man->GetCollapseControllerType();
}

int IAnimLayerControlManager_Imp::GetCollapseControllerType_FPS()
{
	MasterLayerControlManager * man = GetMLCMan();
	return static_cast<int>(man->GetCollapseControllerType());
}

void IAnimLayerControlManager_Imp::SetCollapsePerFrame(bool v)
{
	MasterLayerControlManager * man = GetMLCMan();
	man->SetCollapsePerFrame(v);
}
bool IAnimLayerControlManager_Imp::GetCollapsePerFrame()
{
	MasterLayerControlManager * man = GetMLCMan();
	return man->GetCollapsePerFrame();
}
void IAnimLayerControlManager_Imp::SetCollapsePerFrameActiveRange(bool v)
{
	MasterLayerControlManager * man = GetMLCMan();
	man->SetCollapsePerFrameActiveRange(v);
}
bool IAnimLayerControlManager_Imp::GetCollapsePerFrameActiveRange()
{
	MasterLayerControlManager * man = GetMLCMan();
	return man->GetCollapsePerFrameActiveRange();
}
void IAnimLayerControlManager_Imp::SetCollapseRange(Interval range)
{
	MasterLayerControlManager * man = GetMLCMan();
	man->SetCollapseRange(range);

}
Interval IAnimLayerControlManager_Imp::GetCollapseRange()
{
	MasterLayerControlManager * man = GetMLCMan();
	return man->GetCollapseRange();
}




/*
snum = trackviews.current.getSelectedSubNum
getSelectedSubNum()
snum = trackviews.current.getSelectedSubNum 1
1
pp = trackviews.current.getParentOfSelected 1
ReferenceTarget:ParamBlock
an = pp[1]
SubAnim:Radius
AnimLayerManager.enableAnimLayer
-- Unknown property: "enableAnimLayer" in <Interface:AnimLayerManager>
AnimLayerManager.enableLayer
enableLayer()
AnimLayerManager.enableLayer an pp snum
false
AnimLayerManager.enableLayer an.controller pp snum
false


*/
//in UI
//Item1=5|208|240|50502|0|19|2|1344276755|ComboBox|
//5 = CTB_OTHER, type keep that, then 208,240, is height and width,
//50502 is id (we have our own50612),134427655 is some style stuff, ComboBox is name (keep that)
//0 is helpID, 19 is orient, and 2 is 'y' in ToolOtherItem
//Item1=5|208|240|50502|0|19|2|1344276755|ComboBox|
#define TOL 2

//id's for these special cases.
#define NO_LAYERS -2
#define MULTIPLE_LAYERS -1

WNDPROC AnimLayerToolbarControl::m_defToolProc = NULL;
WNDPROC AnimLayerToolbarControl::m_defListProc = NULL;
HWND AnimLayerToolbarControl::m_hwnd = NULL;
HWND AnimLayerToolbarControl::m_list = NULL;
int AnimLayerToolbarControl::m_col = -1;
int AnimLayerToolbarControl::m_iconColWidth = 0;
HWND AnimLayerToolbarControl::hSpinHWND=NULL;
HWND AnimLayerToolbarControl::hEditHWND=NULL;

// Constructor
AnimLayerToolbarControl::AnimLayerToolbarControl()
  : m_parent(NULL), m_defWndProc(NULL)
{

	GetCOREInterface()->RegisterTimeChangeCallback(this);
}

AnimLayerToolbarControl::~AnimLayerToolbarControl()
{
	GetCOREInterface()->UnRegisterTimeChangeCallback(this);
	//unsubclass the window
	SetHWND(NULL);
}

void AnimLayerToolbarControl::EnableDisableWeight()
{
	if(IsWindowVisible(m_hwnd)==FALSE)
		return;
	ICustEdit* cEdit = GetICustEdit(hEditHWND);
	if(cEdit==NULL)
		return;
	ISpinnerControl* spin = GetISpinner(hSpinHWND);
	if(spin==NULL)
		return;
	MasterLayerControlManager *man = GetMLCMan();
	if(man->currentLayers.Count()==0||man->activeLayers.Count()!=1)
	{
		spin->SetValue(100.0f,FALSE);
		spin->Disable();
	}
	else
	{
		int sel = man->activeLayers[0];
		float weight = man->GetLayerWeight(sel,GetCOREInterface()->GetTime());
		spin->Enable();
		spin->SetValue(weight*100.0f, 0);
		
		MasterLayerControl *mLC = man->GetNthMLC(sel);
		spin->SetKeyBrackets(mLC->pblock->KeyFrameAtTime(kLayerCtrlWeight, GetCOREInterface()->GetTime(), sel));
	}
	ReleaseICustEdit(cEdit);
	ReleaseISpinner(spin);
}

void AnimLayerToolbarControl::Refresh()
{	
	if(IsWindowVisible(m_hwnd)==FALSE)
		return;


	// reset the content
	SendMessage(m_hwnd, CB_RESETCONTENT, 0, 0);

	// add the layer names to the list
	int sel = 0;
	MasterLayerControlManager *man = GetMLCMan();
	EnableDisableWeight();

	if(man->currentLayers.Count()==0)
	{
		//TCHAR * str = GetString(IDS_ENABLE_LAYERS);
		SendMessage(m_hwnd, CB_INSERTSTRING, 0, (LPARAM)NO_LAYERS);
		SendMessage(m_hwnd, CB_SETCURSEL, 0, 0);
		return;
	}
	if(man->activeLayers.Count()>1)
	{
		SendMessage(m_hwnd, CB_INSERTSTRING, 0,(LPARAM)MULTIPLE_LAYERS );
		SendMessage(m_hwnd, CB_SETCURSEL, 0, 0);
	}

	int i;
	for (i =0;i<man->currentLayers.Count();++i)
	{
		SendMessage(m_hwnd, CB_INSERTSTRING, -1, (LPARAM)man->currentLayers[i]);
		if(man->activeLayers.Count()==1&&man->activeLayers[0]==man->currentLayers[i])
			sel = i;
	}

	// select the current layer
	SendMessage(m_hwnd, CB_SETCURSEL, sel, 0);

	GetCUIFrameMgr()->SetMacroButtonStates(FALSE);

}

void AnimLayerToolbarControl::SetParent(HWND hParent)
{
	if (m_parent && m_defToolProc)
		DLSetWindowLongPtr(m_parent, m_defToolProc);

	m_parent = hParent;

	if (m_parent)
		m_defToolProc = DLSetWindowLongPtr(m_parent, ToolbarMsgProc);
	else
		m_defToolProc = NULL;
}

void AnimLayerToolbarControl::SetListBox(HWND hList)
{
	if (IsWindow(m_list) && m_defListProc)
		DLSetWindowLongPtr(m_list, m_defListProc);

	m_list = hList;

	if (m_list)
	{
		m_defListProc = DLSetWindowLongPtr(m_list, ListBoxControlProc);
		assert(m_defListProc);
	}
	else
		m_defListProc = NULL;
}

void AnimLayerToolbarControl::SetSelected(int which)
{
	int i;
	MasterLayerControlManager *man = GetMLCMan();
	for(i=0;i<man->currentLayers.Count();++i)
	{
		if(which==man->currentLayers[i])
			break;
	}

	SendMessage(m_hwnd, CB_SETCURSEL, i < GetMLCMan()->currentLayers.Count() ? i : -1, 0);

	INodeTab nodeTab;
	GetSelectedNodes(nodeTab);
	theHold.SuperBegin();
	theHold.Begin();
	man->SetLayerActiveNodes(man->currentLayers[which],nodeTab);
	macroRecorder->FunctionCall(_T("AnimLayerManager.setLayerActiveNodes"), 2, 0, mr_int, man->currentLayers[which]+1, mr_sel);
	theHold.Accept(GetString(IDS_SET_ACTIVE));
	theHold.SuperAccept(GetString(IDS_SET_ACTIVE));
}


void AnimLayerToolbarControl::SetHWND(HWND hWnd)
{
	if (m_hwnd && m_defWndProc)
	{
		DLSetWindowLongPtr(m_hwnd, m_defWndProc);
		DLSetWindowLongPtr(m_hwnd, 0);
	}

	m_hwnd = hWnd;

	if (m_hwnd)
	{
		m_defWndProc = DLSetWindowLongPtr(m_hwnd, LayerComboControlProc);
		DLSetWindowLongPtr(m_hwnd, this);
                                                // Win64 Cleanup: Martell
	}
	else
		m_defWndProc = NULL;
}

bool AnimLayerToolbarControl::OnCommand(WPARAM wParam, LPARAM lParam)
{
	return true;
}

TSTR AnimLayerToolbarControl::FormatName(TSTR & name, int nLen2, HWND hDlg)
{
	
 	HFONT font = reinterpret_cast<HFONT>(SendMessage(hDlg, WM_GETFONT, 0, 0));
	if (!font)	return name;

	HDC hDc = GetWindowDC(hDlg);
	if (!hDc)	return name;

	HFONT old = static_cast<HFONT>(SelectObject(hDc, font));

	// get the item
	TSTR m_buf(name);

	SIZE nLen;
	GetTextExtentPoint32(hDc, m_buf.data(), m_buf.Length(), &nLen);

	if (nLen.cx >= nLen2) {
		do
		{
			if (m_buf.Length() <= 5)
				return name;

			int index = m_buf.Length() / 2;

			TSTR str1 = m_buf.Substr(0, index-2);
			TSTR str2 = m_buf.Substr(index+2, m_buf.Length());
			m_buf = str1;
			m_buf += TSTR("...");
			m_buf += str2;

			GetTextExtentPoint32(hDc, m_buf.data(), m_buf.Length(), &nLen);
		}
		while (nLen.cx >= nLen2);
	}

	SelectObject(hDc, old);
	ReleaseDC(hDlg, hDc);

	return m_buf;
}

void AnimLayerToolbarControl::SetComboSize(HWND hCombo)
{	
	RECT rectCombo;
	GetWindowRect(hCombo, &rectCombo);
	const int oriLayerNameWidth = rectCombo.right - rectCombo.left - m_iconColWidth;
	int layerNameWidth = oriLayerNameWidth;

	HFONT font = reinterpret_cast<HFONT>(SendMessage(hCombo, WM_GETFONT, 0, 0));
	if (!font)	return;

	HDC hDc = GetWindowDC(hCombo);
	if (!hDc)	return;
	
	HFONT old = static_cast<HFONT>(SelectObject(hDc, font));


	MasterLayerControlManager *man = GetMLCMan();
	int i;
	for (i =0;i<man->currentLayers.Count();++i)
	{
		TSTR name = man->GetLayerName(i);
		
		SIZE size;
		if (DLGetTextExtent(hDc, name, &size))
		{
			if (size.cx > layerNameWidth)
				layerNameWidth = size.cx;
		}
	}


	HFONT oldFont = static_cast<HFONT>(SelectObject(hDc, old));

	// If the drop down list must be made wider
	if (layerNameWidth > oriLayerNameWidth)
	{
		int width = layerNameWidth + m_iconColWidth + GetSystemMetrics(SM_CXVSCROLL) + GetSystemMetrics(SM_CXBORDER) + 4;
		SendMessage(hCombo, CB_SETDROPPEDWIDTH, width,0);
	}
	else
	{
		int width = rectCombo.right - rectCombo.left;
		SendMessage(hCombo, CB_SETDROPPEDWIDTH, width, 0);
	}

	SelectObject(hDc, oldFont);
	ReleaseDC(hCombo, hDc);
}



void AnimLayerToolbarControl::OnDraw(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	LPDRAWITEMSTRUCT lpdis = (LPDRAWITEMSTRUCT) lParam;

    assert(lpdis->CtlType == ODT_COMBOBOX);
    if (lpdis->itemID == -1)
        return;

    TSTR buf;
    int which = (int)lpdis->itemData;

	RECT rectSub;
	int nOffset = 16;
    int xTop = lpdis->rcItem.left;
    int yTop = lpdis->rcItem.top;
    int yBottom = lpdis->rcItem.bottom;
    
	// Draw active
	m_iconColWidth = xTop;
	rectSub.left = xTop;
	rectSub.top = yTop;
	rectSub.right = xTop+nOffset;
	rectSub.bottom = yBottom;
	DrawOutputMuteColumn(which, lpdis->hDC, rectSub);

  	

	// Draw output
    xTop += nOffset;
	m_iconColWidth += nOffset;
	rectSub.left = xTop;
	rectSub.top = yTop;
	rectSub.right = xTop+nOffset;
	rectSub.bottom = yBottom;
	
	DrawMuteColumn(which, lpdis->hDC, rectSub);



	HPEN hPenGrey = CreatePen(PS_SOLID, 1, RGB(192, 192, 192));//GetCustSysColor(COLOR_GRAYTEXT));
	HPEN hPenWhite = CreatePen(PS_SOLID, 1, GetCustSysColor(COLOR_WINDOW)); //RGB(255, 255, 255));
    HPEN hOldPen;
	HBRUSH hBackground = CreateSolidBrush(GetCustSysColor(COLOR_WINDOW));
    HBRUSH hOldBackground = (HBRUSH)SelectObject(lpdis->hDC, hBackground);
    SetTextColor(lpdis->hDC, GetCustSysColor(COLOR_WINDOWTEXT));
    SetBkColor(lpdis->hDC, GetCustSysColor(COLOR_WINDOW));


 	// draw the name of the layer
    xTop += nOffset;
	m_iconColWidth += nOffset;
	rectSub.left = xTop;
	rectSub.top = yTop;
	rectSub.right = lpdis->rcItem.right;
	rectSub.bottom = yBottom;
	

	TSTR nam;
	BOOL isOnAllNodes = FALSE;
	if(which==NO_LAYERS)
		nam = GetString(IDS_ENABLE_LAYERS);
	else if(which==MULTIPLE_LAYERS)
		nam = GetString(IDS_MULTIPLE_ACTIVE_LAYERS);
	else 
	{
		MasterLayerControlManager *man = GetMLCMan();
		nam= man->GetLayerName(which);

		for(int i=0;i<man->currentLayers.Count();++i)
		{
			if(man->currentLayers[i]==which)
			{
				if(man->isLayerOnAll[i]==true)
				{
					isOnAllNodes = TRUE;
					break;
				}
			}
		}

	}

	buf = FormatName(nam, rectSub.right-rectSub.left, m_hwnd);

    HBRUSH hOldBrush = 0;
	HBRUSH hBackgrdSel = CreateSolidBrush(GetCustSysColor(COLOR_HIGHLIGHT));
	COLORREF oldBack = 0;
    if (lpdis->itemState & ODS_SELECTED)
    {
		if (isOnAllNodes)
		{
			hOldBrush = (HBRUSH)SelectObject(lpdis->hDC, hBackgrdSel);
			SetTextColor(lpdis->hDC, GetCustSysColor(COLOR_HIGHLIGHTTEXT));
			oldBack = SetBkColor(lpdis->hDC, GetCustSysColor(COLOR_HIGHLIGHT));
		}
		hOldPen = (HPEN)SelectObject(lpdis->hDC, hPenGrey);
    }
	else
		hOldPen = (HPEN)SelectObject(lpdis->hDC, hPenWhite);


	// Draw the bounding box
	rectSub.left += 1;
	rectSub.top += 1;
	rectSub.bottom -= 1;
	rectSub.right -= 1;
	// Draw the name
	LOGFONT logfont;
	HFONT hFont = (HFONT)SendMessage(m_hwnd, WM_GETFONT, 0, 0);
	GetObject(hFont, sizeof(LOGFONT), &logfont);
	logfont.lfWeight = 700;
	HFONT hFontBold = CreateFontIndirect(&logfont);

	if (isOnAllNodes==FALSE)
		SetTextColor(lpdis->hDC, GetCustSysColor(COLOR_GRAYTEXT));

	ExtTextOut(lpdis->hDC, xTop + 2, yTop + 2, ETO_CLIPPED | ETO_OPAQUE, &rectSub, buf,
             buf.length(), NULL);
    
	if (isOnAllNodes==FALSE)
		SetTextColor(lpdis->hDC, GetCustSysColor(COLOR_WINDOWTEXT));

    if (lpdis->itemState & ODS_SELECTED)
    {
		if (isOnAllNodes)
		{
			SelectObject(lpdis->hDC, hOldBrush);
			SetTextColor(lpdis->hDC, GetCustSysColor(COLOR_WINDOWTEXT));
			SetBkColor(lpdis->hDC, oldBack);
		}
		SelectObject(lpdis->hDC, hOldPen);
	}
	SelectObject(lpdis->hDC, hOldPen);

    // Cleanup
    SelectObject(lpdis->hDC, hOldBackground);
	DeleteObject(hPenGrey);
	DeleteObject(hPenWhite);
	DeleteObject(hBackground);
	DeleteObject(hFontBold);
//	DeleteObject(hNameFont);
	DeleteObject(hBackgrdSel);
}



void AnimLayerToolbarControl::DrawOutputMuteColumn(int which, HDC hDC, RECT r)
{
	if(which<0)
		return;
	int icon;
	int w = 16;

	if (GetMLCMan()->GetLayerMute(which)==true)
		icon = IDI_MUTED;
	else
		icon = IDI_UNMUTED;

	int x = r.left + w/2-8;
	int y = r.top + (r.bottom-r.top)/2-8;
	HBRUSH hBrush = CreateSolidBrush(GetCustSysColor(COLOR_WINDOW));
	FillRect(hDC, &r, hBrush);
	DeleteObject(hBrush);
	HICON hIcon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(icon), IMAGE_ICON, 16, 16, 0);
	DrawIconEx(hDC, x, y, hIcon, 16, 16, 0, 0, DI_NORMAL);
	DestroyIcon(hIcon);
}




void AnimLayerToolbarControl::DrawMuteColumn(int which, HDC hDC, RECT r)
{
	if(which<0)
		return;
	int icon;
	int w = 16;
	if (GetMLCMan()->GetLayerOutputMute(which)==true)
		icon = IDI_OUTPUTMUTED;
	else
		icon = IDI_OUTPUTUNMUTED;

	int x = r.left + w/2-8;
	int y = r.top + (r.bottom-r.top)/2-8;
	HBRUSH hBrush = CreateSolidBrush(GetCustSysColor(COLOR_WINDOW));
	FillRect(hDC, &r, hBrush);
	DeleteObject(hBrush);
	HICON hIcon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(icon), IMAGE_ICON, 16, 16, 0);
	DrawIconEx(hDC, x, y, hIcon, 16, 16, 0, 0, DI_NORMAL);
	DestroyIcon(hIcon);
}


POINT AnimLayerToolbarControl::HitTest(POINT p)
{
	POINT hit;
	int item = SendMessage(m_list, LB_ITEMFROMPOINT, 0, MAKELPARAM(p.x, p.y));

	if (HIWORD(item) == 1)//outside client area
	{
		hit.x = _kNameCol;
		hit.y = -1;
		return hit;
	}
	else
		hit.y = LOWORD(item);//index of the item

	RECT r;
	SendMessage(m_list, LB_GETITEMRECT, hit.y, (LPARAM)(&r));
	hit.x = _kNameCol;
	if (p.x < 0 || p.x > r.right || p.y > r.bottom || p.y < r.top)
	{
		hit.x = _kNameCol;
		return hit;
	}
	int xtop = 16;
	for (int i = 0; i < _kNameCol; i++)
	{
		if (p.x < xtop)
		{
			hit.x = i;
			return hit;
		}
		xtop += 16;
	}

	return hit;
}

bool AnimLayerToolbarControl::OnLButtonDown(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	POINT p;
	p.x = GET_X_LPARAM(lParam);
	p.y = GET_Y_LPARAM(lParam);

	POINT hit = HitTest(p);

	if (hit.y == -1)
		return false;

	int which = (int)SendMessage(m_list, LB_GETITEMDATA, hit.y, 0);

	if (hit.x == _kNameCol)
		return false;
	else
		return true;
}

bool AnimLayerToolbarControl::OnLButtonUp(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	POINT p;
	p.x = GET_X_LPARAM(lParam);
	p.y = GET_Y_LPARAM(lParam);

	POINT hit = HitTest(p);
	m_col = hit.x;

	if (hit.y == -1)
		return false;

	int which = SendMessage(m_list, LB_GETITEMDATA, hit.y, 0);


	MasterLayerControlManager *man = GetMLCMan();
	if (which<0||hit.x == _kNameCol)
		return false;

    SendMessage(hwnd, WM_SETREDRAW, FALSE, 0L);
    int nScrollPos = GetScrollPos(hwnd,   SB_VERT);

	switch(hit.x)
	{
	case _kMuteCol:	// hidden
		{
			bool mute = !man->GetLayerMute(which);
			man->SetLayerMute(which,mute);
		}
		break;

	case _kOutputMuteCol: // frozen
		{
			bool mute = !man->GetLayerOutputMute(which);
			man->SetLayerOutputMute(which,mute);
		}
		break;
	}
      
    SendMessage(hwnd, WM_SETREDRAW, TRUE, 0L);
	SetScrollPos(hwnd, SB_VERT, nScrollPos, FALSE);
    InvalidateRect(hwnd, NULL, TRUE);
    InvalidateRect(m_list, NULL, TRUE);
    SetSelected(which);

	return true;
}


bool AnimLayerToolbarControl::OnRButtonUp(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	POINT p;
	p.x = GET_X_LPARAM(lParam);
	p.y = GET_Y_LPARAM(lParam);
	POINT hit = HitTest(p);
	if (hit.y == -1)
		return false;

	int which = (int)SendMessage(m_list, LB_GETITEMDATA, hit.y, 0);
	if(which<0)
		return false;
	GetMLCMan()->RenameLayer(hwnd, which);

	RECT r;
	SendMessage(m_list, LB_GETITEMRECT, hit.y, (LPARAM)(&r));
	InvalidateRect(m_list, &r, TRUE);
	UpdateWindow(m_list);
	if (GetMLCMan()->activeLayers.Count()==1 &&which == GetMLCMan()->activeLayers[0])
	{
		InvalidateRect(m_hwnd, NULL, TRUE);
		UpdateWindow(m_hwnd);
	}

	return true;
}

INT_PTR MasterLayerControlManager::RenameLayer(HWND hWnd, int which)
{

	if (which==0)
	{
		char tempBuf[256];
		char title[256];
		strcpy(title, GetString(IDS_ANIM_LAYERS));
		strcpy(tempBuf, GetString(IDS_CANNOT_RENAME_BASE_LAYER));
		MessageBox(GetActiveWindow(), tempBuf, title, MB_OK);
		return 0;
	}
	return DialogBoxParam(hInstance,
						  MAKEINTRESOURCE(IDD_LAYER_RENAME),
						  GetCOREInterface()->GetMAXHWnd(),
						  MasterLayerControlManager::RenameLayerProc,
						  (LPARAM)which);
}

INT_PTR CALLBACK MasterLayerControlManager::RenameLayerProc(HWND hWnd, UINT msg,
													WPARAM wParam, LPARAM lParam)
{

	static int which= -1;
	switch (msg)
	{
	case WM_INITDIALOG:

		PositionWindowNearCursor(hWnd);
		which = (int)lParam;
		if (which>=0)
			SetDlgItemText(hWnd, IDC_EDIT_LAYERNAME, GetMLCMan()->GetLayerName(which));
		break;

	case WM_CLOSE:
		EndDialog(hWnd, FALSE);
		return FALSE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDCANCEL:
			EndDialog(hWnd, FALSE);
			return FALSE;

		case IDOK:
			if (which>=0)
			{
				TCHAR buf[256], *bufp = buf, msg[256], fmtmsg[256];

				GetDlgItemText(hWnd, IDC_EDIT_LAYERNAME, buf, 255);
				while (_tcslen(buf) > 0 && (buf[_tcslen(buf)-1] == ' ' || buf[_tcslen(buf)-1] == '\t'))
					buf[_tcslen(buf)-1] = '\0';
				while (_tcslen(bufp) > 0 && (bufp[0] == ' ' || bufp[0] == '\t'))
					bufp++;
				if (_tcslen(bufp) == 0)
				{
					_tcsncpy(msg, GetString(IDS_LAYER_NO_NAME), 255);
					_tcsncpy(fmtmsg, GetString(IDS_ANIM_LAYERS), 255);
					MessageBox(hWnd, msg, fmtmsg, MB_OK);
					return FALSE;
				}
				for (int i=0;i<GetMLCMan()->GetNumMLCs();++i)
				{
					if (i!=which&&GetMLCMan()->GetLayerName(i)==(TSTR)bufp)
					{
						_tcsncpy(fmtmsg, GetString(IDS_LAYER_DUP_NAME), 255);
						_stprintf(msg, fmtmsg, bufp);
						_tcsncpy(fmtmsg, GetString(IDS_ANIM_LAYERS), 255);
						MessageBox(hWnd, msg, fmtmsg, MB_OK);
						return FALSE;
					}
				}
				if(which>=0)
					GetMLCMan()->SetLayerName(which,TSTR(bufp));
			}
			EndDialog(hWnd, TRUE);
			return FALSE;
		}
	}

	return FALSE;
}



LRESULT CALLBACK AnimLayerToolbarControl::ToolbarMsgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	bool bHandled = false;
	static BOOL undoHeldHere = FALSE;

	switch (message)
	{
	case WM_DRAWITEM:
		if (wParam == ANIMLAYERMGR_COMBOBOX_ID)
			OnDraw(hWnd, wParam, lParam);
		break;
		case CC_SPINNER_BUTTONDOWN:
			theHold.Begin();
			undoHeldHere = TRUE;
			break;
		case CC_SPINNER_CHANGE: {
			if (!undoHeldHere) {theHold.Begin();undoHeldHere = TRUE;}
			TimeValue t = GetCOREInterface()->GetTime();
			switch (LOWORD(wParam)) 
			{
				case ANIMLAYERMGR_SPINNER_ID: {
					MasterLayerControlManager *man = GetMLCMan();
					ISpinnerControl *iWeight = GetISpinner(hSpinHWND);
					if(man->activeLayers.Count()==1)
					{
						man->SetLayerWeight(man->activeLayers[0],t,iWeight->GetFVal()/100.0f);
					}
					ReleaseISpinner(iWeight);
					break;
					}
				default:
					break;
			}
			GetCOREInterface()->RedrawViews(t);
			break;
			}
		case WM_CUSTEDIT_ENTER:
		case CC_SPINNER_BUTTONUP:
			if (HIWORD(wParam) || message==WM_CUSTEDIT_ENTER)
				theHold.Accept(GetString(IDS_AF_LIST_WEIGHT_UNDO));
			else theHold.Cancel();
			undoHeldHere = FALSE;
			break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ANIMLAYERMGR_COMBOBOX_ID:
			switch (HIWORD(wParam))
			{
			case CBN_DROPDOWN:
				m_col = -1;
				SetComboSize(m_hwnd);
				break;

			case CBN_SELCHANGE:
				{
					int sel = (int)SendMessage((HWND)lParam, CB_GETITEMDATA, SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0), 0);
					if(sel>=0)
					{
						MasterLayerControlManager *man = GetMLCMan();
						INodeTab nodeTab;
						GetSelectedNodes(nodeTab);
						theHold.SuperBegin();
						theHold.Begin();
						man->SetLayerActiveNodes(sel,nodeTab);
						macroRecorder->FunctionCall(_T("AnimLayerManager.setLayerActiveNodes"), 2, 0, mr_int, sel+1, mr_sel);
						theHold.Accept(GetString(IDS_SET_ACTIVE));
						theHold.SuperAccept(GetString(IDS_SET_ACTIVE));
					}
					bHandled = true;	
				}
				break;


			case CBN_CLOSEUP:
				{
					int sel = (int)SendMessage((HWND)lParam, CB_GETITEMDATA, SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0), 0);

					if(m_col != _kNameCol)
					{
						//do nothingSetSelected(name);
						bHandled = true;
					}
					else if (m_col == _kNameCol)
					{
						if(sel>=0)
						{
							MasterLayerControlManager *man = GetMLCMan();
							INodeTab nodeTab;
							GetSelectedNodes(nodeTab);
							theHold.SuperBegin();
							theHold.Begin();
							man->SetLayerActiveNodes(sel,nodeTab);
							macroRecorder->FunctionCall(_T("AnimLayerManager.setLayerActiveNodes"), 2, 0, mr_int, sel+1, mr_sel);
							theHold.Accept(GetString(IDS_SET_ACTIVE));
							theHold.SuperAccept(GetString(IDS_SET_ACTIVE));
						}
						bHandled = true;	// forestall changing layers of sel objs
											// (code in the AppWndProc -- see ID_LAYER_CONTROL)
					}

				}
				break;
			}
			break;
		}
		break;
	}

	if(bHandled)
		return false;
	return CallWindowProc(m_defToolProc, hWnd, message, wParam, lParam);
}

LRESULT CALLBACK AnimLayerToolbarControl::ListBoxControlProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	bool ret = false;

	switch (message)
	{
	case WM_LBUTTONDOWN:
		ret = OnLButtonDown(hwnd, wParam, lParam);
		break;

	case WM_LBUTTONUP:
		ret = OnLButtonUp(hwnd, wParam, lParam);
		break;

	case WM_RBUTTONUP:
		ret = OnRButtonUp(hwnd, wParam, lParam);
		break;

	}

	//ret will be true if the user clicked on one of the icons and false if they
	//clicked the layer name
	if (ret)
		return FALSE;

	assert(m_defListProc);

	//JH 8/1/01 
	//Not sure what's going on here, why is listbox getting unsubclassed and resubclassed. I'm removing it
//	DLSetWindowLongPtr(m_list, m_defListProc);
	LRESULT lRet = CallWindowProc(m_defListProc, hwnd, message, wParam, lParam);
//	m_defListProc = DLSetWindowLongPtr(m_list, ListBoxControlProc);		

	return lRet;
}

LRESULT CALLBACK AnimLayerToolbarControl::LayerComboControlProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	AnimLayerToolbarControl * propCombo = DLGetWindowLongPtr<AnimLayerToolbarControl *>(hwnd);
	assert(propCombo);

	switch (message)
	{
	case WM_CTLCOLORLISTBOX:
		if (propCombo->m_list != HWND(lParam))
			propCombo->SetListBox(HWND(lParam));
		break;

	case WM_COMMAND:
		propCombo->OnCommand(wParam, lParam);
		break;

	}

	return CallWindowProc(propCombo->m_defWndProc, hwnd, message, wParam, lParam);
}

