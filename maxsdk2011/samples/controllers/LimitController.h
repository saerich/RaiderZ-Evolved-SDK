/*****************************************************************************

	FILE: LimitController.h

	DESCRIPTION: A limit controller is used to restrict the range of values
	produced by other controllers.

	The controller whose range of value is being restricted is called the
	limited controller.  Its presence is optional.

	Limited controllers can be disabled, in which case they just let the  
	limited controller pass through.  Upper and Lower limits can also be turned 
	off independently.

	There is a family of controllers for different data types, all deriving
	from the BaseLimitController class: FloatLimitController, etc.

	The BaseLimitController is responsible for the Enable state as well as
	keeping a reference on the limited controller.

	CREATED BY:	Nicolas Léonard

	HISTORY:	October 7th, 2004	Creation

 	Copyright (c) 2004, All Rights Reserved.
 *****************************************************************************/

#ifndef __LIMITCTRL__H
#define __LIMITCTRL__H

#include "Max.h"
#include "istdplug.h"
#include "iparamb2.h"
#include "iparamm2.h"
#include "ILimitCtrl.h"
#include "resource.h"
#include <ILockedTracks.h>

extern TCHAR *GetString(int id);
extern HINSTANCE hInstance;

#define BASELIMITCTRL_CLASS_ID	Class_ID(0xcef5c686, 0x96fd2228)

#define BASELIMIT_PBLOCK_REF		0
#define BASELIMIT_LIMITEDCTRL_REF	1
#define BASELIMIT_PBLOCK_SUBANIM	0
#define BASELIMIT_LIMITEDCTRL_SUBANIM	1


///////////////////////////////////////////////////////////////////////////////
//
//	The BaseLimitCtrl class is responsible for the Enable state as well as
// keeping a reference on the limited controller.
//
//	References:	2
//		0:	its parameter block
//		1:	the limited controller
//
//	SubAnims: 2
//		0:	its parameter block
//		1:	the limited controller
//
//	Parameter blocks: 1
//		BaseParamBlock
//			enable	bool: global disable flag for the controller.
//
//	Interfaces supported:
//		ILimitControl	ID=IID_LIMIT_CONTROL
//
// Notes:
//	- The base limit controller keeps track of the dimension of what
//		it is animating.  See GetParamDimension() for details.
//	- Derived controllers should implement the following methods:
//		virtual void SetStaticValue(void *in_val, GetSetMethod method);
//		virtual void GetStaticValue(void *in_val, GetSetMethod method) const;
//		virtual void SetUpperLimit(const TimeValue& in_t, void *in_val);
//		virtual void SetLowerLimit(const TimeValue& in_t, void *in_val);
//		virtual void ClampTypedValue(const TimeValue& in_t, void *io_val) const;
//		virtual void GetTypedValueAbsolute(const TimeValue& in_t, void *io_val) const;
///////////////////////////////////////////////////////////////////////////////
class BaseLimitCtrl: public LockableControl,	public ILimitControl

{
public:
	// Enums for various parameters
	enum { baselimitctrl_params };

	enum { 
		pb_enable
	};

	static const float s_lowerLimitDefaultValue;
	static const float s_upperLimitDefaultValue;

	// Constructor/Destructor
	BaseLimitCtrl();
	virtual ~BaseLimitCtrl();

	// ILimitCtrl interface methods
	///////////////////////////////////////////////////////////////////////////

	virtual bool IsEnabled() const;
	virtual void SetEnabled(bool in_enabled);
	virtual Control* GetLimitedControl() const;
	virtual void SetLimitedControl(Control *in_limitedCtrl);

	// Derived classes should override these methods according to type
	virtual void SetUpperLimit(const TimeValue& in_t, void *in_val) {}
	virtual void SetLowerLimit(const TimeValue& in_t, void *in_val) {}

	// From Animatable
	///////////////////////////////////////////////////////////////////////////
	virtual Class_ID ClassID() {return BASELIMITCTRL_CLASS_ID;}		
	virtual SClass_ID SuperClassID() { return CTRL_FLOAT_CLASS_ID; }
	virtual void GetClassName(TSTR& s) {s = GetString(IDS_BASELIMITCTRL_CLASS_NAME);}
	virtual BOOL IsAnimated() {return TRUE;}
	virtual int IsKeyable() {return TRUE;}	// Enables spinner edition of controlled parameters
	virtual int IsLeaf() {return FALSE;}		

