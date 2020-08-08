/**********************************************************************
 *<
	FILE:			MSComObject.cpp

	DESCRIPTION:	MaxScript wrappers for COM data types 

	CREATED BY:		Ravi Karra

	HISTORY:		Started on Nov.3rd 1999

 *>	Copyright (c) 1999, All Rights Reserved.
 **********************************************************************/

#include "stdafx.h"
#include "comwraps.h"
#include "maxobj.h"

/////////////////////////////////////////////////////////////////////////////
// MSDispatch
local_visible_class_instance (MSDispatch, "MSDispatch")

MSDispatch::MSDispatch(IDispatch* ipdisp, TCHAR* iname, ITypeInfo* ti)
{ 
	HRESULT hr;
	CComPtr<ITypeInfo> pTi;
	UINT cTi;
	
	tag = class_tag(MSDispatch);	
	m_comObject = NULL;
	m_name = iname;
	hr = CEventListener::CreateInstance(&m_comObject);
	if ( FAILED(hr) ) throw RuntimeError(GetString(IDS_CREATE_FAILED), iname);
	m_comObject->m_pdisp = ipdisp;
		
	if (!ti)
	{
		ipdisp->GetTypeInfoCount(&cTi);
		if(!cTi) return;
		hr = ipdisp->GetTypeInfo(0, LOCALE_USER_DEFAULT, &pTi);
	}
	else 
	{
		pTi = ti;
	}	
	m_comObject->Init(pTi);

	// determine if this interface is a collection
	CComBSTR bstrName;	unsigned int cNames=0;
	m_isCollection = (SUCCEEDED(pTi->GetNames(DISPID_NEWENUM, &bstrName, 1, &cNames)) && cNames);

	if (m_isCollection)
	{
		CMethodInfo* pMethodInfo = m_comObject->m_infoMethods.FindMethod((DISPID)DISPID_VALUE);
		if (pMethodInfo)
			m_pItemTypeInfo = pMethodInfo->GetTypeInfo();
	}
}

MSDispatch::~MSDispatch()
{
	if (m_comObject) { 
		m_comObject->FreeAll(); 
		delete m_comObject;
		m_comObject = NULL;
	}
}

void
MSDispatch::gc_trace()
{
	Value::gc_trace();
}

Value*
MSDispatch::get_property(Value** arg_list, int count)
{
	Value* val = m_comObject ? m_comObject->get_property(arg_list, count) : NULL;
	if (val && is_com_method(val))  // LAM - 2/10/03 - defect 468513
		((MSComMethod*)val)->m_dispatch = this;
	return (val) ? val : Value::get_property(arg_list, count);
}

Value*
MSDispatch::set_property(Value** arg_list, int count)
{
	Value* val = m_comObject ? m_comObject->set_property(arg_list, count) : NULL;
	return (val) ? val : Value::set_property(arg_list, count);
}

Value*
MSDispatch::get_vf(Value** arg_list, int count)
{
	
	Value* arg; 
	Value* result;
	int	  index = 0;

	check_arg_count(get, 2, count + 1);
	arg = arg_list[0];
	if (is_number(arg) && (index = arg->to_int()) < 0)
		throw RuntimeError (GetString(IDS_ARRAY_INDEX_MUST_BE_VE_NUMBER_GOT), arg);

	CMethodInfo *pMethodInfo = m_comObject ? m_comObject->m_infoMethods.FindMethod((DISPID)DISPID_VALUE) : NULL;
	
	if (!pMethodInfo || pMethodInfo->GetInvokeKind() != INVOKE_PROPERTYGET)
		throw RuntimeError(_T("No default property get exists"));

	if (pMethodInfo->GetNumParams() > 1) 
		return &undefined; //not handled

	VARIANT vIndex, vRet;
	HRESULT hr;
	VariantInit(&vIndex);
	if (is_number(arg))
	{
		V_VT(&vIndex) = VT_I4;	// SR FIXME64: VT_INT_PTR?
		V_I4(&vIndex) = index; //can be 0/1 based index
	}
	else
	{
		USES_CONVERSION;
		V_VT(&vIndex) = VT_BSTR;
		V_BSTR(&vIndex) = T2BSTR(arg->to_string());
	}	
	DISPPARAMS dispparamsArgs = {&vIndex, 0, 1, 0};
	hr = m_comObject->m_pdisp->Invoke(DISPID_VALUE, IID_NULL,
		LOCALE_USER_DEFAULT, DISPATCH_METHOD|DISPATCH_PROPERTYGET,
		&dispparamsArgs, &vRet, NULL, NULL);
	
	if (FAILED(hr))	
		result = &undefined;
	else
		pMethodInfo->GetValue(&vRet, &result);

	return_protected(result);
}

