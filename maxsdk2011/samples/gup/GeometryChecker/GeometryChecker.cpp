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
// DESCRIPTION: Geometry Checker
// AUTHOR: Michael Zyracki created 2008
//***************************************************************************/
#include <Max.h>
#include <bmmlib.h>
#include <guplib.h>
#include <maxapi.h>
#include <iparamb2.h>
#include <notify.h>
#include <iparamm2.h>
#include <istdplug.h>
#include <triobj.h>
#include <polyobj.h>
#include <IGeometryChecker.h>
#include <map>
#include <IMenuMan.h>
#include "checkerimplementations.h"
#include "geometrybutton.h"
#include "notifier.h"
#include <IPathConfigMgr.h>


class IGeometryCheckerManager_Imp;

//handles drawing the resulting values in the viewport. The text button is handled by the viewportbutton system.
class GeometryViewportDisplay : public ViewportDisplayCallback
{
public:
	GeometryViewportDisplay();
	~GeometryViewportDisplay();
	//viewport display callback
	void Display(TimeValue t, ViewExp *vpt, int flags);		
	void GetViewportRect( TimeValue t, ViewExp *vpt, Rect *rect );
	BOOL Foreground(); 
	void SetIGeometryCheckerManager(IGeometryCheckerManager_Imp *check){mGeomCheckerMan = check;}
	DisplayIn3DViewport m3D;

	void DeleteCachedResults(){m3D.DeleteCachedResults();}
private:
	IGeometryCheckerManager_Imp *mGeomCheckerMan;
};

//main implementation of the geometry checker
class  IGeometryCheckerManager_Imp : public GUP,public  IGeometryCheckerManager, public NotifyCallback   //the callback is for the notifymgr, lets us keep track of geom changes to our selection
{
friend class GeometryViewportDisplay;
public:
        
    static  IGeometryCheckerManager& GetInstance() { return _theInstance; }
	static  IGeometryCheckerManager_Imp& GetImpInstance(){return _theInstance;}
	enum {kGetGeometryCheckerOn=0, kSetGeometryCheckerOn,kGetActiveGeometryChecker,
		kSetActiveGeometryChecker,kGetNumGeometryCheckers,kGetGeometryCheckerName,kGetGeometryCheckerID,
		kGetActiveGeometryCheckerID, kSetActiveGeometryCheckerID,
		kPopupMenuSelect,kSelectResults,kDisplayResults,kOutputEnum,kRegisterChecker,kUnRegisterChecker,
		kDoesGeometryCheckerHavePropDlg, kShowGeometryCheckerPropDlg,kGetSeeThrough,kSetSeeThrough,
		kGetAutoUpdate,kSetAutoUpdate,kGetDisplayTextUpTop,kSetDisplayTextUpTop,kRunGeometryCheck,
		kGetCurrentReturnVal,kGetCurrentOutputCount,kGetCurrentString
	};

	BEGIN_FUNCTION_MAP	
		PROP_FNS(kGetGeometryCheckerOn,GetGeometryCheckerOn,kSetGeometryCheckerOn,SetGeometryCheckerOn, TYPE_bool);
		PROP_FNS(kGetActiveGeometryChecker,GetActivatedGeometryCheckerIndex,kSetActiveGeometryChecker,ActivateGeometryChecker, TYPE_INDEX);
		PROP_FNS(kGetSeeThrough,GetSeeThrough,kSetSeeThrough,SetSeeThrough, TYPE_bool);
		PROP_FNS(kGetAutoUpdate,GetAutoUpdate,kSetAutoUpdate,SetAutoUpdate, TYPE_bool);
		PROP_FNS(kGetDisplayTextUpTop,GetDisplayTextUpTop,kSetDisplayTextUpTop,SetDisplayTextUpTop, TYPE_bool);

		FN_0(kGetNumGeometryCheckers, TYPE_INT, GetNumRegisteredGeometryCheckers);
		FN_1(kGetGeometryCheckerName,TYPE_TSTR_BV,GetNthGeometryCheckerName,TYPE_INDEX);
		FN_1(kGetGeometryCheckerID,TYPE_DWORD,GetNthGeometryCheckerID,TYPE_INDEX);
		FN_0(kGetActiveGeometryCheckerID,TYPE_DWORD,GetActivatedGeometryCheckerID);
		VFN_1(kSetActiveGeometryCheckerID,SetActiveGeometryCheckerID,TYPE_DWORD);
		VFN_0(kPopupMenuSelect,PopupMenuSelect);
		VFN_1(kSelectResults,SelectResults,TYPE_TIMEVALUE);
		VFN_6(kDisplayResults,DisplayResults_FPS,TYPE_COLOR,TYPE_TIMEVALUE,TYPE_INODE,TYPE_HWND,TYPE_ENUM,TYPE_INDEX_TAB_BR);
		FN_7(kRegisterChecker, TYPE_INDEX, RegisterChecker_FPS, TYPE_VALUE, TYPE_VALUE,TYPE_ENUM,TYPE_TSTR,TYPE_VALUE,TYPE_VALUE,TYPE_VALUE);
		FN_1(kUnRegisterChecker, TYPE_bool, UnRegisterChecker_FPS, TYPE_TSTR);
		FN_1(kDoesGeometryCheckerHavePropDlg,TYPE_bool,DoesGeometryCheckerHavePropDlg,TYPE_INDEX);
		VFN_1(kShowGeometryCheckerPropDlg,ShowGeometryCheckerPropDlg,TYPE_INDEX);
		VFN_1(kRunGeometryCheck,RunGeometryCheck,TYPE_TIMEVALUE);
		
		FN_0(kGetCurrentReturnVal,TYPE_ENUM,GetCurrentReturnVal);
		FN_0(kGetCurrentOutputCount,TYPE_INT,GetCurrentOutputCount);
		FN_0(kGetCurrentString,TYPE_TSTR_BV,GetCurrentString);

	END_FUNCTION_MAP


	//inherited funcs
	DWORD Start();
	void Stop();
	~IGeometryCheckerManager_Imp();
	int RegisterGeometryChecker(IGeometryChecker * geomChecker);
	void UnRegisterGeometryChecker(IGeometryChecker *geomChecker);
	int GetNumRegisteredGeometryCheckers()const;
	void SelectResults(TimeValue t);
	IGeometryChecker * GetNthGeometryChecker(int nth)const;
	MSTR GetNthGeometryCheckerName(int nth)const;
	void RunGeometryCheck(TimeValue t);
	DWORD GetNthGeometryCheckerID(int nth)const;

	void SetGeometryCheckerOn(bool on);
	bool GetGeometryCheckerOn()const;

	bool ActivateGeometryChecker( int which);
	int GetActivatedGeometryCheckerIndex()const;
	void PopupMenuSelect();
	void SelectionChanged();
	MSTR GetCurrentString();
	void ResetButtonLocations(ViewExp *vpt); //to make sure that they are centered.
	IGeometryChecker::ReturnVal GetCurrentReturnVal();
	IGeometryChecker::ReturnVal GeometryCheck(TimeValue t,INode *nodeToCheck, IGeometryChecker::OutputVal &val);
	int IGeometryCheckerManager::GetCurrentOutputCount(void);
	void DisplayResults(Color *overridecolor,TimeValue t,INode *node, HWND hwnd, IGeometryChecker::ReturnVal type,Tab<int> &indices);
	void DisplayResults_FPS(Color *overridecolor,TimeValue t,INode *node, HWND hwnd, int type,Tab<int> &indices);

	bool DoesGeometryCheckerHavePropDlg(int index)const;
	void ShowGeometryCheckerPropDlg(int index);


	bool GetSeeThrough()const;
	void SetSeeThrough(bool val);
	bool GetAutoUpdate()const;
	void SetAutoUpdate(bool val) ;
	bool GetDisplayTextUpTop()const;
	void SetDisplayTextUpTop(bool val);


    //returns whether or not we should stop computing based upon popping up a dialog to ask the user
	//numfaces is used by the dialog for some text.
	bool ShowWarningDialogThenStop(int numFaces);
	//set the internal flag. This is also stored in the geometrychecker .ini file.
	void SetShowWarningDialog(bool set);   

	DWORD GetNextValidGeometryCheckerID();
	DWORD GetActivatedGeometryCheckerID();
	void SetActiveGeometryCheckerID(DWORD id);

	void Invalidate();

	bool NotValid(TimeValue t);
private:

	
    // Declares the constructor of this class. It is private so a single static instance of this class
    // may be instantiated.
    DECLARE_DESCRIPTOR( IGeometryCheckerManager_Imp );

	// The single instance of this class
    static  IGeometryCheckerManager_Imp  _theInstance;

	//list of geometry checkers
	Tab<IGeometryChecker *> mGeometryCheckers;

	int mWhichActive; 
	bool mCheckOn; 
	bool mSeeThrough;
	bool mAutoUpdate;
	bool mDisplayTextUpTop;
	bool mShowWarningDialog;
	int mNextValidID;
	bool mValid; //flag to say whether or not we are valid
	TimeValue mLastValidTime;
	MSTR mCheckResult; //string for the result passed to the viewport display
	IGeometryChecker::ReturnVal mReturnVal;

	GeometryViewportDisplay mViewportDisplay;

	GeometryButton mGeometryButton; //the button that's used as our main dropdown.
	ConfigureButton mConfigureButton;
	AutoUpdateButton mAutoUpdateButton;

	NotifyMgr mNotifyMgr; //handles nodes that we are watching, basically the current selection

	//private funcs
	bool IsGeometryCheckerAlreadyRegistered(IGeometryChecker *checker);
	void SetUpButton(); //set up the button
	IGeometryChecker * GetActivatedGeometryChecker()const;
	void SetBitArray(BitArray &bitArray,int bitSize,Tab<int>&results);
	void ClearResults();

	//from notify callback
	virtual RefResult NotifyFunc(Interval changeInt,
         RefTargetHandle hTarget, PartID& partID,
            RefMessage message);

	struct LocalResults
	{
		INode *mNode;
		Tab<int> mIndex;
	};

	Tab<LocalResults*> mLocalResults; //contains the cached result for each selected node.. we do this so we don't need to always recalc the results
	void DeleteCachedResults();

	//funcs and data for the MXS based geometry checkers
	int RegisterChecker_FPS(Value *checkerFunc, Value *isSupportedFunc,int type, MSTR name,  Value *popupFunc, Value *textOverrideFunc,
		Value *displayOverrideFunc);
	bool UnRegisterChecker_FPS(MSTR name);

	//ini funcs
	static const TCHAR* kINI_FILE_NAME;
	static const TCHAR* kINI_SECTION_NAME;
	static const TCHAR* kINI_SEE_THROUGH;
	static const TCHAR* kINI_AUTO_UPDATE;
	static const TCHAR* kINI_DISPLAY_TEXT_UP_TOP;
	static const TCHAR* kINI_SHOW_WARNING_DIALOG ;

	MSTR GetINIFileName();
	void SavePreferences ();
	void LoadPreferences ();


};

