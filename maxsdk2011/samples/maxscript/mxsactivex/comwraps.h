/**********************************************************************
 *<
	FILE:			comwraps.h

	DESCRIPTION:	MaxScript wrappers for COM data types

	CREATED BY:		Ravi Karra

	HISTORY:		Started on Nov.3rd 1999

 *>	Copyright (c) 1997, All Rights Reserved.
 **********************************************************************/

#ifndef __COMWRAPS__H
#define __COMWRAPS__H

#include <maxscript\macros\local_class.h>
#include "MSComObject.h"

/*	
	MaxScript wrapper for COM IDispatch, all method invocations are forwarded
    to the encapsulated com object
*/
/////////////////////////////////////////////////////////////////////////////
// MSDispatch	
local_visible_class(MSDispatch);
class MSDispatch : public Value
{
public:
				MSDispatch(IDispatch* ipdisp, TCHAR* iname, ITypeInfo* ti=NULL);
				MSDispatch() { }
				~MSDispatch();

#	define		is_com_dispatch(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == class_tag(MSDispatch))
	IDispatch*	to_idispatch() { return (m_comObject) ? m_comObject->m_pdisp.p : NULL; }
				classof_methods (MSDispatch, Value);
	BOOL		_is_collection() { return m_isCollection; }
	void		collect() { delete this; }
	void		sprin1(CharStream* s) { s->printf(m_name); }
	void		gc_trace();

	Value*		map(node_map& m);
	Value*		get_property(Value** arg_list, int count);
	Value*		set_property(Value** arg_list, int count);

#include <maxscript\macros\local_implementations.h>
#	include "compro.h"
	def_generic		(get,		"get");
	def_generic		(put,		"put");
	def_fwd_generic ( show_props,		"showProperties");
	def_fwd_generic ( get_props,		"getPropNames");
	def_fwd_generic	( show_methods,		"showMethods");
	def_fwd_generic	( show_events,		"showEvents");

private:
	CEventListener*		m_comObject;
	TSTR				m_name;
	BOOL				m_isCollection;
	CComPtr<ITypeInfo>	m_pItemTypeInfo; // only if collection
};

/*	
	MaxScript wrapper for COM functions 
*/
/////////////////////////////////////////////////////////////////////////////
// MSComMethod	
local_visible_class (MSComMethod)
class MSComMethod : public Function
{
public:
	MSComObject	*m_comObject;
    CMethodInfo *m_pMethodInfo;
	DISPID		m_id;
	MSDispatch	*m_dispatch; // LAM - 3/10/03 - defect 468513

				MSComMethod() { }
				MSComMethod(TCHAR* name, DISPID id, MSComObject* com_obj, CMethodInfo *pMethodInfo=NULL);

#	define		is_com_method(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == class_tag(MSComMethod))
				classof_methods (MSComMethod, Function);
	void		collect() { delete this; }
	void		gc_trace();

	Value*		apply(Value** arglist, int count, CallContext* cc);
};


#endif // __COMWRAPS__H