Value*
MSDispatch::put_vf(Value** arg_list, int count)
{
	Value* arg;
	int	  index = 0;

	check_arg_count(put, 3, count + 1);
	arg = arg_list[0];
	if (is_number(arg) && (index = arg->to_int()) < 0)
		throw RuntimeError (GetString(IDS_ARRAY_INDEX_MUST_BE_VE_NUMBER_GOT), arg);
	
	CMethodInfo *pMethodInfo = m_comObject ? m_comObject->m_infoMethods.FindMethod((DISPID)DISPID_VALUE) : NULL;
	if (!pMethodInfo || pMethodInfo->GetInvokeKind() != INVOKE_PROPERTYPUT)
		throw RuntimeError(_T("No default property put exists"));

	if (pMethodInfo->GetNumParams() > 1) 
		return &undefined; //not handled

	VARIANT* rgvarg = new VARIANT[2];
	VARIANT vRet;
	HRESULT hr;
	VariantInit(&rgvarg[0]);
	if (is_number(arg))
	{
		V_VT(&rgvarg[0]) = VT_I4;
		V_I4(&rgvarg[0]) = index; //can be 0/1 based index
	}
	else
	{
		USES_CONVERSION;
		V_VT(&rgvarg[0]) = VT_BSTR;
		V_BSTR(&rgvarg[0]) = T2BSTR(arg->to_string());
	}
	VariantInit(&rgvarg[1]);
	hr = pMethodInfo->GetVariant(arg_list[1], &rgvarg[1]);
	
	DISPPARAMS dispparamsNoArgs = {rgvarg, NULL, 2, 0};
	hr = m_comObject->m_pdisp->Invoke(DISPID_VALUE, IID_NULL,
		LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET,
		&dispparamsNoArgs, &vRet, NULL, NULL);
	
	delete [] rgvarg;
	return FAILED(hr) ? &undefined : arg_list[1];
}

Value*
MSDispatch::map(node_map& m)
{
	// map returns an array of all elements
	USES_CONVERSION;
	HRESULT hr;
	CComPtr<IEnumVARIANT> pEnum;	
	VARIANT var, cVar;
	VARIANT* pvar;
	ULONG cElements;
	ULONG cFetched;
	
	// if a property is set on the collection itself
	if (m.count > 1 && is_name(m.arg_list[1]))
		return set_property(m.arg_list, m.count);

	VariantInit(&var);
	VariantInit(&cVar);
	
	if (m_comObject == NULL) return &undefined;
	// try as a method call
	hr = m_comObject->m_pdisp.Invoke0(DISPID_NEWENUM, &var);	

	// if failed try as a property
	if (FAILED(hr))
		hr = m_comObject->m_pdisp.GetProperty(DISPID_NEWENUM, &var);
	if (FAILED(hr)) return &undefined;

	hr = var.punkVal->QueryInterface(&pEnum);
	if (FAILED(hr)) return &undefined;

	
	hr = m_comObject->m_pdisp.GetPropertyByName(T2OLE(_T("Count")), &cVar);
	if (FAILED(hr)) return &undefined;
	cElements = V_I4(&cVar);

	pvar = new VARIANT[cElements];
	hr = pEnum->Reset();
	if (FAILED(hr)) return &undefined;
	hr = pEnum->Next(cElements, pvar, &cFetched);
	if (FAILED(hr)) return &undefined;

	two_value_locals(index, result);
	save_current_frames();

	for (ULONG l=0; l < cFetched; l++)
		try
		{
			push_alloc_frame();
			if (&pvar[l])
			{
				vl.index = Integer::intern(l + 1);
				if (m.vfn_ptr != NULL)
					vl.result = (vl.index->*(m.vfn_ptr))(m.arg_list, m.count);
				else
				{
					// temporarily replace 1st arg with this node
					Value* arg_save = m.arg_list[0];

					m.arg_list[0] = &undefined;
					value_from_variant(&pvar[l], &m.arg_list[0], m_pItemTypeInfo);
					if (m.flags & NM_MXS_FN)
						vl.result = ((MAXScriptFunction*)m.cfn_ptr)->apply(m.arg_list, m.count);
					else
						vl.result = (*m.cfn_ptr)(m.arg_list, m.count);
					m.arg_list[0] = arg_save;
				}
				if (vl.result == &loopExit)
				{
					pop_alloc_frame();
					break;
				}
				if (m.collection != NULL && vl.result != &dontCollect)
					m.collection->append(vl.result);
			}
			vl.result = NULL; // LAM: 2/12/01 defect 276146 fix
			vl.index = NULL; 
			pop_alloc_frame();
		}
		catch (LoopContinue)
		{
			pop_alloc_frame();	// LAM - 5/18/01 - switched order since push_alloc_frame was done last
			restore_current_frames();
		}
		catch (...)
		{
			pop_alloc_frame();
			for (ULONG l=0; l < cFetched; l++)
				VariantClear(&pvar[l]);
			throw;
		}

	pop_value_locals();
	for (ULONG l=0; l < cFetched; l++)
        VariantClear(&pvar[l]);
	delete [] pvar;
	return &ok;
}

