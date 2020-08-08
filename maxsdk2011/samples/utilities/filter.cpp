/*==============================================================================
// Copyright (c) 1998-2008 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//**************************************************************************/
// FILE:        filter.cpp
// DESCRIPTION: Utilities for filtering track view controllers.
// AUTHOR:      Michael Zyracki 
// HISTORY:     created 2006
//***************************************************************************/
#include "util.h"
#include "tvutil.h"
#include "istdplug.h"

#define FILTER_EULER_NAME		GetString(IDS_MZ_FILTER_EULER)


class EulerFilterUtil : public TrackViewUtility
{
	public:				
		Interface *ip;
		ITVUtility *iu;
		HWND hWnd;
		EulerFilterUtil() {ip=NULL;iu=NULL;hWnd=NULL;addKeysIfNeeded = false;}
		void DeleteThis() {if (hWnd) DestroyWindow(hWnd);} 
		void BeginEditParams(Interface *ip,ITVUtility *iu);		
		void		GetClassName(TSTR& s)	{ s = FILTER_EULER_NAME; }  
		Class_ID	ClassID()				{ return FILTER_EULER_CLASS_ID; }

		void SetupWindow(HWND hWnd);
		void FilterEulers(HWND hWnd);

		void Destroy(){hWnd = NULL; if (iu) iu->TVUtilClosing(this);}

		struct Euler
		{
			Control *euler; //the euler control
			bool eulerSelected;//whether or not the main euler was selected.. if so then if non of the sub tracks are selected we do
							   //all of it, otherwise we just do the ones that are selected
			bool doWhich[3]; //which float tracks are selected.
			Euler():euler(NULL), eulerSelected(false){doWhich[0] =doWhich[1] = doWhich[2] = false;}
		};
		Tab<Euler> eulersToFilter;
		Interval eulerRange;
		bool addKeysIfNeeded;
		void GetEulers();
		Euler *FindEuler(Control *euler);
		void FilterEuler(Euler &euler, Interval &range);

};
EulerFilterUtil theEulerFilterUtil;

class EulerFilterClassDesc:public ClassDesc
{
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading = FALSE) {return &theEulerFilterUtil;}
	const TCHAR *	ClassName() {return FILTER_EULER_NAME;}
	SClass_ID		SuperClassID() {return TRACKVIEW_UTILITY_CLASS_ID;}
	Class_ID		ClassID() {return FILTER_EULER_CLASS_ID;}
	const TCHAR* 	Category() {return _T("");}
};

static EulerFilterClassDesc eulerFilterDesc;
ClassDesc* GetEulerFilterDesc() {return &eulerFilterDesc;}


EulerFilterUtil::Euler* EulerFilterUtil::FindEuler(Control *euler)
{
	for(int i=0;i<eulersToFilter.Count();++i)
	{
		if(eulersToFilter[i].euler==euler)
			return &(eulersToFilter[i]);
	}
	return NULL;
}

