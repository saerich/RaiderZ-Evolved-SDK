/**********************************************************************
*<
FILE: attach.h

DESCRIPTION: Attachment controller

CREATED BY: Rolf Berteig

HISTORY: 11/18/96

*>	Copyright (c) 1996, All Rights Reserved.
**********************************************************************/
// NOTE: included in maxsdk\samples\mxs\agni\lam_ctrl.cpp

#ifndef __ATTACHCTRL__H
#define __ATTACHCTRL__H

#include "ctrl.h"

class AKey {
public:		
	TimeValue time;
	DWORD flags;
	DWORD face;		
	float u0, u1;
	float tens, cont, bias, easeIn, easeOut;

	Point3 pos, norm, din, dout, dir;
	Quat quat, qa, qb;

	AKey() {
		flags=0; face=0; u0=u1=tens=cont=bias=easeIn=easeOut=0.0f;
		pos=din=dout=Point3(0,0,0);
		norm=dir=Point3(1,0,0);
		quat=qa=qb=IdentQuat();
	}		
	AKey &operator=(const AKey &k) {
		time    = k.time;
		flags	= k.flags;
		face	= k.face;
		u0		= k.u0;
		u1		= k.u1;
		tens	= k.tens;
		cont	= k.cont;
		bias	= k.bias;
		easeIn	= k.easeIn;
		easeOut	= k.easeOut;

		pos  = k.pos;
		din  = k.din;
		dout = k.dout;
		norm = k.norm;
		dir  = k.dir;
		quat = k.quat;
		qa   = k.qa;
		qb   = k.qb;
		return *this;
	}

};

class PickObjectMode;
class SetPosCMode;

class AttachCtrl : public StdControl, public TimeChangeCallback, public IAttachCtrl {
public:
	// Controller data
	Tab<AKey> keys;
	INode *node;
	BOOL rangeLinked, align, manUpdate, setPosButton;

	// Current value cache
	Interval range, valid;
	Point3 val;
	Quat qval;
	BOOL trackValid, doManUpdate;

	static HWND hWnd;
	static IObjParam *ip;
	static AttachCtrl *editCont;
	static BOOL uiValid;
	static ISpinnerControl *iTime, *iFace, *iA, *iB, *iTens, *iCont, *iBias, *iEaseTo, *iEaseFrom;
	static ICustButton *iPickOb, *iSetPos, *iPrev, *iNext, *iUpdate;
	static ICustStatus *iStat;
	static int index;
	static PickObjectMode *pickObMode;
	static SetPosCMode *setPosMode;

	AttachCtrl();

	// Animatable methods
	void DeleteThis() {delete this;}		
	int IsKeyable() {return 1;}
	BOOL IsAnimated() {return keys.Count()?TRUE:FALSE;}
	Class_ID ClassID() {return ATTACH_CONTROL_CLASS_ID;}
	SClass_ID SuperClassID() {return CTRL_POSITION_CLASS_ID;}
	void GetClassName(TSTR& s);
	void BeginEditParams(IObjParam *ip, ULONG flags,Animatable *prev); 
	void EndEditParams(IObjParam *ip, ULONG flags,Animatable *next); 

	// Animatable's Schematic View methods
	SvGraphNodeReference SvTraverseAnimGraph(IGraphObjectManager *gom, Animatable *owner, int id, DWORD flags);
	TSTR SvGetRelTip(IGraphObjectManager *gom, IGraphNode *gNodeTarger, int id, IGraphNode *gNodeMaker);
	bool SvCanDetachRel(IGraphObjectManager *gom, IGraphNode *gNodeTarget, int id, IGraphNode *gNodeMaker);
	bool SvDetachRel(IGraphObjectManager *gom, IGraphNode *gNodeTarget, int id, IGraphNode *gNodeMaker);
	bool SvLinkChild(IGraphObjectManager *gom, IGraphNode *gNodeThis, IGraphNode *gNodeChild);
	bool SvCanConcludeLink(IGraphObjectManager *gom, IGraphNode *gNode, IGraphNode *gNodeChild);

	// Reference methods
	RefResult NotifyRefChanged(Interval, RefTargetHandle, PartID&, RefMessage);
	IOResult Save(ISave *isave);
	IOResult Load(ILoad *iload);
	RefTargetHandle Clone(RemapDir &remap = DefaultRemapDir());
	int NumRefs() {return Control::NumRefs()+1;};	
	RefTargetHandle GetReference(int i);
	void SetReference(int i, RefTargetHandle rtarg);

	// Control methods				
	void Copy(Control *from);
	BOOL IsLeaf() {return TRUE;}
	void CommitValue(TimeValue t) {}
	void RestoreValue(TimeValue t) {}

