/*****************************************************************************

	FILE: LimitController.cpp

	DESCRIPTION:	Implementation of the BaseLimitController 
					and derived classes

	CREATED BY: leonarni

	HISTORY: 

	Copyright (c) 2004, All Rights Reserved.
 *****************************************************************************/

#include "ctrl.h"		// Project precompiled header

#include "LimitController.h"
#include "setkeymode.h"			// ISetKey
#include <Animatable.h>
#include <3dsmaxport.h>



///////////////////////////////////////////////////////////////////////////////
// Static members and methods
///////////////////////////////////////////////////////////////////////////////
const float BaseLimitCtrl::s_lowerLimitDefaultValue = -10000.0f;
const float BaseLimitCtrl::s_upperLimitDefaultValue =  10000.0f;

const float FloatLimitCtrl::s_paddingFactor = 2.0f;

const float FloatLimitControlDlg::s_minLimit = -100000000.0f;
const float FloatLimitControlDlg::s_maxLimit =  100000000.0f;


ClassDesc2* FloatLimitCtrl::GetFloatLimitCtrlDesc() 
{ 
	return &s_floatLimitCtrlClassDesc; 
}

FloatLimitCtrlClassDesc	FloatLimitCtrl::s_floatLimitCtrlClassDesc;

BaseLimitCtrlParamBlockDesc FloatLimitCtrl::s_baseLimitCtrlParamBlockDesc(
	FloatLimitCtrl::GetFloatLimitCtrlDesc());


///////////////////////////////////////////////////////////////////////////////
// Function Publishing descriptor for Limit interface
///////////////////////////////////////////////////////////////////////////////

static FPInterfaceDesc limit_interface(
    IID_LIMIT_CONTROL, _T("limits"), 0, NULL, FP_MIXIN,
		ILimitControl::is_enabled, _T("IsEnabled"), 0, TYPE_bool, 0, 0,
		ILimitControl::set_enabled, _T("SetEnabled"), 0, TYPE_VOID, 0, 1,
			_T("enabled"), 0, TYPE_bool,
		ILimitControl::get_limited_control, _T("GetLimitedControl"), 0, TYPE_CONTROL, 0, 0,
		ILimitControl::set_limited_control, _T("SetLimitedControl"), 0, TYPE_VOID, 0, 1,
			_T("limitedCtrl"), 0, TYPE_CONTROL,
		ILimitControl::set_upper_limit, _T("SetUpperLimit"), 0, TYPE_VOID, 0, 2,
			_T("time"), 0, TYPE_TIMEVALUE,
			_T("value"), 0, TYPE_VOID + TYPE_BY_PTR,	// is this the way to declare a void* param?
		ILimitControl::set_lower_limit, _T("SetLowerLimit"), 0, TYPE_VOID, 0, 2,
			_T("time"), 0, TYPE_TIMEVALUE,
			_T("value"), 0, TYPE_VOID + TYPE_BY_PTR,	// is this the way to declare a void* param?
	end );

///////////////////////////////////////////////////////////////////////////////
// Get Descriptor method for Mixin Interface
///////////////////////////////////////////////////////////////////////////////
FPInterfaceDesc* ILimitControl::GetDesc()
{
    return &limit_interface;
}

///////////////////////////////////////////////////////////////////////////////
//	Classes and parameter blocks related to the Base Limit Controller

