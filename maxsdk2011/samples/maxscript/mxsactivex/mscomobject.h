/**********************************************************************
 *<
	FILE:			MSComObject.h

	DESCRIPTION:	ActiveX control creation

	CREATED BY:		Ravi Karra

	HISTORY:		Started on Nov.3rd 1999

 *>	Copyright (c) 1997, All Rights Reserved.
 **********************************************************************/

#ifndef __MSCOMOBJECT__H
#define __MSCOMOBJECT__H

#define _ATL_DEBUG_QI

#include "resource.h"
#include "MethInfo.h"
#include "ExcepInf.h"

#include <maxscript\macros\local_class.h>

#include <buildver.h>

extern TCHAR *GetString(int id);
extern HINSTANCE hInstance;
class MSComObject;
class ActiveXControl;
class MSComObject;


#define def_fwd_generic(fn, name)	def_local_generic (fn, name)
#define def_fwd_generic_debug_ok(fn, name)	def_local_generic_debug_ok (fn, name)
#define ADVISE_MS_STYLE				0
#define ADVISE_OTHER_STYLE			1

#ifdef USE_GMAX_SECURITY	// russom - 05/15/01
	#define USE_ACTIVEX_CONTROL_FILTER
#endif

const IID LIBID_MXSActiveX = {0x7FA22CB1,0xD26F,0x11d0,{0xB2,0x60,0x00,0x60,0x08,0x1C,0x25,0x7E}};

/* 	
	This class represents any type of com object(ActiveX or not). It stores a list 
	of methods, properties and events supported by the com object. It builds the 
	list from the type information passed to it. It also sinks a connection point 
	to the actual com object to listen to events. If this class exists in a MaxScript 
	context(RCMenu, Rollout, plugin, tool...) then event handlers(if any) from 
	the context are called when ever an event is fired by the com object.
*/

/////////////////////////////////////////////////////////////////////////////
// MSComObject
class ATL_NO_VTABLE MSComObject :
	public CComObjectRootEx<CComSingleThreadModel>,	
	public IDispatch
{
public:	
				MSComObject() : m_parent(NULL), m_sink_succeed(false) { }
				virtual ~MSComObject() { }	
	
	HRESULT		Init(ITypeInfo* ti, ActiveXControl* iparent = NULL, bool idefault=false);
	void		AxError(TCHAR* cap, TCHAR* got=NULL);
	Value*		get_property(Value** arg_list, int count);
	Value*		set_property(Value** arg_list, int count);

#include <maxscript\macros\local_implementations.h>
#	include "compro.h"
	def_local_generic_debug_ok ( show_props,		"showProperties");
	def_local_generic_debug_ok ( get_props,			"getPropNames");
	def_local_generic_debug_ok ( show_methods,		"showMethods");
	def_local_generic_debug_ok ( show_events,		"showEvents");
	
	DECLARE_NOT_AGGREGATABLE ( MSComObject)

	DECLARE_PROTECT_FINAL_CONSTRUCT ()

	BEGIN_COM_MAP ( MSComObject)
		COM_INTERFACE_ENTRY ( IDispatch)
	END_COM_MAP ()
	
	STDMETHOD ( GetTypeInfoCount)(UINT* pctinfo);
	STDMETHOD ( GetTypeInfo)(UINT itinfo, LCID lcid, ITypeInfo** pptinfo);
	STDMETHOD ( GetIDsOfNames)(REFIID riid, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgdispid);
	STDMETHOD ( Invoke)(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult, EXCEPINFO* pexcepinfo, UINT* puArgErr);

	CComDispatchDriver	m_pdisp;
	CInterfaceInfo		m_infoEvents;
	CInterfaceInfo		m_infoMethods;
	CComPtr<ITypeInfo>	m_spTypeInfo;
protected:
	bool				m_sink_succeed;	// indicates if a connection point could be established
	DWORD				m_dwCookie;
	ActiveXControl*		m_parent;		// can be NULL, in case of nested objects	
};

class CEventListener : public CComObject<MSComObject>
{
public:
	static HRESULT WINAPI CreateInstance(CEventListener** pp);
	STDMETHOD(QueryInterface)(REFIID iid, void ** ppvObject);

	void		FreeAll();
	bool		SetupEvents(bool advise);
	Value*		setup_events_vf(Value** arg_list, int count);
	int			advise_type;
};

#undef def_fwd_generic
#define def_fwd_generic(fn, name)							\
	def_local_generic (fn, name)							\
	{ return (m_comObject) ? m_comObject->##fn##_vf(arglist, arg_count) : &ok; }

#undef def_fwd_generic_debug_ok
#define def_fwd_generic_debug_ok(fn, name)							\
	def_local_generic_debug_ok (fn, name)							\
{ return (m_comObject) ? m_comObject->##fn##_vf(arglist, arg_count) : &ok; }

/*	
	MaxScript wrapper for ActiveX controls, all method invocations are forwarded
    to the encapsulated com object
*/ 
/////////////////////////////////////////////////////////////////////////////
// ActiveXControl	
local_visible_class (ActiveXControl);
class ActiveXControl : public RolloutControl
{
	RECT		rect;
#ifdef USE_ACTIVEX_CONTROL_FILTER
	int			m_iControlFilter;
#endif

public:
				ActiveXControl(Value* name, Value* caption, Value** keyparms, int keyparm_count);
				~ActiveXControl();
	static RolloutControl* create(Value* name, Value* caption, Value** keyparms, int keyparm_count)
							{ return new ActiveXControl (name, caption, keyparms, keyparm_count); }
	void		gc_trace();
#	define		is_activexcontrol(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == class_tag(ActiveXControl))
				classof_methods (ActiveXControl, RolloutControl);
	void		collect() { delete this; }
	void		sprin1(CharStream* s) { s->printf(_T("ActiveXControl:%s"), name->to_string()); }

	void		add_control(Rollout *ro, HWND parent, HINSTANCE hInstance, int& current_y);				
	LPCTSTR		get_control_class() { return _T(""); }
	void		compute_layout(Rollout *ro, layout_data* pos) { }
	BOOL		handle_message(Rollout *ro, UINT message, WPARAM wParam, LPARAM lParam);
	void		set_enable();
	BOOL		set_focus();
	
	Value*		get_property(Value** arg_list, int count);
	Value*		set_property(Value** arg_list, int count);	

	void		check_hr_error(HRESULT hr);

// operations
#include <maxscript\macros\local_implementations.h>
#	include "compro.h"
	def_fwd_generic_debug_ok ( show_props,		"showProperties");
	def_fwd_generic_debug_ok ( get_props,		"getPropNames");
	def_fwd_generic_debug_ok ( show_methods,	"showMethods");
	def_fwd_generic_debug_ok ( show_events,		"showEvents");

public:
	CAxWindow		m_ax;
	CEventListener	*m_comObject;
	bool			m_releaseOnClose;
	HWND			hwndContainer;
};

#endif // __MSCOMOBJECT__H