IGeometryCheckerManager_Imp IGeometryCheckerManager_Imp::_theInstance(IGEOMETRYCHECKER_MANAGER_INTERFACE, _T("xViewChecker"), 0, NULL, FP_CORE,
	IGeometryCheckerManager_Imp::kGetNumGeometryCheckers, _T("getNumCheckers"), 0,TYPE_INT,0,0,
	IGeometryCheckerManager_Imp::kGetGeometryCheckerName, _T("getCheckerName"), 0,TYPE_TSTR_BV,0,1,
		_T("index"), 0, TYPE_INDEX,
	IGeometryCheckerManager_Imp::kGetGeometryCheckerID, _T("getCheckerID"), 0,TYPE_DWORD,0,1,
		_T("index"), 0, TYPE_INDEX,
	IGeometryCheckerManager_Imp::kGetActiveGeometryCheckerID, _T("getActiveCheckerID"), 0,TYPE_DWORD,0,0,
	IGeometryCheckerManager_Imp::kSetActiveGeometryCheckerID, _T("setActiveCheckerID"), 0,TYPE_VOID,0,1,
		_T("ID"), 0, TYPE_DWORD,
	IGeometryCheckerManager_Imp::kDoesGeometryCheckerHavePropDlg, _T("doesCheckerHavePropDlg"), 0,TYPE_bool,0,1,
		_T("index"), 0, TYPE_INDEX,
	IGeometryCheckerManager_Imp::kShowGeometryCheckerPropDlg, _T("showCheckerPropDlg"), 0,TYPE_VOID,0,1,
		_T("index"), 0, TYPE_INDEX,
	IGeometryCheckerManager_Imp::kPopupMenuSelect,_T("popupMenuSelect"),0,TYPE_VOID,0,0,
	IGeometryCheckerManager_Imp::kSelectResults, _T("selectResults"),0,TYPE_VOID,0,1,
		_T("time"),0,TYPE_TIMEVALUE,
	IGeometryCheckerManager_Imp::kDisplayResults, _T("displayResults"),0,TYPE_VOID,0,6,
		_T("color"),0,TYPE_COLOR,
		_T("time"),0,TYPE_TIMEVALUE,
		_T("node"),0,TYPE_INODE,
		_T("hwnd"),0,TYPE_HWND,
		_T("type"),0,TYPE_ENUM,IGeometryCheckerManager_Imp::kOutputEnum,
		_T("results"),0,TYPE_INDEX_TAB_BR,

	IGeometryCheckerManager_Imp::kRegisterChecker,	_T("registerChecker"), 0, TYPE_INDEX, 0, 7,
		_T("checkerFunc"), 0, TYPE_VALUE,
		_T("isSupportedFunc"), 0,TYPE_VALUE,
		_T("type"), 0,TYPE_ENUM,IGeometryCheckerManager_Imp::kOutputEnum,
		_T("name"), 0, TYPE_TSTR,
		_T("popupDlgFunc"), 0, TYPE_VALUE,
		_T("textOverrideFunc"), 0,TYPE_VALUE,
		_T("displayOverrideFunc"), 0,TYPE_VALUE,
	IGeometryCheckerManager_Imp::kUnRegisterChecker,_T("unRegisterChecker"), 0, TYPE_bool, 0, 1,
		_T("name"), 0, TYPE_TSTR,
	IGeometryCheckerManager_Imp::kRunGeometryCheck,_T("runCheck"),0,TYPE_VOID,0,1,
		_T("time"),0,TYPE_TIMEVALUE,

	IGeometryCheckerManager_Imp::kGetCurrentReturnVal, _T("getCurrentReturnVal"),0,TYPE_ENUM,IGeometryCheckerManager_Imp::kOutputEnum,0,0,
	IGeometryCheckerManager_Imp::kGetCurrentOutputCount, _T("getCurrentOutputCount"),0,TYPE_INT,0,0,
	IGeometryCheckerManager_Imp::kGetCurrentString, _T("getCurrentString"),0,TYPE_TSTR_BV,0,0,

    properties,
	  IGeometryCheckerManager_Imp::kGetGeometryCheckerOn,IGeometryCheckerManager_Imp::kSetGeometryCheckerOn, _T("on"), 0,TYPE_bool,
	  IGeometryCheckerManager_Imp::kGetActiveGeometryChecker,IGeometryCheckerManager_Imp::kSetActiveGeometryChecker, _T("activeIndex"), 0,TYPE_INDEX,
	  IGeometryCheckerManager_Imp::kGetSeeThrough,IGeometryCheckerManager_Imp::kSetSeeThrough, _T("seeThrough"), 0,TYPE_bool,
	  IGeometryCheckerManager_Imp::kGetAutoUpdate,IGeometryCheckerManager_Imp::kSetAutoUpdate, _T("autoUpdate"), 0,TYPE_bool,
	  IGeometryCheckerManager_Imp::kGetDisplayTextUpTop,IGeometryCheckerManager_Imp::kSetDisplayTextUpTop, _T("displayTextUpTop"), 0,TYPE_bool,

	enums,
		IGeometryCheckerManager_Imp::kOutputEnum, 4,	
		_T("Failed"), GUPChecker::kFailed,
		_T("Vertices"), GUPChecker::kVertices,
		_T("Edges"), GUPChecker::kEdges,
		_T("Faces"), GUPChecker::kFaces,

	end	
																			
  );


class GeometryManagerClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return TRUE; }
	void *			Create(BOOL loading = FALSE) {return &IGeometryCheckerManager_Imp::_theInstance; }
	const TCHAR *	ClassName() { return GetString(IDS_GEOMETRY_CHECKER_CLASS); }
	SClass_ID		SuperClassID() { return GUP_CLASS_ID; }
	Class_ID		ClassID() { return GEOMETRY_MANAGER_CLASS_ID; }
	const TCHAR* 	Category() { return _T(""); }

	const TCHAR*	InternalName() { return _T("GeometryCheckerManager"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle

};

static  GeometryManagerClassDesc GeometyManagerClassDesc;
ClassDesc2* GetGeometryManagerClassDesc() { return &GeometyManagerClassDesc; }

static void PreReset(void *param, NotifyInfo *info)
{
	IGeometryCheckerManager_Imp *manager = static_cast<IGeometryCheckerManager_Imp*>(param);
	manager->SetGeometryCheckerOn(false); //off
}

static void PreResetAndLoad(void *param, NotifyInfo *info)
{
	IGeometryCheckerManager_Imp *manager = static_cast<IGeometryCheckerManager_Imp*>(param);
	manager->SetGeometryCheckerOn(false); //off
	manager->LoadPreferences();
}
void NotifySelectionChanged(void *param, NotifyInfo *info)
{
	IGeometryCheckerManager_Imp *manager = static_cast<IGeometryCheckerManager_Imp*>(param);
	if(manager&&manager->GetGeometryCheckerOn()==true)
		manager->SelectionChanged();
}

void  IGeometryCheckerManager_Imp::Stop()
{
	DeleteCachedResults();
}
DWORD IGeometryCheckerManager_Imp::Start()
{

	mWhichActive = 0;
	RegisterNotification(PreResetAndLoad, this, NOTIFY_SYSTEM_PRE_RESET);
	RegisterNotification(PreReset, this, NOTIFY_FILE_PRE_OPEN);
	RegisterNotification(PreReset, this, NOTIFY_SYSTEM_PRE_NEW);
	RegisterNotification(NotifySelectionChanged, this, NOTIFY_SELECTIONSET_CHANGED);
	mViewportDisplay.SetIGeometryCheckerManager(this);
	mNotifyMgr.SetNotifyCB(this);
	DeleteCachedResults();
	IGeometryChecker *geomChecker;
	mValid = false;
	mLastValidTime = -1;

	geomChecker = &FlippedFacesChecker::GetInstance(); 
	RegisterGeometryChecker(geomChecker);
	geomChecker = &OverlappingFacesChecker::GetInstance();
	RegisterGeometryChecker(geomChecker);

	geomChecker = &OpenEdgesChecker::GetInstance();
	RegisterGeometryChecker(geomChecker);
	geomChecker = &MultipleEdgeChecker::GetInstance();
	RegisterGeometryChecker(geomChecker);

	geomChecker = &IsolatedVertexChecker::GetInstance(); 
	RegisterGeometryChecker(geomChecker);
	geomChecker = &OverlappingVerticesChecker::GetInstance();
	RegisterGeometryChecker(geomChecker);
	geomChecker = &TVertsChecker::GetInstance();
	RegisterGeometryChecker(geomChecker);

	geomChecker = &MissingUVCoordinatesChecker::GetInstance();
	RegisterGeometryChecker(geomChecker);
	geomChecker = &FlippedUVWFacesChecker::GetInstance();
	RegisterGeometryChecker(geomChecker);
	geomChecker = &OverlappedUVWFacesChecker::GetInstance();
	RegisterGeometryChecker(geomChecker);
	mNextValidID = 20; //to be safe
	SetUpButton();
	mCheckOn = false; //we do this so the next call is guarenteed to turn stuff on.
	mCheckResult = MSTR("");
	mReturnVal = IGeometryChecker::eFail;
	mSeeThrough = true;
	mAutoUpdate = true;
	mDisplayTextUpTop = false;
	mShowWarningDialog = true;

	//setup the color for the display
	MSTR name, category;
	name.printf("%s",GetString(IDS_DISPLAY_COLOR));
	category.printf("%s",GetString(IDS_GEOMETRY_CHECKER));
	COLORREF displayColor = RGB(0,255,0);
	int iret = ColorMan()->RegisterColor( GEOMETRY_DISPLAY_COLOR, name, category, displayColor);
	//setup the color for the text
	name.printf("%s",GetString(IDS_TEXT_COLOR));
	iret = ColorMan()->RegisterColor( GEOMETRY_TEXT_COLOR, name, category, displayColor);

	//we register the buttons but turn leave them disabled
	IViewportButtonManager *man = static_cast<IViewportButtonManager*>(GetCOREInterface(IVIEWPORTBUTTONMANAGER_INTERFACE ));
	if(man)
	{
		man->RegisterButton(&mAutoUpdateButton); //we register this order so
		//when we turn on the configure button it doesn't return true for the action
		//that turned it on.
		man->RegisterButton(&mConfigureButton);
		man->RegisterButton(&mGeometryButton);
	
		mAutoUpdateButton.SetEnabled(mCheckOn);
		mConfigureButton.SetEnabled(mCheckOn);
		mGeometryButton.SetEnabled(mCheckOn);
	}

	LoadPreferences();
	return GUPRESULT_KEEP;
}

IGeometryCheckerManager_Imp::~IGeometryCheckerManager_Imp()
{
	DeleteCachedResults();
	//unregister the buttons but turn leave them disabled
	IViewportButtonManager *man = static_cast<IViewportButtonManager*>(GetCOREInterface(IVIEWPORTBUTTONMANAGER_INTERFACE ));
	if(man)
	{
		man->UnRegisterButton(&mAutoUpdateButton); //we register this order so
		//when we turn on the configure button it doesn't return true for the action
		//that turned it on.
		man->UnRegisterButton(&mConfigureButton);
		man->UnRegisterButton(&mGeometryButton);
	
	}


	UnRegisterNotification(PreReset, this, NOTIFY_SYSTEM_PRE_RESET);
	UnRegisterNotification(PreReset, this, NOTIFY_FILE_PRE_OPEN);
	UnRegisterNotification(PreReset, this, NOTIFY_SYSTEM_PRE_NEW);
	UnRegisterNotification(NotifySelectionChanged, this, NOTIFY_SELECTIONSET_CHANGED);
	//delete all registered checkers
	for(int i= GetNumRegisteredGeometryCheckers()-1;i>-1;--i)
	{
		UnRegisterGeometryChecker(GetNthGeometryChecker(i));		
	}
}
int IGeometryCheckerManager_Imp::RegisterGeometryChecker(IGeometryChecker * geomChecker)
{
	if(geomChecker) //only register it if it exists.
	{
		if(IsGeometryCheckerAlreadyRegistered(geomChecker)==false)
		{
			mGeometryCheckers.Append(1,&geomChecker);
			return mGeometryCheckers.Count()-1;
		}
	}
	return -1;
}
void IGeometryCheckerManager_Imp::UnRegisterGeometryChecker(IGeometryChecker *geomChecker)
{
	if(geomChecker)
	{
		for(int i =0;0<mGeometryCheckers.Count();++i)
		{
			if(mGeometryCheckers[i]==geomChecker)
			{
				mGeometryCheckers.Delete(i,1);
				//now if MXS checker we delete and remove the funcs on it
				MXSGeometryChecker *mxschecker = dynamic_cast<MXSGeometryChecker*>(geomChecker->GetInterface(MXS_GEOMETRY_CHECKER_INTERFACE));
				if(mxschecker)
				{
					mxschecker->CleanOutFuncs();
					delete mxschecker;
				}
				break;
			}
		}
		int index = GetActivatedGeometryCheckerIndex();
		if(index>=GetNumRegisteredGeometryCheckers())
			index = GetNumRegisteredGeometryCheckers()-1;
	}
}
int IGeometryCheckerManager_Imp::GetNumRegisteredGeometryCheckers()const
{
	return mGeometryCheckers.Count();
}

IGeometryChecker * IGeometryCheckerManager_Imp::GetNthGeometryChecker(int nth)const
{
	if(nth<mGeometryCheckers.Count()&&nth>=0)
	{
		return mGeometryCheckers[nth];
	}
	return NULL;
}

MSTR IGeometryCheckerManager_Imp::GetNthGeometryCheckerName(int nth)const
{
	if(nth<mGeometryCheckers.Count()&&nth>=0&&mGeometryCheckers[nth])
	{
		return mGeometryCheckers[nth]->GetName();
	}
	return MSTR();
}

DWORD IGeometryCheckerManager_Imp::GetNthGeometryCheckerID(int nth)const
{
	if(nth<mGeometryCheckers.Count()&&nth>=0&&mGeometryCheckers[nth])
	{
		return mGeometryCheckers[nth]->GetCheckerID();
	}
	return -1;
}

bool IGeometryCheckerManager_Imp::DoesGeometryCheckerHavePropDlg(int nth)const
{
	if(nth<mGeometryCheckers.Count()&&nth>=0&&mGeometryCheckers[nth])
	{
		return mGeometryCheckers[nth]->HasPropertyDlg();
	}
	return false;
}

void IGeometryCheckerManager_Imp::ShowGeometryCheckerPropDlg(int nth)
{
	if(nth<mGeometryCheckers.Count()&&nth>=0&&mGeometryCheckers[nth])
	{
		mGeometryCheckers[nth]->ShowPropertyDlg();
	}
}


IGeometryChecker * IGeometryCheckerManager_Imp::GetActivatedGeometryChecker()const
{
	int index = GetActivatedGeometryCheckerIndex();
	if(index>=0&&index<GetNumRegisteredGeometryCheckers())
	{
		return GetNthGeometryChecker(index);
	}
	return NULL;
}

bool IGeometryCheckerManager_Imp::ActivateGeometryChecker( int nth)
{
	if(nth<mGeometryCheckers.Count()&&nth>=0)
	{
		mWhichActive = nth;
		SetUpButton();
		SelectionChanged();
		return true;
	}
	return false;
		
}
int IGeometryCheckerManager_Imp::GetActivatedGeometryCheckerIndex()const
{
	return mWhichActive;
}


bool IGeometryCheckerManager_Imp::IsGeometryCheckerAlreadyRegistered(IGeometryChecker *checker)
{
	//okay for lineary search.rarely happens
	for(int i =0;i<mGeometryCheckers.Count();++i)
	{
		if(mGeometryCheckers[i]==checker)
			return true;
	}
	return false;
}



//Set Up the button based upon who is activated
void IGeometryCheckerManager_Imp::SetUpButton()
{

	IGeometryChecker *checker = GetActivatedGeometryChecker();
	if(checker&&GetGeometryCheckerOn())
	{
		mGeometryButton.SetEnabled(true);

		MSTR name = checker->GetName();
		MSTR str = checker->HasTextOverride()&&GetCurrentReturnVal()!=IGeometryChecker::eFail ? checker->TextOverride() : GetCurrentString();
		MSTR output;
		if(str.Length()>0)
			output = name+MSTR(": ")+str;
		else
			output = name;
		mGeometryButton.SetLabel(output);
		
		IPoint2 loc(LABELX,LABELY);
		mGeometryButton.SetLocation(loc);
		Color textColor;
		COLORREF cr = ColorMan()->GetColor(GEOMETRY_TEXT_COLOR);
		textColor = Color(cr);
		mGeometryButton.SetColor(textColor); //refreshes display
		
		if(checker->HasPropertyDlg())
		{
			MSTR str = GetString(IDS_CLICK_HERE_TO_CONFIGURE);
			mConfigureButton.SetEnabled(true);
			mConfigureButton.SetLabel(str);
			mConfigureButton.SetColor(textColor);
			loc.y+=OFFSET;
			mConfigureButton.SetLocation(loc);
		}
		else 
			mConfigureButton.SetEnabled(false);
		if(GetAutoUpdate()==false)
		{
			MSTR str = GetString(IDS_CLICK_HERE_TO_UPDATE);
			mAutoUpdateButton.SetEnabled(true);
			mAutoUpdateButton.SetLabel(str);
			mAutoUpdateButton.SetColor(textColor);
			loc.y+=OFFSET;
			mAutoUpdateButton.SetLocation(loc);
		}
		else
			mAutoUpdateButton.SetEnabled(false);

		ViewExp *vpt = GetCOREInterface()->GetActiveViewport();
		ResetButtonLocations(vpt);
		GetCOREInterface()->ReleaseViewport(vpt);
	}
	else
	{
		mGeometryButton.SetEnabled(false);
		mConfigureButton.SetEnabled(false);
		mAutoUpdateButton.SetEnabled(false);
	}

	//double check to make sure the display is refreshed.
	IViewportButtonManager *vpm = static_cast<IViewportButtonManager*>
											(GetCOREInterface(IVIEWPORTBUTTONMANAGER_INTERFACE ));
	if(vpm)
		vpm->RefreshButtonDisplay();
}

//when the geometry checker is on we need to make sure that we are drawing text. and also make sure that
//we are 
void IGeometryCheckerManager_Imp::SetGeometryCheckerOn(bool on)
{
	if(on!=mCheckOn)
	{
		mCheckOn = on;
		if(mCheckOn)
		{
		    GetCOREInterface()->RegisterViewportDisplayCallback(FALSE,&mViewportDisplay);
			//register the geometry button
			SetUpButton();
			mAutoUpdateButton.SetEnabled(true);
			mConfigureButton.SetEnabled(true);
			mGeometryButton.SetEnabled(true);
			SelectionChanged(); //okay this resets up the UI
		}
		else
		{
		    GetCOREInterface()->UnRegisterViewportDisplayCallback(FALSE,&mViewportDisplay);
			mAutoUpdateButton.SetEnabled(false);
			mConfigureButton.SetEnabled(false);
			mGeometryButton.SetEnabled(false);
			mNotifyMgr.DeleteAllRefsFromMe(); //okay checker is off delete everything.
			IViewportButtonManager *vpm = static_cast<IViewportButtonManager*>
											(GetCOREInterface(IVIEWPORTBUTTONMANAGER_INTERFACE ));
			if(vpm)
				vpm->RefreshButtonDisplay();
			GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
		}
	}
	else if(on==false)
	{
		mNotifyMgr.DeleteAllRefsFromMe(); //okay if checker was off double make sure references are killed.
	}
}

//this opens the menu and selects a new object.
void IGeometryCheckerManager_Imp::PopupMenuSelect()
{

	static const int MENU_SELECT = -1;
	static const int MENU_SEE_THROUGH = -2;
	static const int MENU_AUTO_UPDATE = -3;
	static const int MENU_CONFIGURE = -4;
	static const int MENU_DISPLAY_ON_TOP = -5;
	HMENU hMenu = CreatePopupMenu();
	int checked = 0;
	for (int i = 0; i < mGeometryCheckers.Count();i++)
	{
		if(mGeometryCheckers[i])
		{
			if(i == mWhichActive)
				checked= MF_CHECKED;
			else 
				checked = 0;
			AppendMenu(  hMenu,      // handle to menu to be changed
				   MF_ENABLED | MF_STRING | checked,      // menu-item flags
				  i+1,  // menu-item identifier or handle to drop-down menu or submenu. we add 1 since 0 is a click outside of the menu
				  mGeometryCheckers[i]->GetName() // menu-item content
				  );
		}
	
	}
	if(mGeometryCheckers.Count()>0)
	{
		AppendMenu(  hMenu,      // handle to menu to be changed
				   MF_SEPARATOR,      // menu-item flags
				  0,  // menu-item identifier or handle to drop-down menu or submenu
				  NULL // menu-item content
				  );
		AppendMenu(  hMenu,      // handle to menu to be changed
				   MF_ENABLED | MF_STRING,      // menu-item flags
				  MENU_SELECT,  // menu-item identifier or handle to drop-down menu or submenu
				  GetString(IDS_SELECT_RESULTS) // menu-item content
				  );
		AppendMenu(  hMenu,      // handle to menu to be changed
				   MF_SEPARATOR,      // menu-item flags
				  0,  // menu-item identifier or handle to drop-down menu or submenu
				  NULL // menu-item content
				  );
		if(GetSeeThrough())
			checked= MF_CHECKED;
		else
			checked = 0;
		AppendMenu(  hMenu,      // handle to menu to be changed
				   MF_ENABLED | MF_STRING | checked,      // menu-item flags
				  MENU_SEE_THROUGH,  // menu-item identifier or handle to drop-down menu or submenu
				  GetString(IDS_SEE_THROUGH) // menu-item content
				  );
		if(GetAutoUpdate())
			checked= MF_CHECKED;
		else
			checked = 0;
		AppendMenu(  hMenu,      // handle to menu to be changed
				   MF_ENABLED | MF_STRING | checked,      // menu-item flags
				  MENU_AUTO_UPDATE,  // menu-item identifier or handle to drop-down menu or submenu
				  GetString(IDS_AUTO_UPDATE) // menu-item content
				  );
		if(GetDisplayTextUpTop())
			checked= MF_CHECKED;
		else
			checked = 0;
		AppendMenu(  hMenu,      // handle to menu to be changed
				   MF_ENABLED | MF_STRING | checked,      // menu-item flags
				  MENU_DISPLAY_ON_TOP,  // menu-item identifier or handle to drop-down menu or submenu
				  GetString(IDS_DISPLAY_ON_TOP) // menu-item content
				  );
		AppendMenu(  hMenu,      // handle to menu to be changed
				   MF_SEPARATOR,      // menu-item flags
				  0,  // menu-item identifier or handle to drop-down menu or submenu
				  NULL // menu-item content
				  );
		if(DoesGeometryCheckerHavePropDlg(mWhichActive))
			checked= MF_ENABLED;
		else
			checked = MF_DISABLED|MF_GRAYED;
		AppendMenu(  hMenu,      // handle to menu to be changed
				   MF_STRING | checked,      // menu-item flags
				  MENU_CONFIGURE,  // menu-item identifier or handle to drop-down menu or submenu
				  GetString(IDS_CONFIGURE) // menu-item content
				  );
	}
	POINT lpPoint;   
	GetCursorPos( &lpPoint );  // address of structure for cursor position); 

	ViewExp* vexp =GetCOREInterface()->GetActiveViewport();
	if(vexp)
	{
		HWND hparent = vexp->GetHWnd();

		int id = TrackPopupMenuEx(hMenu, 
			TPM_TOPALIGN | TPM_VCENTERALIGN | TPM_RIGHTBUTTON  | TPM_RETURNCMD , 
			lpPoint.x, lpPoint.y,  hparent, NULL);
		if(id>=1&&id<=GetNumRegisteredGeometryCheckers())
		{
			--id; //subtact the id since we added 1 when it was registered.
			if(id!=mWhichActive)
			{
				SetGeometryCheckerOn(true);
				ActivateGeometryChecker(id);
			}
			else
				SetGeometryCheckerOn(false);//turn it off if it's active

		}
		else if(id==MENU_SELECT)
		{
			SelectResults(GetCOREInterface()->GetTime());
		}
		else if(id==MENU_SEE_THROUGH)
		{
			SetSeeThrough(! GetSeeThrough());
		}
		else if(id==MENU_AUTO_UPDATE)
		{
			SetAutoUpdate(! GetAutoUpdate());
		}
		else if(id==MENU_DISPLAY_ON_TOP)
		{
			SetDisplayTextUpTop(! GetDisplayTextUpTop());
		}
		else if(id==MENU_CONFIGURE)
		{
			ShowGeometryCheckerPropDlg(mWhichActive);
		}
		GetCOREInterface()->ReleaseViewport(vexp);
		DestroyMenu(hMenu);   
	}
}

void IGeometryCheckerManager_Imp::SetBitArray(BitArray &bitArray,int bitSize,Tab<int>&results)
{
	bitArray.SetSize(bitSize);
	bitArray.ClearAll();
	for(int i=0;i<results.Count();++i)
	{
		int index = results[i];
		bitArray.Set(index);
	}
}

//we need to redraw the selection over the error tests,

void IGeometryCheckerManager_Imp::SelectResults(TimeValue t)
{
	if(mLocalResults.Count()>0) //we have some results...
	{
		for(int i =0;i<mLocalResults.Count();++i)
		{
			INode *node = mLocalResults[i]->mNode;
			
			ObjectState os  = node->EvalWorldState(t);
			Object *obj = os.obj;
			BitArray selection;
			if(os.obj&&os.obj->IsSubClassOf(triObjectClassID))
			{
				TriObject *tri = dynamic_cast<TriObject *>(os.obj);
				if(tri)
				{
					Mesh &mesh = tri->GetMesh();
					IMeshSelect* ms = (node) ? GetMeshSelectInterface(tri) : NULL;
					IMeshSelectData* msd = (node) ? GetMeshSelectDataInterface(tri) : NULL;
					if (ms && msd)
					{
						switch(mReturnVal)
						{
							case IGeometryChecker::eVertices:
								SetBitArray(selection,mesh.numVerts,mLocalResults[i]->mIndex);
								msd->SetVertSel(selection, ms, t);
								ms->LocalDataChanged();
								break;
							case IGeometryChecker::eEdges:
								SetBitArray(selection,3*mesh.numFaces,mLocalResults[i]->mIndex);
								msd->SetEdgeSel(selection, ms, t);
								ms->LocalDataChanged();
								break;
							case IGeometryChecker::eFaces:
								SetBitArray(selection,mesh.numFaces,mLocalResults[i]->mIndex);
								msd->SetFaceSel(selection, ms, t);
								ms->LocalDataChanged();
								break;

						}
					}
				}
			}
			else if(os.obj&&os.obj->IsSubClassOf(polyObjectClassID))
			{
				PolyObject *poly = dynamic_cast<PolyObject *>(os.obj);
				if(poly)
				{
					MNMesh &mnMesh = poly->GetMesh();
					IMeshSelect* ms = (node) ? GetMeshSelectInterface(poly) : NULL;
					IMeshSelectData* msd = (node) ? GetMeshSelectDataInterface(poly) : NULL;
					if (ms && msd)
					{
						switch(mReturnVal)
						{
							case IGeometryChecker::eVertices:
								SetBitArray(selection, mnMesh.numv,mLocalResults[i]->mIndex);
								msd->SetVertSel(selection, ms, t);
								ms->LocalDataChanged();
								break;
							case IGeometryChecker::eEdges:
								SetBitArray(selection,mnMesh.nume,mLocalResults[i]->mIndex);
								msd->SetEdgeSel(selection, ms, t);
								ms->LocalDataChanged();
								break;
							case IGeometryChecker::eFaces:
								SetBitArray(selection,mnMesh.numf,mLocalResults[i]->mIndex);
								msd->SetFaceSel(selection, ms, t);
								ms->LocalDataChanged();
								break;

						}
					}
				}
			}
			//send this out to do a redraw
			node->NotifyDependents(Interval(t,t),PART_DISPLAY,REFMSG_CHANGE);
			GetCOREInterface10()->InvalidateAllViewportRects();
		}
	}

}

bool IGeometryCheckerManager_Imp::GetGeometryCheckerOn()const
{
	return mCheckOn;
}

	
bool IGeometryCheckerManager_Imp::GetSeeThrough()const
{
	return mSeeThrough;
}
void IGeometryCheckerManager_Imp::SetSeeThrough(bool val)
{
	mSeeThrough = val;
	IViewportButtonManager *man = static_cast<IViewportButtonManager*>(GetCOREInterface(IVIEWPORTBUTTONMANAGER_INTERFACE ));
	if(man)
		man->RefreshButtonDisplay();
	GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
	SavePreferences();
}

bool IGeometryCheckerManager_Imp::GetAutoUpdate()const
{
	return mAutoUpdate;
}
void IGeometryCheckerManager_Imp::SetAutoUpdate(bool val)
{
	mAutoUpdate = val;
	SetUpButton();
	IViewportButtonManager *man = static_cast<IViewportButtonManager*>(GetCOREInterface(IVIEWPORTBUTTONMANAGER_INTERFACE ));
	if(man)
		man->RefreshButtonDisplay();
	GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
	SavePreferences();

}

bool IGeometryCheckerManager_Imp::GetDisplayTextUpTop()const
{
	return mDisplayTextUpTop;
}

void IGeometryCheckerManager_Imp::SetDisplayTextUpTop(bool val)
{
	mDisplayTextUpTop = val;
	SetUpButton();
	IViewportButtonManager *man = static_cast<IViewportButtonManager*>(GetCOREInterface(IVIEWPORTBUTTONMANAGER_INTERFACE ));
	if(man)
		man->RefreshButtonDisplay();
	GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
	SavePreferences();

}

//struct that gets passed in below...
struct CheckerAndNumOfFaces
{
	IGeometryCheckerManager_Imp *mMan;
	int mNumFaces;
};

static INT_PTR CALLBACK WarningDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CheckerAndNumOfFaces *man = DLGetWindowLongPtr<CheckerAndNumOfFaces *>(hDlg);

	switch (msg) {
	case WM_INITDIALOG:
		{
			PositionWindowNearCursor(hDlg);
			DLSetWindowLongPtr(hDlg, lParam);
			man = (CheckerAndNumOfFaces *)lParam;
			TCHAR newStr[512];
			TCHAR *str = GetString(IDS_TEXT_WARNING);
			sprintf(newStr,str,man->mNumFaces);
			//set the text up with the number of faces.
			SetWindowText(GetDlgItem(hDlg,IDC_TEXT_WARNING),(newStr));
			//check the buttons
			CheckDlgButton(hDlg,IDC_MANUAL,TRUE);
			CheckDlgButton(hDlg,IDC_TURN_OFF,FALSE);
			CheckDlgButton(hDlg,IDC_DONT_SHOW,FALSE);
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
            goto done;
		}
		break;

	case WM_CLOSE:
done:
		if(IsDlgButtonChecked(hDlg,IDC_MANUAL))
			man->mMan->SetAutoUpdate(false);
		if(IsDlgButtonChecked(hDlg,IDC_TURN_OFF))
			man->mMan->SetGeometryCheckerOn(false);
		if(IsDlgButtonChecked(hDlg,IDC_DONT_SHOW))
			man->mMan->SetShowWarningDialog(false);
		EndDialog(hDlg, TRUE);
		return FALSE;
	}
	return FALSE;
}