local_visible_class_instance (MSComMethod, _T("MSComMethod"))

MSComMethod::MSComMethod(TCHAR* iname, DISPID id, MSComObject* icom_obj, CMethodInfo* pMethodInfo)
{
	tag = class_tag(MSComMethod); // LAM - 3/10/03 - defect 468513
//	tag = INTERNAL_CLASS_TAG; 
	name = save_string(iname);
	m_comObject = icom_obj;
	m_pMethodInfo = pMethodInfo;
	m_id = id;
	m_dispatch = NULL;
}

void
MSComMethod::gc_trace()
{
	// scan sub-objects & mark me 
	Function::gc_trace();
	if (m_dispatch && m_dispatch->is_not_marked())
		m_dispatch->gc_trace(); // LAM - 2/10/03 - defect 468513
}

Value* 
MSComMethod::apply(Value** arg_list, int count, CallContext* cc)
{
	one_value_local(result);
    HRESULT		hr;
	
	VARIANTARG  vret;
	CExcepInfo  excepinfo;
	unsigned int argerr;
	DISPPARAMS  dispparams;    

	// initialize param struct & fill from evald_args 
    memset(&dispparams, 0, sizeof(dispparams));

	if (m_pMethodInfo && count > m_pMethodInfo->GetNumParams())
		throw RuntimeError(_T("Too many args for "), this);

	if (count > 0)
	{
		// stack allocate space for temp eval'd args, run through evaling them all
		Value**	evald_args;
		Value	**ap, **eap;
		int		i;
		value_local_array(evald_args, count);
		for (i = count, ap = arg_list, eap = evald_args; i--; eap++, ap++)
			*eap = (*ap)->eval();

        // allocate memory for all VARIANTARG parameters.
		dispparams.cArgs = count;
		VARIANTARG* pvArg = (VARIANTARG*)_alloca(count * sizeof(VARIANTARG));
		
		dispparams.rgvarg = pvArg;
        memset(pvArg, 0, sizeof(VARIANTARG) * count);

        // traverse args, converting to variants
        pvArg += count - 1;   // params go in opposite order.
		for (i = 0, eap = evald_args; i < count; eap++, pvArg--, i++)
		{
			if (m_pMethodInfo)
			{
				int iParam = count-i-1;
				
				VariantInit(pvArg);
				// mark byref params
				if ( (m_pMethodInfo->GetParam(iParam)->m_flags & PARAMFLAG_FOUT) &&	is_thunk(*eap) )
				{					
					pvArg->vt |= VT_BYREF;
					hr = m_pMethodInfo->GetVariant((*eap)->eval(), pvArg, iParam);
				}
				else
					hr = m_pMethodInfo->GetVariant(*eap, pvArg, iParam);
			}
			else
				hr = variant_from_value(*eap, pvArg);
			
			if (FAILED(hr))
				throw RuntimeError (_T("Unsupported arg for COM call, "), *eap);
		}
		pop_value_local_array(evald_args);
    }

	// initialize return value variant 
	VariantInit(&vret);

	// make the call.
    hr = m_comObject->m_pdisp->Invoke(m_id, IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_METHOD, 
		&dispparams, &vret, &excepinfo, &argerr);
    
	for (int i=0; i < count; i++)
	{
		
		VARTYPE type = dispparams.rgvarg[i].vt;
		if ( type & VT_BYREF )
		{
			Value* val;
			int iParam = count-i-1;
			m_pMethodInfo->GetValue(&dispparams.rgvarg[i], &val);
			arg_list[iParam]->eval()->to_thunk()->assign(val);
			if ( type & VT_BSTR ) 
				delete V_BSTRREF(&dispparams.rgvarg[i]);
		}
	}
	if(FAILED(hr))
        throw RuntimeError (_T("ComObject method call failed, "), this);
    
	// convert result to Value
	if (m_pMethodInfo)
		hr = m_pMethodInfo->GetValue(&vret, &vl.result);
	else
		hr = value_from_variant(&vret, &vl.result);
	
	if (FAILED(hr))
        throw RuntimeError (_T("Unsupported return type for function, "), name);

	return_value(vl.result);
}
