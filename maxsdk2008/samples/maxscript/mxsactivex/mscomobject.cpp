/**********************************************************************
 *<
	FILE:			MSComObject.cpp

	DESCRIPTION:	ActiveX control creation

	CREATED BY:		Ravi Karra

	HISTORY:		Started on Nov.3rd 1999

 *>	Copyright (c) 1999, All Rights Reserved.
 **********************************************************************/
#include "stdafx.h"
#include "VarUtils.h"
#include "comwraps.h"
#include "3dmath.h"
#include "MaxObj.h"
#include "MaxKeys.h"
#include "CodeTree.h"
#include "lclinsfn.h"
#include "IPathConfigMgr.h"
#include "mszippackage.h"

Value*
set_enable_accel(Value* val)
{
	val->to_bool() ? EnableAccelerators() : DisableAccelerators();
	return val;
}

Value*
get_enable_accel()
{
	return AcceleratorsEnabled() ? &true_value : &false_value;
}

static bool atl_init = true;
void activex_init()
{
	install_rollout_control(Name::intern("ActiveXControl"), ActiveXControl::create);
	define_system_global ( "enableAccelerators", get_enable_accel,	set_enable_accel);	
}


class AxThunkReference : public Thunk
{
public:
	Value**	ref_list;			// the target thunk
	int index;
	bool first_time;

	AxThunkReference(Value** arg_list, int iindex)
	{
		ref_list = arg_list;
		index = iindex;
		tag = INTERNAL_THUNK_REF_TAG;
		first_time = true;
	}
	void	gc_trace()
	{
		/* trace sub-objects & mark me */
		if (ref_list[index] && ref_list[index]->is_not_marked())
			ref_list[index]->gc_trace();

	}
	void	collect() { delete this; }
	void	sprin1(CharStream* s)
	{
		s->printf(_T("&")); ref_list[index]->sprin1(s);
	}

	Value*  eval() 
	{ 
		if (first_time) 
		{
			first_time = false;
			return this;
		}
		else 
			return ref_list[index];
	} 
	Value* assign_vf(Value**arg_list, int count)
	{
		Value* rval = arg_list[0]->get_heap_ptr();
		return ref_list[index] = rval;
	}
};

