/**********************************************************************
*<
FILE: eulrctrl.h

DESCRIPTION: An Euler angle rotation controller

CREATED BY: Rolf Berteig

HISTORY: created 13 June 1995

*>	Copyright (c) 1994, All Rights Reserved.
**********************************************************************/

#ifndef __EULRCTRL__H
#define __EULRCTRL__H

#include "ctrl.h"
#include <ILockedTracks.h>
class EulerDlg;

class EulerRotation : public LockableControl,public IEulerControl {
public:
	Control *rotX;
	Control *rotY;
	Control *rotZ;
	int order;
	Point3 curval;
	Interval ivalid;
	BOOL blockUpdate;
	Tab<TimeValue> keyTimes;

	static EulerDlg *dlg;
	static IObjParam *ip;
	static ULONG beginFlags;
	static EulerRotation *editControl; // The one being edited.

	EulerRotation(const EulerRotation &ctrl);
	EulerRotation(BOOL loading=FALSE);
	~EulerRotation();
	void Update(TimeValue t);

	// Animatable methods
	Class_ID ClassID() { return Class_ID(EULER_CONTROL_CLASS_ID,0); }  
	SClass_ID SuperClassID() { return CTRL_ROTATION_CLASS_ID; }  		

	void GetClassName(TSTR& s);
	void DeleteThis() {delete this;}		
	int IsKeyable() {return 1;}		

	int NumSubs()  {return 3;}
	Animatable* SubAnim(int i);
	TSTR SubAnimName(int i);
	int SubNumToRefNum(int subNum);
	DWORD GetSubAnimCurveColor(int subNum);

	ParamDimension* GetParamDimension(int i) {return stdAngleDim;}
	BOOL AssignController(Animatable *control,int subAnim);
	void AddNewKey(TimeValue t,DWORD flags);
	int NumKeys();
	TimeValue GetKeyTime(int index);
	void SelectKeyByIndex(int i,BOOL sel);
	BOOL IsKeySelected(int i);
	void CopyKeysFromTime(TimeValue src,TimeValue dst,DWORD flags);
	BOOL IsKeyAtTime(TimeValue t,DWORD flags);
	BOOL GetNextKeyTime(TimeValue t,DWORD flags,TimeValue &nt);
	void DeleteKeyAtTime(TimeValue t);
	void DeleteKeys(DWORD flags);
	void DeleteKeyByIndex(int index);

	void BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev );
	void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next );

	int SetProperty(ULONG id, void *data);
	void *GetProperty(ULONG id);

	// Reference methods
	int NumRefs() { return 3; };	
	RefTargetHandle GetReference(int i);
	void SetReference(int i, RefTargetHandle rtarg);
	int RemapRefOnLoad(int iref);
	RefResult NotifyRefChanged(Interval, RefTargetHandle, PartID&, RefMessage);
	void RescaleWorldUnits(float f) {}

	IOResult Save(ISave *isave);
	IOResult Load(ILoad *iload);

	// Control methods
	Control *GetXController() {return rotX;}
	Control *GetYController() {return rotY;}
	Control *GetZController() {return rotZ;}
	void Copy(Control *from);
	RefTargetHandle Clone(RemapDir& remap);
	BOOL IsLeaf() {return FALSE;}
	void GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method);	
	void SetValue(TimeValue t, void *val, int commit, GetSetMethod method);
	bool GetLocalTMComponents(TimeValue, TMComponentsArg&, Matrix3Indirect& parentTM);
	void CommitValue(TimeValue t);
	void RestoreValue(TimeValue t);
	void EnumIKParams(IKEnumCallback &callback);
	BOOL CompDeriv(TimeValue t,Matrix3& ptm,IKDeriv& derivs,DWORD flags);
	float IncIKParam(TimeValue t,int index,float delta);
	void ClearIKParam(Interval iv,int index);
	void EnableORTs(BOOL enable);
	// AF (09/20/02) Pass these methods onto the subAnims
	int GetORT(int type);
	void SetORT(int ort, int type);
	void MirrorIKConstraints(int axis,int which);		
	BOOL CanCopyIKParams(int which);
	IKClipObject *CopyIKParams(int which);
	BOOL CanPasteIKParams(IKClipObject *co,int which);
	void PasteIKParams(IKClipObject *co,int which);

	// JZ, 11/14/2000. To support HD IK (263950). This method is
	// called when assign HD IK solver.
	BOOL GetIKJoints(InitJointData *posData,InitJointData *rotData);

	// RB 10/27/2000: Implemented to support HI IK
	void InitIKJoints2(InitJointData2 *posData,InitJointData2 *rotData);
	BOOL GetIKJoints2(InitJointData2 *posData,InitJointData2 *rotData);

	void ChangeOrdering(int newOrder);

	BOOL CreateLockKey(TimeValue t, int which);

	inline Control* SubControl(int i);
	byte reorderOnLoad : 1;

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

	class SetValueRestore : public RestoreObj {
		EulerRotation* m_cont;
		bool m_preSetValue;
	public:
		SetValueRestore::SetValueRestore(EulerRotation* cont, bool preSetValue) : m_cont(cont), m_preSetValue(preSetValue) 
		{
			m_cont->SetAFlag(A_HELD); 
		}
		void Restore(int isUndo) {
			m_cont->blockUpdate = !m_preSetValue;
			if (m_preSetValue) {
				m_cont->ivalid.SetEmpty();
				PreRefNotifyDependents();
				m_cont->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
				PostRefNotifyDependents();
			}
		}
		void Redo() {
			m_cont->blockUpdate = m_preSetValue;
			if (!m_preSetValue) {
				m_cont->ivalid.SetEmpty();
				PreRefNotifyDependents();
				m_cont->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
				PostRefNotifyDependents();
			}
		}
		int Size() {return 1;}
		void EndHold() { m_cont->ClearAFlag(A_HELD); }
		TSTR Description() { return TSTR(_T("SetValueRestore")); }
	};
};

#define EDIT_X	0
#define EDIT_Y	1
#define EDIT_Z	2

#define EULER_BEGIN		1
#define EULER_MIDDLE	2
#define EULER_END		3

class EulerDlg {
public:
	EulerRotation *cont;
	HWND hWnd;
	IObjParam *ip;
	ICustButton *iEdit[3];
	static int cur;

	EulerDlg(EulerRotation *cont,IObjParam *ip);
	~EulerDlg();

	void Init();
	void SetButtonText();
	void EndingEdit(EulerRotation *next);
	void BeginingEdit(EulerRotation *cont,IObjParam *ip,EulerRotation *prev);
	void SetCur(int c,int code=EULER_MIDDLE);
	void AdjustCur(int prev_ref);
	void WMCommand(int id, int notify, HWND hCtrl);
};

#endif // __EULRCTRL__H