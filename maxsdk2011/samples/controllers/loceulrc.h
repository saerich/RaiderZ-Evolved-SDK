/**********************************************************************
*<
FILE: loceulrc.h

DESCRIPTION: A Local Euler angle rotation controller

CREATED BY: Pete Samson

HISTORY: modified from eulrctrl.cpp

*> Copyright (c) 1994, All Rights Reserved.
**********************************************************************/

#ifndef __LOCEULRC__H
#define __LOCEULRC__H

#include "ctrl.h"
#include <ILockedTracks.h>

class LocalEulerDlg;

class LocalEulerRotation : public LockableControl,public IEulerControl {
public:
	Control *rotX;
	Control *rotY;
	Control *rotZ;
	int order;
	Quat curval;
	Interval ivalid;
	Tab<TimeValue> keyTimes;

	static LocalEulerDlg *dlg;
	static IObjParam *ip;
	static ULONG beginFlags;
	static LocalEulerRotation *editControl; // The one being edited.

	LocalEulerRotation(const LocalEulerRotation &ctrl);
	LocalEulerRotation(BOOL loading=FALSE);
	~LocalEulerRotation();
	void Update(TimeValue t);
	DWORD GetDefaultInTan();
	DWORD GetDefaultOutTan();

	// Animatable methods
	Class_ID ClassID() { return Class_ID(LOCAL_EULER_CONTROL_CLASS_ID,0); }  
	SClass_ID SuperClassID() { return CTRL_ROTATION_CLASS_ID; }         

	void GetClassName(TSTR& s);
	void DeleteThis() { delete this; }        
	int IsKeyable() { return 1; }     

	int NumSubs()  { return 3; }
	Animatable* SubAnim(int i);
	TSTR SubAnimName(int i);
	int SubNumToRefNum(int subNum) { return subNum; }

	DWORD GetSubAnimCurveColor(int subNum);

	ParamDimension* GetParamDimension(int i) { return stdAngleDim; }
	BOOL AssignController(Animatable *control,int subAnim);
	void AddNewKey(TimeValue t,DWORD flags);
	int NumKeys();
	TimeValue GetKeyTime(int index);
	void SelectKeyByIndex(int i,BOOL sel);
	BOOL IsKeySelected(int i);
	void CopyKeysFromTime(TimeValue src, TimeValue dst, DWORD flags);
	BOOL IsKeyAtTime(TimeValue t, DWORD flags);
	BOOL GetNextKeyTime(TimeValue t, DWORD flags, TimeValue &nt);
	void DeleteKeyAtTime(TimeValue t);
	void DeleteKeys(DWORD flags);
	void DeleteKeyByIndex(int index);

	void BeginEditParams(IObjParam *ip, ULONG flags,Animatable *prev);
	void EndEditParams(IObjParam *ip, ULONG flags,Animatable *next);

	int SetProperty(ULONG id, void *data);
	void *GetProperty(ULONG id);

	// Reference methods
	int NumRefs() { return 3; };    
	RefTargetHandle GetReference(int i);
	void SetReference(int i, RefTargetHandle rtarg);
	RefResult NotifyRefChanged(Interval, RefTargetHandle, PartID&, RefMessage);
	void RescaleWorldUnits(float f) {}

	IOResult Save(ISave *isave);
	IOResult Load(ILoad *iload);

	// Control methods
	Control *GetXController() { return rotX; }
	Control *GetYController() { return rotY; }
	Control *GetZController() { return rotZ; }
	void Copy(Control *from);
	RefTargetHandle Clone(RemapDir& remap);
	BOOL IsLeaf() { return FALSE; }
	void GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method);    
	void SetValue(TimeValue t, void *val, int commit, GetSetMethod method);
	bool GetLocalTMComponents(TimeValue, TMComponentsArg&, Matrix3Indirect& parentTM);
	void CommitValue(TimeValue t);
	void RestoreValue(TimeValue t);
	void EnumIKParams(IKEnumCallback &callback);
	BOOL CompDeriv(TimeValue t, Matrix3& ptm, IKDeriv& derivs, DWORD flags);
	float IncIKParam(TimeValue t, int index, float delta);
	void ClearIKParam(Interval iv, int index);
	void EnableORTs(BOOL enable);
	void MirrorIKConstraints(int axis, int which);       
	BOOL CanCopyIKParams(int which);
	IKClipObject *CopyIKParams(int which);
	BOOL CanPasteIKParams(IKClipObject *co, int which);
	void PasteIKParams(IKClipObject *co, int which);		
	void ChangeOrdering(int newOrder);

	// RB 10/27/2000: Implemented to support HI IK
	void InitIKJoints2(InitJointData2 *posData,InitJointData2 *rotData);
	BOOL GetIKJoints2(InitJointData2 *posData,InitJointData2 *rotData);

	int GetOrder(){return order;}
	void SetOrder(int newOrder);

	void* GetInterface(ULONG id)
	{
		if (id==I_EULERCTRL)
			return static_cast<IEulerControl*>(this);
		else if (id == I_LOCKED)
			return static_cast<ILockedTrackImp*>(this);
		else 
			return Control::GetInterface(id);
	}
};

#define EDIT_X  0
#define EDIT_Y  1
#define EDIT_Z  2

#define EULER_BEGIN     1
#define EULER_MIDDLE    2
#define EULER_END       3

class LocalEulerDlg {
public:
	LocalEulerRotation *cont;
	HWND hWnd;
	IObjParam *ip;
	ICustButton *iEdit[3];
	static int cur;

	LocalEulerDlg(LocalEulerRotation *cont,IObjParam *ip);
	~LocalEulerDlg();

	void Init();
	void SetButtonText();
	void EndingEdit(LocalEulerRotation *next);
	void BeginingEdit(LocalEulerRotation *cont, IObjParam *ip,
		LocalEulerRotation *prev);
	void SetCur(int c,int code=EULER_MIDDLE);
	void WMCommand(int id, int notify, HWND hCtrl);
};

#endif // __LOCEULRC__H