	virtual ParamDimension* GetParamDimension(int in_sub);

	virtual int TrackParamsType() { 
		// Track parameters are invoked by right clicking anywhere in the track
		if(GetLocked()==false) return TRACKPARAMS_WHOLE; else return TRACKPARAMS_NONE; 
	}
	IOResult Save(ISave *isave);
	IOResult Load(ILoad *iload);
	// Returns a default control appropriate for the limit data type
	Control* GetDefaultControlForDataType();

	// Reference methods
	///////////////////////////////////////////////////////////////////////////
	virtual	RefResult NotifyRefChanged(Interval changeInt,  
		RefTargetHandle hTarget, PartID& partID,  RefMessage message) { 
			return REF_SUCCEED;}

	virtual	int NumSubs() { return 2; }	// The parameter block and limited control
	virtual	TSTR SubAnimName(int i) { 
		if (i == 0) 
			return GetString(IDS_BASELIMIT_PARAMS);
		else {
			TSTR name(GetString(IDS_LIMITCTRL_LIMITEDCTRL));
			if (GetLimitedControl()) {
				TSTR clsName;
				GetLimitedControl()->GetClassName(clsName);
				name.Append(": ");
				name.Append(clsName);
			}
			return name;
		}
	}
	virtual	Animatable* SubAnim(int i) {
		if (i == 0) 
			return m_pblock;
		else
			return m_limitedCtrl;
	}
	virtual	int SubNumToRefNum(int subNum) {
		if (subNum == 0) 
			return BASELIMIT_PBLOCK_REF;
		else
			return BASELIMIT_LIMITEDCTRL_REF;
	}
	virtual BOOL AssignController(Animatable *control, int subAnim);

	// Maintain the number of references here
	virtual	int NumRefs() { return 2; }
	virtual	RefTargetHandle GetReference(int in_noRef);
	virtual	void SetReference(int in_noRef, RefTargetHandle rtarg);

	// Return the number of ParamBlocks in this instance
	virtual	int	NumParamBlocks() { return 1; }
	virtual	IParamBlock2* GetParamBlock(int i) { 
		return m_pblock; } // return i'th ParamBlock
	virtual	IParamBlock2* GetParamBlockByID(BlockID id) { 
		return (m_pblock->ID() == id) ? m_pblock : NULL; } // return id'd ParamBlock

	virtual	void DeleteThis() { delete this; }		

	// Control methods
	///////////////////////////////////////////////////////////////////////////
	virtual void Copy(Control *from);

	virtual void GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method=CTRL_ABSOLUTE);
	virtual	void SetValue(TimeValue t, void *val, int commit=1, GetSetMethod method=CTRL_ABSOLUTE);

	// The following control methods simply delegate the calls to the limited controller
	virtual	Interval GetTimeRange(DWORD flags);
	virtual	void AddNewKey(TimeValue t,DWORD flags);
	virtual	void CloneSelectedKeys(BOOL offset);
	virtual	void DeleteKeys(DWORD flags);
	virtual	void SelectKeys(TrackHitTab& sel, DWORD flags);
	virtual	BOOL IsKeySelected(int index);
	virtual	void CopyKeysFromTime(TimeValue src,TimeValue dst,DWORD flags);
	virtual	void DeleteKeyAtTime(TimeValue t);
	virtual	BOOL IsKeyAtTime(TimeValue t,DWORD flags);
	virtual	BOOL GetNextKeyTime(TimeValue t,DWORD flags,TimeValue &nt);
	virtual	int GetKeyTimes(Tab<TimeValue> &times,Interval range,DWORD flags);
	virtual	int GetKeySelState(BitArray &sel,Interval range,DWORD flags);
	virtual	void CommitValue(TimeValue t);
	virtual	void RestoreValue(TimeValue t);
	virtual	void EnumIKParams(IKEnumCallback &callback);
	virtual	BOOL CompDeriv(TimeValue t, Matrix3& ptm, IKDeriv& derivs, DWORD flags);
	virtual	void MouseCycleCompleted(TimeValue t);
	virtual	BOOL InheritsParentTransform();
	virtual	void NotifyForeground(TimeValue t);

	// FPMixinInterface
	///////////////////////////////////////////////////////////////////////////
	virtual BaseInterface* GetInterface(Interface_ID in_interfaceID) 
	{ 
		if (in_interfaceID == IID_LIMIT_CONTROL) 
			return (ILimitControl*)this; 
		else 
			return FPMixinInterface::GetInterface(in_interfaceID);
	}