class BaseLimitCtrlClassDesc : public ClassDesc2 {
public:
	int 			IsPublic() { return TRUE; }
	void *			Create(BOOL loading = FALSE) { 
		return new BaseLimitCtrl(); }
	const TCHAR *	ClassName() { return GetString(IDS_BASELIMITCTRL_CLASS_NAME); }
	SClass_ID		SuperClassID() { return CTRL_USERTYPE_CLASS_ID; }
	Class_ID		ClassID() { return BASELIMITCTRL_CLASS_ID; }
	const TCHAR* 	Category() { return _T(""); }
	const TCHAR*	InternalName() { return _T("BaseLimitCtrl"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }			// returns owning module handle
};

static BaseLimitCtrlClassDesc BaseLimitCtrlDesc;
ClassDesc2* GetBaseLimitCtrlDesc() { return &BaseLimitCtrlDesc; }

BaseLimitCtrlParamBlockDesc::BaseLimitCtrlParamBlockDesc(
    ClassDesc2 *io_descriptor) :
    ParamBlockDesc2( 
		BaseLimitCtrl::baselimitctrl_params, _T("baseLimitParameters"),
		IDS_BASELIMIT_PARAMS, io_descriptor, 
		P_AUTO_CONSTRUCT, BASELIMIT_PBLOCK_REF, 
		// params
		BaseLimitCtrl::pb_enable, 	_T("enable"), 	TYPE_BOOL, 	
			P_RESET_DEFAULT,	IDS_LIMITCTRL_ENABLE,
			p_default,	TRUE, 
			end,
		end
	)
{
}

BaseLimitCtrlParamBlockDesc::~BaseLimitCtrlParamBlockDesc()
{
}

///////////////////////////////////////////////////////////////////////////////
//
//	Base Limit Controller methods
//
///////////////////////////////////////////////////////////////////////////////
BaseLimitCtrl::BaseLimitCtrl():
	m_pblock(NULL),
	m_limitedCtrl(NULL)
{
	BaseLimitCtrlDesc.MakeAutoParamBlocks(this);
}

BaseLimitCtrl::~BaseLimitCtrl() 
{
	m_pblock = NULL;
	m_limitedCtrl = NULL;
}

bool BaseLimitCtrl::IsEnabled() const
{
	DbgAssert(m_pblock);
	return m_pblock->GetInt(pb_enable) != 0;
}

void BaseLimitCtrl::SetEnabled(bool in_enabled)
{
	DbgAssert(m_pblock);
	if (m_pblock)
		m_pblock->SetValue(pb_enable, 0, in_enabled);
}

Control* BaseLimitCtrl::GetLimitedControl() const
{
	return m_limitedCtrl;
}

void BaseLimitCtrl::SetLimitedControl(Control *in_limitedCtrl)
{
	ReplaceReference(BASELIMIT_LIMITEDCTRL_REF, in_limitedCtrl, true);
}

ParamDimension* BaseLimitCtrl::GetParamDimension(int in_sub)
{
	ParamDimension* dim = defaultDim;
	if (in_sub == BASELIMIT_LIMITEDCTRL_SUBANIM) {
		// The dimension of our limited controller should be whatever our  
		// own dimension is.  Finding that out is tricky.  We do it by  
		// asking whoever it is we are animating.
		ReferenceMaker* refMaker = NULL;
		DependentIterator di(this);
		while ((refMaker = di.Next()) != NULL) {
			// Enumerate its subAnims to find out which one WE are
			int nbSubs = refMaker->NumSubs();
			for (int i = 0; i < nbSubs; i++) {
				// If any is ourselves, bingo!
				Animatable* anim = refMaker->SubAnim(i);
				if (anim == (Animatable*)this) {
					// Ask IT what our dimension is
					dim = refMaker->GetParamDimension(i);
					return dim;
				}
			}
		}
	}

	return dim;
}

Control* BaseLimitCtrl::GetDefaultControlForDataType() 
{
	switch(SuperClassID()) {
		case CTRL_FLOAT_CLASS_ID:
		case CTRL_SHORT_CLASS_ID:
		case CTRL_INTEGER_CLASS_ID:
			return NewDefaultFloatController();
		case CTRL_POINT3_CLASS_ID:
			return NewDefaultPoint3Controller();
		case CTRL_POSITION_CLASS_ID:
			return NewDefaultPositionController();
		case CTRL_ROTATION_CLASS_ID:
			return NewDefaultRotationController();
		case CTRL_SCALE_CLASS_ID:
			return NewDefaultScaleController();
		case CTRL_MATRIX3_CLASS_ID:
			return NewDefaultMatrix3Controller();
		case CTRL_MASTERPOINT_CLASS_ID:
			return NewDefaultMasterPointController();
		case CTRL_POINT4_CLASS_ID:
			return NewDefaultPoint4Controller();
		case CTRL_COLOR24_CLASS_ID:
			return NewDefaultColorController();
		case CTRL_FRGBA_CLASS_ID:
			return NewDefaultFRGBAController();
		case CTRL_MORPH_CLASS_ID:
		default:
			return NULL;
	}
}

void *BaseLimitCtrl::CreateTempValue()
{
	switch(SuperClassID()) {
		case CTRL_FLOAT_CLASS_ID:
		case CTRL_SHORT_CLASS_ID:
		case CTRL_INTEGER_CLASS_ID:
			return new float;
		case CTRL_POINT3_CLASS_ID:
		case CTRL_POSITION_CLASS_ID:
			return new Point3;
		case CTRL_ROTATION_CLASS_ID:
			return new Quat;
		case CTRL_SCALE_CLASS_ID:
			return new ScaleValue;
		default:
			return NULL;
	}
}

void BaseLimitCtrl::DeleteTempValue(void *val)
{
	switch(SuperClassID()) {
		case CTRL_FLOAT_CLASS_ID:
		case CTRL_SHORT_CLASS_ID:
		case CTRL_INTEGER_CLASS_ID:
			delete (float *)val;
			break;
		case CTRL_POINT3_CLASS_ID:
		case CTRL_POSITION_CLASS_ID:
			delete (Point3 *)val;
			break;
		case CTRL_ROTATION_CLASS_ID:
			delete (Quat *)val;
			break;
		case CTRL_SCALE_CLASS_ID:
			delete (ScaleValue *)val;
			break;
	}
}

BOOL BaseLimitCtrl::AssignController(Animatable *in_control, int in_subAnim)
{
	if(GetLocked()==false)
	{
		if (in_subAnim == BASELIMIT_LIMITEDCTRL_SUBANIM) {
			Animatable *cont = SubAnim(in_subAnim);
			if(cont&&GetLockedTrackInterface(cont)&&GetLockedTrackInterface(cont)->GetLocked()==true)//the control is locked, don't assign over it.
				return FALSE;
			// Accept and replace the previous limited controller with the new one
			ReplaceReference(SubNumToRefNum(in_subAnim), (RefTargetHandle)in_control);
			NotifyDependents(FOREVER, 0, REFMSG_CHANGE);
			NotifyDependents(FOREVER, 0, REFMSG_SUBANIM_STRUCTURE_CHANGED);
			return TRUE;
		}
		return TRUE;
	}
	return FALSE;
}

void BaseLimitCtrl::Copy(Control *in_from)
{
	if(GetLocked()==false)
	{
		// This is called from Assign Controller, with the former controller as parameter
		if (in_from) {
			if (in_from->ClassID() != Class_ID(DUMMYCHANNEL_CLASS_ID, 0) &&
				in_from->ClassID() != Class_ID(DUMMY_CONTROL_CLASS_ID, 0)) {
				// Use the former control as the limited control
				SetLimitedControl(static_cast<Control*>(in_from));
			}
			else
				SetLimitedControl(NULL);
		}
	}
}


void BaseLimitCtrl::SetValue(TimeValue in_t, void *in_val, int in_commit, GetSetMethod in_method)
{
	if(GetLocked()==false)
	{
		if (IsEnabled()) {
			if (in_method == CTRL_RELATIVE)
				GetTypedValueAbsolute(in_t, in_val);
			ClampTypedValue(in_t, in_val);
		}

		// Check if we have a controller assigned.  If so, SetValue on it, if it's not locked!
		Control *limitedCtrl = GetLimitedControl();
		if (limitedCtrl) {
			limitedCtrl->SetValue(in_t, in_val, in_commit, IsEnabled()? CTRL_ABSOLUTE: in_method);
		}
		else {
			// If in AutoKey or SetKey mode, ParamBlock::SetValue() and ParamBlock2::SetValue()
			// create a default controller (an fcurve) and then set key on that   
			// controller.  So if there is no limited controller, I'll do the same:
			// create one, and set key on it.
			if (GetSetKeyMode() || (Animating() && GetCOREInterface10()->GetAutoKeyDefaultKeyOn()&& in_t!=0 &&!(in_t==GetTicksPerFrame()&&GetCOREInterface10()->GetAutoKeyDefaultKeyTime()==GetTicksPerFrame()))) {
				// Create the proper default controller according to type
				Control *newLimitedCtrl = GetDefaultControlForDataType();
				if (newLimitedCtrl) {
					void* ptrVal = CreateTempValue();
					GetStaticValue(ptrVal, in_method);

					SetLimitedControl(newLimitedCtrl);
					if (GetSetKeyMode()) {
						ISetKey *skey = (ISetKey *)newLimitedCtrl->GetInterface(I_SETKEYCONTROL);
						if (skey)
							skey->SetCurrentValue(ptrVal);
						newLimitedCtrl->SetValue(in_t, in_val);
					}
					else {
						newLimitedCtrl->SetValue(TimeValue(GetCOREInterface10()->GetAutoKeyDefaultKeyTime()), ptrVal);
						newLimitedCtrl->SetValue(in_t, in_val);
					}
					DeleteTempValue(ptrVal);
				}
			}

			// Set the limit controller static value
			SetStaticValue(in_val, CTRL_ABSOLUTE);
		}
	}
}

void BaseLimitCtrl::GetUnlimitedValue
	(
		const TimeValue&	in_t, 
		void*				in_val, 
		Interval&			out_valid, 
		GetSetMethod		in_method	// = CTRL_ABSOLUTE
	) const
{
	// Check if we have a controller assigned.  If so, GetValue on it
	Control *limitedCtrl = GetLimitedControl();
	if (limitedCtrl) {
		limitedCtrl->GetValue(in_t, in_val, out_valid, in_method);
	}
	else {
		// Get the limit controller static value
		out_valid = FOREVER;
		GetStaticValue(in_val, in_method);
	}
}

void BaseLimitCtrl::GetValue
	(
		TimeValue	in_t, 
		void*		in_val, 
		Interval&	out_valid, 
		GetSetMethod in_method	// = CTRL_ABSOLUTE
	)
{
	GetUnlimitedValue(in_t, in_val, out_valid, in_method);
	if (IsEnabled())
		ClampTypedValue(in_t, in_val);
}	

RefTargetHandle BaseLimitCtrl::GetReference(int in_noRef)
{ 
	if (in_noRef == BASELIMIT_PBLOCK_REF)
		return m_pblock;
	else
		return GetLimitedControl();
}

void BaseLimitCtrl::SetReference(int in_noRef, RefTargetHandle rtarg)
{
	if (in_noRef == BASELIMIT_PBLOCK_REF)
		m_pblock = (IParamBlock2*)rtarg;
	else
		m_limitedCtrl = (Control*)rtarg;
}

Interval BaseLimitCtrl::GetTimeRange(DWORD flags)
{
	Control *limitedCtrl = GetLimitedControl();
	if (limitedCtrl)
		return limitedCtrl->GetTimeRange(flags);
	else
		return NEVER;
}

void BaseLimitCtrl::AddNewKey(TimeValue t, DWORD flags)
{
	if(GetLocked()==false)
	{
		Control *limitedCtrl = GetLimitedControl();
		if (limitedCtrl)
			return limitedCtrl->AddNewKey(t, flags);
	}
}

void BaseLimitCtrl::CloneSelectedKeys(BOOL offset)
{
	if(GetLocked()==false)
	{
		Control *limitedCtrl = GetLimitedControl();
		if (limitedCtrl)
			return limitedCtrl->CloneSelectedKeys(offset);
	}
}

void BaseLimitCtrl::DeleteKeys(DWORD flags)
{
	if(GetLocked()==false)
	{
		Control *limitedCtrl = GetLimitedControl();
		if (limitedCtrl)
			return limitedCtrl->DeleteKeys(flags);
	}
}

void BaseLimitCtrl::SelectKeys(TrackHitTab& sel, DWORD flags)
{
	Control *limitedCtrl = GetLimitedControl();
	if (limitedCtrl)
		return limitedCtrl->SelectKeys(sel, flags);
}

BOOL BaseLimitCtrl::IsKeySelected(int index)
{
	Control *limitedCtrl = GetLimitedControl();
	if (limitedCtrl)
		return limitedCtrl->IsKeySelected(index);
	else
		return FALSE;
}

void BaseLimitCtrl::CopyKeysFromTime(TimeValue src, TimeValue dst, DWORD flags)
{
	if(GetLocked()==false)
	{
		Control *limitedCtrl = GetLimitedControl();
		if (limitedCtrl)
			return limitedCtrl->CopyKeysFromTime(src, dst, flags);
	}
}

void BaseLimitCtrl::DeleteKeyAtTime(TimeValue t)
{
	if(GetLocked()==false)
	{
		Control *limitedCtrl = GetLimitedControl();
		if (limitedCtrl)
			return limitedCtrl->DeleteKeyAtTime(t);
	}
}

BOOL BaseLimitCtrl::IsKeyAtTime(TimeValue t, DWORD flags)
{
	Control *limitedCtrl = GetLimitedControl();
	if (limitedCtrl)
		return limitedCtrl->IsKeyAtTime(t, flags);
	else
		return FALSE;
}

BOOL BaseLimitCtrl::GetNextKeyTime(TimeValue t, DWORD flags, TimeValue &nt)
{
	Control *limitedCtrl = GetLimitedControl();
	if (limitedCtrl)
		return limitedCtrl->GetNextKeyTime(t, flags, nt);
	else
		return FALSE;
}

int BaseLimitCtrl::GetKeyTimes(Tab<TimeValue> &times, Interval range, DWORD flags)
{
	Control *limitedCtrl = GetLimitedControl();
	if (limitedCtrl)
		return limitedCtrl->GetKeyTimes(times, range, flags);
	else
		return 0;
}

int BaseLimitCtrl::GetKeySelState(BitArray &sel, Interval range, DWORD flags)
{
	Control *limitedCtrl = GetLimitedControl();
	if (limitedCtrl)
		return limitedCtrl->GetKeySelState(sel, range, flags);
	else
		return 0;
}

void BaseLimitCtrl::CommitValue(TimeValue t)
{
	Control *limitedCtrl = GetLimitedControl();
	if (limitedCtrl)
		return limitedCtrl->CommitValue(t);
}

void BaseLimitCtrl::RestoreValue(TimeValue t)
{
	Control *limitedCtrl = GetLimitedControl();
	if (limitedCtrl)
		return limitedCtrl->RestoreValue(t);
}

void BaseLimitCtrl::EnumIKParams(IKEnumCallback &callback)
{
	Control *limitedCtrl = GetLimitedControl();
	if (limitedCtrl)
		return limitedCtrl->EnumIKParams(callback);
}

BOOL BaseLimitCtrl::CompDeriv(TimeValue t, Matrix3& ptm, IKDeriv& derivs, DWORD flags)
{
	Control *limitedCtrl = GetLimitedControl();
	if (limitedCtrl)
		return limitedCtrl->CompDeriv(t, ptm, derivs, flags);
	else
		return FALSE;
}

void BaseLimitCtrl::MouseCycleCompleted(TimeValue t)
{
	Control *limitedCtrl = GetLimitedControl();
	if (limitedCtrl)
		return limitedCtrl->MouseCycleCompleted(t);
}

BOOL BaseLimitCtrl::InheritsParentTransform()
{
	Control *limitedCtrl = GetLimitedControl();
	if (limitedCtrl)
		return limitedCtrl->InheritsParentTransform();
	else
		return FALSE;
}

void BaseLimitCtrl::NotifyForeground(TimeValue t)
{
	Control *limitedCtrl = GetLimitedControl();
	if (limitedCtrl)
		return limitedCtrl->NotifyForeground(t);
}

#define LOCK_CHUNK		0x2535  //the lock value
IOResult BaseLimitCtrl::Save(ISave *isave)
{
	Control::Save(isave);

	// note: if you add chunks, it must follow the LOCK_CHUNK chunk due to Renoir error in 
	// placement of Control::Save(isave);
	ULONG nb;
	int on = (mLocked==true) ? 1 :0;
	isave->BeginChunk(LOCK_CHUNK);
	isave->Write(&on,sizeof(on),&nb);	
	isave->EndChunk();	

	return IO_OK;
}

IOResult BaseLimitCtrl::Load(ILoad *iload)
{
	ULONG nb;
	IOResult res;

	res = Control::Load(iload);
	if (res!=IO_OK)  return res;

	// We can't do the standard 'while' loop of opening chunks and checking ID
	// since that will eat the Control ORT chunks that were saved improperly in Renoir
	USHORT next = iload->PeekNextChunkID();
	if (next == LOCK_CHUNK) 
	{
		iload->OpenChunk();
		int on;
		res=iload->Read(&on,sizeof(on),&nb);
		if(on)
			mLocked = true;
		else
			mLocked = false;
		iload->CloseChunk();
		if (res!=IO_OK)  return res;
	}

	// Only do anything if this is the control base classes chunk
	next = iload->PeekNextChunkID();
	if (next == CONTROLBASE_CHUNK) 
		res = Control::Load(iload);  // handle improper Renoir Save order
	return res;	
}



///////////////////////////////////////////////////////////////////////////////
//	Classes and parameter blocks related to the Float Limit Controller

void * FloatLimitCtrlClassDesc::Create(BOOL loading) 
{
	AddInterface(&limit_interface);
	return new FloatLimitCtrl(); 
}

static ParamBlockDesc2 floatlimitctrl_param_blk ( FloatLimitCtrl::float_limitctrl_params, _T("floatLimitParameters"),  
										IDS_FLOATLIMIT_PARAMS, FloatLimitCtrl::GetFloatLimitCtrlDesc(), 
	P_AUTO_CONSTRUCT, FLOATLIMIT_PBLOCK_REF, 
	// params
	FloatLimitCtrl::pb_upper_limit, 	_T("upper_limit"), 	TYPE_FLOAT, 	
		P_ANIMATABLE + P_RESET_DEFAULT,	IDS_FLOATLIMITCTRL_UPPERLIMITUI,
		p_default,	BaseLimitCtrl::s_upperLimitDefaultValue, 
		p_ui, 		TYPE_SPINNER, EDITTYPE_FLOAT, IDC_LIMITCTRL_UPPERLIMIT, IDC_LIMITCTRL_UPPERLIMITSPIN, 1.0f, 
		end,

	FloatLimitCtrl::pb_lower_limit, 	_T("lower_limit"), 	TYPE_FLOAT, 	
		P_ANIMATABLE + P_RESET_DEFAULT,	IDS_FLOATLIMITCTRL_LOWERLIMITUI,
		p_ui, 		TYPE_SPINNER, EDITTYPE_FLOAT, IDC_LIMITCTRL_LOWERLIMIT, IDC_LIMITCTRL_LOWERLIMITSPIN, 1.0f, 
		p_default,	BaseLimitCtrl::s_lowerLimitDefaultValue, 
		end,

	FloatLimitCtrl::pb_upper_limit_enabled, 	_T("upper_limit_enabled"), 	TYPE_BOOL, 	
		P_RESET_DEFAULT,	IDS_FLOATLIMITCTRL_UPPERLIMITENABLED,
		p_default,	TRUE, 
		end,

	FloatLimitCtrl::pb_lower_limit_enabled, 	_T("lower_limit_enabled"), 	TYPE_BOOL, 	
		P_RESET_DEFAULT,	IDS_FLOATLIMITCTRL_LOWERLIMITENABLED,
		p_default,	TRUE, 
		end,

	FloatLimitCtrl::pb_static_value, 	_T("static_value"), 	TYPE_FLOAT, 	
		P_RESET_DEFAULT,	IDS_FLOATLIMITCTRL_STATICVALUE,
		p_default,	0.0f, 
		end,

	FloatLimitCtrl::pb_upper_smoothing, 	_T("upper_smoothing"), 	TYPE_FLOAT, 	
		P_ANIMATABLE + P_RESET_DEFAULT,	IDS_FLOATLIMITCTRL_UPPERLIMIT_SMOOTHING,
		p_default,	0.0f, 
		p_range,	0.0f,	99999999.9f,
		p_ui, 		TYPE_SPINNER, EDITTYPE_FLOAT, IDC_LIMITCTRL_UPPERLIMIT_W, IDC_LIMITCTRL_UPPERLIMIT_W_SPIN, 1.0f, 
		end,

	FloatLimitCtrl::pb_lower_smoothing, 	_T("lower_smoothing"), 	TYPE_FLOAT, 	
		P_ANIMATABLE + P_RESET_DEFAULT,	IDS_FLOATLIMITCTRL_LOWERLIMIT_SMOOTHING,
		p_default,	0.0f, 
		p_range,	0.0f,	99999999.9f,
		p_ui, 		TYPE_SPINNER, EDITTYPE_FLOAT, IDC_LIMITCTRL_LOWERLIMIT_W, IDC_LIMITCTRL_LOWERLIMIT_W_SPIN, 0.1f, 
		end,

	end

	);

///////////////////////////////////////////////////////////////////////////////
//
//	Float Limit Controller
//
///////////////////////////////////////////////////////////////////////////////
FloatLimitCtrl::FloatLimitCtrl():
	m_pfloatBlock(NULL)
{
	FloatLimitCtrl::GetFloatLimitCtrlDesc()->MakeAutoParamBlocks(this);
}

FloatLimitCtrl::~FloatLimitCtrl() 
{
}

///////////////////////////////////////////////////////////////////////////////
//	Accessor methods
void FloatLimitCtrl::SetStaticValue(void *in_val, GetSetMethod in_method)
{
	DbgAssert(m_pfloatBlock);
	float val = *(float *)in_val;
	m_pfloatBlock->SetValue(pb_static_value, 0, val);
}

void FloatLimitCtrl::GetStaticValue(void *in_val, GetSetMethod in_method) const
{
	DbgAssert(m_pfloatBlock);
	float ctrlVal = 0.0f;
	if (m_pfloatBlock)
		ctrlVal = m_pfloatBlock->GetFloat(pb_static_value);
	if (in_method == CTRL_ABSOLUTE)
		*(float *)in_val = ctrlVal;
	else
		*(float *)in_val += ctrlVal;
}

void FloatLimitCtrl::SetUpperLimit(const TimeValue& in_t, void *in_val)
{
	if (in_val)
		SetUpperLimit(in_t, *static_cast<float *>(in_val));
}

void FloatLimitCtrl::SetLowerLimit(const TimeValue& in_t, void *in_val)
{
	if (in_val)
		SetLowerLimit(in_t, *static_cast<float *>(in_val));
}

void FloatLimitCtrl::GetTypedValueAbsolute
	(
		const TimeValue&	in_t, 
		void*				in_val
	) const
{
	Interval dummy;
	float currVal = 0.0f;
	GetUnlimitedValue(in_t, &currVal, dummy, CTRL_ABSOLUTE);
	ClampTypedValue(in_t, &currVal);
	*static_cast<float*>(in_val) += currVal;
}

void FloatLimitCtrl::ClampTypedValue(const TimeValue& in_t, void *io_val) const
{
	// The original ClampTypedValue's job (before the introduction of limit
	// smoothing) was pretty simple: if the value was under the lower limit
	// and the lower limit was active, clamp to the lower limit; if the value
	// was above the upper limit, clamp to the upper limit; else do nothing.
	// Smooth limits introduced intermediate zones in which the limits
	// progressively take effect.  See the spec document for more details
	float* val = (float *)io_val;
	float upper = GetUpperLimit(in_t);
	float lower = GetLowerLimit(in_t);
	float w = GetUpperWidth(in_t);
	float d = s_paddingFactor*w;
	float low_w = GetLowerWidth(in_t);
	float low_d = s_paddingFactor*low_w;
	if (IsUpperLimitActive() && *val > upper + d)
		*val = upper;
	else if (IsLowerLimitActive() && *val < lower - low_d)
		*val = lower;
	else {
		if (d > 0.0f) {
			if (IsUpperLimitActive() && *val > upper - w) {
				float newX = *val - upper - d;
				float fact = d + w;
				*val = upper + (d - w)*newX*newX*newX/(fact*fact*fact) + 
					(d - 2.0f*w)*newX*newX/(fact*fact);
			}
		}
		if (low_d > 0.0f) {
			if (IsLowerLimitActive() && *val < lower + low_w) {
				float newX = *val - (lower - low_d);
				float fact = low_d + low_w;
				*val = (low_d - low_w)*newX*newX*newX/(fact*fact*fact) +
					(2.0f*low_w - low_d)*newX*newX/(fact*fact) + lower;
			}
		}
	}
}

float FloatLimitCtrl::GetUpperWidth(const TimeValue& in_t) const
{
	DbgAssert(m_pfloatBlock);
	return m_pfloatBlock->GetFloat(pb_upper_smoothing, in_t);
}

void FloatLimitCtrl::SetUpperWidth(const TimeValue& in_t, float in_w)
{
	DbgAssert(m_pfloatBlock);
	if (IsUpperLimitActive() && IsLowerLimitActive()) {
		float lower = GetLowerLimit(in_t);
		float upper = GetUpperLimit(in_t);
		float lowerWidth = GetLowerWidth(in_t);
		if (lowerWidth + in_w > upper - lower)
			in_w = max(0, upper - lower - lowerWidth);
	}
	m_pfloatBlock->SetValue(pb_upper_smoothing, in_t, in_w);
}

float FloatLimitCtrl::GetLowerWidth(const TimeValue& in_t) const
{
	DbgAssert(m_pfloatBlock);
	return m_pfloatBlock->GetFloat(pb_lower_smoothing, in_t);
}

void FloatLimitCtrl::SetLowerWidth(const TimeValue& in_t, float in_w)
{
	DbgAssert(m_pfloatBlock);
	if (IsUpperLimitActive() && IsLowerLimitActive()) {
		float lower = GetLowerLimit(in_t);
		float upper = GetUpperLimit(in_t);
		float upperWidth = GetUpperWidth(in_t);
		if (upperWidth + in_w > upper - lower)
			in_w = max(0, upper - lower - upperWidth);
	}
	m_pfloatBlock->SetValue(pb_lower_smoothing, in_t, in_w);
}

float FloatLimitCtrl::GetUpperLimit(const TimeValue& in_t) const
{
	DbgAssert(m_pfloatBlock);
	return m_pfloatBlock->GetFloat(pb_upper_limit, in_t);
}

void FloatLimitCtrl::SetUpperLimit(const TimeValue& in_t, float in_upperLimit)
{
	DbgAssert(m_pfloatBlock);
	m_pfloatBlock->SetValue(pb_upper_limit, in_t, in_upperLimit);
	float lowerLimit = GetLowerLimit(in_t);
	if (lowerLimit > in_upperLimit)
		SetLowerLimit(in_t, in_upperLimit);
	// Also check for padding overlap.
	if (IsUpperLimitActive() && IsLowerLimitActive()) {
		float range = GetUpperLimit(in_t) - GetLowerLimit(in_t);
		float lowPadding = GetLowerWidth(in_t);
		float upPadding = GetUpperWidth(in_t);
		// In case of overlap, modify the upper limit padding
		if (upPadding + lowPadding > range) {
			if (upPadding > range) {
				m_pfloatBlock->SetValue(pb_upper_smoothing, in_t, range);
				upPadding = range;
			}
			SetUpperWidth(in_t, range - upPadding);
		}
	}
}

bool FloatLimitCtrl::IsUpperLimitActive() const
{
	DbgAssert(m_pfloatBlock);
	return m_pfloatBlock->GetInt(pb_upper_limit_enabled) != 0;
}

void FloatLimitCtrl::SetUpperLimitActive(const TimeValue& in_t, bool in_active)
{
	DbgAssert(m_pfloatBlock);
	m_pfloatBlock->SetValue(pb_upper_limit_enabled, 0, in_active);
	if (in_active && IsLowerLimitActive()) {
		// If activating the upper limit, make sure the paddings 
		// don't overlap - priority given to the active padding
		float range = GetUpperLimit(in_t) - GetLowerLimit(in_t);
		float lowPadding = GetLowerWidth(in_t);
		float upPadding = GetUpperWidth(in_t);
		if (upPadding + lowPadding > range) {
			if (lowPadding > range) {
				m_pfloatBlock->SetValue(pb_lower_smoothing, in_t, range);
				lowPadding = range;
			}
			SetUpperWidth(in_t, range - lowPadding);
		}
	}
}

float FloatLimitCtrl::GetLowerLimit(const TimeValue& in_t) const
{
	DbgAssert(m_pfloatBlock);
	return m_pfloatBlock->GetFloat(pb_lower_limit, in_t);
}

void FloatLimitCtrl::SetLowerLimit(const TimeValue& in_t, float in_lowerLimit)
{
	DbgAssert(m_pfloatBlock);
	m_pfloatBlock->SetValue(pb_lower_limit, in_t, in_lowerLimit);
	float upperLimit = GetUpperLimit(in_t);
	if (upperLimit < in_lowerLimit)
		SetUpperLimit(in_t, in_lowerLimit);
	// Also check for padding overlap.
	if (IsUpperLimitActive() && IsLowerLimitActive()) {
		float range = GetUpperLimit(in_t) - GetLowerLimit(in_t);
		float lowPadding = GetLowerWidth(in_t);
		float upPadding = GetUpperWidth(in_t);
		// In case of overlap, modify the lower limit padding
		if (upPadding + lowPadding > range) {
			if (lowPadding > range) {
				m_pfloatBlock->SetValue(pb_lower_smoothing, in_t, range);
				lowPadding = range;
			}
			SetLowerWidth(in_t, range - lowPadding);
		}
	}
}

bool FloatLimitCtrl::IsLowerLimitActive() const
{
	DbgAssert(m_pfloatBlock);
	return m_pfloatBlock->GetInt(pb_lower_limit_enabled) != 0;
}

void FloatLimitCtrl::SetLowerLimitActive(const TimeValue& in_t, bool in_active)
{
	DbgAssert(m_pfloatBlock);
	m_pfloatBlock->SetValue(pb_lower_limit_enabled, 0, in_active);
	if (in_active && IsUpperLimitActive()) {
		// If activating the lower limit, make sure the paddings 
		// don't overlap - priority given to the active padding
		float range = GetUpperLimit(in_t) - GetLowerLimit(in_t);
		float lowPadding = GetLowerWidth(in_t);
		float upPadding = GetUpperWidth(in_t);
		if (upPadding + lowPadding > range) {
			if (upPadding > range) {
				m_pfloatBlock->SetValue(pb_upper_smoothing, in_t, range);
				upPadding = range;
			}
			SetLowerWidth(in_t, range - upPadding);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
//	Animatable methods
TSTR FloatLimitCtrl::SubAnimName(int i) 
{ 
	if (i < BaseLimitCtrl::NumSubs()) 
		return BaseLimitCtrl::SubAnimName(i); 
	else 
		return GetString(IDS_FLOATLIMIT_PARAMS); 
}				

Animatable* FloatLimitCtrl::SubAnim(int i) 
{ 
	if (i < BaseLimitCtrl::NumSubs()) 
		return BaseLimitCtrl::SubAnim(i); 
	else 
		return m_pfloatBlock; 
}

int FloatLimitCtrl::SubNumToRefNum(int subNum) 
{
	if (subNum < BaseLimitCtrl::NumSubs()) 
		return BaseLimitCtrl::SubNumToRefNum(subNum); 
	else 
		return FLOATLIMIT_PBLOCK_REF; 
}

RefTargetHandle FloatLimitCtrl::GetReference(int in_noRef)
{
	if (in_noRef < BaseLimitCtrl::NumRefs()) 
		return BaseLimitCtrl::GetReference(in_noRef); 
	else 
		return m_pfloatBlock; 
}

ParamDimension* FloatLimitCtrl::GetParamDimension(int in_sub)
{
	// Upper and Lower limits dimension is the same as the limited controller
	// dimension.  See BaseLimitController::GetParamDimension() for details
	// @todo: validate index
	return BaseLimitCtrl::GetParamDimension(BASELIMIT_LIMITEDCTRL_SUBANIM);
}

void FloatLimitCtrl::SetReference(int in_noRef, RefTargetHandle rtarg)
{
	if (in_noRef < BaseLimitCtrl::NumRefs()) 
		return BaseLimitCtrl::SetReference(in_noRef, rtarg); 
	else 
		m_pfloatBlock = (IParamBlock2*)rtarg; 
}

IParamBlock2* FloatLimitCtrl::GetParamBlock(int i) 
{ 
	if (i < BaseLimitCtrl::NumParamBlocks()) 
		return BaseLimitCtrl::GetParamBlock(i);
	else 
		return m_pfloatBlock; 
}

IParamBlock2* FloatLimitCtrl::GetParamBlockByID(BlockID id) 
{
	return (m_pfloatBlock->ID() == id)? 
		m_pfloatBlock: BaseLimitCtrl::GetParamBlockByID(id); 
}

RefTargetHandle FloatLimitCtrl::Clone(RemapDir& remap)
{
	// Make a new float limit controller and give it our param values.
	FloatLimitCtrl *newCtrl = new FloatLimitCtrl;

	if (newCtrl) {
		// Clone limited controller
		newCtrl->ReplaceReference(BASELIMIT_LIMITEDCTRL_REF, 
			remap.CloneRef(GetLimitedControl()));

		// Clone parameter blocks
		newCtrl->ReplaceReference(BASELIMIT_PBLOCK_REF, remap.CloneRef(m_pblock));
		newCtrl->ReplaceReference(FLOATLIMIT_PBLOCK_REF, remap.CloneRef(m_pfloatBlock));
		newCtrl->mLocked = mLocked;
		CloneControl(newCtrl, remap);
		BaseClone(this, newCtrl, remap);
	}
	return newCtrl;
}

void FloatLimitCtrl::EditTrackParams(
		TimeValue t,
		ParamDimensionBase *dim,
		TCHAR *pname,
		HWND hParent,
		IObjParam *ip,
		DWORD flags)
{
	if(GetLocked()==false)
	{
		FloatLimitControlDlg *dlg = 
			LimitControlDlgManager::GetLimitCtrlDlgManager()->FindDialogFromCtrl(this);
		if (dlg) {
			SetActiveWindow(dlg->GetHWnd());
			return;
		}

		dlg = LimitControlDlgManager::GetLimitCtrlDlgManager()->CreateLimitDialog();
		dlg->Set(ip, dim, this);
		dlg->Init(hParent);
	}
}

RefResult FloatLimitCtrl::NotifyRefChanged
	(
		Interval changeInt, 
		RefTargetHandle hTarget, 
		PartID& partID, 
		RefMessage message 
	)
{
	switch (message) {
		case REFMSG_WANT_SHOWPARAMLEVEL:	// show the paramblock2 entry in trackview
			if (hTarget == (RefTargetHandle)m_pfloatBlock) {
				BOOL *pb = (BOOL *)(partID);
				*pb = TRUE;
				return REF_HALT;
			}
			break;
		case REFMSG_CHANGE:
			changeInt.SetEmpty();
			NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			break;
		default:
			break;
	}

	return REF_SUCCEED;
}

///////////////////////////////////////////////////////////////////////////////
//
//	Dialog classes and related code
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//	FloatLimitControlDlg

FloatLimitControlDlg::FloatLimitControlDlg(IObjParam *in_objParam, FloatLimitCtrl *in_ctrl):
	mIP(in_objParam),
	mValid(FALSE),
	mHWnd(NULL),
	mCtrl(NULL),
	mUpperLimitSpin(NULL), 
	mLowerLimitSpin(NULL),
	mUpperWidthSpin(NULL), 
	mLowerWidthSpin(NULL),
	mDim(NULL)
{
	if (in_ctrl != NULL) {
		theHold.Suspend();
		ReplaceReference(BASELIMIT_LIMITEDCTRL_REF, in_ctrl);
		theHold.Resume();
	}
	
	GetCOREInterface()->RegisterTimeChangeCallback(this);
}

FloatLimitControlDlg::~FloatLimitControlDlg()
{
	ReleaseISpinner(mUpperLimitSpin);
	ReleaseISpinner(mLowerLimitSpin);
	ReleaseISpinner(mUpperWidthSpin);
	ReleaseISpinner(mLowerWidthSpin);

	LimitControlDlgManager::GetLimitCtrlDlgManager()->RemoveDialog(this);

	theHold.Suspend();
	DeleteAllRefsFromMe();
	theHold.Resume();
	GetCOREInterface()->UnRegisterTimeChangeCallback(this);
}

RefTargetHandle FloatLimitControlDlg::GetReference(int i) 
{
	switch (i) {
		case FLOATLIMITDLG_CONTREF:
			return mCtrl;
		default:
			return NULL;
	}
}

void FloatLimitControlDlg::SetReference(int i, RefTargetHandle rtarg) 
{
	switch (i) {
		case FLOATLIMITDLG_CONTREF:
			mCtrl = (FloatLimitCtrl*)rtarg;
			break;
		}
}

RefResult FloatLimitControlDlg::NotifyRefChanged(
		Interval iv, 
		RefTargetHandle hTarg, 
		PartID& partID, 
		RefMessage msg) 
{
	switch (msg) {
		case REFMSG_CHANGE:
			Invalidate();
			break;
	}
	return REF_SUCCEED;
}

void FloatLimitControlDlg::Invalidate()
{
	mValid = FALSE;
	Rect rect(IPoint2(0, 0), IPoint2(10, 10));
	InvalidateRect(mHWnd, &rect, FALSE);
}

class CheckForNonLimitDlg : public DependentEnumProc {
	public:		
		BOOL non;
		ReferenceMaker *me;
		CheckForNonLimitDlg(ReferenceMaker *m) {non = FALSE; me = m;}
		int proc(ReferenceMaker *rmaker) {
			if (rmaker == me) 
				return DEP_ENUM_CONTINUE;
			if (rmaker->SuperClassID() != REF_MAKER_CLASS_ID &&
				rmaker->ClassID() != Class_ID(FLOATLIMIT_DLG_CLASS_ID, 0)) {
				non = TRUE;
				return DEP_ENUM_HALT;
			}
			return DEP_ENUM_SKIP; // just look at direct dependents
		}
};

void FloatLimitControlTrackDlg::MaybeCloseWindow()
{
	CheckForNonLimitDlg check(mCtrl);
	mCtrl->DoEnumDependents(&check);
	if (!check.non) 
		PostMessage(mHWnd, WM_CLOSE, 0, 0);
}

void FloatLimitControlDlg::Update()
{
	if (mCtrl) {
		TimeValue t = GetCOREInterface()->GetTime();
		IParamBlock2* floatPBlock = mCtrl->GetParamBlockByID(FloatLimitCtrl::float_limitctrl_params);
		if (mUpperLimitSpin) {
			mUpperLimitSpin->SetValue(
				mDim->Convert(mCtrl->GetUpperLimit(t)), FALSE);
			if (floatPBlock)
				mUpperLimitSpin->SetKeyBrackets(floatPBlock->KeyFrameAtTime(
					floatPBlock->IDtoIndex(FloatLimitCtrl::pb_upper_limit), t));
		}
		if (mLowerLimitSpin) {
			mLowerLimitSpin->SetValue(
				mDim->Convert(mCtrl->GetLowerLimit(t)), FALSE);
			if (floatPBlock)
				mLowerLimitSpin->SetKeyBrackets(floatPBlock->KeyFrameAtTime(
					floatPBlock->IDtoIndex(FloatLimitCtrl::pb_lower_limit), t));
		}
		if (mUpperWidthSpin) {
			mUpperWidthSpin->SetValue(
				mDim->Convert(mCtrl->GetUpperWidth(t)), FALSE);
			if (floatPBlock)
				mUpperWidthSpin->SetKeyBrackets(floatPBlock->KeyFrameAtTime(
					floatPBlock->IDtoIndex(FloatLimitCtrl::pb_upper_smoothing), t));
			mUpperWidthSpin->Enable(mCtrl->IsEnabled() && mCtrl->IsUpperLimitActive());
		}
		if (mLowerWidthSpin) {
			mLowerWidthSpin->SetValue(
				mDim->Convert(mCtrl->GetLowerWidth(t)), FALSE);
			if (floatPBlock)
				mLowerWidthSpin->SetKeyBrackets(floatPBlock->KeyFrameAtTime(
					floatPBlock->IDtoIndex(FloatLimitCtrl::pb_lower_smoothing), t));
			mLowerWidthSpin->Enable(mCtrl->IsEnabled() && mCtrl->IsLowerLimitActive());
		}
		CheckDlgButton(mHWnd, IDC_LIMITCTRL_ENABLE, mCtrl->IsEnabled());
		CheckDlgButton(mHWnd, IDC_LIMITCTRL_UPPERLIMITCHECK, mCtrl->IsUpperLimitActive());
		CheckDlgButton(mHWnd, IDC_LIMITCTRL_LOWERLIMITCHECK, mCtrl->IsLowerLimitActive());
		EnableWindow(GetDlgItem(mHWnd, IDC_LIMITCTRL_UPPERLIMITCHECK), mCtrl->IsEnabled());
		EnableWindow(GetDlgItem(mHWnd, IDC_LIMITCTRL_UPPERLIMIT), 
			mCtrl->IsEnabled() && mCtrl->IsUpperLimitActive());
		EnableWindow(GetDlgItem(mHWnd, IDC_LIMITCTRL_UPPERLIMITSPIN), 
			mCtrl->IsEnabled() && mCtrl->IsUpperLimitActive());
		EnableWindow(GetDlgItem(mHWnd, IDC_LIMITCTRL_LOWERLIMITCHECK), mCtrl->IsEnabled());
		EnableWindow(GetDlgItem(mHWnd, IDC_LIMITCTRL_LOWERLIMIT), 
			mCtrl->IsEnabled() && mCtrl->IsLowerLimitActive());
		EnableWindow(GetDlgItem(mHWnd, IDC_LIMITCTRL_LOWERLIMITSPIN), 
			mCtrl->IsEnabled() && mCtrl->IsLowerLimitActive());
	}
}

void FloatLimitControlDlg::SetupUI()
{
	// Set the control labels
	::SetWindowText(GetDlgItem(mHWnd, IDC_LIMITCTRL_UPPERBOX), GetString(IDS_FLOATLIMITCTRL_UPPERLIMITUI));
	::SetWindowText(GetDlgItem(mHWnd, IDC_LIMITCTRL_LOWERBOX), GetString(IDS_FLOATLIMITCTRL_LOWERLIMITUI));
	::SetWindowText(GetDlgItem(mHWnd, IDC_LIMITCTRL_UPPERLIMITCHECK), GetString(IDS_FLOATLIMITCTRL_ENABLED_UI));
	::SetWindowText(GetDlgItem(mHWnd, IDC_LIMITCTRL_LOWERLIMITCHECK), GetString(IDS_FLOATLIMITCTRL_ENABLED_UI));
	::SetWindowText(GetDlgItem(mHWnd, IDC_LIMITCTRL_UPPER_WIDTH_STATIC), GetString(IDS_FLOATLIMITCTRL_SMOOTHING_UI));
	::SetWindowText(GetDlgItem(mHWnd, IDC_LIMITCTRL_LOWER_WIDTH_STATIC), GetString(IDS_FLOATLIMITCTRL_SMOOTHING_UI));

	mUpperLimitSpin = GetISpinner(GetDlgItem(mHWnd, IDC_LIMITCTRL_UPPERLIMITSPIN));
	if (mUpperLimitSpin)
	{
		mUpperLimitSpin->SetLimits(s_minLimit, s_maxLimit, FALSE);
		mUpperLimitSpin->SetScale(1.0f);
		mUpperLimitSpin->LinkToEdit(
			GetDlgItem(mHWnd, IDC_LIMITCTRL_UPPERLIMIT), EDITTYPE_FLOAT);
	}
	mLowerLimitSpin = GetISpinner(GetDlgItem(mHWnd, IDC_LIMITCTRL_LOWERLIMITSPIN));
	if (mLowerLimitSpin)
	{
		mLowerLimitSpin->SetLimits(s_minLimit, s_maxLimit, FALSE);
		mLowerLimitSpin->SetScale(1.0f);
		mLowerLimitSpin->LinkToEdit(
			GetDlgItem(mHWnd, IDC_LIMITCTRL_LOWERLIMIT), EDITTYPE_FLOAT);
	}

	mUpperWidthSpin = GetISpinner(GetDlgItem(mHWnd, IDC_LIMITCTRL_UPPERLIMIT_W_SPIN));
	if (mUpperWidthSpin)
	{
		mUpperWidthSpin->SetLimits(0.0f, s_maxLimit, FALSE);
		mUpperWidthSpin->SetScale(1.0f);
		mUpperWidthSpin->LinkToEdit(
			GetDlgItem(mHWnd, IDC_LIMITCTRL_UPPERLIMIT_W), EDITTYPE_FLOAT);
	}
	mLowerWidthSpin = GetISpinner(GetDlgItem(mHWnd, IDC_LIMITCTRL_LOWERLIMIT_W_SPIN));
	if (mLowerWidthSpin)
	{
		mLowerWidthSpin->SetLimits(0.0f, s_maxLimit, FALSE);
		mLowerWidthSpin->SetScale(1.0f);
		mLowerWidthSpin->LinkToEdit(
			GetDlgItem(mHWnd, IDC_LIMITCTRL_LOWERLIMIT_W), EDITTYPE_FLOAT);
	}
}

void FloatLimitControlDlg::SpinnerChange(int in_ctrlID, BOOL in_drag)
{
	// A right click on the spinner causes both a mouse down and up 
	// (but in_drag is FALSE) so there is no need to turn the hold on again.
	if (!in_drag && !theHold.Holding())
		SpinnerStart(in_ctrlID);

	switch (in_ctrlID) {
		case IDC_LIMITCTRL_UPPERLIMITSPIN:
			mCtrl->SetUpperLimit(GetCOREInterface()->GetTime(),
				mDim->UnConvert(mUpperLimitSpin->GetFVal()));
			break;
		case IDC_LIMITCTRL_LOWERLIMITSPIN:
			mCtrl->SetLowerLimit(GetCOREInterface()->GetTime(),
				mDim->UnConvert(mLowerLimitSpin->GetFVal()));
			break;

		case IDC_LIMITCTRL_UPPERLIMIT_W_SPIN:
			mCtrl->SetUpperWidth(GetCOREInterface()->GetTime(),
				mDim->UnConvert(mUpperWidthSpin->GetFVal()));
			break;
		case IDC_LIMITCTRL_LOWERLIMIT_W_SPIN:
			mCtrl->SetLowerWidth(GetCOREInterface()->GetTime(),
				mDim->UnConvert(mLowerWidthSpin->GetFVal()));
			break;
	}
	mCtrl->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
}

void FloatLimitControlDlg::SpinnerStart(int in_ctrlID)
{
	theHold.Begin();
}

void FloatLimitControlDlg::SpinnerEnd(int in_ctrlID, BOOL in_cancel)
{
	if (in_cancel) {
		theHold.Cancel();
	} 
	else {
		theHold.Accept(GetString(IDS_LIMITCTRL_CHANGELIMIT));
	}
	GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
}

void FloatLimitControlDlg::Init(HWND in_hParent)
{
	mHWnd = CreateWin(in_hParent);
}

void FloatLimitControlDlg::TimeChanged(TimeValue t) 
{
	Invalidate(); 
	Update();
}

HWND FloatLimitControlTrackDlg::CreateWin(HWND in_hParent)
{
	return CreateDialogParam(
		hInstance,
		MAKEINTRESOURCE(IDD_LIMITPARAMS_TRACK),
		in_hParent,
		FloatLimitControlDlg::FloatDlgProc,
		(LPARAM)this);
}

INT_PTR CALLBACK FloatLimitControlDlg::FloatDlgProc
	(
		HWND	in_hWnd,
		UINT	in_message,
		WPARAM	in_wParam,
		LPARAM	in_lParam
	)
{
	FloatLimitControlDlg *ld = DLGetWindowLongPtr<FloatLimitControlDlg*>(in_hWnd);	
	
	switch (in_message) {
		case WM_INITDIALOG: {
			DLSetWindowLongPtr(in_hWnd, in_lParam);
			SendMessage(in_hWnd, WM_SETICON, ICON_SMALL, DLGetClassLongPtr<LPARAM>(GetCOREInterface()->GetMAXHWnd(), GCLP_HICONSM));
			ld = (FloatLimitControlDlg*)in_lParam;
			if (!ld)
				break;
			ld->SetHWnd(in_hWnd);
			ld->SetupUI();
			ld->Update();
			break;
		}
		case WM_COMMAND:
			ld->WMCommand(LOWORD(in_wParam), HIWORD(in_wParam), (HWND)in_lParam);						
			break;

		case WM_PAINT:
			ld->Update();
			return 0;			
		
		case CC_SPINNER_BUTTONDOWN:
			ld->SpinnerStart(LOWORD(in_wParam));
			break;

		case CC_SPINNER_CHANGE:
			ld->SpinnerChange(LOWORD(in_wParam), HIWORD(in_wParam));
			break;

		case WM_CUSTEDIT_ENTER:
			ld->SpinnerEnd(LOWORD(in_wParam), FALSE);
			break;

		case CC_SPINNER_BUTTONUP:
			ld->SpinnerEnd(LOWORD(in_wParam), !HIWORD(in_wParam));
			break;

		case WM_CLOSE:
			DestroyWindow(in_hWnd);			
			break;

		case WM_DESTROY:						
			delete ld;
			break;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:			
			ld->MouseMessage(in_message, in_wParam, in_lParam);
			return FALSE;

		default:
			return 0;
		}
	return 1;
}

void FloatLimitControlDlg::WMCommand(int in_id, int in_notify, HWND in_hCtrl)
{
	if (!mCtrl)
		return;

	bool edited = true;

	switch (in_id) {
		case IDC_LIMITCTRL_ENABLE:
			theHold.Begin();
			mCtrl->SetEnabled(IsDlgButtonChecked(mHWnd, in_id) != 0);
			theHold.Accept(GetString(IDS_LIMITCTRL_CHANGELIMIT));
			Update();
			break;
		case IDC_LIMITCTRL_UPPERLIMITCHECK:
			theHold.Begin();
			mCtrl->SetUpperLimitActive(GetCOREInterface()->GetTime(), IsDlgButtonChecked(mHWnd, in_id) != 0);
			theHold.Accept(GetString(IDS_LIMITCTRL_CHANGELIMIT));
			Update();
			break;
		case IDC_LIMITCTRL_LOWERLIMITCHECK:
			theHold.Begin();
			mCtrl->SetLowerLimitActive(GetCOREInterface()->GetTime(), IsDlgButtonChecked(mHWnd, in_id) != 0);
			theHold.Accept(GetString(IDS_LIMITCTRL_CHANGELIMIT));
			Update();
			break;
		default:
			edited = false;
			break;
	}
	if (edited) {
		mCtrl->NotifyDependents(FOREVER,PART_ALL, REFMSG_CHANGE);
		GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
	}
}

///////////////////////////////////////////////////////////////////////////////
//	LimitControlDlgManager
LimitControlDlgManager *LimitControlDlgManager::s_instance = NULL;


LimitControlDlgManager::LimitControlDlgManager()//: mDialog(NULL)
{
}

LimitControlDlgManager::~LimitControlDlgManager()
{
	mDialogTab.Delete(0, mDialogTab.Count());
}

FloatLimitControlDlg *LimitControlDlgManager::CreateLimitDialog()
{
	FloatLimitControlDlg* dlg = new FloatLimitControlTrackDlg;
	if (dlg)
		mDialogTab.Append(1, &dlg);

	return dlg;
}

FloatLimitControlDlg *LimitControlDlgManager::GetDialogForCtrl(BaseLimitCtrl* in_limitCtrl)
{
	FloatLimitControlDlg *dlg = FindDialogFromCtrl(in_limitCtrl);
	if (!dlg)
		dlg = CreateLimitDialog();
	return dlg;
}

FloatLimitControlDlg *LimitControlDlgManager::FindDialogFromCtrl(BaseLimitCtrl* in_limitCtrl)
{
	for (int i = 0; i < mDialogTab.Count(); i++)
		if (mDialogTab[i] && mDialogTab[i]->GetController() == in_limitCtrl)
			return mDialogTab[i];

	return NULL;
}

// Returns true iff the dialog was found and removed
bool LimitControlDlgManager::RemoveDialog(FloatLimitControlDlg *in_dlg)
{
	for (int i = 0; i < mDialogTab.Count(); i++)
		if (mDialogTab[i] == in_dlg) {
			mDialogTab.Delete(i, 1);
			return true;
		}

	return false;
}

LimitControlDlgManager*  LimitControlDlgManager::GetLimitCtrlDlgManager()
{
	if (!s_instance)
		s_instance = new LimitControlDlgManager;

	return s_instance;
}