#define def_create_fail()	throw RuntimeError(GetString(IDS_CREATE_FAILED), caption);
#define def_hr_error(hr)	if ( FAILED(hr) ) def_create_fail();
#define def_hr_return(hr)	if ( FAILED(hr) ) return hr;
#define def_prop_error(gs,p)															\
	throw RuntimeError(_T("Cannot "###gs##" the property, ."), p->to_string());

void 
ActiveXControl::check_hr_error(HRESULT hr)
{
	if ( FAILED(hr))
	{
		LPVOID lpMsgBuf;
		TSTR msg (GetString(IDS_CREATE_FAILED));
		msg.append(caption->to_string());
		msg.append(GetString(IDS_REPORT_ERR_MSG));
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL,
			hr,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			(LPTSTR) &lpMsgBuf, 0, NULL);
		msg.append((LPTSTR) lpMsgBuf);
		LocalFree(lpMsgBuf);
		if (m_comObject) 
		{
			m_comObject->FreeAll();
			m_comObject = NULL;
		}
		if (m_ax.IsWindow())
			m_ax.DestroyWindow();
		throw RuntimeError(msg);
	}
}



// russom - 05/15/01
#ifdef USE_ACTIVEX_CONTROL_FILTER

	struct ControlFilter {
		TCHAR *szControlName;
		TCHAR **pszProperties;
	};

#if defined(GAME_VER)

	// ShockwaveFlash properties
	static TCHAR *s_pszFlashProperties[] =
	{
		"SetVariable",		"GetVariable",		"TSetProperty",
		"TGetProperty",		"TCallLabel",		"TCallFrame",
		"Movie",			"Menu",				"BGColor", 
		"BackgroundColor",	"IsPlaying",		"Quality",			
		"Quality2",			"ReadyState",		"TotalFrames",		
		"Playing",			"ScaleMode",		"AlignMode",		
		"Loop",				"FrameNum",			"WMode",
		"SAlign",			"Base",				"scale",			
		"DeviceFont",		"EmbedMovie",		"SWRemote",
		"Stacking",			"GotoFrame",		"LoadMovie",
		"Pan",				"PercentLoaded",	"Play",
		"Rewind",			"SetZoomRect",		"StopPlay",		
		"TCurrentFrame",	"TCurrentLabel",	"TGetPropertyAsNumber",	
		"TGotoFrame",		"TGotoLabel",		"TotalFrames",	
		"TPlay",			"TStopPlay",		"Zoom",			
		"ShockwaveVersion",	"EvalScript",
		NULL
	};

	// This is a list of allowed ProgIDs and CLSIDs for activeXControl.
	// activeXControl params will be compared. It will perform a strnicmp
	// based on the length of the ControlFilter entry.  This allows
	// SHOCKWAVEFLASH.SHOCKWAVEFLASH. to cover all ShockwaveFlash
	// ProgIDs, for example.  In addition to CLSIDs and ProgIDs, a pointer
	// to a list of properties is also provided.  This pointer can be NULL
	// to allow all properties.

	static ControlFilter s_ControlFilter[] =
	{
		{ "ShockwaveFlash.ShockwaveFlash.", s_pszFlashProperties },
		{ "{D27CDB6E-AE6D-11CF-96B8-444553540000}", s_pszFlashProperties },
		{ NULL, NULL }
	};
#else

	static ControlFilter s_ControlFilter[] =
	{
		{ NULL, NULL }
	};

#endif

	bool IsValidProperty( TCHAR *szProp, TCHAR **pszPropList )
	{
		if( szProp == NULL )
			return false;

		// TODO: hash this - the property list for flash has tripled
		// Most used properties are put towards the front of the list
		for( TCHAR** p = pszPropList; *p != NULL; p++ ) {
			if( _tcsicmp( (*p), szProp ) == 0 )
				return true;
		}

		return false;
	}

#endif // USE_ACTIVEX_CONTROL_FILTER

/////////////////////////////////////////////////////////////////////////////
// ActiveXControl
/////////////////////////////////////////////////////////////////////////////
local_visible_class_instance (ActiveXControl, _T("ActiveXControl"))
void
ActiveXControl::add_control(Rollout *ro, HWND parent, HINSTANCE hInst, int& current_y)
{
	// activeXControl <name> [ <caption> ] [events:<true>] [ param1: ] [ param2: ] ...

	caption = caption->eval();

	TCHAR*	control_text = caption->eval()->to_string();
	WORD	control_ID = next_id();
	parent_rollout = ro;
	
#ifdef USE_ACTIVEX_CONTROL_FILTER
	if( control_text ) {
		int i=0;
		bool bValidControl = false;
		while( s_ControlFilter[i].szControlName != NULL ) {
			if( _tcsncicmp( s_ControlFilter[i].szControlName, control_text, _tcslen(s_ControlFilter[i].szControlName)) == 0 ) {
				bValidControl = true;
				m_iControlFilter = i;
				break;
			}
			i++;
		}

		if( !bValidControl ) {
			throw RuntimeError (_T("Unsupported control type: "), control_text);
			return;
		}
	}
#endif // USE_ACTIVEX_CONTROL_FILTER

	// compute bounding box, apply layout params
	layout_data pos;
	setup_layout(ro, &pos, current_y);
	process_layout_params(ro, &pos, current_y);
	current_y = pos.top + pos.height;

	// create the ActiveX control	
	rect.left = pos.left; rect.top = pos.top;
	rect.bottom = pos.top + pos.height;
	rect.right = pos.left + pos.width;
	hwndContainer = m_ax.Create(parent, rect, 0,
									WS_VISIBLE | WS_CHILD | WS_GROUP);

// russom - 08/01/01
// if m_iControlFilter == -1 at this point, something is wrong.
// prevent this control from being loaded.
#ifdef USE_ACTIVEX_CONTROL_FILTER
	int iCharOffset = (m_iControlFilter == -1) ? 4 : 8;
#endif

	if (!hwndContainer) def_create_fail();
	MAXScript_interface->RegisterDlgWnd(hwndContainer);

	CComPtr<IAxWinHostWindow>		spAxWindow;
	CComPtr<IAxWinAmbientDispatch>	spAmbientDispatch;
	CComPtr<IClassFactory2>			pCF;
	CComPtr<IProvideClassInfo>		pPCI;
	CComPtr<ITypeInfo>				pClassInfo;


	HRESULT hr = m_ax.QueryHost(&spAxWindow);
	check_hr_error(hr);
	
	USES_CONVERSION;
// russom - 08/01/01
// apply proper iCharOffset
#ifndef USE_ACTIVEX_CONTROL_FILTER
	LPCOLESTR pszName = T2COLE(control_text);
#else
	LPCOLESTR pszName = T2COLE((control_text+(8-iCharOffset)));
#endif

	ActiveXLicensor activeXLicensor; // temporarily add some licenses

	hr = spAxWindow->CreateControl(pszName, hwndContainer, 0);
	check_hr_error(hr);

	hr = CEventListener::CreateInstance(&m_comObject);
	check_hr_error(hr);

	// query for IDispatch
	hr = m_ax.QueryControl(&m_comObject->m_pdisp);
	check_hr_error(hr);

	// check if it is a licensed copy, otherwise throw an error
	hr = m_ax.QueryControl(&pCF); // get the class factory	
	if (SUCCEEDED(hr))
	{
		LICINFO li;
		pCF->GetLicInfo(&li);
		if (!li.fRuntimeKeyAvail) 
		{
			if (m_comObject) 
			{
				m_comObject->FreeAll();
				m_comObject = NULL;
			}
			if (m_ax.IsWindow())
				m_ax.DestroyWindow();
			throw RuntimeError((GetString(IDS_UNLICENSED_CONTROL)), caption);
			return;
		}
	}
	// query for the properties
	hr = m_ax.QueryControl(&pPCI);
		
	if (SUCCEEDED(hr))
		// get the class info
		hr = pPCI->GetClassInfo(&pClassInfo);
	else
		hr = m_comObject->m_pdisp->GetTypeInfo(0, LOCALE_USER_DEFAULT, &pClassInfo);
	check_hr_error(hr);	
	
	m_ax.QueryHost(&spAmbientDispatch);
	spAmbientDispatch->put_BackColor(0);
	//WNDCLASS wc;
	//GetClassInfo(_Module.GetModuleInstance(), CAxWindow::GetWndClassName(), &wc);
	//wc.hbrBackground = GetCustSysColorBrush(COLOR_BTNFACE);
	//((CAxHostWindow*)spAxWindow.p)->
	

	// initialize the COM object
	hr = m_comObject->Init(pClassInfo, this, true);
	check_hr_error(hr);
	
	Value* events = _get_control_param(keyparms, keyparm_count, Name::intern("setupEvents"));
	if (events == &unsupplied || events == &true_value)
		m_comObject->SetupEvents(true);

	Value* roc = _get_control_param(keyparms, keyparm_count, Name::intern("releaseOnClose"));
	m_releaseOnClose = (roc == &unsupplied || roc == &true_value) ? true : false; // LAM - 8/12/02 - defect 439807
	
}

ActiveXControl::ActiveXControl(Value* name, Value* caption, Value** keyparms, int keyparm_count) 
	: RolloutControl(name, caption, keyparms, keyparm_count)
{ 
	tag = class_tag(ActiveXControl);
	m_comObject = NULL;
	if (atl_init) 
	{ 
		AtlAxWinInit(); 
		atl_init = false;
//		EnableUndoDebugPrintout(1);
	}

#ifdef USE_ACTIVEX_CONTROL_FILTER
	m_iControlFilter = -1;
#endif
}

ActiveXControl::~ActiveXControl()
{
	 // release the activeX control
	if (m_comObject) {
		m_comObject->FreeAll();
		m_comObject = NULL;
	}
}

void
ActiveXControl::gc_trace()
{
	RolloutControl::gc_trace();
}

BOOL
ActiveXControl::handle_message(Rollout *ro, UINT message, WPARAM wParam, LPARAM lParam)
{
	// all messages are handled in the dialog proc
	if (message == WM_CLOSE)
	{
		MAXScript_interface->UnRegisterDlgWnd(hwndContainer);
		if (m_comObject) m_comObject->FreeAll();
		if ( m_releaseOnClose )
		{
			m_comObject = NULL;  // LAM - 8/12/02 - defect 439807
			if (m_ax.IsWindow())
				m_ax.DestroyWindow();
			DestroyWindow(GetParent(hwndContainer));			
			//m_comObject->Release();			
		}			
	}
	return FALSE;
}

void
ActiveXControl::set_enable()
{
	if (parent_rollout != NULL && parent_rollout->page != NULL)
	{
		// set ActiveX control enable	
		m_ax.EnableWindow(enabled);
		::InvalidateRect(parent_rollout->page, NULL, TRUE);
	}
}

Value*
ActiveXControl::get_property(Value** arg_list, int count)
{
	Value* val = NULL;
	Value* prop = arg_list[0];

	if ( prop == n_size )
		return new Point2Value(Point2((float)(rect.right-rect.left), (float)(rect.bottom-rect.top)));
	else if ( prop == n_pos )
		return new Point2Value(Point2((float)rect.left, (float)rect.top));
	else if ( prop == n_description )
	{
		TSTR desc;
		// return the type library info
		ITypeLib *pTypeLib = NULL; unsigned index; HRESULT hr;
		if (m_comObject == NULL) return new String(desc);
		hr = m_comObject->m_spTypeInfo->GetContainingTypeLib(&pTypeLib, &index);
		if (SUCCEEDED(hr))
		{
			USES_CONVERSION;
			CComBSTR bstrName;
			CComBSTR bstrDoc;
			CComBSTR bstrHelp;
			unsigned long lHelpContext;
			hr = pTypeLib->GetDocumentation(MEMBERID_NIL, &bstrName, &bstrDoc, &lHelpContext, &bstrHelp);
			desc.printf(_T("%s (%s)?%s-%lu"), OLE2T(bstrName), OLE2T(bstrDoc),
				(!bstrHelp) ? _T("") : OLE2T(bstrHelp), lHelpContext);
		}
		return new String(desc);
	}

#ifdef USE_ACTIVEX_CONTROL_FILTER
	if( m_iControlFilter != -1 ) {
		if( s_ControlFilter[m_iControlFilter].pszProperties != NULL ) {
			if( !IsValidProperty(prop->to_string(), s_ControlFilter[m_iControlFilter].pszProperties) )
				return RolloutControl::get_property(arg_list, count);
		}
	}
	else {
		return RolloutControl::get_property(arg_list, count);
	}
#endif

	try
	{
		if (m_comObject && (val = m_comObject->get_property(arg_list, count)) != NULL)
			return val;
	}
	catch (...)
	{
		val = get_wrapped_event_handler(arg_list[0]);
		if(val != NULL)
			return val;
	}
	return RolloutControl::get_property(arg_list, count);
}

Value*
ActiveXControl::set_property(Value** arg_list, int count)
{
	Value* val = arg_list[0];
	Value* prop = arg_list[1];

	if ( prop == n_size )
	{
		if (m_ax.m_hWnd && IsWindow(m_ax.m_hWnd)) 
		{
			Point2 size = val->to_point2();
			rect.right = rect.left + size.x;
			rect.bottom = rect.top + size.y;
			m_ax.MoveWindow(&rect);			
		}
		return val;
	}
	else if ( prop == n_pos )
	{
		if (m_ax.m_hWnd  && IsWindow(m_ax.m_hWnd)) 
		{
			Point2 pos = val->to_point2();
			rect.left = pos.x;
			rect.top = pos.y;
			m_ax.MoveWindow(&rect);
		}
		return val;
	}

#ifdef USE_ACTIVEX_CONTROL_FILTER
	if( m_iControlFilter != -1 ) {
		if( s_ControlFilter[m_iControlFilter].pszProperties != NULL ) {
			if( !IsValidProperty(prop->to_string(), s_ControlFilter[m_iControlFilter].pszProperties) )
				return RolloutControl::set_property(arg_list, count);
		}
	}
	else {
		return RolloutControl::set_property(arg_list, count);
	}
#endif

	val = m_comObject ? m_comObject->set_property(arg_list, count) : NULL;
	return (val) ? val : RolloutControl::set_property(arg_list, count);
}

BOOL
ActiveXControl::set_focus()
{
	if (parent_rollout && parent_rollout->page)
	{
		// set ActiveX control enable	
		if (m_ax.m_hWnd  && IsWindow(m_ax.m_hWnd))
		{
			SetFocus(m_ax);
			return TRUE;
		}
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// MSComObject
/////////////////////////////////////////////////////////////////////////////
HRESULT MSComObject::Init(ITypeInfo* ti, ActiveXControl* iparent, bool idefault)
{
	
	UINT				iType;
	HREFTYPE			hRefType;
	HRESULT				hr;	
	CComPtr<ITypeInfo>	pTypeInfo;
	
	m_spTypeInfo = ti;
	m_parent = iparent;

	CSmartTypeAttr pTypeAttr( m_spTypeInfo );
	hr = m_spTypeInfo->GetTypeAttr( &pTypeAttr );
	def_hr_return(hr);
//	DbgAssert( pTypeAttr->typekind == TKIND_COCLASS );

	// look for the default interface
	if (idefault)
	{
		bool tFoundDefaultSource=false, tFoundDefaultInterface=false;
		int iFlags;

		for( iType = 0; (iType < pTypeAttr->cImplTypes) && !(tFoundDefaultSource &&
		  tFoundDefaultInterface); iType++ )
		{
			hr = m_spTypeInfo->GetImplTypeFlags( iType, &iFlags );
			if( SUCCEEDED( hr ) )
			{
				if( (iFlags&IMPLTYPE_MASK) == IMPLTYPE_DEFAULTSOURCE)
				{
					DbgAssert( !tFoundDefaultSource );
					tFoundDefaultSource = true;
					
					hr = m_spTypeInfo->GetRefTypeOfImplType( iType, &hRefType );
					def_hr_return(hr);
					
					hr = m_spTypeInfo->GetRefTypeInfo( hRefType, &pTypeInfo.p );
					def_hr_return(hr);

					hr = m_infoEvents.Init( pTypeInfo );
					def_hr_return(hr);
				}
				else if( (iFlags&IMPLTYPE_MASK) == IMPLTYPE_DEFAULTINTERFACE )
				{
					DbgAssert( !tFoundDefaultInterface );
					tFoundDefaultInterface = true;
					
					hr = m_spTypeInfo->GetRefTypeOfImplType( iType, &hRefType );
					def_hr_return(hr);
					
					hr = m_spTypeInfo->GetRefTypeInfo( hRefType, &pTypeInfo.p );
					def_hr_return(hr);

					hr = m_infoMethods.Init( pTypeInfo );
					def_hr_return(hr);
				}
			}
		}
		if ( !tFoundDefaultInterface )
			m_infoMethods.Init( m_spTypeInfo );
	}
	else
	{
		hr = m_infoMethods.Init( m_spTypeInfo );
		def_hr_return(hr);
	}
	return hr;
}

void
MSComObject::AxError(TCHAR* icap, TCHAR* got)
{
	if (thread_local(try_mode)) // don't display and error
		throw RuntimeError (icap); // LAM - 11/23/02 - defect 469605
	if ( m_parent && !(m_parent->parent_rollout->flags & RO_SILENT_ERRORS))
	{
		TCHAR cap[1024];
		_tcscpy(cap, icap);
		if (got)
			_tcscat(cap, got);
		listener_message(MXS_ERROR_MESSAGE_BOX, (WPARAM)save_string("MAXScript"),
			(LPARAM)save_string(cap), FALSE);
	}
}

Value*
MSComObject::get_property(Value** arg_list, int count)
{
	Value* val = &undefined;
	Value* prop = arg_list[0];
	HRESULT hr;
	UINT iArgErr;
	CExcepInfo excepInfo;
	CMethodInfo *pMethodInfo=0;	
	VARIANT vRet;
	VariantInit(&vRet);
	excepInfo.Clear();
	
	try
	{		
		pMethodInfo = m_infoMethods.FindMethod(prop->to_string());
		if (pMethodInfo)
		{
			INVOKEKIND ik = pMethodInfo->GetInvokeKind();
			if (ik == INVOKE_FUNC )
			{
				return new MSComMethod(prop->to_string(), pMethodInfo->GetID(), this, pMethodInfo);
			}
			else
			{
				VARIANT vIndex;
				
				if (count>1) // for index'ed properties
				{
					// LAM - 12/12/05 - defect 712475 
					// we might have a setter method. Try looking for a getter method
					if (pMethodInfo->GetInvokeKind() != INVOKE_PROPERTYGET)
					{
						CMethodInfo* pGetterMethodInfo = m_infoMethods.FindPropertyGet(prop->to_string());
						if (pGetterMethodInfo)
							pMethodInfo = pGetterMethodInfo;
					}
					if (pMethodInfo->m_apParamInfo.Count() != 1)
						AxError(_T("getIndexedProperty requires an index'ed property, got: "), prop->to_string());
					
					VariantInit(&vIndex);
					
					V_VT(&vIndex) = VT_I4;
					V_I4(&vIndex) = arg_list[1]->to_int();
				}
				else if (pMethodInfo->m_apParamInfo.Count() > 0)
					AxError(_T("This property needs an index, use getIndexedProperty #"), prop->to_string());
				
				DISPPARAMS dispparams = {(count>1) ? &vIndex : NULL, NULL, (count>1) ? 1 : 0, 0};
				
				hr = m_pdisp->Invoke(pMethodInfo->GetID(), IID_NULL,
						GetUserDefaultLCID(), WORD(INVOKE_PROPERTYGET),
						&dispparams, &vRet, &excepInfo, &iArgErr );
				
				if ( FAILED(hr) )
					hr = m_pdisp.GetProperty(pMethodInfo->GetID(), &vRet);
				
				if ( FAILED(hr) ) def_prop_error(get, prop);
			}
		}
		else
		{
			//alternative to the above method, kind of slow
			USES_CONVERSION;
			LPCOLESTR pname = T2OLE(prop->to_string());
			DISPID dispid;
			
			hr = m_pdisp->GetIDsOfNames(IID_NULL, (LPOLESTR*)&pname, 1, LOCALE_USER_DEFAULT, &dispid);
			if ( FAILED(hr) ) def_prop_error(get, prop);
			
			if (thread_local(current_prop) == NULL)
				return new MSComMethod(prop->to_string(), dispid, this);
			
			hr = m_pdisp.GetProperty(dispid, &vRet);
			if ( FAILED(hr) ) def_prop_error(get, prop);			
		}
	}
	catch (...)
	{
		def_prop_error(get, prop);
	}
	if (pMethodInfo)
		pMethodInfo->GetValue(&vRet, &val);
	else
		value_from_variant(&vRet, &val);
	return val;
}

Value*
MSComObject::is_readonly_vf(Value** arg_list, int count)
{
	Value* val = NULL;
	Value* prop = arg_list[0];
	CMethodInfo *pMethodInfo=0;
	try
	{
		pMethodInfo = m_infoMethods.FindMethod(prop->to_string());
		if (!pMethodInfo) { def_prop_error(get, prop); return &false_value; }
		INVOKEKIND ik = pMethodInfo->GetInvokeKind();
		if (ik == INVOKE_FUNC )
		{
			def_prop_error(get, prop); return &false_value;
		}
		else if(ik == INVOKE_PROPERTYGET && pMethodInfo->IsReadOnly())
		{
			return &true_value;
		}		
	}
	catch (MAXScriptException& e)
	{
		error_message_box(e, GetString(IDS_ROLLOUT_HANDLER));
	}
	catch (...)
	{
		def_prop_error(get, prop);
	}	
	return &false_value;
}

Value*
MSComObject::set_property(Value** arg_list, int count)
{
	HRESULT hr;
	Value* val	= (count>2) ? arg_list[2] : arg_list[0];
	Value* prop = (count>2) ? arg_list[0] : arg_list[1];
	VARIANT var;
	CMethodInfo *pMethodInfo=0;	

	USES_CONVERSION;
	try
	{	
		pMethodInfo = m_infoMethods.FindMethod(prop->to_string());
		if (!pMethodInfo)
		{
			variant_from_value(val, &var);
			hr = m_pdisp.PutPropertyByName(T2OLE(prop->to_string()), &var);
			if ( FAILED(hr) ) def_prop_error(set, prop);
			return val;
		}
		
		if (count == 2 && pMethodInfo->m_apParamInfo.Count() > 0)
			AxError(_T("This property needs an index, use setIndexedProperty #"), prop->to_string());
		
		VariantInit(&var);
		hr = pMethodInfo->GetVariant(val, &var);
		if ( FAILED(hr) ) def_prop_error(set, prop);

		if (count<3)
		{
			hr = m_pdisp.PutProperty(pMethodInfo->GetID(), &var);
			if ( FAILED(hr) ) def_prop_error(set, prop);
		}
		else
		{
			// for index'ed properties
			// LAM - 12/12/05 - defect 712475 
			// we might have a getter method. Try looking for a setter method
			if ((pMethodInfo->GetInvokeKind() != INVOKE_PROPERTYPUT) && 
				(pMethodInfo->GetInvokeKind() != INVOKE_PROPERTYPUTREF))
			{
				CMethodInfo* pSetterMethodInfo = m_infoMethods.FindPropertySet(prop->to_string());
				if (pSetterMethodInfo)
					pMethodInfo = pSetterMethodInfo;
			}

			if (pMethodInfo->m_apParamInfo.Count() != 2)
				AxError(_T("setIndexedProperty requires an index'ed property, got: "), prop->to_string());
			
			VARIANT vIndex;				
			VariantInit(&vIndex);
			V_VT(&vIndex) = VT_I4;
			V_I4(&vIndex) = arg_list[1]->to_int();

			CComVariant varArgs[2] = { var, vIndex };
			DISPPARAMS dispparams = {NULL, NULL, 2, 1};
			dispparams.rgvarg = &varArgs[0];
			DISPID dispidPut = DISPID_PROPERTYPUT;
			dispparams.rgdispidNamedArgs = &dispidPut;

			DISPID id = pMethodInfo->GetID();

			if (var.vt == VT_UNKNOWN || var.vt == VT_DISPATCH ||
				(var.vt & VT_ARRAY) || (var.vt & VT_BYREF))
			{
				HRESULT hrp = m_pdisp->Invoke(id, IID_NULL,
					LOCALE_USER_DEFAULT, DISPATCH_PROPERTYPUTREF,
					&dispparams, NULL, NULL, NULL);
				if (SUCCEEDED(hrp))
					return val;
			}
			hr= m_pdisp->Invoke(id, IID_NULL,
					LOCALE_USER_DEFAULT, DISPATCH_PROPERTYPUT,
					&dispparams, NULL, NULL, NULL);				
			if ( FAILED(hr) ) def_prop_error(set, prop);
		}
	}
	catch (...)
	{
		def_prop_error(set, prop);
	}
	return val;	
}

Value*
MSComObject::get_indexedProperty_vf(Value** arg_list, int count)
{
	check_arg_count(getIndexedProperty, 3, count+1);	
	return get_property(arg_list, count);
}

Value*
MSComObject::set_indexedProperty_vf(Value** arg_list, int count)
{

	check_arg_count(setIndexedProperty, 4, count+1);
	return set_property(arg_list, count);
}

Value*
MSComObject::get_props_vf(Value** arg_list, int count)
{
	// getPropNames <max_object> [showHidden:<false>] -- returns array of prop names of wrapped obj
	one_typed_value_local(Array* result);
	Value*		val;
	BOOL		show_hidden = bool_key_arg(showHidden, val, FALSE);

	int num_props = m_infoMethods.GetNumMethods();
	vl.result = new Array (num_props);
	
	CMethodInfo *pMethodInfo, *pNextMethodInfo;

	for (int i=0; i < num_props; i++)
	{
		pMethodInfo = m_infoMethods.GetMethod(i);
		INVOKEKIND ik = pMethodInfo->GetInvokeKind();
		
		// hey! skip all functions which accept more than one param
		if ( pMethodInfo->GetNumParams() > 1 ) continue;

		BOOL hidden = pMethodInfo->IsHidden(); 
		if (show_hidden || !hidden)
		{
			// add only one entry for get/put type, and none for method types
			if ( ik == INVOKE_PROPERTYGET || ik == INVOKE_PROPERTYPUT )
				vl.result->append(Name::intern(pMethodInfo->GetName()));

			if (i != num_props - 1)
			{
				pNextMethodInfo = m_infoMethods.GetMethod(i+1);
				// poke the next item and see if it is the put property, if so skip it
				if (pMethodInfo->GetID() == pNextMethodInfo->GetID() &&
					pNextMethodInfo->GetInvokeKind() != ik)
					i++;
			}
		}
	}
	return_value(vl.result); 
}

Value*
MSComObject::show_props_vf(Value** arg_list, int count)
{
	// showProperties <max_object> ["prop_pat"] [to:<file>] [showHidden:<false>]
	CharStream*	out;
	TCHAR*		prop_pat = _T("*");
	Value*		val;
	BOOL		show_hidden;
	Value*		res = &false_value;
	
	// pick up args
	if (count >= 1 && arg_list[0] != &keyarg_marker)	
		prop_pat = arg_list[0]->to_string();	
	
	if ((out = (CharStream*)key_arg(to)) != (CharStream*)&unsupplied)
	{
		if (out == (CharStream*)&undefined)   // to:undefined -> no output, return true or false if prop found
			out = NULL;
		else if (!is_charstream(out))
			throw TypeError (GetString(IDS_SHOWPROPERTIES_NEED_STREAM), out);
	}
	else
		out = thread_local(current_stdout);
	
	show_hidden = bool_key_arg(showHidden, val, FALSE);

	int num_props = m_infoMethods.GetNumMethods();
	CMethodInfo *pMethodInfo, *pNextMethodInfo;
	TSTR buf, ps;

	for (int i=0; i < num_props; i++)
	{
		pMethodInfo = m_infoMethods.GetMethod(i);
		INVOKEKIND ik = pMethodInfo->GetInvokeKind();		

		// hey! skip all functions which accept more than one param
		if ( pMethodInfo->GetNumParams() > 1 ) continue;

		// add only one entry for get/put type, and none for method types
		if ( ik == INVOKE_PROPERTYGET || ik == INVOKE_PROPERTYPUT)
		{
			if (show_hidden || !pMethodInfo->IsHidden())
			{
				TSTR name = pMethodInfo->GetName();
				if (max_name_match(name, prop_pat))
				{
					res = &true_value;
					if (out == NULL)
						return res;
				buf = _T("  .");
					buf += name;
				pMethodInfo->GetParamString(ps);
				buf += ps;
				if ( i != num_props - 1 )
				{
					pNextMethodInfo = m_infoMethods.GetMethod(i+1);
					// poke the next item and see if it is the put property, if so, skip it
					if (pMethodInfo->GetID() == pNextMethodInfo->GetID() &&
						pNextMethodInfo->GetInvokeKind() != ik)
					{
						i++;
					}
					else
						buf += _T(", read-only");
				}
				buf += _T("\n");
				if (out != NULL)
					out->puts(buf);
			}			
		}
	}
	}
	return res;
}

Value*
MSComObject::show_methods_vf(Value** arg_list, int count)
{
	// showMethods <max_object> ["prop_pat"] [to:<file>] [showHidden:<false>]
	CharStream*	out;
	TCHAR*		prop_pat = _T("*");
	Value*		val;
	BOOL		show_hidden = bool_key_arg(showHidden, val, FALSE);
	Value*		res = &false_value;
	
	// pick up args
	if (count >= 1 && arg_list[0] != &keyarg_marker)
		prop_pat = arg_list[0]->to_string();
	
	if ((out = (CharStream*)key_arg(to)) != (CharStream*)&unsupplied)
	{
		if (out == (CharStream*)&undefined)   // to:undefined -> no output, return true or false if prop found
			out = NULL;
		if (!is_charstream(out))
			throw TypeError (GetString(IDS_SHOWMETHODS_NEED_STREAM), out);
	}
	else
		out = thread_local(current_stdout);
	
	int num_props = m_infoMethods.GetNumMethods();
	CMethodInfo *pMethodInfo;	
	TSTR buf, ps;

	for (int i=0; i < num_props; i++)
	{
		pMethodInfo = m_infoMethods.GetMethod(i);
		INVOKEKIND ik = pMethodInfo->GetInvokeKind();
		
		if (ik != INVOKE_FUNC) continue;
		
		BOOL hidden = pMethodInfo->IsHidden(); 
		if (show_hidden || !hidden)
		{
			TSTR name = pMethodInfo->GetName();
			if (max_name_match(name, prop_pat))
			{
				res = &true_value;
				if (out == NULL)
					return res;
			buf = _T("  .");
			ps = _T("()");
				buf += name;
			if (pMethodInfo->GetNumParams())
				pMethodInfo->GetParamString(ps);
			buf += ps;
			buf += _T("\n");
			out->puts(buf);
		}
	}
	}

	return res;
}

Value*
MSComObject::map_point_vf(Value** arg_list, int count)
{
	check_arg_count(mapPoint, 2, count+1);
	Point2 p = arg_list[0]->to_point2();
	POINT point = {p.x, p.y};
	ClientToScreen(GetParent(GetParent(m_parent->m_ax.m_hWnd)), &point);
	return new Point2Value(point.x, point.y);
}

Value*
MSComObject::get_CursorPos_vf(Value** arg_list, int count)
{
	check_arg_count(getCursorPos, 1, count+1);
	POINT p;
	GetCursorPos(&p);
	if (m_parent && m_parent->m_ax.m_hWnd)
		ScreenToClient(m_parent->m_ax.m_hWnd, &p);
	return new Point2Value(p.x, p.y);
}

Value*
MSComObject::update_window_vf(Value** arg_list, int count)
{
	check_arg_count(updateWindow, 1, count+1);
	if (m_parent && m_parent->m_ax.m_hWnd)
		UpdateWindow(m_parent->m_ax.m_hWnd);
	return &ok;
}

Value*
MSComObject::show_events_vf(Value** arg_list, int count)
{
	// showEvents <max_object> ["prop_pat"] [to:<file>]
	CharStream*	out;
	TCHAR*		prop_pat = _T("*");
	Value*		val;
	BOOL		show_hidden = bool_key_arg(showHidden, val, FALSE);
	Value*		res = &false_value;
	
	// pick up args
	if (count >= 1 && arg_list[0] != &keyarg_marker)	
		prop_pat = arg_list[0]->to_string();	
	
	if ((out = (CharStream*)key_arg(to)) != (CharStream*)&unsupplied)
	{
		if (out == (CharStream*)&undefined)   // to:undefined -> no output, return true or false if prop found
			out = NULL;
		if (!is_charstream(out))
			throw TypeError (GetString(IDS_SHOWEVENTS_NEED_STREAM), out);
	}
	else
		out = thread_local(current_stdout);
	
	int num_props = m_infoEvents.GetNumMethods();
	CMethodInfo *pEventInfo;
	CParamInfo* pParamInfo = 0;
	CComVariant* pvarParam = 0;	
	TSTR buf, ps;

	for (int i=0; i < num_props; i++)
	{
		pEventInfo = m_infoEvents.GetMethod(i);
		INVOKEKIND ik = pEventInfo->GetInvokeKind();
		
		if (ik != INVOKE_FUNC) continue;
		TSTR name = pEventInfo->GetName();
		if (max_name_match(name, prop_pat))
		{
			res = &true_value;
			if (out == NULL)
				return res;
			buf.printf( _T("  on <control_name> %s"), name );
		pEventInfo->GetParamString(ps);
		buf += ps + _T(" do ( ... )\n");
		out->puts(buf);
	}
	}
	return res;
}

STDMETHODIMP MSComObject::Invoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult, EXCEPINFO* pexcepinfo, UINT* puArgErr)
{
	HRESULT	hr = S_OK;
	int cArgs = pdispparams->cArgs;

	if (!m_parent) return hr; // do nothing if no context is associated 
	
	// check for NULL
	if	(pdispparams)
	{
		CMethodInfo* mi = m_infoEvents.FindMethod(dispidMember);
		if (mi) 
		{
			init_thread_locals();
			push_alloc_frame();
			Value**		arg_list;
			Value**		ref_list;
			one_value_local(event);
			TSTR event_name = mi->GetName();
			vl.event = Name::intern(event_name);
			Value* handler = m_parent->get_event_handler(vl.event);
			// see if an handler exists for this event
			if ( handler != &undefined)
			{
#ifdef MAX_TRACE				
				DebugPrint(_T("--->Event Fired: %s\n"), event_name);
#endif
				SourceFileWrapper *sfw = (SourceFileWrapper*)handler;
				// make the event handler argument list
				value_local_array(arg_list, cArgs);
				value_local_array(ref_list, cArgs);
				for (int i=0; i < cArgs; i++)
				{
					int iParam = cArgs-i-1;
					HRESULT hr = mi->GetValue(&pdispparams->rgvarg[i], &arg_list[iParam], iParam);

					if (mi->GetParam(iParam)->m_flags & PARAMFLAG_FOUT)
					{
						ref_list[iParam] = arg_list[iParam];
						//arg_list[iParam] = new AxThunkReference(ref_list, iParam);
					}
					else
						ref_list[iParam] = NULL;

					if (FAILED(hr)) arg_list[i] = &undefined;
				}
				
				// call the event handler
				m_parent->run_event_handler(m_parent->parent_rollout, vl.event, arg_list, cArgs);
				
				// set the values back for byref params
				for (int i=0; i < cArgs; i++)
				{
					int iParam = cArgs-i-1;
					if (mi->GetParam(iParam)->m_flags & PARAMFLAG_FOUT)
					{
						Value* val = ref_list[iParam]->eval();
						if (val == &true_value) // exclusive for IE, MS sucks
						{
							pvarResult = &pdispparams->rgvarg[i];
							VariantInit(pvarResult);
							V_VT(pvarResult) |= VT_BOOL;
							V_BOOL(pvarResult) = VARIANT_TRUE;

						}
						else
						{
							// for all others
							pdispparams->rgvarg[i].vt &= ~VT_BYREF;
							mi->GetVariant(val, &pdispparams->rgvarg[i], iParam);
						}
					}
				}
				pop_value_local_array(ref_list);
				pop_value_local_array(arg_list);
			}
			pop_value_locals();
			pop_alloc_frame();
		}
	}
	else
		hr = DISP_E_PARAMNOTFOUND;

	return	hr;
}

STDMETHODIMP MSComObject::GetTypeInfoCount(UINT* pctinfo)
{
	return	E_NOTIMPL;
}

STDMETHODIMP MSComObject::GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo** pptinfo)
{
	return	E_NOTIMPL;
}

STDMETHODIMP MSComObject::GetIDsOfNames(REFIID riid, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgdispid)
{
	return	E_NOTIMPL;
}

STDMETHODIMP
CEventListener::QueryInterface(REFIID iid, void ** ppvObject)
{
	if ( advise_type == ADVISE_OTHER_STYLE && iid == m_infoEvents.GetIID())
	{
		AddRef();
		*ppvObject = this;
		return S_OK;
	}
	else return CComObject<MSComObject>::QueryInterface(iid, ppvObject);
}

HRESULT WINAPI 
CEventListener::CreateInstance(CEventListener** pp)
{
	ATLASSERT(pp != NULL);
	HRESULT hRes = E_OUTOFMEMORY;
	CEventListener* p = NULL;
	ATLTRY(p = new CEventListener())
	if (p != NULL)
	{
		p->SetVoid(NULL);
		p->InternalFinalConstructAddRef();
		hRes = p->FinalConstruct();
		p->InternalFinalConstructRelease();
		if (hRes != S_OK)
		{
			delete p;
			p = NULL;
		}
	}
	*pp = p;
	return hRes;
}

void
CEventListener::FreeAll()
{
	if (!m_parent || !m_sink_succeed) return;
	if (advise_type == ADVISE_MS_STYLE)
		SetupEvents(false);
}

bool
CEventListener::SetupEvents(bool advise)
{
	if (m_sink_succeed == advise) return m_sink_succeed; //nothing has to be done
	// setup connection point sink for event handling
	CComPtr<IUnknown> pUnkControl;
	if (!m_parent) return false;
	HRESULT hr = m_parent->m_ax.QueryControl(&pUnkControl);
	if( SUCCEEDED( hr ) )
	{
		if (advise)
		{
			m_dwCookie = 0;
			advise_type = ADVISE_MS_STYLE; // works for most of the standard controls
			m_sink_succeed = SUCCEEDED( AtlAdvise(pUnkControl, this, m_infoEvents.GetIID(), &m_dwCookie) );
			if (m_sink_succeed) return true;			
			
			advise_type = ADVISE_OTHER_STYLE; // works for controls like flash object, etc.
			m_sink_succeed = SUCCEEDED( AtlAdvise(pUnkControl, this, m_infoEvents.GetIID(), &m_dwCookie) );
			
		}
		else
		{
			if (m_sink_succeed && advise_type == ADVISE_MS_STYLE)
				AtlUnadvise(pUnkControl, m_infoEvents.GetIID(), m_dwCookie);
			m_sink_succeed = false;
		}
	}
	return m_sink_succeed;
}

Value*
CEventListener::setup_events_vf(Value** arg_list, int count)
{
	// setupEvents <max_object>
	check_arg_count(setupEvents, 2, count+1);
	return SetupEvents(arg_list[0]->to_bool() ? true : false) ? &true_value : &false_value;
	
}

#include "definsfn.h"

/////////////////////////////////////////////////////////////////////////////
// global funcs
/////////////////////////////////////////////////////////////////////////////
def_visible_primitive_debug_ok ( show_all_axcontrols, "ShowAllActiveXControls");
Value*
show_all_axcontrols_cf(Value** arg_list, int count)
{
	// showAllActiveXControls <max_object> [_T("prop_pat")] [to:<file>]
	CharStream*	out;
	TCHAR*		prop_pat = _T("*");		
	
	// pick up args, nothing is done with prop_pat
	if (count >= 1 && arg_list[0] != &keyarg_marker)
		prop_pat = arg_list[0]->to_string();
	
	if ((out = (CharStream*)key_arg(to)) != (CharStream*)&unsupplied)
	{
		if (!is_charstream(out))
			throw TypeError (GetString(IDS_SHOWALLAXCONTROLS_NEED_STREAM), out);
	}
	else
		out = thread_local(current_stdout);
	
	CComPtr<ICatInformation>	pCatInfo;
	CComPtr<IEnumGUID>			pEnum;
	HRESULT						hResult;
	CATID						catid = CATID_Control;
	bool						tDone = false;	
	CLSID						clsid;
	LPOLESTR					pszName, pszProgid, pszClsid;

	hResult = pCatInfo.CoCreateInstance(CLSID_StdComponentCategoriesMgr);
	if( FAILED( hResult ) ) throw RuntimeError( GetString(IDS_SHOWALLAXCONTROLS_FAILED) );

	hResult = pCatInfo->EnumClassesOfCategories( 1, &catid, ULONG( -1 ), NULL, &pEnum);

	USES_CONVERSION;
	while( !tDone )
	{
		hResult = pEnum->Next( 1, &clsid, NULL );
		if( hResult == S_OK )
		{
			pszName = NULL;
			hResult = OleRegGetUserType( clsid, USERCLASSTYPE_FULL, &pszName );
			if( SUCCEEDED( hResult ) )
			{
				ProgIDFromCLSID( clsid, &pszProgid );
				StringFromCLSID( clsid, &pszClsid );
				out->printf(_T("\"%s\" \"%s\" \"%s\" \n"), OLE2T(pszName), OLE2T(pszProgid), OLE2T(pszClsid));
				CoTaskMemFree( pszName );
				CoTaskMemFree( pszProgid );
				CoTaskMemFree( pszClsid );
				pszName = NULL;
				pszProgid = NULL;
				pszClsid = NULL;
			}
		}
		else
		{
			tDone = true;
		}
	}
	return &ok;
}

// this is to expose class generics as visible primitives until there is a better way 
#undef def_fwd_generic
#define def_fwd_generic(fn, name)													\
	def_visible_primitive(fn, name);												\
	Value* fn##_cf(Value** arg_list, int count)										\
	{																				\
		Value*  result;																\
		Value**	evald_args;															\
		Value	**ap, **eap;														\
		int		i;																	\
		if (count < 1)																\
			throw ArgCountError("Generic apply", 1, count);							\
		value_local_array(evald_args, count);										\
		for (i = count, ap = arg_list, eap = evald_args; i--; eap++, ap++)			\
			*eap = (*ap)->eval();													\
		if (is_activexcontrol(evald_args[0]))										\
			result = ((ActiveXControl*)evald_args[0])->##fn##_vf(&evald_args[1], count - 1); \
		else if (is_com_dispatch(evald_args[0]))										\
			result = ((MSDispatch*)evald_args[0])->##fn##_vf(&evald_args[1], count - 1); \
		else throw NoMethodError (name, evald_args[0]);								\
		pop_value_local_array(evald_args);											\
		return result;																\
	}
#undef def_fwd_generic_debug_ok
#define def_fwd_generic_debug_ok(fn, name)											\
	def_visible_primitive_debug_ok(fn, name);										\
	Value* fn##_cf(Value** arg_list, int count)										\
	{																				\
		Value*  result;																\
		Value**	evald_args;															\
		Value	**ap, **eap;														\
		int		i;																	\
		if (count < 1)																\
			throw ArgCountError("Generic apply", 1, count);							\
		value_local_array(evald_args, count);										\
		for (i = count, ap = arg_list, eap = evald_args; i--; eap++, ap++)			\
			*eap = (*ap)->eval();													\
		if (is_activexcontrol(evald_args[0]))										\
			result = ((ActiveXControl*)evald_args[0])->##fn##_vf(&evald_args[1], count - 1); \
		else if (is_com_dispatch(evald_args[0]))										\
			result = ((MSDispatch*)evald_args[0])->##fn##_vf(&evald_args[1], count - 1); \
		else throw NoMethodError (name, evald_args[0]);								\
		pop_value_local_array(evald_args);											\
		return result;																\
	}

#	include "compro.h"

def_visible_primitive			( load_picture,		"loadPicture");

def_struct_primitive			( add_child,		windows,			"addChild");

def_struct_primitive			( hittest,			listview,			"hitTest");
def_struct_primitive			( get_indent,		listview,			"getIndent");
def_struct_primitive			( set_indent,		listview,			"setIndent");

Value*
load_picture_cf(Value** arg_list, int count)
{
	check_arg_count(loadPicture, 1, count);
	
	TSTR fn = arg_list[0]->to_filename();

	TCHAR  path[MAX_PATH];
	TCHAR* fpart;

	IPathConfigMgr* path_mgr = IPathConfigMgr::GetPathConfigMgr();
	const TCHAR* scripts_dir = path_mgr->GetDir(APP_SCRIPTS_DIR);
	const TCHAR* userScripts_dir = path_mgr->GetDir(APP_USER_SCRIPTS_DIR);

	const TCHAR* image_dir = path_mgr->GetDir(APP_IMAGE_DIR);

	const TCHAR* startup_dir = path_mgr->GetDir(APP_STARTUPSCRIPTS_DIR);
	const TCHAR* userStartup_dir = path_mgr->GetDir(APP_USER_STARTUPSCRIPTS_DIR);

	const TCHAR* icon_dir = GetColorManager()->GetIconFolder();
	const TCHAR* userIcon_dir = path_mgr->GetDir(APP_USER_ICONS_DIR);

	TSTR maxSysIconFolder = path_mgr->GetDir(APP_MAX_SYS_ROOT_DIR); // maxroot
	BMMAppendSlash(maxSysIconFolder);
	maxSysIconFolder.Append(_T("UI\\Icons"));

	TCHAR source_dir[MAX_PATH] = _T("");
	if ( thread_local(source_file) )
		BMMSplitFilename(thread_local(source_file)->to_string(), source_dir, NULL, NULL);

	BOOL found = MSZipPackage::search_current_package(fn, path) ||
		SearchPath(source_dir, fn, NULL, MAX_PATH, path, &fpart) ||
		SearchPath(userIcon_dir, fn, NULL, MAX_PATH, path, &fpart) ||
		SearchPath(icon_dir, fn, NULL, MAX_PATH, path, &fpart) ||
		SearchPath(maxSysIconFolder, fn, NULL, MAX_PATH, path, &fpart) ||
		SearchPath(userStartup_dir, fn, NULL, MAX_PATH, path, &fpart) ||
		SearchPath(userScripts_dir, fn, NULL, MAX_PATH, path, &fpart) ||
		SearchPath(startup_dir, fn, NULL, MAX_PATH, path, &fpart) ||
		SearchPath(scripts_dir, fn, NULL, MAX_PATH, path, &fpart) ;
	if (!found)
		for (int i = 0; i < TheManager->GetMapDirCount(); i++){
			found = SearchPath(TheManager->GetMapDir(i), fn, NULL, MAX_PATH, path, &fpart);
			if (found == TRUE)
				break;
		}
	if (!found)
		found = SearchPath(image_dir, fn, NULL, MAX_PATH, path, &fpart) ||
		SearchPath(NULL, fn, NULL, MAX_PATH, path, &fpart);

	if (found)
	{
		LPDISPATCH picDisp;
		VARIANT var;
		V_VT(&var) = VT_BSTR;	

		USES_CONVERSION;
		V_BSTR(&var) = T2BSTR(path);

		HRESULT hr = OleLoadPictureFile(var, &picDisp);
		if (SUCCEEDED(hr)) 
			return new MSDispatch(picDisp, _T("IPictureDisp"));
	}
	return &undefined;
}


Value*
hittest_cf(Value** arg_list, int count)
{
	check_arg_count(hitTest, 2, count);
	one_typed_value_local(Array* result);
	vl.result = new Array (2);

	LVHITTESTINFO info;
	memset(&info, 0, sizeof(LVHITTESTINFO));
	Point2 p = arg_list[1]->to_point2();
	info.pt.x = (int)p.x; info.pt.y = (int)p.y;

	HRESULT hRes = SendMessage((HWND)arg_list[0]->to_intptr(), LVM_SUBITEMHITTEST, 0, (LPARAM)&info);
	vl.result->append(Integer::intern(info.iItem + 1));
	vl.result->append(Integer::intern(info.iSubItem));

	return_value(vl.result);
}

Value*
get_indent_cf(Value** arg_list, int count)
{
	check_arg_count(getIndent, 2, count);

	LV_ITEM item;
	memset(&item, 0, sizeof(LV_ITEM));
	item.mask = LVIF_INDENT;
	item.iItem = arg_list[1]->to_int() - 1; //convert from 1-based to 0-based index	

	HRESULT hRes = SendMessage((HWND)arg_list[0]->to_intptr(), LVM_GETITEM, 0, (LPARAM)&item);
	return Integer::intern(item.iIndent);
}

Value*
set_indent_cf(Value** arg_list, int count)
{
	check_arg_count(setIndent, 3, count);

	LV_ITEM item;
	memset(&item, 0, sizeof(LV_ITEM));
	item.mask = LVIF_INDENT;
	item.iItem = arg_list[1]->to_int() - 1; //convert from 1-based to 0-based index
	item.iIndent = arg_list[2]->to_int();

	HRESULT hRes = SendMessage((HWND)arg_list[0]->to_intptr(), LVM_SETITEM, 0, (LPARAM)&item);
	return Integer::intern(hRes);
}

Value*
add_child_cf(Value** arg_list, int count)
{
	check_arg_count(sendMessage, 2, count);

#ifdef USE_GMAX_SECURITY
	NOT_SUPPORTED_BY_PRODUCT("addChild");
#else

	Value* parent = arg_list[0];
	if (!is_activexcontrol(parent))
		throw TypeError (_T("addChild needs a activeX control as arg, got: "), parent);
	HWND hRes = SetParent(
		(HWND)arg_list[1]->to_intptr(),
		(HWND)( ((ActiveXControl*)parent))->hwndContainer);
	return hRes ? &true_value : &false_value ;
#endif // USE_GMAX_SECURITY
}



