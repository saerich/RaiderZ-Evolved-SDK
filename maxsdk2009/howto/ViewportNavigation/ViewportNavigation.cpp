/**********************************************************************
 *<
	FILE: ViewportNavigation.cpp

	DESCRIPTION:	Appwizard generated plugin

	CREATED BY: 

	HISTORY: 

 *>	Copyright (c) 2003, All Rights Reserved.
 **********************************************************************/

#include "ViewportNavigation.h"

// Singleton
class ViewportNavigator : public UtilityObj 
{
	public:
		HWND			hPanel;
		IUtil			*iu;
		Interface		*ip;
		
		void BeginEditParams(Interface *ip,IUtil *iu);
		void EndEditParams(Interface *ip,IUtil *iu);

		void Init(HWND hWnd);
		void Destroy(HWND hWnd);
		
		void DeleteThis() { }		
		//Constructor/Destructor

		ViewportNavigator();
		~ViewportNavigator();	

	private:
		static const Class_ID ViewportNavigator_CLASS_ID;	
		static ViewportNavigator mInstance;
		friend class ViewportNavigatorClassDesc;

		// Singleton
		ViewportNavigator();
		ViewportNavigator(const ViewportNavigator& other);
};

const Class_ID ViewportNavigator::ViewportNavigator_CLASS_ID = Class_ID(0x831b0af4, 0x8c02bcc1);

class ViewportNavigatorClassDesc : public ClassDesc2 
{
	public:
	int 			IsPublic() { return TRUE; }
	void *			Create(BOOL loading = FALSE) { return &ViewportNavigator::mInstance; }
	const TCHAR *	ClassName() { return GetString(IDS_CLASS_NAME); }
	SClass_ID		SuperClassID() { return UTILITY_CLASS_ID; }
	Class_ID		ClassID() { return ViewportNavigator::ViewportNavigator_CLASS_ID; }
	const TCHAR* 	Category() { return GetString(IDS_CATEGORY); }

	const TCHAR*	InternalName() { return _T("ViewportNavigator"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }					// returns owning module handle
};

static ViewportNavigatorClassDesc ViewportNavigatorDesc;
ClassDesc2* GetViewportNavigatorDesc() { return &ViewportNavigatorDesc; }

static INT_PTR CALLBACK ViewportNavigatorDlgProc(
	HWND hWnd, 
	UINT msg, 
	WPARAM wParam, 
	LPARAM lParam)
{
	switch (msg) {
		case WM_INITDIALOG:
			theViewportNavigator.Init(hWnd);
			break;

		case WM_DESTROY:
			theViewportNavigator.Destroy(hWnd);
			break;

		case WM_COMMAND:
			break;


		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:
			theViewportNavigator.ip->RollupMouseMessage(hWnd,msg,wParam,lParam); 
			break;

		default:
			return 0;
	}
	return 1;
}



//--- ViewportNavigator -------------------------------------------------------
ViewportNavigator::ViewportNavigator()
{
	iu = NULL;
	ip = NULL;	
	hPanel = NULL;
}

ViewportNavigator::~ViewportNavigator()
{

}

void ViewportNavigator::BeginEditParams(Interface *ip,IUtil *iu) 
{
	this->iu = iu;
	this->ip = ip;
	hPanel = ip->AddRollupPage(
		hInstance,
		MAKEINTRESOURCE(IDD_PANEL),
		ViewportNavigatorDlgProc,
		GetString(IDS_PARAMS),
		0);
}
	
void ViewportNavigator::EndEditParams(Interface *ip,IUtil *iu) 
{
	this->iu = NULL;
	this->ip = NULL;
	ip->DeleteRollupPage(hPanel);
	hPanel = NULL;
}

void ViewportNavigator::Init(HWND hWnd)
{

}

void ViewportNavigator::Destroy(HWND hWnd)
{

}