//pops up the warning dialog if autoupdate is on, and the checker is on of course, and we are still showing warnings.
bool IGeometryCheckerManager_Imp::ShowWarningDialogThenStop(int numFaces) 
{											
	if(mShowWarningDialog && GetAutoUpdate()==true &&GetGeometryCheckerOn()==true)
	{
		CheckerAndNumOfFaces tempStruct;
		tempStruct.mMan = this; 
		tempStruct.mNumFaces = numFaces;
		HWND hDlg = GetCOREInterface()->GetMAXHWnd();
		DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_WARNING_DIALOG),
			hDlg, WarningDlgProc, (LPARAM)&tempStruct);
		if(GetAutoUpdate()==false||GetGeometryCheckerOn()==false) //user turned off auto update or the checker off so don't continue!
			return true;
	}
	return false;
}

//unction that's in CheckerImplementations.h
bool DialogChecker::ShowWarningDialogThenStop(int numFaces)
{
	IGeometryCheckerManager_Imp& instance =  IGeometryCheckerManager_Imp::GetImpInstance();
	return instance.ShowWarningDialogThenStop(numFaces);
}
void IGeometryCheckerManager_Imp::SetShowWarningDialog(bool set)   //set the internal flag. This is also stored in the geometrychecker .ini file.
{
	mShowWarningDialog = set;
}

void IGeometryCheckerManager_Imp::DeleteCachedResults()
{
	for(int i =0;i<mLocalResults.Count();++i)
	{
		if(mLocalResults[i])
			delete mLocalResults[i];
	}
	mLocalResults.ZeroCount();
	mViewportDisplay.DeleteCachedResults();

}

void IGeometryCheckerManager_Imp::ClearResults()
{
	mValid = false;
	mLastValidTime = -1;
	mCheckResult = MSTR(_T(GetString(IDS_FAILED)));
	mReturnVal = IGeometryChecker::eFail;
	DeleteCachedResults();
	SetUpButton();
}

bool IGeometryCheckerManager_Imp::NotValid(TimeValue t)
{
	if(GetAutoUpdate()==true) //if auto update is off, it's always valid
	{
		if(mValid==false)
			return true;
		//now check all of the validatiy intervals of the objects against the time and the last time run
		IGeometryChecker *checker = GetActivatedGeometryChecker();
		if(checker&&mNotifyMgr.NumRefs()>0)
		{
			int i;
			for(i =0;i<mNotifyMgr.NumRefs();++i)
			{
				INode *node= static_cast<INode *>(mNotifyMgr.GetReference(i));
				if(node)
				{
					if(checker->IsSupported(node))
					{
						Object * obj   = node->EvalWorldState(GetCOREInterface()->GetTime()).obj;
						if(obj)
						{
							Interval valid = obj->ChannelValidity(t,GEOM_CHAN_NUM);  //we use ChannelValidity instead of Object::Validity since we now the objects aren't
					        valid &= obj->ChannelValidity(t,TOPO_CHAN_NUM);			 //procedural. They are meshes or polys.
							if(valid.InInterval(mLastValidTime)==FALSE)
								return true;
						}
					}
				}
			}
		}
	}
	return false;

}