	// Animatable methods
	Interval GetTimeRange(DWORD flags);
	void EditTimeRange(Interval range,DWORD flags);
	void MapKeys(TimeMap *map,DWORD flags );		
	int NumKeys() {return keys.Count();}
	TimeValue GetKeyTime(int index) {return keys[index].time;}
	int GetKeyIndex(TimeValue t);		
	void DeleteKeyAtTime(TimeValue t);
	BOOL IsKeyAtTime(TimeValue t,DWORD flags);				
	int GetKeyTimes(Tab<TimeValue> &times,Interval range,DWORD flags);
	int GetKeySelState(BitArray &sel,Interval range,DWORD flags);
	BOOL GetNextKeyTime(TimeValue t,DWORD flags,TimeValue &nt);
	void DeleteTime(Interval iv, DWORD flags);
	void ReverseTime(Interval iv, DWORD flags);
	void ScaleTime(Interval iv, float s);
	void InsertTime(TimeValue ins, TimeValue amount);
	BOOL SupportTimeOperations() {return TRUE;}
	void DeleteKeys(DWORD flags);
	void DeleteKeyByIndex(int index);
	void SelectKeys(TrackHitTab& sel, DWORD flags);
	void SelectKeyByIndex(int i,BOOL sel);
	void FlagKey(TrackHitRecord hit);
	int GetFlagKeyIndex();
	int NumSelKeys();
	void CloneSelectedKeys(BOOL offset=FALSE);
	void AddNewKey(TimeValue t,DWORD flags);		
	BOOL IsKeySelected(int index);
	BOOL CanCopyTrack(Interval iv, DWORD flags) {return TRUE;}
	BOOL CanPasteTrack(TrackClipObject *cobj,Interval iv, DWORD flags) {return cobj->ClassID()==ClassID();}
	TrackClipObject *CopyTrack(Interval iv, DWORD flags);
	void PasteTrack(TrackClipObject *cobj,Interval iv, DWORD flags);
	int HitTestTrack(			
		TrackHitTab& hits,
		Rect& rcHit,
		Rect& rcTrack,			
		float zoom,
		int scroll,
		DWORD flags);
	int PaintTrack(
		ParamDimensionBase *dim,
		HDC hdc,
		Rect& rcTrack,
		Rect& rcPaint,
		float zoom,
		int scroll,
		DWORD flags);
	void EditTrackParams(
		TimeValue t,
		ParamDimensionBase *dim,
		TCHAR *pname,
		HWND hParent,
		IObjParam *ip,
		DWORD flags);
	int TrackParamsType() {return TRACKPARAMS_KEY;}

	// StdControl methods
	void GetValueLocalTime(TimeValue t, void *val, Interval &valid, GetSetMethod method=CTRL_ABSOLUTE) {}
	void GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method=CTRL_ABSOLUTE);
	void SetValueLocalTime(TimeValue t, void *val, int commit, GetSetMethod method) {}
	void Extrapolate(Interval range,TimeValue t,void *val,Interval &valid,int type) {}
	void *CreateTempValue() {return new Point3;}
	void DeleteTempValue(void *val) {delete (Point3*)val;}
	void ApplyValue(void *val, void *delta) {((Matrix3*)val)->PreTranslate(*((Point3*)delta));}
	void MultiplyValue(void *val, float m) {*((Point3*)val) *= m;}

	// TimeChangedCallback
	void TimeChanged(TimeValue t) {InvalidateUI();}

	// IAttachCtrl
	BOOL SetObject(INode *node);
	INode* GetObject();
	void SetKeyPos(TimeValue t, DWORD fi, Point3 bary);
	AKey* GetKey(int index);
	BOOL GetAlign();
	void SetAlign(BOOL align);
	BOOL GetManualUpdate();
	void SetManualUpdate(BOOL manUpdate);
	void Invalidate(BOOL forceUpdate = FALSE);

	// BaseInterface
	Interface_ID GetID() { return I_ATTACHCTRL; }

	// Local methods
	void SortKeys();
	void HoldTrack();
	void PrepareTrack(TimeValue t);
	float GetInterpVal(TimeValue t,int &n0, int &n1);
	Point3 PointOnPath(TimeValue t);
	Quat QuatOnPath(TimeValue t);		

	Interval CompValidity(TimeValue t);

	void CompFirstDeriv();
	void CompLastDeriv();
	void Comp2KeyDeriv();
	void CompMiddleDeriv(int i);
	void CompAB(int i);

	void InvalidateUI();
	void UpdateUI();
	void UpdateTCBGraph();
	void UpdateBaryGraph();
	void SetupWindow(HWND hWnd);
	void DestroyWindow();
	void SpinnerChange(int id);
	void Command(int id, LPARAM lParam);

	BaseInterface* GetInterface(Interface_ID id)
	{
		if (id==I_ATTACHCTRL)
			return static_cast<IAttachCtrl*>(this);
		else 
			return StdControl::GetInterface(id);
	}

};


#endif // __ATTACHCTRL__H