void EulerFilterUtil::GetEulers()
{
	eulersToFilter.ZeroCount();
	eulerRange.SetEmpty();
	BOOL timeMode   = iu->GetMajorMode()==TVMODE_EDITTIME;
    BOOL fcurveMode = iu->GetMajorMode()==TVMODE_EDITFCURVE;

	for (int i=0; i<iu->GetNumTracks(); i++)
	{
		if ((timeMode||fcurveMode) && !iu->IsSelected(i)) continue;
      	// Get Interfaces
		Animatable *anim   = iu->GetAnim(i);
		if (fcurveMode && !anim->IsCurveSelected()) continue;
		Control *cont = GetControlInterface(anim);
		if(cont==NULL) continue;
		Animatable *client = iu->GetClient(i);
		int subNum         = iu->GetSubNum(i);
		//check and see which track we have selected
		//if it's an euler check and see if it exists already (probably not but maybe)
		if(anim->GetInterface(I_EULERCTRL))
	    {
			Euler *euler=  FindEuler(cont);
			if(euler==NULL)
			{
				Euler eu;
				eu.euler = cont;
				eu.eulerSelected = true;
				eulersToFilter.Append(1,&eu);

				Interval localRange = anim->GetTimeRange(TIMERANGE_ALL|TIMERANGE_CHILDANIMS);
				if(eulerRange.Empty())
					eulerRange = localRange;
				else
				{
					eulerRange+= localRange.Start(); eulerRange +=localRange.End();
				}
			}
		}
		if(client->GetInterface(I_EULERCTRL))
		{
			Control *clientCont = GetControlInterface(client);
			//need to make sure that the anim is keyable
			if(clientCont&&cont->IsKeyable()&&GetKeyControlInterface(anim))
			{
				//okay we can key it so find it
				Euler *euler = FindEuler(clientCont);
				if(euler==NULL)
				{
					Euler eu;
					eu.euler =clientCont;
					eu.eulerSelected = false;
					eulersToFilter.Append(1,&eu);
					euler = &(eulersToFilter[eulersToFilter.Count()-1]);
				}
				
				if(clientCont->GetXController()==cont)
					euler->doWhich[0] = true;
				else if(clientCont->GetYController()==cont)
					euler->doWhich[1]  = true;
				else if(clientCont->GetZController()==cont)
					euler->doWhich[2] = true;
				
				Interval localRange = anim->GetTimeRange(TIMERANGE_ALL|TIMERANGE_CHILDANIMS);
				if(eulerRange.Empty())
					eulerRange = localRange;
				else
				{
					eulerRange+= localRange.Start(); eulerRange +=localRange.End();
				}
			}
		}
	}
	Interval range = GetCOREInterface()->GetAnimRange();
	if(eulerRange.Empty())
		eulerRange = range;
	else
	{
		if(eulerRange.Start()<range.Start())
			eulerRange.SetStart(range.Start());
		if(eulerRange.End()>range.End())
			eulerRange.SetEnd(range.End());
	}
 
}

void EulerFilterUtil::SetupWindow(HWND hWnd)
{
	GetEulers();

	Interval iv = eulerRange;
	ISpinnerControl *spin;
	spin = GetISpinner(GetDlgItem(hWnd,IDC_STARTSPIN));
	spin->SetLimits(TIME_NegInfinity, TIME_PosInfinity, FALSE);
	spin->SetScale(10.0f);
	spin->LinkToEdit(GetDlgItem(hWnd,IDC_START), EDITTYPE_TIME);
	spin->SetValue(iv.Start(),FALSE);
	ReleaseISpinner(spin);

	spin = GetISpinner(GetDlgItem(hWnd,IDC_ENDSPIN));
	spin->SetLimits(TIME_NegInfinity, TIME_PosInfinity, FALSE);
	spin->SetScale(10.0f);
	spin->LinkToEdit(GetDlgItem(hWnd,IDC_END), EDITTYPE_TIME);
	spin->SetValue(iv.End(),FALSE);
	ReleaseISpinner(spin);

	if(addKeysIfNeeded==true)
		 CheckDlgButton(hWnd,IDC_ADD_KEYS_IF_NEEDED,TRUE);
	else
		 CheckDlgButton(hWnd,IDC_ADD_KEYS_IF_NEEDED,FALSE);
}