protected:
	// Derived classes should override these methods.  
	virtual void SetStaticValue(void *in_val, GetSetMethod method) {}
	virtual void GetStaticValue(void *in_val, GetSetMethod method) const {}
	
	// io_val is a pointer to data of type depending on the specific controller (float etc).  
	// It is always an absolute (not relative) value.  The value is clamped if need be, and returned
	virtual void ClampTypedValue(const TimeValue& in_t, void *io_val) const {}
	virtual void GetTypedValueAbsolute(const TimeValue& in_t, void *io_val) const {}

	virtual void GetUnlimitedValue(const TimeValue& t, void *val, Interval &valid, 
		GetSetMethod method=CTRL_ABSOLUTE) const;

	virtual void *CreateTempValue();
	virtual void DeleteTempValue(void *val);

	IParamBlock2	*m_pblock;		// Parameter block: ref 0
	Control			*m_limitedCtrl;	// Limited controller: ref 1
};

///////////////////////////////////////////////////////////////////////////////
class BaseLimitCtrlParamBlockDesc: public ParamBlockDesc2
{
public:
    BaseLimitCtrlParamBlockDesc(
        ClassDesc2 *io_descriptor);
    virtual ~BaseLimitCtrlParamBlockDesc();

protected:
private:
    // not implemented
    BaseLimitCtrlParamBlockDesc(const BaseLimitCtrlParamBlockDesc &in_baseLimitCtrlParamBlockDescriptor);
    BaseLimitCtrlParamBlockDesc &operator=(const BaseLimitCtrlParamBlockDesc &in_baseLimitCtrlParamBlockDescriptor);
};

#define FLOATLIMIT_PBLOCK_REF	2

///////////////////////////////////////////////////////////////////////////////
//
//	Class descriptor for the FloatLimitCtrl.
//
///////////////////////////////////////////////////////////////////////////////
class FloatLimitCtrlClassDesc : public ClassDesc2 {
public:
	int 			IsPublic() { return TRUE; }
	void *			Create(BOOL loading = FALSE);
	const TCHAR *	ClassName() { return GetString(IDS_FLOATLIMITCTRL_CLASS_NAME); }
	SClass_ID		SuperClassID() { return CTRL_FLOAT_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(FLOATLIMITCTRL_CLASS_ID, 0); }
	const TCHAR* 	Category() { return _T(""); }	// @todo GetString()
	const TCHAR*	InternalName() { return _T("FloatLimitCtrl"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }			// returns owning module handle
};


///////////////////////////////////////////////////////////////////////////////
//
//	The FloatLimitCtrl class is the float version of a limit controller.
//
//	The float limit controller may also define a buffer zone near the limits
// where the limit progressively takes effect.  The smoothing is defined with 
// a buffer width parameter, controlled independently at the lower and upper
// end.  If the width is 0 no smoothing is done.
//
//	Let's say the upper limit is at 100.  An upper buffer width of 10 will mean
// that values of the limited controller below 90 will not be affected by the 
// limit.  But above that the controller will limit value more and more, until
// a point where the upper limit value will be reached.  This point is reached 
// for a value of the limited controller of upper limit + 2.0f*width.  In our 
// example, the upper limit value will be reached at 120.  See the spec document
// for details.
//
//	References:	3 (1 of its own, plus those kept by the base limit controller)
//		2:	the float parameter block
//
//	SubAnims: 3 (1 of its own, plus those kept by the base limit controller)
//		2:	the float parameter block
//
//	Parameter blocks: 1
//		FloatParamBlock
//			upper_limit			float: maximum value returned by the controller 
//									if enabled
//			lower_limit			float: minimum value returned by the controller 
//									if enabled
//			upper_limit_enabled	bool: whether there is a maximum value that the 
//									controller can return
//			lower_limit_enabled	bool: whether there is a minimum value that the 
//									controller can return
//			static_value		float: (internal) used when there is no limited 
//									controller
//			upper_smoothing		float: distance from the upper limit at which the 
//									controller starts to have an effect
//			lower_smoothing		float: distance from the lower limit at which the 
//									controller starts to have an effect
//
//	Interfaces supported:	(base class only)
//
///////////////////////////////////////////////////////////////////////////////
class FloatLimitCtrl : public BaseLimitCtrl
{
public:
	// Enums for various parameters
	enum { float_limitctrl_params = 1 };

