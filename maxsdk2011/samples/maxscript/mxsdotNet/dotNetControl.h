//**************************************************************************/
// Copyright (c) 1998-2006 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//**************************************************************************/
// DESCRIPTION: dotNetControl.h  : MAXScript dotNetControl header
// AUTHOR: Larry.Minton - created Jan.1.2006
//***************************************************************************/
//

#pragma once

#include "DotNetObjectWrapper.h"
#include "mxsCDotNetHostWnd.h"
#include "DotNetClassManaged.h"
#include "DotNetObjectManaged.h"
#include "DotNetHashTables.h"

#define LIFETIME_CONTROLLED_BY_DOTNET_FLAG VALUE_FLAGBIT_0

// gets rid of build warnings
class MSPlugin {};
class Struct {};

#ifdef ScripterExport
	#undef ScripterExport
#endif
#define ScripterExport __declspec( dllexport )

namespace MXS_dotNet
{
class dotNetControl;
class dotNetObject;
class dotNetMethod;
class dotNetClass;
class dotNetMXSValue;

/* -------------------- dotNetControl  ------------------- */
// 
#pragma unmanaged

visible_class (dotNetControl)

class dotNetControl : public RolloutControl
{
private:
	mxsCDotNetHostWnd	*m_pDotNetHostWnd;   // To host .NET control
	MSTR			m_typeName;
public:

	dotNetControl(Value* name, Value* caption, Value** keyparms, int keyparm_count);

	~dotNetControl();

	static	RolloutControl* create(Value* name, Value* caption, Value** keyparms, int keyparm_count)
	{ return new dotNetControl (name, caption, keyparms, keyparm_count); }

	void		gc_trace();
#	define		is_dotNetControl(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == class_tag(dotNetControl))
	classof_methods (dotNetControl, RolloutControl);
	void		collect() { delete this; }
	void		sprin1(CharStream* s);

	DotNetObjectWrapper* GetDotNetObjectWrapper() { return m_pDotNetHostWnd; }

	void		add_control(Rollout *ro, HWND parent, HINSTANCE hInstance, int& current_y);
	LPCMSTR		get_control_class() { return _M("STATIC"); }
	BOOL		handle_message(Rollout *ro, UINT message, WPARAM wParam, LPARAM lParam);
	void		set_enable();
	BOOL		set_focus();

	Value*		get_property(Value** arg_list, int count);
	Value*		set_property(Value** arg_list, int count);

	// operations
	def_generic_debug_ok	( show_props,			"showProperties");
	def_generic_debug_ok	( get_props,			"getPropNames");
	def_generic_debug_ok	( show_methods,			"showMethods");
	def_generic_debug_ok	( show_events,			"showEvents");
/*
// see comments in utils.cpp
	def_generic_debug_ok	( show_interfaces,		"showInterfaces");
	def_generic_debug_ok	( get_interfaces,		"getInterfaces");
*/
};

/* -------------------- dotNetBase  ------------------- */
// 
#pragma unmanaged

// define base class to be used by dotNetObject and dotNetClass
class dotNetBase : public Value
{
protected:
	dotNetBase();
	~dotNetBase();
#ifdef _DEBUG
	static int instanceCounter;
#endif

protected:
	MSTR		m_typeName;
	HashTable	*m_pEventHandlers;
public:
	static dotNetBaseDotNetLifetimeControlledHashTable	*m_pDotNetLifetimeControlledValues;
	virtual DotNetObjectWrapper* GetDotNetObjectWrapper() = 0;
#	define		is_dotNetBase(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == class_tag(dotNetObject) || (v)->tag == class_tag(dotNetClass))
	virtual	Value* DefaultShowStaticOnly() { return &false_value; }

	void		gc_trace();

	Value*		get_property(Value** arg_list, int count);
	Value*		set_property(Value** arg_list, int count);

	// operations
	def_generic_debug_ok	( show_props,			"showProperties");
	def_generic_debug_ok	( get_props,			"getPropNames");
	def_generic_debug_ok	( show_methods,			"showMethods");
	def_generic_debug_ok	( show_events,			"showEvents");
/*
// see comments in utils.cpp
	def_generic_debug_ok	( show_interfaces,		"showInterfaces");
	def_generic_debug_ok	( get_interfaces,		"getInterfaces");
*/

