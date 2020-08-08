#include "StdAfx.H"
#include "VarUtils.h"
#include "comwraps.h"
#include "maxobj.h"

class VARTYPE_ENTRY
{
public:
	VARTYPE vt;
	LPCTSTR mxsName;   
};

static VARTYPE_ENTRY g_aVarTypes[] =
{
#ifdef __DEBUG
	{ VT_EMPTY,			_T( "empty"		) },
#endif
	{ VT_I2,			_T( "integer"	) },
	{ VT_I4,			_T( "integer"	) },
	{ VT_R4,			_T( "float"		) },
	{ VT_R8,			_T( "double"	) },
#ifdef __DEBUG
	{ VT_CY				_T( "float"		) },
	{ VT_DATE,			_T( "date"		) },
#endif
	{ VT_BSTR,			_T( "string"	) },
	{ VT_DISPATCH,		_T( "MSDispatch") },
#ifdef __DEBUG
	{ VT_ERROR,			_T( "error"		) },
#endif
	{ VT_BOOL,			_T( "boolean"	) },
#ifdef __DEBUG
	{ VT_VARIANT,		_T( "variant"	) },
	{ VT_UNKNOWN,		_T( "IUnknown"	) },
	{ VT_DECIMAL,		_T( "decimal"	) },
#endif
	{ VT_ARRAY | VT_VARIANT,		_T( "safearraywrapper"	) },
	{ VT_I1,			_T( "integer"	) },
	{ VT_UI1,			_T( "integer"	) },
	{ VT_UI2,			_T( "integer"	) },
	{ VT_UI4,			_T( "integer"	) },
	{ VT_I8,			_T( "integer64") },
	{ VT_UI8,			_T( "integer64") },
	{ VT_INT,			_T( "integer"  ) },
	{ VT_INT_PTR,		_T( "integerptr"  ) },
	{ VT_UINT,			_T( "integer"	) },
	{ VT_UINT_PTR,		_T( "integerptr"  ) },

#ifdef __DEBUG
	{ VT_VOID,			_T( "void"		) },
#endif
	{ VT_HRESULT,		_T( "hresult"	) },
	{ VT_PTR,			_T( "ptr"		) },
	{ VT_USERDEFINED,	_T( "userdefined")},
	{ VT_LPSTR,			_T( "string"	) },
	{ VT_LPWSTR,		_T( "string"	) },
	{ VT_COLOR,			_T( "color"		) },
	{ VT_FONT,			_T( "font"		) },
};

const int NUM_VARTYPES = sizeof( g_aVarTypes )/sizeof( g_aVarTypes[0] );

void VTToMxsString( VARTYPE vt, TSTR& str)
{
	str = _T("");
	for( int iType = 0; iType < NUM_VARTYPES; iType++ )
	{
		if( (g_aVarTypes[iType].vt == vt) ||  ((g_aVarTypes[iType].vt | VT_BYREF) == vt) )
		{
			str =  g_aVarTypes[iType].mxsName;
			return;
		}
	}
	str = _T("undefined");
}

