/**********************************************************************
 *
	FILE: Link_timectrl.h

	DESCRIPTION: Class declaration for LinkTimeControl
				
	            

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

#pragma once
#include "ctrl.h"


class LinkConstTransform;


//  LinkTimeControl takes the responsability of exposing the keys in the various animation editors for LinkConstTransform
//  Instead of adding more complexity to the parent object, it was decided to code things here. The LinkConstTransform object creates this object and exposes the relevant information 
//  so keys can be exposed and edited in the trackbar.
//
class LinkTimeControl : public StdControl {
protected:
	~LinkTimeControl();
public:
	LinkTimeControl();

	Class_ID ClassID() {return LINK_TIME_CONTROL_CLASS_ID;} 
	SClass_ID SuperClassID() {return CTRL_FLOAT_CLASS_ID;}
	void GetClassName(TSTR& s) {s = "LinkTimeControl";}

	RefResult NotifyRefChanged(Interval, RefTargetHandle, PartID&, RefMessage) {return REF_SUCCEED;}
	RefTargetHandle Clone( RemapDir& remap );

	void Copy(Control *from) {}

	void GetValueLocalTime(TimeValue t, void *val, Interval &valid, GetSetMethod method=CTRL_ABSOLUTE);
	void SetValueLocalTime(TimeValue t, void *val, int commit, GetSetMethod method) {}
	void Extrapolate(Interval range,TimeValue t,void *val,Interval &valid,int type) {}
	void *CreateTempValue() {return new float;}
	void DeleteTempValue(void *val) {delete (float*)val;}
	void ApplyValue(void *val, void *delta) {*((float*)val) += *((float*)delta);}
	void MultiplyValue(void *val, float m) {*((float*)val) *= m;}

	int  NumKeys();

	TimeValue  GetKeyTime (int index);

	void SelectKeyByIndex(int i, BOOL sel);
	BOOL IsKeySelected(int i);

	int NumSelKeys()
	{
		int num = 0;
		const int n = NumKeys();
		for ( int i=0; i<n; ++i )
		{
			if ( IsKeySelected( i ) )
				num += 1;
		}
		return num;
	}
	void ChangeKeyTime( int i, TimeValue t );

	void MapKeys(TimeMap *map,DWORD flags);
	void  SelectKeys (TrackHitTab &sel, DWORD flags)
	{
		if ( flags & SELKEYS_CLEARKEYS )
		{
	        const int n = NumKeys();
			for ( int i=0; i<n; ++i )
				SelectKeyByIndex( i, FALSE );
		}
		else if ( flags & SELKEYS_SELECT )
		{
			const int n = sel.Count();
			for ( int i=0; i<n; ++i )
				SelectKeyByIndex( sel[i].hit, TRUE );
		}
		else if ( flags & SELKEYS_DESELECT )
		{
			const int n = sel.Count();
			for ( int i=0; i<n; ++i )
				SelectKeyByIndex( sel[i].hit, FALSE );
		}
	}
	

	Interval GetTimeRange(DWORD flags)
	{
		Interval range;
        const int n = NumKeys();
		for ( int i=0; i<n; ++i )
		{
			if ( ( flags & TIMERANGE_SELONLY ) && !IsKeySelected( i ) )
				continue;
			if ( range.Empty() )
				range.SetInstant( GetKeyTime( i ) );
			else
				range += GetKeyTime( i );
		}
		return range;
	}
	void  DeleteKey( int index );

	void  DeleteTime (Interval iv, DWORD flags)
	{
		Interval test = TestInterval( iv, flags );
		// delete the keys in the interval
        const int n = NumKeys();
		for ( int i=n-1; i>=0; i-- )
		{
			TimeValue t = GetKeyTime( i );
			if ( test.InInterval( t ) )
			{
				DeleteKey( i );
			}
		}
		// move the remaining keys
		if ( !(flags & TIME_NOSLIDE) )
		{
			int d = iv.Duration() - 1;
			if ( d < 0 )
				d = 0;
	        const int n = NumKeys();
			for ( int i=0; i<n; ++i )
			{
				TimeValue t = GetKeyTime( i );
				if ( t > test.End() )
				{
					ChangeKeyTime( i, t-d );
				}
			}
		}
		NotifyDependents( FOREVER, PART_ALL, REFMSG_CHANGE );
	}
	void  ReverseTime (Interval iv, DWORD flags)
	{
		Interval test = TestInterval(iv,flags);
		const int n = NumKeys();
		for ( int i=0; i<n; ++i )
		{
			TimeValue t = GetKeyTime( i );
			if ( test.InInterval( t ) )
			{
				TimeValue delta = t - iv.Start();
				ChangeKeyTime( i, iv.End() - delta );
			}
		}
		NotifyDependents( FOREVER, PART_ALL, REFMSG_CHANGE );
	}
	int PaintTrack( ParamDimensionBase *dim, HDC hdc, Rect& rcTrack, Rect& rcPaint, float zoom, int scroll, DWORD flags )
	{
		return TRACK_DOSTANDARD;
	}
	int HitTestTrack( TrackHitTab& hits, Rect& rcHit, Rect& rcTrack, float zoom, int scroll, DWORD flags )
	{
		return TRACK_DOSTANDARD;
	}

	int PaintFCurves( ParamDimensionBase *dim, HDC hdc, Rect& rcGraph, Rect& rcPaint,
			float tzoom, int tscroll, float vzoom, int vscroll, DWORD flags );
	int HitTestFCurves( ParamDimensionBase *dim, TrackHitTab& hits, Rect& rcHit, Rect& rcGraph,			
			float tzoom, int tscroll, float vzoom, int vscroll, DWORD flags );

	void SelectCurve(BOOL sel)
	{
		fCurveSelected = sel;
	}
	BOOL IsCurveSelected()
	{
		return fCurveSelected;
	}		
	BOOL IsAnimated()
	{
		return ( NumKeys() > 0 );
	}

	void	SetOwner( LinkConstTransform* owner )	{ fOwner = owner; }

private:
	LinkConstTransform*	fOwner;
	BOOL		fCurveSelected;

};

class LinkTimeControlClassDesc : public ClassDesc {
public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading) {return new LinkTimeControl();}
	const TCHAR *	ClassName() {return "LinkTimeControl";}
	SClass_ID		SuperClassID() {return CTRL_FLOAT_CLASS_ID; }
	Class_ID		ClassID() {return LINK_TIME_CONTROL_CLASS_ID;}
	const TCHAR* 	Category() {return _T("");}
	};