	Value*		get_event_handlers(Value* eventName);
	void		add_event_handler(Value* eventName, Value* handler);
	void		remove_event_handler(Value* eventName, Value* handler);
	void		remove_all_event_handlers(Value* eventName);
	void		remove_all_event_handlers();
	void		set_lifetime_controlled_by_dotnet(bool lifetime_controlled_by_dotnet);
	bool		get_lifetime_controlled_by_dotnet() { return (flags3 & LIFETIME_CONTROLLED_BY_DOTNET_FLAG) != 0; }
};

/* -------------------- dotNetObject  ------------------- */
// 
#pragma unmanaged

applyable_class_debug_ok (dotNetObject);

class dotNetObject : public dotNetBase
{
private:
	DotNetObjectManaged    *m_pDotNetObject;   // To host .NET object
public:

	// use DotNetObjectManaged::intern to wrap existing System::Object vals
	// use DotNetObjectManaged::create to create and wrap new System::Object vals
	dotNetObject(); 
	~dotNetObject();

#	define		is_dotNetObject(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == class_tag(dotNetObject))
	classof_methods (dotNetObject, Value);
	void		collect();

	void		sprin1(CharStream* s);

	use_generic( eq,		"=");
	use_generic( ne,		"!=");

	Value*  copy_vf(Value** arg_list, int count);

	DotNetObjectWrapper* GetDotNetObjectWrapper() { return m_pDotNetObject; }
	void SetObject(DotNetObjectManaged* pDotNetObject);
};

/* -------------------- dotNetClass  ------------------- */
// 
#pragma unmanaged

applyable_class_debug_ok (dotNetClass);

class dotNetClass : public dotNetBase
{
private:
	DotNetClassManaged		*m_pDotNetClass;   // To host .NET object
public:

	// use DotNetObjectManaged::intern to wrap existing System::Object vals
	// use DotNetObjectManaged::create to create and wrap new System::Object vals
	dotNetClass(); 
	~dotNetClass();

#	define		is_dotNetClass(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == class_tag(dotNetClass))
	classof_methods (dotNetClass, Value);
	void		collect() { delete this; }

	void		sprin1(CharStream* s);

	DotNetObjectWrapper* GetDotNetObjectWrapper() { return m_pDotNetClass; }
	Value* DefaultShowStaticOnly() { return &true_value; }
	void SetDotNetClass(DotNetClassManaged* pDotNetClass);
};

/* -------------------- DotNetMethod  ------------------- */
// 
#pragma managed

// TODO: need a cache of methods. Doing this will require the wrapper based on the System::Type
// (see comments on DotNetObjectWrapper::get_property
public class DotNetMethod
{
public:

	DotNetMethod::DotNetMethod(System::String ^ pMethodName, System::Type ^ pWrapperType);
	static dotNetMethod* intern(System::String ^ pMethodName, DotNetObjectWrapper *pWrapper, System::Type ^ pObjectType);
	Value*		apply(DotNetObjectWrapper *pWrapper, Value** arglist, int count, bool asDotNetObject);

	ref class delegate_proxy_type;
	msclr::delegate_map::internal::delegate_proxy_factory<DotNetMethod> m_delegate_map_proxy;

	ref class delegate_proxy_type
	{
		DotNetMethod* m_p_native_target;
		// the method name 
		System::String ^ m_p_MethodName;
		// the type the method is associated with
		System::Type ^ m_p_ObjectType;
		// the owning MXS dotNetMethod
		dotNetMethod *m_p_dotNetMethod;

	public:
		delegate_proxy_type(DotNetMethod* pNativeTarget) : m_p_native_target(pNativeTarget), m_p_dotNetMethod(NULL) {}
		void detach();

		System::String ^ get_MethodName() { return m_p_MethodName; }
		void set_MethodName(System::String ^ name) { m_p_MethodName = name; }
		System::Type ^ get_type() { return m_p_ObjectType; }
		void set_type(System::Type ^ type) { m_p_ObjectType = type; }
		dotNetMethod* get_dotNetMethod() { return m_p_dotNetMethod; }
		void set_dotNetMethod(dotNetMethod *_dotNetMethod) { m_p_dotNetMethod = _dotNetMethod; }
	};
};

/* -------------------- dotNetMethod  ------------------- */
// 
#pragma unmanaged

visible_class (dotNetMethod)

class dotNetMethod : public Function
{
private:
	DotNetObjectWrapper	*m_pWrapper;
	DotNetMethod		*m_pMethod;
public:

	dotNetMethod(MCHAR* pMethodName, DotNetObjectWrapper* pWrapper);
	~dotNetMethod();

	void		gc_trace();
#	define		is_dotNetMethod(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == class_tag(dotNetMethod))
	classof_methods (dotNetMethod, Function);
	void		collect() { delete this; }

	Value*		apply(Value** arglist, int count, CallContext* cc);
	Value*		get_vf(Value** arg_list, int count);
	Value*		put_vf(Value** arg_list, int count);

	void		SetMethod(DotNetMethod *pMethod);
};

/* -------------------- DotNetMXSValue  ------------------- */
// 
#pragma managed

public class DotNetMXSValue
{
public:

	DotNetMXSValue::DotNetMXSValue(dotNetMXSValue *p_dotNetMXSValue);

	ref class delegate_proxy_type;
	msclr::delegate_map::internal::delegate_proxy_factory<DotNetMXSValue> m_delegate_map_proxy;

	ref class DotNetMXSValue_proxy
	{
		delegate_proxy_type ^ m_p_delegate_proxy_type;
	public:
		DotNetMXSValue_proxy(delegate_proxy_type ^ p_delegate_proxy_type) : m_p_delegate_proxy_type(p_delegate_proxy_type) {}
		delegate_proxy_type ^ get_proxy() { return m_p_delegate_proxy_type; }
		Value* get_value();
	};

	ref class delegate_proxy_type
	{
		DotNetMXSValue* m_p_native_target;
		// the owning MXS dotNetMXSValue
		dotNetMXSValue *m_p_dotNetMXSValue;
		// the weak reference that points at the object can be used by .net objects
		System::WeakReference^ m_pDotNetMXSValue_proxy_weakRef;

	public:
		delegate_proxy_type(DotNetMXSValue* pNativeTarget) : m_p_native_target(pNativeTarget), m_p_dotNetMXSValue(NULL) {}
		void detach();

		dotNetMXSValue* get_dotNetMXSValue() { return m_p_dotNetMXSValue; }
		void set_dotNetMXSValue(dotNetMXSValue *_dotNetMXSValue) { m_p_dotNetMXSValue = _dotNetMXSValue; }
		DotNetMXSValue_proxy ^ get_weak_proxy();
		bool is_alive();
	};
	dotNetMXSValue* Get_dotNetMXSValue();
	DotNetMXSValue_proxy ^ GetWeakProxy();
	bool IsAlive();
};

/* -------------------- dotNetMXSValue  ------------------- */
// 
#pragma unmanaged

applyable_class_debug_ok (dotNetMXSValue)

class dotNetMXSValue : public Value
{
private:
	dotNetMXSValue(Value *pMXSValue); // use intern

public:
	DotNetMXSValue		*m_pDotNetMXSValue;
	Value				*m_pMXSValue;
	static dotNetMXSValueHashTable	*m_pExisting_dotNetMXSValues;

	~dotNetMXSValue();
	static dotNetMXSValue* intern(Value *pMXSValue);
	Value*		get_value() { return m_pMXSValue; }

	void		gc_trace();
#	define		is_dotNetMXSValue(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == class_tag(dotNetMXSValue))
	classof_methods (dotNetMXSValue, Value);
	void		collect() { delete this; }
	void		sprin1(CharStream* s);

	Value*		get_property(Value** arg_list, int count);
	Value*		set_property(Value** arg_list, int count);

	// operations
	def_generic_debug_ok	( show_props,			"showProperties");
	def_generic_debug_ok	( get_props,			"getPropNames");
};

}  // end of namespace MXS_dotNet