	enum { 
		pb_upper_limit,
		pb_upper_limit_enabled,
		pb_upper_smoothing,
		pb_upper_distance,			// obsolete
		pb_lower_limit,
		pb_lower_limit_enabled,
		pb_lower_smoothing,
		pb_lower_distance,			// obsolete
		pb_static_value
	};

	// The padding factor is used to map the value of the smoothing padding to
	// a value of the limited controller that will reach the limit value.
	// It should not be more than 2.0f, otherwise the curve used to do the
	// smoothing will have an inflexion point between the smoothing start and
	// the limit value.  See spec document for details.
	const static float s_paddingFactor;

	// Constructor/Destructor
	FloatLimitCtrl();
	virtual ~FloatLimitCtrl();

	// Float Limit Controller methods
	float GetUpperLimit(const TimeValue& in_t) const;
	void SetUpperLimit(const TimeValue& in_t, float in_upperLimit);
	bool IsUpperLimitActive() const;
	void SetUpperLimitActive(const TimeValue& in_t, bool in_active);

	float GetLowerLimit(const TimeValue& in_t) const;
	void SetLowerLimit(const TimeValue& in_t, float in_lowerLimit);
	bool IsLowerLimitActive() const;
	void SetLowerLimitActive(const TimeValue& in_t, bool in_active);

	// Base Limit Controller overrides
	virtual void SetStaticValue(void *in_val, GetSetMethod method);
	virtual void GetStaticValue(void *in_val, GetSetMethod method) const;
	virtual void SetUpperLimit(const TimeValue& in_t, void *in_val);
	virtual void SetLowerLimit(const TimeValue& in_t, void *in_val);
	virtual void ClampTypedValue(const TimeValue& in_t, void *io_val) const;
	virtual void GetTypedValueAbsolute(const TimeValue& in_t, void *io_val) const;

	///////////////////////////////////////////////////////////////////////////
	float GetUpperWidth(const TimeValue& in_t) const;
	void SetUpperWidth(const TimeValue& in_t, float in_w);

	float GetLowerWidth(const TimeValue& in_t) const;
	void SetLowerWidth(const TimeValue& in_t, float in_w);

	// From Animatable
	virtual Class_ID ClassID() {return Class_ID(FLOATLIMITCTRL_CLASS_ID, 0);}		
	virtual SClass_ID SuperClassID() { return CTRL_FLOAT_CLASS_ID; }
	virtual void GetClassName(TSTR& s) {s = GetString(IDS_FLOATLIMITCTRL_CLASS_NAME);}
	virtual ParamDimension* GetParamDimension(int in_sub);

	RefTargetHandle Clone( RemapDir &remap );

	// Reference methods
	virtual RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
		PartID& partID,  RefMessage message);

	// Index and SubAnim 0 is the BaseLimitCtrl paramblock
	// Index and SubAnim 1 is the BaseLimitCtrl limited controller
	// Index and SubAnim 2 is the FloatLimitCtrl paramblock
	virtual int NumSubs() { return 1 + BaseLimitCtrl::NumSubs(); }	// The parameter block
	virtual TSTR SubAnimName(int i);				
	virtual Animatable* SubAnim(int i);
	virtual int SubNumToRefNum(int subNum);

	// Maintain the number of references here
	virtual int NumRefs() { return 1 + BaseLimitCtrl::NumRefs(); }
	virtual RefTargetHandle GetReference(int in_noRef);
	virtual void SetReference(int in_noRef, RefTargetHandle rtarg);

	virtual int	NumParamBlocks() { // return number of ParamBlocks in this instance
		return 1 + BaseLimitCtrl::NumParamBlocks(); 
	}
	virtual IParamBlock2* GetParamBlock(int i);
	virtual IParamBlock2* GetParamBlockByID(BlockID id);

	// UI methods
	virtual void EditTrackParams(
		TimeValue t,
		ParamDimensionBase *dim,
		TCHAR *pname,
		HWND hParent,
		IObjParam *ip,
		DWORD flags);

protected:
	// Parameter block
	IParamBlock2	*m_pfloatBlock;		// reference 2

	// Static members and methods
	//////////////////////////////////////////////////////////////////////////////
public:
	static ClassDesc2* GetFloatLimitCtrlDesc();

private:
	static FloatLimitCtrlClassDesc	s_floatLimitCtrlClassDesc;
	static BaseLimitCtrlParamBlockDesc s_baseLimitCtrlParamBlockDesc;
};