HRESULT
value_from_variant(VARIANTARG* var, Value** pval, ITypeInfo* pTypeInfo, Value* progID)
{
	USES_CONVERSION;
	HRESULT hr = S_OK;
	bool handled = false;

	DWORD type = V_VT(var);
	BOOL isSafeArray = type & VT_ARRAY;
	BOOL isVector = type & VT_VECTOR;
	BOOL isRef = type & VT_BYREF;

	if (pTypeInfo != NULL)
	{
		CSmartTypeAttr pta(pTypeInfo);
		hr = pTypeInfo->GetTypeAttr(&pta);
		if (SUCCEEDED(hr))
		{
			if(pta->typekind == TKIND_ENUM)
			{
				VARDESC* pvd=NULL;			
				for (int i=0; i<pta->cVars; i++)
				{
					pTypeInfo->GetVarDesc(i, &pvd);
					DISPID idMember = pvd->memid;
					CComBSTR bstrName;
					pTypeInfo->GetDocumentation(idMember, &bstrName, NULL, NULL, NULL);
					if (pvd->lpvarValue->lVal == var->lVal)
					{
						*pval = Name::intern(OLE2T(bstrName));
						pTypeInfo->ReleaseVarDesc(pvd);
						handled = true;
						break;
					}
					pTypeInfo->ReleaseVarDesc(pvd);
				}
			}
			else if (pta->typekind == TKIND_DISPATCH)
			{
				if (V_VT(var) == VT_DISPATCH && V_DISPATCH(var))
				{
					CComBSTR bstrName;
					pTypeInfo->GetDocumentation(MEMBERID_NIL, &bstrName, NULL, NULL, NULL);
					*pval = new MSDispatch (V_DISPATCH(var), OLE2T(bstrName), pTypeInfo);
					handled = true;
				}
				else
					*pval = &undefined;
			}
			else if (pta->typekind == TKIND_COCLASS && pta->cImplTypes >0)
			{
				CComPtr<ITypeInfo> pTI;
				CComPtr<IDispatch> pDisp;
				HREFTYPE hRefType;				

				// get the first interface
				pTypeInfo->GetRefTypeOfImplType(0, &hRefType);
				pTypeInfo->GetRefTypeInfo(hRefType, &pTI);

				if (V_VT(var) == VT_DISPATCH)
					pDisp = V_DISPATCH(var);
				else
					pTI->QueryInterface(&pDisp);
				
				if (pDisp.p)
				{
					CComBSTR bstrName;
					pTypeInfo->GetDocumentation(MEMBERID_NIL, &bstrName, NULL, NULL, NULL);
					*pval = new MSDispatch (pDisp, OLE2T(bstrName), pTI);
					handled = true;
				}
				else
					*pval = &undefined;				
			}

		}
	}
	if (handled) return hr;	

	if (isVector)
		return ResultFromScode(DISP_E_BADVARTYPE);

	if (isSafeArray)
	{
		SAFEARRAY *psa = var->parray;
		*pval = new SafeArrayWrapper(psa, progID);
		return hr;
	}

	switch (V_VT(var))
	{
		// coerce those types we handle at this point, only by-value for now
		case VT_EMPTY:
		case VT_NULL:
			*pval = &undefined; break;
		case VT_EMPTY | VT_BYREF:
			*pval = &undefined; break;

		case VT_VOID:
			*pval = &ok; break;

		case VT_UI1:
			*pval = Integer::intern((int)V_UI1(var)); break;
		case VT_UI1 | VT_BYREF:
			*pval = Integer::intern((int)*V_UI1REF(var)); break;

		case VT_UI2:
			*pval = Integer::intern((int)V_UI2(var)); break;
		case VT_UI2 | VT_BYREF:
			*pval = Integer::intern((int)*V_UI2REF(var)); break;

		case VT_UI4:
			*pval = Integer::intern((int)V_UI4(var)); break;
		case VT_UI4 | VT_BYREF:
			*pval = Integer::intern((int)*V_UI4REF(var)); break;

		case VT_I2:
			*pval = Integer::intern((int)V_I2(var)); break;
		case VT_I2 | VT_BYREF:
			*pval = Integer::intern((int)*V_I2REF(var)); break;

		case VT_I4:
			*pval = Integer::intern((int)V_I4(var)); break;
		case VT_I4 | VT_BYREF:
			*pval = Integer::intern((int)*V_I4REF(var)); break;

		case VT_INT:
			*pval = Integer::intern((int)V_INT(var)); break;
		case VT_INT | VT_BYREF:
			*pval = Integer::intern((int)*V_INTREF(var)); break;

		case VT_INT_PTR: 
			*pval = IntegerPtr::intern(INT_PTR(V_INT_PTR(var)));   break;
		case VT_INT_PTR|VT_BYREF: 
			*pval = IntegerPtr::intern(INT_PTR(*V_INT_PTRREF(var)));  break;

		case VT_I8: 
			*pval = Integer64::intern(INT64(V_I8(var)));  break;
		case VT_I8|VT_BYREF: 
			*pval = Integer64::intern(INT64(*V_I8REF(var)));   break;

		case VT_UINT: 
			*pval = Integer::intern(UINT(V_UINT(var)));   break;
		case VT_UINT|VT_BYREF: 
			*pval = Integer::intern(UINT(*V_UINTREF(var)));  break;

		case VT_UINT_PTR: 
			*pval = IntegerPtr::intern(UINT_PTR(V_UINT_PTR(var)));   break;
		case VT_UINT_PTR|VT_BYREF: 
			*pval = IntegerPtr::intern(UINT_PTR(*V_UINT_PTRREF(var)));   break;

		case VT_R4:
			{
#ifdef MAX_TRACE
				_variant_t vDisp;
				_variant_t vSrc(var);
				vDisp.ChangeType(VT_BSTR, &vSrc);
				DebugPrint("%s\n", OLE2T(vDisp.bstrVal));
#endif
				*pval = Float::intern((float)V_R4(var)); break;
			}
		case VT_R4 | VT_BYREF:
			*pval = Float::intern((float)*V_R4REF(var)); break;

		case VT_R8:
			*pval = Double::intern((double)V_R8(var)); break;
		case VT_R8 | VT_BYREF:
			*pval = Double::intern((double)*V_R8REF(var)); break;

		case VT_CY:
			{
				float v;
				VarR4FromCy(V_CY(var), &v);
				*pval = Float::intern(v);
			}
			break;
		case VT_CY | VT_BYREF:
			{
				float v;
				VarR4FromCy(*V_CYREF(var), &v);
				*pval = Float::intern(v);
			}
			break;

		case VT_BSTR:
			{
				CW2T bufObj (V_BSTR(var));
				TCHAR *buf = bufObj;
				if (buf) *pval = new String (buf);
				else *pval = &undefined;
			}
			break;
		case VT_BSTR | VT_BYREF:
			{
				CW2T bufObj (*V_BSTRREF(var));
				TCHAR *buf = bufObj;
				if (buf) *pval = new String (buf);
				else *pval = &undefined;
			}
			break;

		case VT_BOOL:
			*pval = V_BOOL(var) != 0 ? &true_value : &false_value; break;
		case VT_BOOL | VT_BYREF:
			*pval = *V_BOOLREF(var) != 0 ? &true_value : &false_value; break;
		
		/*case VT_COLOR:
			// convert BGR(OLE_COLOR) to RGB
			OLE_COLOR color =  V_I4(var);
			*pval = new ColorValue(GetBValue(color), GetGValue(color), GetRValue(color)); break;
		case VT_COLOR | VT_BYREF:
			OLE_COLOR color =  *V_I4REF(var);
			*pval = new ColorValue(GetBValue(color), GetGValue(color), GetRValue(color)); break;
		*/

		case VT_DISPATCH:
			if (V_DISPATCH(var))
				*pval = new MSDispatch (V_DISPATCH(var), _T("MSDispatch") );
			else
				*pval = &undefined; 
			break;
		case VT_DISPATCH | VT_BYREF: 
			if((*V_DISPATCHREF(var)))
				*pval = new MSDispatch (*V_DISPATCHREF(var), _T("MSDispatch") );
			else
				*pval = &undefined; 
			break;
		case VT_VARIANT | VT_BYREF:
			return value_from_variant(V_VARIANTREF(var), pval, pTypeInfo, progID);

		case VT_ERROR:
			if (var->scode != DISP_E_PARAMNOTFOUND)
			{
				ReferenceTarget* ref = (ReferenceTarget*)var->byref;	// SR FIXME64: !!!
				if (ref->SuperClassID() == INODE_SUPERCLASS_ID)
					*pval = MAXNode::intern((INode*)ref);
				else
					*pval = MAXClass::make_wrapper_for(ref);
			}
		default:
			hr = ResultFromScode(DISP_E_BADVARTYPE);
			break;
	}

	return hr;
}