//This adds
void AddTimes(Tab<TimeValue> &keyTimes,Tab<int> &howMany,Tab<TimeValue> &tempTimes)
{
	int k=0;
	TimeValue t;
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
					int one = 1;
					howMany.Append(1,&one);
				}
				else if(tempTimes[k]==keyTimes[j])
				{
					int hM = howMany[j];
					howMany[j] = ++hM;
				}
				++k;
			}
		}
	}
	for(k;k<tempTimes.Count();++k)
	{
		t = tempTimes[k];
		keyTimes.Append(1,&t);
		int one = 1;
		howMany.Append(1,&one);
	}
}
void EulerFilterUtil::FilterEuler(EulerFilterUtil::Euler &euler,Interval &range)
{
	//we do 2 different types of filters based upon what's 'selected'.
	//if we only have one euler track selected, then we do a simply filter to check and see if the rotation is greater than 180 degrees, in which
	//case we swap up.

	//The 2nd case, where we have 2 or more picked is more complicated.
	
	//first we check to see if the euler is selected but no curves are selected. In this case we act as if everybody is selected.
	if(euler.eulerSelected==true)
	{
		if(euler.doWhich[0]==false&& euler.doWhich[1]==false &&euler.doWhich[2]==false)
		{
			euler.doWhich[0] = euler.doWhich[1] = euler.doWhich[2] =true;
		}
	}

	GetCOREInterface()->BeginProgressiveMode();

	//set up the temp euler control that we will progressively set keys into.. we then get keys out of this guy.
	theHold.Suspend(); //don't want this to be undone since we will delete c later, it's just a temp to do the euler progressive caching.
	void * v = GetCOREInterface()->CreateInstance(CTRL_ROTATION_CLASS_ID, Class_ID(EULER_CONTROL_CLASS_ID,0));
	Control *c	= static_cast<Control *>(v);
	IEulerControl *eulerCtrl = static_cast<IEulerControl*>(euler.euler->GetInterface(I_EULERCTRL));
	IEulerControl *eulerCtrl2 = static_cast<IEulerControl*>(c->GetInterface(I_EULERCTRL));
	eulerCtrl2->SetOrder(eulerCtrl->GetOrder());

	//find out all of the keys we need to set..
	Control *rotX = euler.euler->GetXController();   
	Control *rotY = euler.euler->GetYController(); 
	Control *rotZ = euler.euler->GetZController();
	//temps that we may use for setting the temp directly
	Control *trotX = c->GetXController(); 
	Control *trotY = c->GetYController(); 
	Control *trotZ = c->GetZController();


	Tab<TimeValue> xkeys;
	Tab<TimeValue> ykeys;
	Tab<TimeValue> zkeys;
	Tab<TimeValue> combinedTimes;
	Tab<int> howMany; //how many keys at that frame.. 
	if (rotX) rotX->GetKeyTimes(xkeys, range, KEYAT_ROTATION);
	if (rotY) rotY->GetKeyTimes(ykeys, range, KEYAT_ROTATION);
	if (rotZ) rotZ->GetKeyTimes(zkeys, range, KEYAT_ROTATION);
	AddTimes(combinedTimes,howMany,xkeys);
	AddTimes(combinedTimes,howMany,ykeys);
	AddTimes(combinedTimes,howMany,zkeys);
	
	//now we set those eulers for the combined times..
	int i;
	Interval valid  = FOREVER;
	Quat q;
	if(addKeysIfNeeded)
	{
		for(i=0;i<combinedTimes.Count();++i)
		{
			euler.euler->GetValue(combinedTimes[i],&q,FOREVER,CTRL_ABSOLUTE);
			c->SetValue(combinedTimes[i],q,TRUE,CTRL_ABSOLUTE);
		}
	}
	else
	{
		//okay if we don't add keys we explicitly set the euler for those frames where we don't have 3 keys.
		for(i=0;i<combinedTimes.Count();++i)
		{
			if(howMany[i]==3)
			{
				euler.euler->GetValue(combinedTimes[i],&q,FOREVER,CTRL_ABSOLUTE);
				c->SetValue(combinedTimes[i],&q,TRUE,CTRL_ABSOLUTE);
			}
			else
			{
				float f;
				rotX->GetValue(combinedTimes[i],&f,FOREVER,CTRL_ABSOLUTE);
				trotX->SetValue(combinedTimes[i],&f,TRUE,CTRL_ABSOLUTE);
				rotY->GetValue(combinedTimes[i],&f,FOREVER,CTRL_ABSOLUTE);
				trotY->SetValue(combinedTimes[i],&f,TRUE,CTRL_ABSOLUTE);
				rotZ->GetValue(combinedTimes[i],&f,FOREVER,CTRL_ABSOLUTE);
				trotZ->SetValue(combinedTimes[i],&f,TRUE,CTRL_ABSOLUTE);
			}
		}
	}
	theHold.Resume();
	
	//now the c control has the correct eulers.. we now set them based upon our flag
	Control *newRotX = c->GetXController();
	Control *newRotY = c->GetYController();
	Control *newRotZ = c->GetZController();
	float val;
	if(addKeysIfNeeded) //just set them all up...
	{
		for(i=0;i<combinedTimes.Count();++i)
		{
			newRotX->GetValue(combinedTimes[i],&val,FOREVER,CTRL_ABSOLUTE);
			rotX->SetValue(combinedTimes[i],&val,TRUE,CTRL_ABSOLUTE);
			newRotY->GetValue(combinedTimes[i],&val,FOREVER,CTRL_ABSOLUTE);
			rotY->SetValue(combinedTimes[i],&val,TRUE,CTRL_ABSOLUTE);
			newRotZ->GetValue(combinedTimes[i],&val,FOREVER,CTRL_ABSOLUTE);
			rotZ->SetValue(combinedTimes[i],&val,TRUE,CTRL_ABSOLUTE);
		}
	}
	else
	{
		for(i=0;i<combinedTimes.Count();++i)
		{
			if(howMany[i]==3) //only if we have 3 keys!
			{
				newRotX->GetValue(combinedTimes[i],&val,FOREVER,CTRL_ABSOLUTE);
				rotX->SetValue(combinedTimes[i],&val,TRUE,CTRL_ABSOLUTE);
				newRotY->GetValue(combinedTimes[i],&val,FOREVER,CTRL_ABSOLUTE);
				rotY->SetValue(combinedTimes[i],&val,TRUE,CTRL_ABSOLUTE);
				newRotZ->GetValue(combinedTimes[i],&val,FOREVER,CTRL_ABSOLUTE);
				rotZ->SetValue(combinedTimes[i],&val,TRUE,CTRL_ABSOLUTE);
			}
		}
	}

		
	GetCOREInterface()->EndProgressiveMode();
	
	if (c != NULL)
	{
		// Delete the temp controller
		HoldSuspend hs;
		RefResult res = c->MaybeAutoDelete();
		DbgAssert(REF_SUCCEED == res);
		c = NULL;
	}
}
void EulerFilterUtil::FilterEulers(HWND hWnd)
{

	ISpinnerControl *spin;
	spin = GetISpinner(GetDlgItem(hWnd,IDC_STARTSPIN));
	TimeValue start = spin->GetIVal();
	ReleaseISpinner(spin);
	spin = GetISpinner(GetDlgItem(hWnd,IDC_ENDSPIN));
	TimeValue end = spin->GetIVal();
	ReleaseISpinner(spin);
	if (start>end) {
		TimeValue temp = end;
		end   = start;
		start = temp;
		}
	Interval range(start,end);

	if(IsDlgButtonChecked(hWnd,IDC_ADD_KEYS_IF_NEEDED))
		addKeysIfNeeded = true;
	else
		addKeysIfNeeded = false;
	
	theHold.Begin();
	// Turn animation on
	SuspendAnimate();
	AnimateOn();

	for(int i=0;i<eulersToFilter.Count();++i)
	{
		FilterEuler(eulersToFilter[i],range);
	}
	ResumeAnimate();
	theHold.Accept(FILTER_EULER_NAME);
}

static INT_PTR CALLBACK EulerFilterDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
		case WM_INITDIALOG:
			CenterWindow(hWnd,GetParent(hWnd));
			theEulerFilterUtil.SetupWindow(hWnd);
			break;

		case WM_COMMAND:			
			switch (LOWORD(wParam)) {				
				case IDOK:
					theEulerFilterUtil.FilterEulers(hWnd);
					EndDialog(hWnd, 1);
					break;

				case IDCANCEL:
					EndDialog(hWnd,0);
					break;
				}
			break;

		case WM_DESTROY:
			theEulerFilterUtil.Destroy();			
			break;

		default:
			return FALSE;
		}
	return TRUE;
}

void EulerFilterUtil::BeginEditParams(Interface *ip,ITVUtility *iu)
{
	this->ip = ip;
	this->iu = iu;
	DialogBox(
		hInstance,
		MAKEINTRESOURCE(IDD_EULER_FILTER),
		iu->GetTrackViewHWnd(),
		EulerFilterDlgProc);
}


 