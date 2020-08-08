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
// DESCRIPTION: Geometry Checker Button Prototype
// AUTHOR: Michael Zyracki created 2008
//***************************************************************************/

#include "GeometryButton.h"
#include "IGeometryChecker.h"
#include "checkerimplementations.h"

GeometryButton::GeometryButton()
{
}

GeometryButton::~GeometryButton()
{
}

bool GeometryButton::OnAction(HWND hwnd,GraphicsWindow *gw,IPoint2 hitLoc,IViewportButton::Action action)
{
	bool val = false;
	if(GetEnabled()==true)
	{
		IViewportButtonManager *vpm = static_cast<IViewportButtonManager*>(GetCOREInterface(IVIEWPORTBUTTONMANAGER_INTERFACE ));
		if(vpm)
			val =  vpm->HitTest(hwnd,gw,mLocation,hitLoc,mLabel);
	}
	if(val==true&&(action == IViewportButton::eRightClick||action == IViewportButton::eLeftClick))
	{
		IGeometryCheckerManager *man = GetIGeometryCheckerManager();
		if(man)
			man->PopupMenuSelect();
	}
	//now make sure we set the color correclty, but only set it when it changes
	Color textColor;
	if(val==true) //okay we hit it so change the color
	{
		 textColor = GetUIColor(COLOR_VP_LABEL_HIGHLIGHT);
	}
	else
	{
		COLORREF cr = ColorMan()->GetColor(GEOMETRY_TEXT_COLOR);
		textColor = Color(cr);
	}
	if(textColor!=mColor)
		SetColor(textColor);
	return val;
}

void GeometryButton::ResetLocation(ViewExp *vpt)
{
	if(GetEnabled()==true&&mLabel.Length()>0)
	{
		SIZE sp;
		GraphicsWindow *gw = vpt->getGW();
		gw->getTextExtents(mLabel.data(),&sp);
		RECT rect;
		HWND hWnd = vpt->GetHWnd();
		GetWindowRect (hWnd, &rect);
		int width = rect.right - rect.left;
		mLocation.x = width/2 - sp.cx/2;

		//need to reset the y here also...
		IGeometryCheckerManager *man = GetIGeometryCheckerManager();
		if(man->GetDisplayTextUpTop()==false)
		{
			int mult = 1; // at least for top
			if(man->DoesGeometryCheckerHavePropDlg(man->GetActivatedGeometryCheckerIndex()))
				++mult;
			if(man->GetAutoUpdate()==false)
				++mult;
			mLocation.y = (rect.bottom-rect.top) - mult*(4+OFFSET) + OFFSET;
		}
	}
}


void ConfigureButton::ResetLocation(ViewExp *vpt)
{
	if(GetEnabled()==true&&mLabel.Length()>0)
	{
		SIZE sp;
		GraphicsWindow *gw = vpt->getGW();
		gw->getTextExtents(mLabel.data(),&sp);
		RECT rect;
		HWND hWnd = vpt->GetHWnd();
		GetWindowRect (hWnd, &rect);
		int width = rect.right - rect.left;
		mLocation.x = width/2 - sp.cx/2;

		//need to reset the y here also...
		IGeometryCheckerManager *man = GetIGeometryCheckerManager();
		if(man->GetDisplayTextUpTop()==false)
		{
			int mult = 1; // at least for top
			int locationOffset = 0;
			if(man->DoesGeometryCheckerHavePropDlg(man->GetActivatedGeometryCheckerIndex()))
			{
				++mult;
			}
			if(man->GetAutoUpdate()==false)
				++mult;
			mLocation.y = (rect.bottom-rect.top) - mult*(4+OFFSET) + OFFSET +  OFFSET;
		}
	}
}