void IGeometryCheckerManager_Imp::RunGeometryCheck(TimeValue t)
{
	if(GetGeometryCheckerOn())
	{
		static bool runningCheck = false;

		if(runningCheck==false)
		{
			HCURSOR hOldCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
			runningCheck = true;
			IGeometryChecker *checker = GetActivatedGeometryChecker();
			ClearResults();

			if(checker&&mNotifyMgr.NumRefs()>0)
			{
				IGeometryChecker::ReturnVal returnVal;
				IGeometryChecker::OutputVal val;
				int totalCount=0;
				int i;
				bool noSelectionWasValid = true;
				for(i =0;i<mNotifyMgr.NumRefs();++i)
				{
					INode *nodeToCheck = static_cast<INode *>(mNotifyMgr.GetReference(i));
					if(nodeToCheck)
					{
						if(checker->IsSupported(nodeToCheck))
						{
							noSelectionWasValid = false;
							returnVal = checker->GeometryCheck(t,nodeToCheck,val);
							if(returnVal!=IGeometryChecker::eFail)
							{
								//at least one doesn't fail then we set the returnVal to the proper type.
								mReturnVal = returnVal;
								totalCount += val.mIndex.Count();
								LocalResults *lresult = new LocalResults();
								lresult->mIndex = val.mIndex;
								lresult->mNode = nodeToCheck;
								mLocalResults.Append(1,&lresult);
							}
						}
					}
				}
				MSTR type;
				TCHAR count[32];
				sprintf(count,"%d",totalCount);
				if(noSelectionWasValid==false)
				{
					switch(mReturnVal)
					{
					case IGeometryChecker::eFail:
						mCheckResult = MSTR(_T(GetString(IDS_FAILED)));
						break;
					case IGeometryChecker::eVertices:
						type = MSTR(_T(GetString(IDS_VERTICES)));
						mCheckResult = MSTR(count) + MSTR(" ") + type;
						break;
					case IGeometryChecker::eEdges:
						type = MSTR(_T(GetString(IDS_EDGES)));
						mCheckResult = MSTR(count) + MSTR(" ") + type;
						break;
					case IGeometryChecker::eFaces:
						type = MSTR(_T(GetString(IDS_FACES)));
						mCheckResult = MSTR(count) + MSTR(" ") + type;
						break;
					}
				}
				else
					mCheckResult = MSTR(GetString(IDS_UNSUPPORTED_OBJECT_TYPE));
			}
			else
			{
				mCheckResult = MSTR(GetString(IDS_NO_SELECTION));
			}
			runningCheck = false;
			mValid = true;
			mLastValidTime = t;
			SetCursor(hOldCursor);
		}//if runningCheck
		//the checker may get turned off. so check again.
		if(GetGeometryCheckerOn())
			SetUpButton();
		GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
	}
	else
		ClearResults();

}

IGeometryChecker::ReturnVal IGeometryCheckerManager_Imp::GeometryCheck(TimeValue t,INode *nodeToCheck, IGeometryChecker::OutputVal &val)
{
	IGeometryChecker *checker = GetActivatedGeometryChecker();
	IGeometryChecker::ReturnVal returnVal = IGeometryChecker::eFail;
	val.mIndex.ZeroCount();

	if(checker&&nodeToCheck>0)
	{
		returnVal = checker->GeometryCheck(t, nodeToCheck,val);
	}
	return returnVal;
}

void IGeometryCheckerManager_Imp::SelectionChanged()
{
	mNotifyMgr.RemoveAllReferences();
	int selCount = 	GetCOREInterface()->GetSelNodeCount();
	//need to finish this of course
	if(selCount>0)
	{
		mNotifyMgr.SetNumberOfReferences(selCount);
		for(int i =0;i<selCount;++i)
		{
			INode *nodeToCheck = GetCOREInterface()->GetSelNode(i);
			mNotifyMgr.CreateReference(i,nodeToCheck);
		}  
	}
	if(mAutoUpdate==true)
		RunGeometryCheck(GetCOREInterface()->GetTime());
	else
		ClearResults();

	IViewportButtonManager *vpm = static_cast<IViewportButtonManager*>
									(GetCOREInterface(IVIEWPORTBUTTONMANAGER_INTERFACE ));
	if(vpm)
		vpm->RefreshButtonDisplay();
	GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
}

RefResult IGeometryCheckerManager_Imp::NotifyFunc(Interval changeInt,
         RefTargetHandle hTarget, PartID& partID,
            RefMessage message)
{
	RefResult res = REF_SUCCEED;
	if( mCheckOn&& (
		(message ==REFMSG_CHANGE&& ( (partID&PART_GEOM)||partID&PART_TOPO||partID&PART_TEXMAP||partID&PART_MTL|| partID&PART_VERTCOLOR))
		 ||message==REFMSG_SUBANIM_STRUCTURE_CHANGED //REFMSG_SUBANIM_STRUCTURE_CHANGED handles case when we convert the object to a different type
		 ))
	{
		if(mAutoUpdate)
			Invalidate();
		else
			ClearResults();
	}
	return res;
}

void IGeometryCheckerManager_Imp::Invalidate()
{
	mValid= false;
	mLastValidTime = -1;
	//also invalidate the active viewport...
	ViewExp* vp = GetCOREInterface()->GetActiveViewport();
	ViewExp10* vp10 = NULL;
	if (vp != NULL)
	{
		vp10 = reinterpret_cast<ViewExp10*>(vp->Execute(ViewExp::kEXECUTE_GET_VIEWEXP_10));
		if(vp10)
			vp10->Invalidate();
		GetCOREInterface()->ReleaseViewport(vp);
	}
}
MSTR IGeometryCheckerManager_Imp::GetCurrentString()
{
	return mCheckResult;
}

void IGeometryCheckerManager_Imp::ResetButtonLocations(ViewExp *vpt)
{
	//first found out if we are up top or on the bottom..
	IPoint2 loc(LABELX,LABELY);
	IGeometryChecker *checker = GetActivatedGeometryChecker();
	if(checker&&vpt)
	{


		//now reset the center location, basically the x
		mGeometryButton.ResetLocation(vpt);
		mConfigureButton.ResetLocation(vpt);
		mAutoUpdateButton.ResetLocation(vpt);
	}
}


IGeometryChecker::ReturnVal IGeometryCheckerManager_Imp::GetCurrentReturnVal() 
{
	return mReturnVal;
}

void IGeometryCheckerManager_Imp::DisplayResults_FPS(Color *c,TimeValue t,INode *node, HWND hwnd,int type,Tab<int> &indices)
{
	IGeometryChecker::ReturnVal returnval= IGeometryChecker::eFail;
	switch(type)
	{
	case 0x0:
		returnval = IGeometryChecker::eFail;
	case 0x1:
		returnval = IGeometryChecker::eVertices;
	case 0x2:
		returnval = IGeometryChecker::eEdges;
	case 0x3:
		returnval = IGeometryChecker::eFaces;
	}
	DisplayIn3DViewport::DisplayParams params;
	params.mC = c;
	params.mSeeThrough = mSeeThrough;
	params.mLighted = true;
	params.mFlipFaces= false;
	params.mDrawEdges = false;
	mViewportDisplay.m3D.DisplayResults(params,t,node,hwnd,returnval,indices);
}


void IGeometryCheckerManager_Imp::DisplayResults(Color *c,TimeValue t,INode *node, HWND hwnd, IGeometryChecker::ReturnVal type,Tab<int> &indices)
{
	DisplayIn3DViewport::DisplayParams params;
	params.mC = c;
	params.mSeeThrough = mSeeThrough;
	params.mLighted = true;
	params.mFlipFaces= false;
	params.mDrawEdges = false;
	mViewportDisplay.m3D.DisplayResults(params,t,node,hwnd,type,indices);
}


int IGeometryCheckerManager_Imp::GetCurrentOutputCount()
{
	int totalCount = 0;
	for(int i =0;i<mLocalResults.Count();++i)
	{
		LocalResults *lr = mLocalResults[i];
		if(lr)
			totalCount += lr->mIndex.Count();
	}
	return totalCount;
}

GeometryViewportDisplay::GeometryViewportDisplay()
{
	mGeomCheckerMan = NULL;
}

GeometryViewportDisplay::~GeometryViewportDisplay()
{
}


//viewport display callback
void GeometryViewportDisplay::Display(TimeValue t, ViewExp *vpt, int flags)
{
	//first make sure that the geom checker is on and we have a geom checker we can check and we aren't adaptively degrading.
	ViewExp11* vp11 = NULL;
	vp11 = reinterpret_cast<ViewExp11*>(vpt->Execute(ViewExp::kEXECUTE_GET_VIEWEXP_11));
	if(mGeomCheckerMan&&mGeomCheckerMan->GetNumRegisteredGeometryCheckers()>0&&
		mGeomCheckerMan->GetActivatedGeometryCheckerIndex()>-1&&mGeomCheckerMan->GetActivatedGeometryChecker()&&(vp11==NULL||vp11->IsDegrading()==FALSE))
	{
		if(mGeomCheckerMan->NotValid(t)==true)
		{
			mGeomCheckerMan->RunGeometryCheck(t);
			mGeomCheckerMan->mValid = true;
			mGeomCheckerMan->mLastValidTime = t;
		}
		//now check that the viewport is good.
		if(vpt->IsActive()==TRUE &&vpt->GetHWnd())
		{
			IGeometryChecker *activeChecker =mGeomCheckerMan->GetActivatedGeometryChecker();
			mGeomCheckerMan->ResetButtonLocations(vpt);
				
			IGeometryChecker::ReturnVal returnVal = mGeomCheckerMan->GetCurrentReturnVal();
			for(int i=0;i<mGeomCheckerMan->mLocalResults.Count();++i)
			{
				IGeometryCheckerManager_Imp::LocalResults *lr = mGeomCheckerMan->mLocalResults[i];
				if(lr&&lr->mNode)
				{
					//also display the 3D..
					if(activeChecker->HasDisplayOverride())
					{
						activeChecker->DisplayOverride(t,lr->mNode,vpt->GetHWnd(),lr->mIndex);
					}
					else
					{

						Color c;
						COLORREF cr = ColorMan()->GetColor(GEOMETRY_DISPLAY_COLOR);
						c = Color(cr);
						ObjectState os = lr->mNode->EvalWorldState(t);
						IGeometryChecker::OutputVal val;
						val.mIndex =  lr->mIndex;

						DisplayIn3DViewport::DisplayParams params;
						params.mC = &c;
						params.mSeeThrough = mGeomCheckerMan->GetSeeThrough();
						params.mLighted = true;
						params.mFlipFaces= false;
						params.mDrawEdges = false;
						if(os.obj&&os.obj->IsSubClassOf(triObjectClassID))
						{
							TriObject *tri = dynamic_cast<TriObject *>(os.obj);
							m3D.DisplayTriObjectResults(params,t,lr->mNode,vpt,tri,mGeomCheckerMan->GetCurrentReturnVal(),val);
						}
						else if(os.obj&&os.obj->IsSubClassOf(polyObjectClassID))
						{
							PolyObject *poly = dynamic_cast<PolyObject *>(os.obj);
							m3D.DisplayPolyObjectResults(params,t,lr->mNode,vpt,poly,mGeomCheckerMan->GetCurrentReturnVal(),val);
						}
					}
				}
			}
		}
	}
}

void GeometryViewportDisplay::GetViewportRect( TimeValue t, ViewExp *vpt, Rect *rect )
{
	if(mGeomCheckerMan)
	{
		GraphicsWindow *gw = vpt->getGW();
		mGeomCheckerMan->mGeometryButton.GetViewportRect(t,vpt,rect);
		if(mGeomCheckerMan->mConfigureButton.GetEnabled())
		{
			Rect newRect;
			mGeomCheckerMan->mConfigureButton.GetViewportRect(t,vpt,&newRect);
			*rect += newRect;
		}
		if(mGeomCheckerMan->mAutoUpdateButton.GetEnabled())
		{
			Rect newRect;
			mGeomCheckerMan->mAutoUpdateButton.GetViewportRect(t,vpt,&newRect);
			*rect += newRect;
		}

		for(int i=0;i<mGeomCheckerMan->mLocalResults.Count();++i)
		{
			IGeometryCheckerManager_Imp::LocalResults *lr = mGeomCheckerMan->mLocalResults[i];
			if(lr&&lr->mNode)
			{
				//get world bbox and put into screen space. Grow the rect by that location 
				ObjectState os = lr->mNode->EvalWorldState(t);
				Box3 box;
				os.obj->GetWorldBoundBox(t,lr->mNode,vpt,box);
			
				Matrix3 identTM(1);
				gw->setTransform(identTM);		
				IPoint3 pt;
				for ( int j = 0; j < 8; j++ )
				{
					gw->wTransPoint( &box[j], &pt );
					*rect += IPoint2(pt.x,pt.y);
				}
			}
		}
	}
}