///////////////////////////////////////////////////////////////////////////////
//	Dialog classes
///////////////////////////////////////////////////////////////////////////////

#define FLOATLIMITDLG_CONTREF	0
#define FLOATLIMIT_DLG_CLASS_ID	0x6fc35ebb

class FloatLimitControlDlg: public ReferenceMaker, public TimeChangeCallback {
public:
	FloatLimitControlDlg(IObjParam *in_objParam = NULL, FloatLimitCtrl *in_ctrl = NULL);
	virtual ~FloatLimitControlDlg();

	static const float s_minLimit;
	static const float s_maxLimit;

	Class_ID ClassID() {return Class_ID(FLOATLIMIT_DLG_CLASS_ID, 0);}
	SClass_ID SuperClassID() {return REF_MAKER_CLASS_ID;}

	FloatLimitCtrl* GetController() { return mCtrl; }
	void Set(IObjParam *in_objParam = NULL, ParamDimensionBase *in_dim = NULL, FloatLimitCtrl *in_ctrl = NULL)
	{
		mIP = in_objParam;
		mDim = in_dim;
		ReplaceReference(0, in_ctrl);
	}
	HWND GetHWnd() { return mHWnd; }
	void SetHWnd(HWND in_hwnd) { mHWnd = in_hwnd; }

	void Invalidate();
	void Update();
	void SetupUI();
	void SpinnerChange(int id, BOOL drag);
	void SpinnerStart(int id);
	void SpinnerEnd(int id, BOOL cancel);

	void Init(HWND hParent);
	virtual HWND CreateWin(HWND in_hParent)=0;

	static INT_PTR CALLBACK FloatDlgProc
		(
			HWND	in_hWnd,
			UINT	in_message,
			WPARAM	in_wParam,
			LPARAM	in_lParam
		);

	virtual void MouseMessage(UINT message,WPARAM wParam,LPARAM lParam) {};	

	virtual void MaybeCloseWindow() {}

	void WMCommand(int id, int notify, HWND hCtrl);
	
	int NumRefs() { return 1; }
	RefTargetHandle GetReference(int i);
	void SetReference(int i, RefTargetHandle rtarg);
	RefResult NotifyRefChanged(Interval, RefTargetHandle, PartID&, RefMessage);
	BOOL CanTransferReference(int i) { return FALSE; }

	// TimeChangeCallback
	void TimeChanged(TimeValue t);

protected:
	FloatLimitCtrl *mCtrl;
	IObjParam *mIP;
	HWND mHWnd;
	BOOL mValid; 
	ISpinnerControl *mUpperLimitSpin, *mLowerLimitSpin;
	ISpinnerControl *mUpperWidthSpin, *mLowerWidthSpin;
	ParamDimensionBase *mDim;
};


class FloatLimitControlTrackDlg: public FloatLimitControlDlg {
	public:
		
		FloatLimitControlTrackDlg(IObjParam *in_objParam = NULL, FloatLimitCtrl *in_ctrl = NULL):
			FloatLimitControlDlg(in_objParam, in_ctrl) {}
					
		HWND CreateWin(HWND hParent);
		void MouseMessage(UINT message, WPARAM wParam, LPARAM lParam) {};
		void MaybeCloseWindow();
};

// LimitControlDlgManager is a singleton class
class LimitControlDlgManager
{
public:
	FloatLimitControlDlg *CreateLimitDialog();
	// Checks for an existing dialog for control in_limitCtrl
	FloatLimitControlDlg *FindDialogFromCtrl(BaseLimitCtrl* in_limitCtrl);
	// Checks for an existing dialog for control in_limitCtrl.  If none exists, creates it.
	FloatLimitControlDlg *GetDialogForCtrl(BaseLimitCtrl* in_limitCtrl);
	// Returns true iff the dialog was found and removed)
	bool RemoveDialog(FloatLimitControlDlg *in_dlg);

	static LimitControlDlgManager*  GetLimitCtrlDlgManager();

protected:
	LimitControlDlgManager();
	~LimitControlDlgManager();

private:
	LimitControlDlgManager(const LimitControlDlgManager& in_mgr);
	LimitControlDlgManager& LimitControlDlgManager::operator= (const LimitControlDlgManager& in_mgr);

	Tab<FloatLimitControlDlg*> mDialogTab;

	static LimitControlDlgManager*	s_instance;
};


#endif __LIMITCTRL__H