void AutoUpdateButton ::ResetLocation(ViewExp *vpt)
{
	if(GetEnabled()==true&&mLabel.Length()>0)
	{
		SIZE sp;
		GraphicsWindow *gw = vpt->getGW();
		gw->getTextExtents(mLabel.data(),&sp);
		RECT rect;
		HWND hWnd = vpt->GetHWnd();
		GetWindowRect (hWnd, &rect);
		int width = rect.right - rect.left;
		mLocation.x = width/2 - sp.cx/2;

		//need to reset the y here also...
		IGeometryCheckerManager *man = GetIGeometryCheckerManager();
		if(man->GetDisplayTextUpTop()==false)
		{
			int mult = 1; // at least for top
			int locationOffset = 0;
			if(man->DoesGeometryCheckerHavePropDlg(man->GetActivatedGeometryCheckerIndex()))
			{
				locationOffset = OFFSET;
				++mult;
			}
			if(man->GetAutoUpdate()==false)
				++mult;
			mLocation.y = (rect.bottom-rect.top) - mult*(4+OFFSET) + OFFSET + locationOffset + OFFSET;
		}
	}
}

void GeometryButton::Display(TimeValue t, ViewExp *vpt, int flags)
{
	//set the location before it's displayed.
	ResetLocation(vpt);
	if(vpt->IsActive()==TRUE &&vpt->GetHWnd()&&GetEnabled()==true)
	{
		MSTR st = GetLabel();
		vpt->getGW()->setRndLimits(vpt->getGW()->getRndMode() & ~GW_Z_BUFFER);
		vpt->getGW()->setColor(TEXT_COLOR, mColor);
		vpt->getGW()->wText(&IPoint3(mLocation.x,mLocation.y, 0), st.data()); //draw it undrneath the viewport text!
	}
}

void GeometryButton::GetViewportRect( TimeValue t, ViewExp *vpt, Rect *rect )
{
	//set the location
	ResetLocation(vpt);
	ViewportTextButton::GetViewportRect(t,vpt,rect);
}

bool ConfigureButton::OnAction(HWND hwnd,GraphicsWindow *gw,IPoint2 hitLoc,IViewportButton::Action action)
{
	bool val = false;
	if(GetEnabled()==true)
	{
		IViewportButtonManager *vpm = static_cast<IViewportButtonManager*>(GetCOREInterface(IVIEWPORTBUTTONMANAGER_INTERFACE ));
		if(vpm)
			val =  vpm->HitTest(hwnd,gw,mLocation,hitLoc,mLabel);
	}
	if(val==true&&(action == IViewportButton::eRightClick||action == IViewportButton::eLeftClick))
	{
		IGeometryCheckerManager *man = GetIGeometryCheckerManager();
		if(man)
			man->ShowGeometryCheckerPropDlg(man->GetActivatedGeometryCheckerIndex());
	}
	//now make sure we set the color correclty, but only set it when it changes
	Color textColor;
	if(val==true) //okay we hit it so change the color
	{
		 textColor = GetUIColor(COLOR_VP_LABEL_HIGHLIGHT);
	}
	else
	{
		COLORREF cr = ColorMan()->GetColor(GEOMETRY_TEXT_COLOR);
		textColor = Color(cr);
	}
	if(textColor!=mColor)
		SetColor(textColor);
	return val;
}


bool AutoUpdateButton::OnAction(HWND hwnd,GraphicsWindow *gw,IPoint2 hitLoc,IViewportButton::Action action)
{
	bool val = false;
	if(GetEnabled()==true)
	{
		IViewportButtonManager *vpm = static_cast<IViewportButtonManager*>(GetCOREInterface(IVIEWPORTBUTTONMANAGER_INTERFACE ));
		if(vpm)
			val =  vpm->HitTest(hwnd,gw,mLocation,hitLoc,mLabel);
	}
	if(val==true&&(action == IViewportButton::eRightClick||action == IViewportButton::eLeftClick))
	{
		IGeometryCheckerManager *man = GetIGeometryCheckerManager();
		if(man)
			man->RunGeometryCheck(GetCOREInterface()->GetTime());
	}
	//now make sure we set the color correclty, but only set it when it changes
	Color textColor;
	if(val==true) //okay we hit it so change the color
	{
		 textColor = GetUIColor(COLOR_VP_LABEL_HIGHLIGHT);
	}
	else
	{
		COLORREF cr = ColorMan()->GetColor(GEOMETRY_TEXT_COLOR);
		textColor = Color(cr);
	}
	if(textColor!=mColor)
		SetColor(textColor);
	return val;
}