/**********************************************************************
 *
	FILE: Link_timectrl.cpp

	DESCRIPTION: Implementation for LinkTimeControl

	            

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

#include "ctrl.h"
#include "interpik.h"
#include "istdplug.h"
#include "iparamm2.h"
#include "notify.h"
#include "macrorec.h"
#include "3dsmaxport.h"
#include <ILockedTracks.h>
#include "link_timectrl.h"
#include "link_cnstrnt.h"

static LinkTimeControlClassDesc linkControlCD;
ClassDesc* GetLinkTimeCtrlDesc() {return &linkControlCD;}

LinkTimeControl::LinkTimeControl()
: fOwner( NULL )
, fCurveSelected( FALSE )
{
}

LinkTimeControl::~LinkTimeControl()
{
}

RefTargetHandle		LinkTimeControl::Clone( RemapDir& remap )
{
	LinkTimeControl* p = new LinkTimeControl();
	BaseClone( this, p, remap );
	return p;
}

void	LinkTimeControl::GetValueLocalTime(TimeValue t, void *val, Interval &valid, GetSetMethod method)
{
	*(float*)val = 0.0f;
	if ( !fOwner )
		return;
	const int num = fOwner->GetNumTargets();
	if ( num == 0 )
		return;
	for ( int i=1; i<num; ++i )
	{
		if ( fOwner->GetLinkTime( i ) > t )
		{
			*(float*)val = (float)(i-1);
			return;
		}
	}
	*(float*)val = (float)(num-1);

	valid.SetInstant(t);
}

int		LinkTimeControl::PaintFCurves( ParamDimensionBase *dim, HDC hdc, Rect& rcGraph, Rect& rcPaint,
			float tzoom, int tscroll, float vzoom, int vscroll, DWORD flags )
{
	const int n = NumKeys();
	if ( n == 0 )
		return 0;

	Interval valid;
	int h = rcGraph.h()-1;
	HPEN dpen,spen;
	BOOL init=FALSE;		
	Interval range = GetTimeRange(TIMERANGE_ALL);	
	SetBkMode(hdc,TRANSPARENT);	

	dpen = CreatePen(PS_DOT,0,GetColorManager()->GetColor(kFunctionCurveFloat));
	spen = CreatePen(PS_SOLID,0,GetColorManager()->GetColor(kFunctionCurveFloat));

	SIZE size;
	GetTextExtentPoint( hdc, _T("0"), 1, &size );

	float val;
	TimeValue leftTime = ScreenToTime(rcPaint.left,tzoom,tscroll);
	TimeValue rightTime = ScreenToTime(rcPaint.right,tzoom,tscroll);
	int x, y;

	// dotted line to left of keys
	if ( leftTime < range.Start() )
	{
		SelectObject(hdc,dpen);
		GetValue(range.Start(),&val,valid);
		y = ValueToScreen(dim->Convert(val),h,vzoom,vscroll);
		MoveToEx(hdc,rcPaint.left,y,NULL);
		LineTo(hdc,TimeToScreen(range.Start(),tzoom,tscroll),y);
	}

	SelectObject(hdc,spen);

	// first node text
	{
		TimeValue t = GetKeyTime( 0 );
		if ( t >= leftTime && t <= rightTime )
		{
			GetValue(t,&val,valid);
			y = ValueToScreen(dim->Convert(val),h,vzoom,vscroll);
			x = TimeToScreen(t,tzoom,tscroll);
			INode* node = fOwner->GetNode( 0 );
			DLTextOut( hdc, x, y-1-size.cy, node ? node->GetName() : "World" );
		}
	}

	// solid line between keys
	for ( int i=1; i<n; ++i )
	{
		TimeValue t0 = GetKeyTime( i-1 );
		TimeValue t1 = GetKeyTime( i );
		if ( t1 < leftTime || t0 > rightTime )
			continue;
		GetValue(t0,&val,valid);
		y = ValueToScreen(dim->Convert(val),h,vzoom,vscroll);
		MoveToEx(hdc,TimeToScreen(t0,tzoom,tscroll),y,NULL);
		x = TimeToScreen(t1,tzoom,tscroll);
		LineTo(hdc,x,y);
		GetValue(t1,&val,valid);
		y = ValueToScreen(dim->Convert(val),h,vzoom,vscroll);
		LineTo(hdc,x,y);

		INode* node = fOwner->GetNode( i );
		DLTextOut( hdc, x, y-1-size.cy, node ? node->GetName() : "World" );
	}

	// dotted line to right of keys
	if ( rightTime > range.End() )
	{
		SelectObject(hdc,dpen);
		GetValue(range.End(),&val,valid);
		y = ValueToScreen(dim->Convert(val),h,vzoom,vscroll);
		MoveToEx(hdc,TimeToScreen(range.End(),tzoom,tscroll),y,NULL);
		LineTo(hdc,rcPaint.right,y);
	}

	SelectObject( hdc, spen );
	HBRUSH hUnselBrush = CreateSolidBrush(GetColorManager()->GetColor(kTrackbarKeys));
	HBRUSH hSelBrush = CreateSolidBrush(GetColorManager()->GetColor(kTrackbarSelKeys));

	// render keys themselves
	for ( int i=0; i<n; ++i )
	{
		TimeValue t = GetKeyTime( i );
		if ( t < leftTime || t > rightTime )
			continue;
		GetValue(t,&val,valid);
		y = ValueToScreen(dim->Convert(val),h,vzoom,vscroll);
		x = TimeToScreen(t,tzoom,tscroll);
		SelectObject( hdc, IsKeySelected( i ) ? hSelBrush : hUnselBrush );
		Rectangle(hdc,x-3,y-3,x+3,y+3);
	}

	SetBkMode(hdc,OPAQUE);
	SelectObject(hdc,GetStockObject(BLACK_PEN));	

	DeleteObject(spen);
	DeleteObject(dpen);
	DeleteObject(hUnselBrush);
	DeleteObject(hSelBrush);

	return 0;
}

int		LinkTimeControl::HitTestFCurves( ParamDimensionBase *dim, TrackHitTab& hits, Rect& rcHit, Rect& rcGraph,			
			float tzoom, int tscroll, float vzoom, int vscroll, DWORD flags )
{
	int h = rcGraph.h()-1;

	bool hit = false;

	const int n = NumKeys();
	for ( int i=0; i<n; ++i )
	{
		BOOL sel = IsKeySelected( i );
		if ( ( flags & HITTRACK_SELONLY ) && !sel )
			continue;
		if ( ( flags & HITTRACK_UNSELONLY ) && sel )
			continue;
		TimeValue t = GetKeyTime( i );
		int x = TimeToScreen( t, tzoom, tscroll );
		float val;
		Interval valid;
		GetValue( t, &val, valid );
		int y = ValueToScreen( dim->Convert(val), h, vzoom, vscroll );
		if ( rcHit.Contains( IPoint2( x, y ) ) )
		{
			hit = true;
			TrackHitRecord rec( i, 0 );
			hits.Append( 1, &rec );
			if ( flags & HITTRACK_ABORTONHIT )
				break;
		}
	}

	return hit ?  HITCURVE_KEY : HITCURVE_NONE;
}
void LinkTimeControl::MapKeys(TimeMap *map,DWORD flags)
{
    int n = NumKeys();
    BOOL changed = FALSE;
    if (n)
	{
		fOwner->ActivateSort(false);
		if (flags&TRACK_DOALL) {
				for (int i = 0; i < n; i++) {
						ChangeKeyTime( i, map->map( GetKeyTime(i) ) );
						changed = TRUE;
				}
		} else if (flags&TRACK_DOSEL) {
				BOOL slide = flags&TRACK_SLIDEUNSEL;
				TimeValue delta = 0, prev;
				int start, end, inc;
				if (flags&TRACK_RIGHTTOLEFT) {
						start = n-1;
						end = -1;
						inc = -1;
				}
				else {
						start = 0;
						end = n;
						inc = 1;
				}
				for (int i = start; i != end; i += inc) {
						if ( IsKeySelected(i) ) {
								prev = GetKeyTime(i);
								ChangeKeyTime( i,
										map->map( GetKeyTime(i) ) );
								delta = GetKeyTime(i) - prev;
								changed = TRUE;
								static int a=2;
								if (i!=a)
								{
									int b =0;
									b++;
								}

						}
						else if (slide) {
								ChangeKeyTime( i, GetKeyTime(i) + delta );
								changed = TRUE;
						}
				}
		}
		fOwner->ActivateSort(true);
		if (changed) {
			fOwner->LinkTimeChanged();
			NotifyDependents(FOREVER, PART_ALL,
						REFMSG_CHANGE);
		}
	}
    StdControl::MapKeys(map,flags);
}
int  LinkTimeControl::NumKeys()
{
	if ( fOwner )
		return fOwner->GetNumTargets();
	return 0;
}
TimeValue  LinkTimeControl::GetKeyTime (int index)
{
	if ( fOwner )
		return fOwner->GetLinkTime( index );
	return 0;
}

void LinkTimeControl::ChangeKeyTime( int i, TimeValue t )
{
	if ( fOwner )
		fOwner->SetLinkTime( i, t );
}
void  LinkTimeControl::DeleteKey( int index )
{
	if ( fOwner )
		fOwner->DeleteTarget( index );
//	if ( index < fSelected.GetSize() )
//		fSelected.Shift( LEFT_BITSHIFT, 1, index );
}


void LinkTimeControl::SelectKeyByIndex(int i, BOOL sel) 
{
/*	if (i < 0)
		return;
	if ( i >= fSelected.GetSize() )
		fSelected.SetSize( i+1, 1 );
	fSelected.Set( i, sel );
*/
	fOwner->SelectKeyByIndex(i,sel);

	NotifyDependents(FOREVER, PART_ALL, REFMSG_KEY_SELECTION_CHANGED);
}
BOOL LinkTimeControl::IsKeySelected(int i)
{
/*	if (i < 0)
		return FALSE;
	if ( i >= fSelected.GetSize() )
		fSelected.SetSize( i+1, 1 );
	return fSelected[i];
*/
	return fOwner->IsKeySelected(i);
}