HRESULT
variant_from_value(Value* val, VARIANTARG* var, ITypeInfo* pTypeInfo)
{
	USES_CONVERSION;
	HRESULT hr = S_OK;
	//	VariantInit(var);
	bool handled = false;

	// SR FIXME64: I don't think we can actually do 'byref'

	if (val == &undefined)
	{
		V_VT(var) |= VT_DISPATCH;
		V_DISPATCH(var) = NULL;
		return S_OK;
	}

	if (pTypeInfo != NULL)
	{
		CSmartTypeAttr pta(pTypeInfo);
		hr = pTypeInfo->GetTypeAttr(&pta);

		if (SUCCEEDED(hr))
		{	
			if(pta->typekind == TKIND_ENUM)
			{
				TCHAR* name_val = val->to_string();
				VARDESC* pvd=NULL;
				for (int i=0;i<pta->cVars;i++)
				{
					pTypeInfo->GetVarDesc(i, &pvd);
					DISPID idMember = pvd->memid;
					CComBSTR bstrName;
					pTypeInfo->GetDocumentation(idMember, &bstrName, NULL, NULL, NULL);
					if (_tcsicmp(OLE2T(bstrName), name_val)==0)
					{
						V_VT(var) = VT_I4;
						V_I4(var) = pvd->lpvarValue->lVal;
						pTypeInfo->ReleaseVarDesc(pvd);
						handled = true;
						break;
					}
					pTypeInfo->ReleaseVarDesc(pvd);
				}
			}			

		}
	}
	if (handled) return hr;
	if (is_com_dispatch(val))
	{
		V_VT(var) |= VT_DISPATCH;
		if (var->vt & VT_BYREF)
			var->vt &= ~VT_BYREF;
		else
			V_DISPATCH(var) = ((MSDispatch*)val)->to_idispatch();
	}
	else if (is_activexcontrol(val))
	{
		V_VT(var) |= VT_DISPATCH;
		if (var->vt & VT_BYREF)
			var->vt &= ~VT_BYREF;
		else
			V_DISPATCH(var) = ((ActiveXControl*)val)->m_comObject->m_pdisp;
	}
	else if (is_integer(val))
	{
		V_VT(var) = VT_I4;
		V_I4(var) = val->to_int();
	}
	else if (is_integer64(val)) 
	{
		V_VT(var) = VT_I8;
		V_I8(var) = val->to_int64();
	}
	else if (is_integerptr(val)) 
	{
		V_VT(var)      = VT_INT_PTR;
		V_INT_PTR(var) = val->to_intptr();
	}
	else if (is_float(val))
	{
		V_VT(var) = VT_R4;
		V_R4(var) = val->to_float();
	}
	else if (is_double(val))
	{
		V_VT(var) = VT_R8;
		V_R8(var) = val->to_double();
	}
	else if (is_bool(val))
	{
		V_VT(var) = VT_BOOL;
		V_BOOL(var) = (val == &true_value) ? VARIANT_TRUE : VARIANT_FALSE;
	}
	else if (is_color(val))
	{
		V_VT(var) = VT_COLOR;

		AColor color = val->to_acolor();
		DWORD col = RGB((int)(color.b * 255.0f), (int)(color.g * 255.0f), (int)(color.r * 255.0f));
		// convert RGB to BGR(OLE_COLOR)
		V_I4(var) = col;
	}
	else if (is_string(val))
	{
		V_VT(var) = VT_BSTR;

		USES_CONVERSION;
		V_BSTR(var) = T2BSTR(val->to_string());		// The destructor for CComBSTR will free the returned BSTR, so...
	}

	else if (val->tag == class_tag(Rollout))
	{
		V_VT(var) = VT_UINT_PTR;
		V_INT_PTR(var) = reinterpret_cast<UINT_PTR>(((Rollout*)val)->page);	
	}
	else if (val->tag == class_tag(RolloutFloater))
	{
		HWND hwnd = ((RolloutFloater*)val)->window;
		V_VT(var) = VT_UINT_PTR;
		V_UINT_PTR(var) = reinterpret_cast<UINT_PTR>(hwnd);
	}	
	else if (val == &unsupplied)
	{
		V_VT(var)  = VT_ERROR;
		var->scode = DISP_E_PARAMNOTFOUND;
	}	
	else if (val->is_kind_of(class_tag(MAXWrapper)))
	{
		V_VT(var)  = VT_ERROR;
		var->byref = ((MAXWrapper*)val)->GetReference(0);	// SR FIXME64: In the scode??
	}
	else if (is_array(val))
	{
		one_typed_value_local(SafeArrayWrapper* safearray);
		// check number of dimensions, make sure array is "square"
		Array* theDataArray = (Array*)val;
		int nDims = SafeArrayWrapper::GetDataArrayDimensions(theDataArray);
		SafeArrayWrapper::CheckDataArray(theDataArray, nDims);
		vl.safearray = new SafeArrayWrapper(theDataArray, nDims);
		V_VT(var) = VT_ARRAY | VT_VARIANT; 
		V_ARRAY(var) = vl.safearray->to_SAFEARRAY();
		pop_value_locals();
	}
	else if (is_SafeArrayWrapper(val))
	{
		V_VT(var) = VT_ARRAY | VT_VARIANT; 
		V_ARRAY(var) = ((SafeArrayWrapper*)val)->to_SAFEARRAY();
	}
	else
	{
		V_VT(var)  = VT_ERROR;
		var->scode = DISP_E_PARAMNOTFOUND;
	}

	return hr;
}