BOOL GeometryViewportDisplay::Foreground()
{
	return TRUE; //needs to be TRUE
}




void DisplayIn3DViewport::DisplayResults(DisplayIn3DViewport::DisplayParams &params,TimeValue t,INode *node, HWND hwnd, IGeometryChecker::ReturnVal type,Tab<int> &indices)
{
	
	ObjectState os = node->EvalWorldState(t);
	IGeometryChecker::OutputVal val;
	ViewExp *vpt = GetCOREInterface()->GetViewport(hwnd);
	if(vpt)
	{
		val.mIndex = indices;
		if(os.obj&&os.obj->IsSubClassOf(triObjectClassID))
		{
			TriObject *tri = dynamic_cast<TriObject *>(os.obj);
			DisplayTriObjectResults(params,t,node,vpt,tri,type,val);
		}
		else if(os.obj&&os.obj->IsSubClassOf(polyObjectClassID))
		{
			PolyObject *poly = dynamic_cast<PolyObject *>(os.obj);
			DisplayPolyObjectResults(params,t,node,vpt,poly,type,val);
		}
		GetCOREInterface()->ReleaseViewport(vpt);
	}

}
void DisplayIn3DViewport::DisplayPolyObjectResults(DisplayIn3DViewport::DisplayParams &params,TimeValue t, INode* inode, ViewExp *vpt,PolyObject *poly,IGeometryChecker::ReturnVal type,IGeometryChecker::OutputVal &results)
{
	
	if(poly&&results.mIndex.Count()>0&&type!=IGeometryChecker::eFail)
	{
		GraphicsWindow *gw = vpt->getGW();
		Matrix3 tm = inode->GetObjectTM(t);
		DWORD savedLimits = gw->getRndLimits();
		//also need to use the ObjectState tm if it exists. defect 1159328  MZ
		ObjectState os = inode->EvalWorldState(t);
		if(os.GetTM())
		{
			tm = (*os.GetTM()) *tm;
		}
		gw->setTransform(tm);

		Color c;
		if(params.mC==NULL)
		{
			COLORREF cr = ColorMan()->GetColor(GEOMETRY_DISPLAY_COLOR);
			c = Color(cr);
		}
		else
			c = Color(*params.mC);

		gw->setColor (FILL_COLOR,c);
		gw->setColor (LINE_COLOR,c);
		Point3 pColor(c);

		IColorManager *pClrMan = GetColorManager();;
		Point3 oldSubColor = pClrMan->GetOldUIColor(COLOR_SUBSELECTION); //hack to fix bud with render3dface in mnmesh.. automatically takes it from this!
		pClrMan->SetOldUIColor(COLOR_SUBSELECTION,&pColor);
		int i;
		switch(type)
		{
			case IGeometryChecker::eVertices:
				{
					if(params.mSeeThrough)
						gw->setRndLimits((savedLimits) & ~( GW_COLOR_VERTS) & ~(GW_Z_BUFFER)  | (GW_CONSTANT|GW_WIREFRAME) );
					else
						gw->setRndLimits((savedLimits) & ~( GW_COLOR_VERTS)| (GW_Z_BUFFER)|(GW_CONSTANT|GW_WIREFRAME) );
					bool dontDrawSelected = poly->GetSubselState()==MNM_SL_VERTEX ? true :false;
					for(i=0;i<results.mIndex.Count();++i)
					{
						DisplayResultVert(t,gw,&poly->mm,results.mIndex[i],dontDrawSelected);
					}
					break;
				}
			case IGeometryChecker::eEdges:
				{
					if(params.mSeeThrough)
						gw->setRndLimits((savedLimits) & ~( GW_COLOR_VERTS) & ~(GW_Z_BUFFER)  | (GW_CONSTANT|GW_WIREFRAME) );
					else
						gw->setRndLimits((savedLimits) & ~( GW_COLOR_VERTS)| (GW_Z_BUFFER)|(GW_CONSTANT|GW_WIREFRAME) );

					bool dontDrawSelected = poly->GetSubselState()==MNM_SL_EDGE ? true :false;
					for(i=0;i<results.mIndex.Count();++i)
					{
						DisplayResultEdge(t,gw,&poly->mm,results.mIndex[i],dontDrawSelected);
					}
				}
				break;
			case IGeometryChecker::eFaces:
				{

					DWORD illum =0;
					if(params.mLighted)
					{
						if(savedLimits &GW_ILLUM)
							illum = GW_ILLUM;
						else if(savedLimits & GW_FLAT)
							illum = GW_FLAT;
						DWORD zbuffer =0;
						if( (savedLimits&GW_WIREFRAME)==FALSE)
						{ 
							if(params.mSeeThrough==false)
								zbuffer = GW_Z_BUFFER;
						}
						else
							illum = GW_ILLUM;
						gw->setRndLimits(GW_BACKCULL|GW_SHADE_SEL_FACES|zbuffer|illum|GW_TRANSPARENCY|GW_TEXTURE| GW_TRANSPARENT_PASS);

						Material mat;
						//Set Material properties
						mat.dblSided = 1;
						//diffuse, ambient and specular
						mat.Ka = c;
						mat.Kd = c;
						mat.Ks = c;
						// Set opacity
						mat.opacity = 0.5f;
						//Set shininess
						mat.shininess = 0.0f;
						mat.shinStrength = 0.0f;
						gw->setMaterial(mat);
						gw->setTransparency( GW_TRANSPARENT_PASS|GW_TRANSPARENCY);
						bool dontDrawSelected = poly->GetSubselState()==MNM_SL_FACE ? true :false;
						gw->startTriangles();
						for(i=0;i<results.mIndex.Count();++i)
						{
							DisplayResultFace(t,gw,&poly->mm,results.mIndex[i],savedLimits,params.mFlipFaces,dontDrawSelected,NULL);
						}
						gw->endTriangles();
						gw->setTransparency(0);
					}
					else
					{

						illum = GW_FLAT;
						DWORD zbuffer =0;
						if(params.mSeeThrough==false)
							zbuffer = GW_Z_BUFFER;
						gw->setRndLimits(GW_BACKCULL|GW_SHADE_SEL_FACES|zbuffer|illum);
						bool dontDrawSelected = poly->GetSubselState()==MNM_SL_FACE ? true :false;

						Point3 colorOverride[3];
						colorOverride[0] = pColor; colorOverride[1] = pColor; colorOverride[2] = pColor;
						gw->startTriangles();
						for(i=0;i<results.mIndex.Count();++i)
						{
							DisplayResultFace(t,gw,&poly->mm,results.mIndex[i],savedLimits,params.mFlipFaces,dontDrawSelected,colorOverride);
						}
						gw->endTriangles();
					}
					//okay now draw edges if we are doing edged faces
					if(GetCOREInterface7()->GetActiveViewportShowEdgeFaces()  && (params.mDrawEdges))
					{
						//use the selection color for the edges, since it will be selected...
					    Point3 selColor =  pClrMan->GetOldUIColor(COLOR_SELECTION); 
						gw->setColor (LINE_COLOR,selColor);
						if(params.mSeeThrough)
							gw->setRndLimits((savedLimits) & ~( GW_COLOR_VERTS) & ~(GW_Z_BUFFER)  | (GW_CONSTANT|GW_WIREFRAME) );
						else
							gw->setRndLimits((savedLimits) & ~( GW_COLOR_VERTS)| (GW_Z_BUFFER)|(GW_CONSTANT|GW_WIREFRAME) );

						bool dontDrawSelected = poly->GetSubselState()==MNM_SL_EDGE ? true :false;
						for(i=0;i<results.mIndex.Count();++i)
						{

							MNFace *face = &(poly->mm.f[results.mIndex[i]]);
							int deg = face->deg;
							for(int j=0;j<deg;++j)
							{
								DisplayResultEdge(t,gw,&poly->mm,face->edg[j],dontDrawSelected);
							}
						}
					}

				}
				break;
				
		};

	     pClrMan->SetOldUIColor(COLOR_SUBSELECTION,&oldSubColor);

		gw->setRndLimits(savedLimits);
	}
}

void DisplayIn3DViewport::DisplayTriObjectResults(DisplayIn3DViewport::DisplayParams &params,TimeValue t, INode* inode, ViewExp *vpt,TriObject *tri,IGeometryChecker::ReturnVal type,IGeometryChecker::OutputVal &results)
{

	if(tri&&results.mIndex.Count()>0&&type!=IGeometryChecker::eFail)
	{
		GraphicsWindow *gw = vpt->getGW();
		Matrix3 tm = inode->GetObjectTM(t);
		int savedLimits = gw->getRndLimits();
		//also need to use the ObjectState tm if it exists. defect 1159328  MZ
		ObjectState os = inode->EvalWorldState(t);
		if(os.GetTM())
		{
			tm =  (*os.GetTM())*tm;
		}
		gw->setTransform(tm);

		Color c;
		if(params.mC==NULL)
		{
			COLORREF cr = ColorMan()->GetColor(GEOMETRY_DISPLAY_COLOR);
			c = Color(cr);
		}
		else
			c = Color(*params.mC);
		Point3 pColor(c);
		gw->setColor (FILL_COLOR,c);
		gw->setColor (LINE_COLOR,c);

		IColorManager *pClrMan = GetColorManager();;
		Point3 oldSubColor = pClrMan->GetOldUIColor(COLOR_SUBSELECTION); //hack to fix bud with render3dface in mnmesh.. automatically takes it from this!
		pClrMan->SetOldUIColor(COLOR_SUBSELECTION,&pColor);
		int i;
		switch(type)
		{
			case IGeometryChecker::eVertices:
				{
					if(params.mSeeThrough)
						gw->setRndLimits((savedLimits) & ~( GW_COLOR_VERTS) & ~(GW_Z_BUFFER)  | (GW_CONSTANT|GW_WIREFRAME) );
					else
						gw->setRndLimits((savedLimits) & ~( GW_COLOR_VERTS)| (GW_Z_BUFFER)|(GW_CONSTANT|GW_WIREFRAME) );
					bool dontDrawSelected = tri->GetSubselState() == MESH_VERTEX;
					for(i=0;i<results.mIndex.Count();++i)
					{
						DisplayResultVert(t,gw,&tri->mesh,results.mIndex[i],dontDrawSelected);
					}
				}
				break;
			case IGeometryChecker::eEdges:
				{
					if(params.mSeeThrough)
						gw->setRndLimits((savedLimits) & ~( GW_COLOR_VERTS) & ~(GW_Z_BUFFER)  | (GW_CONSTANT|GW_WIREFRAME) );
					else
						gw->setRndLimits((savedLimits) & ~( GW_COLOR_VERTS)| (GW_Z_BUFFER)|(GW_CONSTANT|GW_WIREFRAME) );
					bool dontDrawSelected = tri->GetSubselState() == MESH_EDGE;
					for(i=0;i<results.mIndex.Count();++i)
					{
						DisplayResultEdge(t,gw,inode,&tri->mesh,results.mIndex[i],dontDrawSelected,false);
					}
					break;
				}
			case IGeometryChecker::eFaces:
				{
					if(params.mLighted)
					{

						DWORD illum =0;
						if(savedLimits &GW_ILLUM)
							illum = GW_ILLUM;
						else if(savedLimits & GW_FLAT)
							illum = GW_FLAT;
						DWORD zbuffer =0;
						if( (savedLimits&GW_WIREFRAME)==FALSE)
						{ 
							if(params.mSeeThrough==false)
								zbuffer = GW_Z_BUFFER;
						}
						else
							illum = GW_ILLUM;
						gw->setRndLimits(GW_BACKCULL|GW_SHADE_SEL_FACES|zbuffer|illum|GW_TRANSPARENCY|GW_TEXTURE| GW_TRANSPARENT_PASS);

						Material mat;
						//Set Material properties
						mat.dblSided = 1;
						//diffuse, ambient and specular
						mat.Ka = c;
						mat.Kd = c;
						mat.Ks = c;
						// Set opacity
						mat.opacity = 0.5f;
						//Set shininess
						mat.shininess = 0.0f;
						mat.shinStrength = 0.0f;
						gw->setMaterial(mat);
						gw->setTransparency( GW_TRANSPARENT_PASS|GW_TRANSPARENCY);
						tri->mesh.checkNormals(TRUE);
						gw->startTriangles();
						bool dontDrawSelected = tri->GetSubselState() == MESH_FACE;
						for(i=0;i<results.mIndex.Count();++i)
						{
							DisplayResultFace(t,gw,&tri->mesh,results.mIndex[i],savedLimits,params.mFlipFaces,dontDrawSelected,NULL);
						}
						gw->endTriangles();
						gw->setTransparency(0);
					}
					else
					{
						DWORD illum = GW_FLAT;
						DWORD zbuffer =0;
						if(params.mSeeThrough==false)
							zbuffer = GW_Z_BUFFER;
						gw->setRndLimits(GW_BACKCULL|GW_SHADE_SEL_FACES|zbuffer|illum);
						bool dontDrawSelected = tri->GetSubselState() == MESH_FACE;
						tri->mesh.checkNormals(TRUE); //still need to do this here Defect 1117199 
						Point3 colorOverride[3];
						colorOverride[0] = pColor; colorOverride[1] = pColor; colorOverride[2] = pColor;
						gw->startTriangles();
						for(i=0;i<results.mIndex.Count();++i)
						{
							DisplayResultFace(t,gw,&tri->mesh,results.mIndex[i],savedLimits,params.mFlipFaces,dontDrawSelected,colorOverride);
						}
						gw->endTriangles();

					}
					//okay now draw edges if we are doing edged faces
					if(GetCOREInterface7()->GetActiveViewportShowEdgeFaces() && (params.mDrawEdges))
					{
						//use the selection color for the edges, since it will be selected...
					    Point3 selColor =  pClrMan->GetOldUIColor(COLOR_SELECTION); 
						gw->setColor (LINE_COLOR,selColor);
						if(params.mSeeThrough)
							gw->setRndLimits((savedLimits) & ~( GW_COLOR_VERTS) & ~(GW_Z_BUFFER)  | (GW_CONSTANT|GW_WIREFRAME) );
						else
							gw->setRndLimits((savedLimits) & ~( GW_COLOR_VERTS)| (GW_Z_BUFFER)|(GW_CONSTANT|GW_WIREFRAME) );

						bool dontDrawSelected = tri->GetSubselState() == MESH_EDGE;
						for(i=0;i<results.mIndex.Count();++i)
						{
							//thee results are triangles, we need to draw the edge for them..
							//so edge index = faceIndex * 3 + (0, 1 or 2).
							for(int j=0;j<3;++j)
							{
								DisplayResultEdge(t,gw,inode,&tri->mesh,results.mIndex[i]*3+j,dontDrawSelected,true);
								
							}
						}
					}
				}
				break;
		};

	     pClrMan->SetOldUIColor(COLOR_SUBSELECTION,&oldSubColor);

		gw->setRndLimits(savedLimits);
	}   
}
void DisplayIn3DViewport::DisplayResultFace (TimeValue t,GraphicsWindow *gw, MNMesh *mesh, int ff,DWORD savedLimits,bool flipFaces,bool dontdrawSelected,Point3 *color)
{
	
	if (!mesh||ff<0||ff>=mesh->numf||mesh->f[ff].GetFlag(MN_HIDDEN)||mesh->f[ff].GetFlag(MN_DEAD)||(dontdrawSelected&&mesh->f[ff].GetFlag(MN_SEL))) return;

	Point3 xyz[4];
	Point3 nor[4];
	Point3 uvw[4];
	uvw[0] = Point3(0.0f,0.0f,0.0f);
	uvw[1] = Point3(1.0f,0.0f,0.0f);
	uvw[2] = Point3(0.0f,1.0f,0.0f);

	Tab<int> tri;				
	tri.ZeroCount();
	MNFace *face = &mesh->f[ff];
	face->GetTriangles(tri);
	int		*vv		= face->vtx;
	int		deg		= face->deg;
	DWORD	smGroup = face->smGroup;
	for (int tt=0; tt<tri.Count(); tt+=3)
	{
		int *triv = tri.Addr(tt);
		if(flipFaces==true&&color)
		{
			xyz[0] = mesh->v[vv[triv[2]]].p;
			xyz[1] = mesh->v[vv[triv[1]]].p;
			xyz[2] = mesh->v[vv[triv[0]]].p;
			gw->triangle(xyz,NULL);
		}
		else
		{
			if(flipFaces==false)
				for (int i=0; i<3; i++) xyz[i] = mesh->v[vv[triv[i]]].p;
			else
			{
				xyz[0] = mesh->v[vv[triv[2]]].p;
				xyz[1] = mesh->v[vv[triv[1]]].p;
				xyz[2] = mesh->v[vv[triv[0]]].p;
			}
			float flipVal = flipFaces==false ? 1.0f : -1.0f;
			Point3 tnorm(mesh->GetFaceNormal(ff,TRUE));
			nor[0] = nor[1] = nor[2] = flipVal*tnorm;

			if(color==NULL)
				gw->triangleN(xyz,nor,uvw);
			else
				gw->triangleNC(xyz,nor,color);
		}
	}
}

void DisplayIn3DViewport::DisplayResultEdge (TimeValue t, GraphicsWindow *gw, MNMesh *mesh, int ee,bool dontdrawSelected)
{
	if (!mesh||ee<0 ||ee>=mesh->nume||mesh->e[ee].GetFlag(MN_HIDDEN)||mesh->e[ee].GetFlag(MN_DEAD)||
		((mesh->e[ee].f1==-1||mesh->f[mesh->e[ee].f1].GetFlag(MN_HIDDEN))&&(mesh->e[ee].f2==-1||mesh->f[mesh->e[ee].f2].GetFlag(MN_HIDDEN)))||
		(dontdrawSelected&&mesh->e[ee].GetFlag(MN_SEL))) return;
	Point3 xyz[3];
	xyz[0] = mesh->v[mesh->e[ee].v1].p;
	xyz[1] = mesh->v[mesh->e[ee].v2].p;
	gw->polyline(2, xyz, NULL, NULL, 0, NULL);
}

void DisplayIn3DViewport::DisplayResultVert (TimeValue t, GraphicsWindow *gw,MNMesh *mesh, int vv,bool dontdrawSelected)
{
	if (!mesh||vv<0||vv>=mesh->numv||mesh->v[vv].GetFlag(MN_HIDDEN)||mesh->v[vv].GetFlag(MN_DEAD)||(dontdrawSelected&&mesh->v[vv].GetFlag(MN_SEL))) return;
	Point3 & p = mesh->v[vv].p;
	if (getUseVertexDots()) gw->marker (&p, VERTEX_DOT_MARKER(getVertexDotType()));
	else gw->marker (&p, PLUS_SIGN_MRKR);
}

void DisplayIn3DViewport::DisplayResultFace (TimeValue t, GraphicsWindow *gw, Mesh *mesh, int ff,DWORD savedLimits,bool flipFaces,bool dontdrawSelected,Point3 *color)
{
	if (!mesh||ff<0||ff>=mesh->numFaces||mesh->faces[ff].Hidden()||(dontdrawSelected&&mesh->faceSel[ff])) return;

	Point3 xyz[3];
	Point3 nor[3];
	Point3 uvw[3];
	uvw[0] = Point3(0.0f,0.0f,0.0f);
	uvw[1] = Point3(1.0f,0.0f,0.0f);
	uvw[2] = Point3(0.0f,1.0f,0.0f);
	float flipVal = flipFaces==false ? 1.0f : -1.0f;
	Point3 tnorm(mesh->getFaceNormal(ff));
	nor[0] = nor[1] = nor[2] = flipVal*tnorm;
	if(flipFaces==FALSE)
	{
		xyz[0] = mesh->verts[mesh->faces[ff].v[0]];
		xyz[1] = mesh->verts[mesh->faces[ff].v[1]];
		xyz[2] = mesh->verts[mesh->faces[ff].v[2]];
	}
	else
	{
		xyz[0] = mesh->verts[mesh->faces[ff].v[2]];
		xyz[1] = mesh->verts[mesh->faces[ff].v[1]];
		xyz[2] = mesh->verts[mesh->faces[ff].v[0]];
	}

	if(flipFaces==true&&color)
	{
		gw->triangle(xyz,NULL);
	}
	else if(color==NULL)
		gw->triangleN(xyz,nor,uvw);
	else
		gw->triangleNC(xyz,nor,color);



}
void DisplayIn3DViewport::DeleteCachedResults()
{
	NodeAndEdges::iterator iter;
	iter = mNodeAndEdges.begin();
	while(iter!= mNodeAndEdges.end())
	{
		if(iter->second)
			delete iter->second;
		++iter;
	}
	mNodeAndEdges.clear();
}

AdjEdgeList * DisplayIn3DViewport::FindAdjEdgeList(INode *item,Mesh &mesh) 
{
	NodeAndEdges::iterator iter;
	iter= mNodeAndEdges.find(item);
	if(iter != mNodeAndEdges.end())
	{
		return iter->second;
	}
	if(item)
	{
		AdjEdgeList *adjList = new AdjEdgeList(mesh);
		mNodeAndEdges.insert(NodeAndEdges::value_type(item,adjList));
		return adjList;
	}
	return NULL;
}


void DisplayIn3DViewport::DisplayResultEdge (TimeValue t, GraphicsWindow *gw, INode *node,Mesh *mesh, int ee,bool dontdrawSelected,bool checkVisible)
{
	if(mesh)
	{
		int triNum,localEdge;
		int numTris = mesh->numFaces;
		if (ee<0 ||ee>= GetNumOfEdges(numTris)||(dontdrawSelected&&mesh->edgeSel[ee])) return;
		//now check edge for hidden faces next to it..
		CheckerMeshEdge::GetTriLocalEdge(ee,triNum,localEdge);
		AdjEdgeList *adjList = FindAdjEdgeList(node,*mesh);
		int vert1 = mesh->faces[triNum].v[localEdge];
		int vert2 = mesh->faces[triNum].v[(localEdge+1)%3];
		if(vert1!=vert2) //don't display degenerate edges Defect 1110906 
		{
			int adjEdge = adjList->FindEdge(vert1,vert2);
			if(adjEdge>-1)
			{
				if(adjList->edges[adjEdge].Hidden(mesh->faces)==FALSE  //it's hidden or if we are checking for not visible and it's not visible
					&&(checkVisible==false||adjList->edges[adjEdge].Visible(mesh->faces)==TRUE))
				{
					
					Point3 xyz[3];
					xyz[0] = mesh->verts[vert1];
					xyz[1] = mesh->verts[vert2];
					gw->polyline(2, xyz, NULL, NULL, 0, NULL);
				}
			}
		}
	}
}

void DisplayIn3DViewport::DisplayResultVert (TimeValue t, GraphicsWindow *gw,Mesh *mesh, int vv,bool dontdrawSelected)
{
	if (!mesh||vv<0||vv>=mesh->numVerts|| mesh->vertHide[vv]||(dontdrawSelected&&mesh->vertSel[vv])) return;
	Point3 & p = mesh->verts[vv];
	if (getUseVertexDots()) gw->marker (&p, VERTEX_DOT_MARKER(getVertexDotType()));
	else gw->marker (&p, PLUS_SIGN_MRKR);
}




//this is where we store the funcs so we can clean them out for garbage collection
Array* MXSGeometryChecker::mxsFuncs = NULL;


//This function lets you register a set of 
int IGeometryCheckerManager_Imp::RegisterChecker_FPS(Value *checkerFunc, Value *isSupportedFunc,int type, MSTR name,  Value *popupFunc, Value *textOverrideFunc,
		Value *displayOverrideFunc)
{
	//make stuff NULL
	checkerFunc = (checkerFunc == &undefined) ? NULL : checkerFunc;
	isSupportedFunc = (isSupportedFunc == &undefined) ? NULL : isSupportedFunc;
	popupFunc = (popupFunc == &undefined) ? NULL : popupFunc;
	textOverrideFunc = (textOverrideFunc == &undefined) ? NULL : textOverrideFunc;
	displayOverrideFunc = (displayOverrideFunc == &undefined) ? NULL : displayOverrideFunc;
	
	//check incoming variables
	if(name.Length()<=0)
		throw RuntimeError(GetString(IDS_REGISTER_NEED_NAME));
	IGeometryChecker::ReturnVal val;
	switch(type)
	{
	case 1:
		val = IGeometryChecker::eVertices;
		break;
	case 2:
		val = IGeometryChecker::eEdges;
		break;
	case 3:
		val = IGeometryChecker::eFaces;
		break;
	default:
		throw RuntimeError(GetString(IDS_REGISTER_WRONG_TYPE));
	};
	if (checkerFunc)
	{
		if (!is_function(checkerFunc))
			throw RuntimeError (GetString(IDS_REGISTER_BAD_FUNC), checkerFunc);
		MAXScriptFunction* fn = (MAXScriptFunction*)checkerFunc; 
		if (is_structMethod(checkerFunc))
			fn = (MAXScriptFunction*)((StructMethod*)checkerFunc)->fn;
		else if (is_pluginMethod(checkerFunc))
			fn = (MAXScriptFunction*)((PluginMethod*)checkerFunc)->fn;
		if (fn->parameter_count != 3)
			throw RuntimeError (GetString(IDS_REGISTER_BAD_NUM_ARGS_CHECKER), Integer::intern(fn->parameter_count));
	}
	else
		throw RuntimeError(GetString(IDS_REGISTER_NEED_CHECKER));
	if (isSupportedFunc)
	{
		if (!is_function(isSupportedFunc))
			throw RuntimeError (GetString(IDS_REGISTER_BAD_FUNC), isSupportedFunc);
		MAXScriptFunction* fn = (MAXScriptFunction*)isSupportedFunc; 
		if (is_structMethod(isSupportedFunc))
			fn = (MAXScriptFunction*)((StructMethod*)isSupportedFunc)->fn;
		else if (is_pluginMethod(isSupportedFunc))
			fn = (MAXScriptFunction*)((PluginMethod*)isSupportedFunc)->fn;
		if (fn->parameter_count != 1)
			throw RuntimeError (GetString(IDS_REGISTER_BAD_NUM_ARGS_SUPPORT), Integer::intern(fn->parameter_count));
	}
	else
		throw RuntimeError(GetString(IDS_REGISTER_NEED_SUPPORT));

	if (popupFunc)
	{
		if (!is_function(popupFunc))
			throw RuntimeError (GetString(IDS_REGISTER_BAD_FUNC), popupFunc);
		MAXScriptFunction* fn = (MAXScriptFunction*)popupFunc; 
		if (is_structMethod(popupFunc))
			fn = (MAXScriptFunction*)((StructMethod*)popupFunc)->fn;
		else if (is_pluginMethod(popupFunc))
			fn = (MAXScriptFunction*)((PluginMethod*)popupFunc)->fn;
		if (fn->parameter_count != 0)
			throw RuntimeError (GetString(IDS_REGISTER_BAD_NUM_ARGS_POPUP), Integer::intern(fn->parameter_count));
	}
	if (textOverrideFunc)
	{
		if (!is_function(textOverrideFunc))
			throw RuntimeError (GetString(IDS_REGISTER_BAD_FUNC), textOverrideFunc);
		MAXScriptFunction* fn = (MAXScriptFunction*)textOverrideFunc; 
		if (is_structMethod(textOverrideFunc))
			fn = (MAXScriptFunction*)((StructMethod*)textOverrideFunc)->fn;
		else if (is_pluginMethod(textOverrideFunc))
			fn = (MAXScriptFunction*)((PluginMethod*)textOverrideFunc)->fn;
		if (fn->parameter_count != 0)
			throw RuntimeError (GetString(IDS_REGISTER_BAD_NUM_ARGS_TEXT), Integer::intern(fn->parameter_count));
	}
	if (displayOverrideFunc)
	{
		if (!is_function(displayOverrideFunc))
			throw RuntimeError (GetString(IDS_REGISTER_BAD_FUNC), displayOverrideFunc);
		MAXScriptFunction* fn = (MAXScriptFunction*)displayOverrideFunc; 
		if (is_structMethod(displayOverrideFunc))
			fn = (MAXScriptFunction*)((StructMethod*)displayOverrideFunc)->fn;
		else if (is_pluginMethod(displayOverrideFunc))
			fn = (MAXScriptFunction*)((PluginMethod*)displayOverrideFunc)->fn;
		if (fn->parameter_count != 4)
			throw RuntimeError (GetString(IDS_REGISTER_BAD_NUM_ARGS_DISPLAY), Integer::intern(fn->parameter_count));
	}
	
	//create gc array
	if(MXSGeometryChecker::mxsFuncs == NULL)
		MXSGeometryChecker::mxsFuncs = new (GC_PERMANENT) Array (0);

	//unregister it if the name already exists.
	UnRegisterChecker_FPS(name);

	//create the checker and register it
	MXSGeometryChecker *mxsChecker = new MXSGeometryChecker(checkerFunc, isSupportedFunc,val, name,  popupFunc, textOverrideFunc,
		displayOverrideFunc);
	int index = RegisterGeometryChecker(mxsChecker);

	//add the inputted funcs to the array for later garbage collection
	if (checkerFunc)
		MXSGeometryChecker::mxsFuncs->append(checkerFunc);
	if (isSupportedFunc)
		MXSGeometryChecker::mxsFuncs->append(isSupportedFunc);
	if (popupFunc)
		MXSGeometryChecker::mxsFuncs->append(popupFunc);
	if (textOverrideFunc)
		MXSGeometryChecker::mxsFuncs->append(textOverrideFunc);
	if (displayOverrideFunc)
		MXSGeometryChecker::mxsFuncs->append(displayOverrideFunc);


	return index;
}


bool IGeometryCheckerManager_Imp::UnRegisterChecker_FPS(MSTR name)
{

	for(int i = GetNumRegisteredGeometryCheckers() -1;i>-1;--i)
	{
		IGeometryChecker *checker = mGeometryCheckers[i];
		if(checker&&checker->GetName()==name)
		{
			UnRegisterGeometryChecker(checker);
			return true;
		}
	}
	return false;
}

DWORD IGeometryCheckerManager_Imp::GetNextValidGeometryCheckerID()
{
	return ++mNextValidID;
}


DWORD IGeometryCheckerManager_Imp::GetActivatedGeometryCheckerID()
{
	return GetNthGeometryCheckerID(GetActivatedGeometryCheckerIndex());
}

void IGeometryCheckerManager_Imp::SetActiveGeometryCheckerID(DWORD id)
{
	for(int i = GetNumRegisteredGeometryCheckers() -1;i>-1;--i)
	{
		IGeometryChecker *checker = mGeometryCheckers[i];
		if(checker&&checker->GetCheckerID()==id)
		{
			ActivateGeometryChecker(i);
			break;
		}
	}
}

// The following strings do not need to be localized
const TCHAR* IGeometryCheckerManager_Imp::kINI_FILE_NAME = _T("GeometryChecker.ini");
const TCHAR* IGeometryCheckerManager_Imp::kINI_SECTION_NAME = _T("GeometryCheckerSettings");
const TCHAR* IGeometryCheckerManager_Imp::kINI_SEE_THROUGH  = _T("SeeThrough");
const TCHAR* IGeometryCheckerManager_Imp::kINI_AUTO_UPDATE = _T("AutoUpdate");
const TCHAR* IGeometryCheckerManager_Imp::kINI_DISPLAY_TEXT_UP_TOP = _T("DisplayTextUpTop");
const TCHAR* IGeometryCheckerManager_Imp::kINI_SHOW_WARNING_DIALOG = _T("ShowWarningDialogThenStop");

MSTR IGeometryCheckerManager_Imp::GetINIFileName()
{
	IPathConfigMgr* pPathMgr = IPathConfigMgr::GetPathConfigMgr();
	MSTR iniFileName(pPathMgr->GetDir(APP_PLUGCFG_DIR));
	iniFileName += _T("\\");
	iniFileName += kINI_FILE_NAME;
	return iniFileName;
}

void IGeometryCheckerManager_Imp::SavePreferences ()
{
	MSTR iniFileName = GetINIFileName();
	TCHAR buf[MAX_PATH] = {0};
	// The ini file should always be saved and read with the same locale settings
	MaxLocaleHandler localeGuard;
	int val = GetSeeThrough()== true ? 1 : 0;
	_stprintf_s(buf, sizeof(buf), "%d", val);
	BOOL res = WritePrivateProfileString(kINI_SECTION_NAME, kINI_SEE_THROUGH, buf, iniFileName);
	DbgAssert(res);
	val = GetAutoUpdate()== true ? 1 : 0;
	_stprintf_s(buf, sizeof(buf), "%d",val);
	res = WritePrivateProfileString(kINI_SECTION_NAME,kINI_AUTO_UPDATE, buf, iniFileName);
	DbgAssert(res);
	val = GetDisplayTextUpTop()== true ? 1 : 0;
	_stprintf_s(buf, sizeof(buf), "%d", val);
	res = WritePrivateProfileString(kINI_SECTION_NAME, kINI_DISPLAY_TEXT_UP_TOP, buf, iniFileName);
	DbgAssert(res);
}

void IGeometryCheckerManager_Imp::LoadPreferences ()
{
	void SetSeeThrough(bool val);
	void SetAutoUpdate(bool val) ;
	void SetDisplayTextUpTop(bool val);

	MSTR iniFileName = GetINIFileName();
	TCHAR buf[MAX_PATH] = {0};
	TCHAR defaultVal[MAX_PATH] = {0};
	IPathConfigMgr* pathMgr = IPathConfigMgr::GetPathConfigMgr();
	if (pathMgr->DoesFileExist(iniFileName)) 
	{

		// The ini file should always be saved and read with the same locale settings
		MaxLocaleHandler localeGuard;
		_stprintf_s(defaultVal, sizeof(defaultVal), "%d", ((GetSeeThrough()== true) ? 1 : 0));
		if (GetPrivateProfileString(kINI_SECTION_NAME, kINI_SEE_THROUGH, defaultVal, buf, sizeof(buf), iniFileName))
		{
			int val = atoi(buf);
			mSeeThrough = ( (val==1) ? true: false);
		}
		_stprintf_s(defaultVal, sizeof(defaultVal), "%d", ((GetAutoUpdate()== true) ? 1 : 0));
		if (GetPrivateProfileString(kINI_SECTION_NAME,kINI_AUTO_UPDATE, defaultVal, buf, sizeof(buf), iniFileName))
		{
			int val = atoi(buf);
			mAutoUpdate = ( (val==1) ? true: false);
		}
		_stprintf_s(defaultVal, sizeof(defaultVal), "%d", ((GetDisplayTextUpTop()== true) ? 1 : 0));
		if (GetPrivateProfileString(kINI_SECTION_NAME, kINI_DISPLAY_TEXT_UP_TOP, defaultVal, buf, sizeof(buf), iniFileName))
		{
			int val = atoi(buf);
			mDisplayTextUpTop = ( (val==1) ? true: false);
		}
		
	}
}





