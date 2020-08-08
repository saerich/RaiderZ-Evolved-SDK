#include "StdAfx.H"
#include "MethInfo.h"
#include "comwraps.h"
HRESULT	CParamInfo::Init( LPCOLESTR pszName)
{
	m_strName = pszName;
	return( S_OK );
}
CMethodInfo::CMethodInfo() : m_eRequestEditResponse( REQUESTEDIT_ALWAYS )
{
}

CMethodInfo::~CMethodInfo()
{
	int iParam;

	for( iParam = 0; iParam < m_apParamInfo.Count(); iParam++ )
	{
		delete m_apParamInfo[iParam];
	}
}

VARTYPE CMethodInfo::GetUserDefinedType(ITypeInfo *pTI, HREFTYPE hrt)
{
	CComPtr<ITypeInfo> spTypeInfo;
	VARTYPE vt = VT_USERDEFINED;
	HRESULT hr = E_FAIL;
	hr = pTI->GetRefTypeInfo(hrt, &spTypeInfo);
	if(FAILED(hr))
		return vt;
	CSmartTypeAttr pta(spTypeInfo);
	spTypeInfo->GetTypeAttr(&pta);

	if(&pta && pta->typekind == TKIND_ALIAS)
	{
		if (pta->tdescAlias.vt == VT_USERDEFINED)
			GetUserDefinedType(spTypeInfo,pta->tdescAlias.hreftype);
		else
			vt = pta->tdescAlias.vt;
	}
	return vt;
}

HRESULT CMethodInfo::GetEnumTypeInfo(ITypeInfo *pTI, HREFTYPE hrt, ITypeInfo** ppEnumInfo)
{
	CComPtr<ITypeInfo> spTypeInfo;
	HRESULT hr = E_FAIL;
	hr = pTI->GetRefTypeInfo(hrt, &spTypeInfo);
	if(FAILED(hr)) return hr;
	
	CSmartTypeAttr pta(spTypeInfo);
	hr = spTypeInfo->GetTypeAttr(&pta);
	if(FAILED(hr)) return hr;
	
	if (pta->typekind == TKIND_ALIAS)
	{
		if (pta->tdescAlias.vt == VT_USERDEFINED)
			return GetEnumTypeInfo(spTypeInfo,pta->tdescAlias.hreftype, ppEnumInfo);
	}
	else if (pta->typekind == TKIND_ENUM)
		spTypeInfo.CopyTo(ppEnumInfo);
	
	return (*ppEnumInfo != NULL) ? S_OK : E_FAIL;
}

HRESULT CMethodInfo::GetEnumString(ITypeInfo* ppEnumInfo, TSTR& str)
{
	USES_CONVERSION;
	str = _T("");
	HRESULT hr = E_FAIL;
	if (ppEnumInfo != NULL)
	{
		CSmartTypeAttr pta(ppEnumInfo);
		hr = ppEnumInfo->GetTypeAttr(&pta);
		if (FAILED(hr)) return hr;
		
		if(pta->typekind == TKIND_ENUM)
		{
			VARDESC* pvd=NULL;
			str = _T("( ");
			for (int i=0; i<pta->cVars; i++)
			{ 
				ppEnumInfo->GetVarDesc(i, &pvd);
				DISPID idMember = pvd->memid;
				CComBSTR bstrName;
				ppEnumInfo->GetDocumentation(idMember, &bstrName, NULL, NULL, NULL);
				str += _T("#");
				str += OLE2T(bstrName);
				if (i < (pta->cVars-1)) str += _T(" | ");
				ppEnumInfo->ReleaseVarDesc(pvd);
			}
			str += _T(" )");
			hr = S_OK;
		}		
	}
	return hr;
}

HRESULT	CMethodInfo::GetValue(VARIANTARG* var, Value** pval, int iParam)
{
#ifdef MAX_TRACE
	if (iParam != -1)
		DebugPrint(_T("%i --> %s %s\n"), iParam, m_apParamInfo[iParam]->m_strType, m_apParamInfo[iParam]->GetName());
#endif
	return value_from_variant(var, pval, (iParam<0) ? m_spTypeInfo :
				m_apParamInfo[iParam]->m_spTI);
}

HRESULT	CMethodInfo::GetVariant(Value* pval, VARIANTARG* var, int iParam)
{
	// get the return value of the function itself, not one of params
	return variant_from_value(pval, var, (iParam<0) ? m_spTypeInfo : 
					m_apParamInfo[iParam]->m_spTI);
}

void CMethodInfo::GetTypeString(const TYPEDESC* tdesc, TSTR& str, bool expand, ITypeInfo** ppEnumInfo)
{
	USES_CONVERSION;
	HRESULT hr = E_FAIL;
	str = _T("");
	
	if (tdesc->vt == VT_USERDEFINED || tdesc->vt == VT_DISPATCH)
	{
		CComPtr<ITypeInfo> spTypeInfo;
		TSTR enum_str;
		
		hr = GetEnumTypeInfo(m_pIntTypeInfo, tdesc->hreftype, &spTypeInfo);
		if (expand && SUCCEEDED(hr))
			GetEnumString(spTypeInfo, enum_str);
	
		if (!spTypeInfo.p)
		{
			hr = m_pIntTypeInfo->GetRefTypeInfo(tdesc->hreftype, &spTypeInfo);
			/*if (FAILED(hr))
			{
				HREFTYPE hrt;
				hr = m_pIntTypeInfo->GetRefTypeOfImplType(-1, &hrt);
				if (SUCCEEDED(hr))
					hr = m_pIntTypeInfo->GetRefTypeInfo(hrt, &spTypeInfo);

			}*/
		}
		if (SUCCEEDED(hr))
		{
			CComBSTR bstrName;
			spTypeInfo->GetDocumentation(MEMBERID_NIL, &bstrName, NULL, NULL, NULL);
			str = OLE2T(bstrName);
			if(ppEnumInfo) spTypeInfo.CopyTo(ppEnumInfo);
		}
		str += enum_str;
	}
	if (FAILED(hr)) 
		VTToMxsString(tdesc->vt, str);	// pointer to a variant, eg: long*	
}

void CMethodInfo::GetParamString(TSTR& str)
{
	str = _T("");
	int numParams = GetNumParams();
	CParamInfo  *pParamInfo = 0;

	if (m_invkind == INVOKE_FUNC)
	{
		for( int iParam = 0; iParam < numParams; iParam++ )
		{
			pParamInfo = m_apParamInfo[iParam];
			str += ((pParamInfo->m_flags & PARAMFLAG_FOUT) ? TSTR(_T(" &")) : TSTR(_T(" "))) +  
						pParamInfo->GetName() + _T(":") + pParamInfo->m_strType;
		}
	}
	else
	{
		TSTR buf(_T(" "));
		if (numParams > ((m_invkind == INVOKE_PROPERTYGET) ? 0 : 1))
		{
			pParamInfo = m_apParamInfo[0];
			buf.printf(_T("[%s:%s] "), pParamInfo->GetName(), pParamInfo->m_strType);
		}		
		str.printf(_T("%s: %s"), buf, 
			m_strReturn.isNull() ? m_apParamInfo[0]->m_strType : m_strReturn);
		
		if (m_dispid == DISPID_VALUE)	str += _T(", default");
		if (m_isHidden)
			str += _T(", hidden");		
	}
}

HRESULT CMethodInfo::Init( ITypeInfo* pTypeInfo, const FUNCDESC* pFuncDesc )
{
	USES_CONVERSION;
	HRESULT hResult;
	BSTR *pbstrNames;
	UINT nNames;
	int iParam;
	UINT iName;
	LPCOLESTR pszParamName;
	TSTR strPropertyValue;	

	DbgAssert( pTypeInfo != NULL );
	DbgAssert( pFuncDesc != NULL );

	m_pIntTypeInfo = pTypeInfo;
	m_dispid = pFuncDesc->memid;
	m_invkind = pFuncDesc->invkind;
	m_tBindable = pFuncDesc->wFuncFlags&FUNCFLAG_FBINDABLE;
	m_tRequestEdit = pFuncDesc->wFuncFlags&FUNCFLAG_FREQUESTEDIT;
	m_isHidden = pFuncDesc->wFuncFlags&(FUNCFLAG_FHIDDEN/*|FUNCFLAG_FNONBROWSABLE*/);
	const TYPEDESC* tdesc = &(pFuncDesc->elemdescFunc.tdesc);

	m_apParamInfo.SetCount( pFuncDesc->cParams );
	for( iParam = 0; iParam < m_apParamInfo.Count(); iParam++ )
	{
		m_apParamInfo[iParam] = new CParamInfo;
		if( m_apParamInfo[iParam] == NULL )
		{
			return( E_OUTOFMEMORY );
		}
	}

	pbstrNames = (BSTR*)alloca( (pFuncDesc->cParams+1)*sizeof( BSTR ) );
	for( iName = 0; iName < UINT( pFuncDesc->cParams+1 ); iName++ )
	{
		pbstrNames[iName] = NULL;
	}

	hResult = pTypeInfo->GetNames( m_dispid, pbstrNames, pFuncDesc->cParams+1, &nNames );
	if( FAILED( hResult ) )
	{
		return( hResult );
	}
	DbgAssert( nNames > 0 );

	m_strName = pbstrNames[0];
#ifdef MAX_TRACE
	//DebugPrint(_T("  Method: %s "), LPCTSTR( m_strName ) );
#endif
	switch( m_invkind )
	{
		case INVOKE_FUNC:
			DbgAssert( nNames == UINT( pFuncDesc->cParams+1 ) );
			break;

		case INVOKE_PROPERTYGET:
			DbgAssert( nNames == UINT( pFuncDesc->cParams+1 ) );
			break;

		case INVOKE_PROPERTYPUT:
			DbgAssert( nNames == UINT( pFuncDesc->cParams ) );
			break;

		case INVOKE_PROPERTYPUTREF:
			DbgAssert( nNames == UINT( pFuncDesc->cParams ) );
			break;

		default:
			DbgAssert( FALSE );
			break;
	}
	for( iParam = 0; iParam < m_apParamInfo.Count(); iParam++ )
	{
		if( pbstrNames[iParam+1] == NULL )
		{
			// This must be the right-hand side of a property put or putref.
			strPropertyValue = _T("Property"); //!Hey
			pszParamName = T2OLE( LPCTSTR( strPropertyValue ) );
		}
		else
		{
			pszParamName = pbstrNames[iParam+1];
		}
		m_apParamInfo[iParam]->Init( pszParamName );
		TYPEDESC* pdesc = &pFuncDesc->lprgelemdescParam[iParam].tdesc;
		m_apParamInfo[iParam]->m_flags = pFuncDesc->lprgelemdescParam[iParam].paramdesc.wParamFlags;

		while (pdesc->vt == VT_PTR)
			pdesc = pdesc->lptdesc;
		GetTypeString(pdesc, m_apParamInfo[iParam]->m_strType, true, &(m_apParamInfo[iParam]->m_spTI));
	}
	for( iName = 0; iName < UINT( pFuncDesc->cParams+1 ); iName++ )
	{
		if( pbstrNames[iName] != NULL )
		{
			::SysFreeString( pbstrNames[iName] );
		}
	}	
	while (tdesc->vt == VT_PTR)
		tdesc = tdesc->lptdesc;
	
	// get the return string
	m_returnType = tdesc->vt;
	GetTypeString(tdesc, m_strReturn, true, &m_spTypeInfo);
	return( S_OK );
}

HRESULT CMethodInfo::InitPropertyGet( ITypeInfo* pTypeInfo,	const VARDESC* pVarDesc )
{
	HRESULT hResult;
	CComBSTR bstrName;
	UINT nNames;

	DbgAssert( pTypeInfo != NULL );
	DbgAssert( pVarDesc != NULL );

	m_pIntTypeInfo = pTypeInfo;
	m_dispid = pVarDesc->memid;
	m_invkind = INVOKE_PROPERTYGET;
	m_readOnly = pVarDesc->wVarFlags&VARFLAG_FREADONLY;
	m_isHidden = pVarDesc->wVarFlags&(VARFLAG_FHIDDEN);
	m_tBindable = pVarDesc->wVarFlags&VARFLAG_FBINDABLE;
	m_tRequestEdit = pVarDesc->wVarFlags&FUNCFLAG_FREQUESTEDIT;

	hResult = pTypeInfo->GetNames( m_dispid, &bstrName, 1, &nNames );
	if( FAILED( hResult ) )
	{
		return( hResult );
	}
	DbgAssert( nNames == 1 );

	m_strName = bstrName;
	
	// get the return string
	const TYPEDESC* tdesc = &(pVarDesc->elemdescVar.tdesc);
	while (tdesc->vt == VT_PTR)
		tdesc = tdesc->lptdesc;
	GetTypeString(tdesc, m_strReturn, true, &m_spTypeInfo);

	return( S_OK );
}

HRESULT CMethodInfo::InitPropertyPut( ITypeInfo* pTypeInfo, const VARDESC* pVarDesc )
{
	USES_CONVERSION;
	TSTR strPropertyValue;
	HRESULT hResult;
	CComBSTR bstrName;
	UINT nNames;

	DbgAssert( pTypeInfo != NULL );
	DbgAssert( pVarDesc != NULL );

	m_pIntTypeInfo = pTypeInfo;
	m_dispid = pVarDesc->memid;
	m_invkind = INVOKE_PROPERTYPUT;
	m_readOnly = pVarDesc->wVarFlags&VARFLAG_FREADONLY;
	m_isHidden = pVarDesc->wVarFlags&(VARFLAG_FHIDDEN);
	m_tBindable = pVarDesc->wVarFlags&VARFLAG_FBINDABLE;
	m_tRequestEdit = pVarDesc->wVarFlags&FUNCFLAG_FREQUESTEDIT;

	hResult = pTypeInfo->GetNames( m_dispid, &bstrName, 1, &nNames );
	if( FAILED( hResult ) )
	{
		return( hResult );
	}
	DbgAssert( nNames == 1 );

	m_strName = bstrName; 

	m_apParamInfo.SetCount( 1 );
	m_apParamInfo[0] = new CParamInfo;
	if( m_apParamInfo[0] == NULL )
	{
		return( E_OUTOFMEMORY );
	}

	strPropertyValue = _T("Property"); //!Hey

	m_apParamInfo[0]->Init( T2COLE( LPCTSTR( strPropertyValue ) ));

	// get the return string
	const TYPEDESC* tdesc = &(pVarDesc->elemdescVar.tdesc);
	while (tdesc->vt == VT_PTR)
		tdesc = tdesc->lptdesc;
	GetTypeString(tdesc, m_strReturn, true, &m_spTypeInfo);

	return( S_OK );
}

void CMethodInfo::SetRequestEditResponse( int eResponse )
{
	m_eRequestEditResponse = eResponse;
}


CInterfaceInfo::CInterfaceInfo() : m_iid( IID_NULL )
{
}

CInterfaceInfo::~CInterfaceInfo()
{
	int iMethod;

	for( iMethod = 0; iMethod < m_apMethodInfo.Count(); iMethod++ )
	{
		delete m_apMethodInfo[iMethod];
	}
}

CMethodInfo* CInterfaceInfo::FindMethod( DISPID dispid ) const
{
	int iMethod;

	for( iMethod = 0; iMethod < m_apMethodInfo.Count(); iMethod++ )
	{
		if( m_apMethodInfo[iMethod]->GetID() == dispid )
		{
			return( m_apMethodInfo[iMethod] );
		}
	}
	return NULL;
}

CMethodInfo* CInterfaceInfo::FindMethod( const TCHAR* name ) const
{
	int iMethod;

	for( iMethod = 0; iMethod < m_apMethodInfo.Count(); iMethod++ )
	{
		if( _tcsicmp(m_apMethodInfo[iMethod]->GetName(), name)==0 )
		{
			 return( m_apMethodInfo[iMethod] );
		}
	}
	return NULL;
}

CMethodInfo* CInterfaceInfo::FindPropertyGet( DISPID dispid ) const
{
	int iMethod;
	CMethodInfo* pMethodInfo;

	for( iMethod = 0; iMethod < m_apMethodInfo.Count(); iMethod++ )
	{
		pMethodInfo = m_apMethodInfo[iMethod];
		if( (pMethodInfo->GetID() == dispid) && 
			(pMethodInfo->GetInvokeKind() == INVOKE_PROPERTYGET) )
		{
			 return( pMethodInfo );
		}
	}
	return NULL;
}

CMethodInfo* CInterfaceInfo::FindPropertySet( DISPID dispid ) const
{
	int iMethod;
	CMethodInfo* pMethodInfo;

	for( iMethod = 0; iMethod < m_apMethodInfo.Count(); iMethod++ )
	{
		pMethodInfo = m_apMethodInfo[iMethod];
		if( (pMethodInfo->GetID() == dispid) &&
			( (pMethodInfo->GetInvokeKind() == INVOKE_PROPERTYPUT) || 
			  (pMethodInfo->GetInvokeKind() == INVOKE_PROPERTYPUTREF) )
			)
		{
			return( pMethodInfo );
		}
	}
	return NULL;
}

CMethodInfo* CInterfaceInfo::FindPropertyGet( const TCHAR* name ) const
{
	int iMethod;
	CMethodInfo* pMethodInfo;

	for( iMethod = 0; iMethod < m_apMethodInfo.Count(); iMethod++ )
	{
		pMethodInfo = m_apMethodInfo[iMethod];
		if( (_tcsicmp(pMethodInfo->GetName(), name)==0) && 
			(pMethodInfo->GetInvokeKind() == INVOKE_PROPERTYGET) )
		{
			return( pMethodInfo );
		}
	}
	return NULL;
}

CMethodInfo* CInterfaceInfo::FindPropertySet( const TCHAR* name ) const
{
	int iMethod;
	CMethodInfo* pMethodInfo;

	for( iMethod = 0; iMethod < m_apMethodInfo.Count(); iMethod++ )
	{
		pMethodInfo = m_apMethodInfo[iMethod];
		if( (_tcsicmp(pMethodInfo->GetName(), name)==0) && 
			( (pMethodInfo->GetInvokeKind() == INVOKE_PROPERTYPUT) || 
			  (pMethodInfo->GetInvokeKind() == INVOKE_PROPERTYPUTREF) )
			)
		{
			 return( pMethodInfo );
		}
	}
	return NULL;
}

HRESULT CInterfaceInfo::Init( ITypeInfo* pTypeInfo )
{
	HRESULT hResult;
	CSmartTypeAttr pTypeAttr( pTypeInfo );
	CSmartFuncDesc pFuncDesc( pTypeInfo );
	CSmartVarDesc pVarDesc( pTypeInfo );
	int iVar;
	int iMethod;
	int iDestMethod;
	bool tFound;

	DbgAssert( pTypeInfo != NULL );

	hResult = pTypeInfo->GetTypeAttr( &pTypeAttr );
	if( FAILED( hResult ) )
	{
		return( hResult );
	}

	m_iid = pTypeAttr->guid;

	// Allocate enough space for the maximum number of methods that we could
	// possibly have, plus room for two methods for each property in case the
	// type info doesn't specify properties as put and set methods.
	m_apMethodInfo.SetCount( pTypeAttr->cFuncs+(2*pTypeAttr->cVars) );

	iDestMethod = 0;
	for( iMethod = 0; iMethod < pTypeAttr->cFuncs; iMethod++ )
	{
		hResult = pTypeInfo->GetFuncDesc( iMethod, &pFuncDesc );
		if( FAILED( hResult ) )
		{
			return( hResult );
		}

		// Only add the function to our list if it is at least at nesting level
		// 2 (i.e. defined in an interface derived from IDispatch).
		if( !(pFuncDesc->wFuncFlags&FUNCFLAG_FRESTRICTED) &&
		(pFuncDesc->funckind == FUNC_DISPATCH) )
		{
			m_apMethodInfo[iDestMethod] = new CMethodInfo;
			if( m_apMethodInfo[iDestMethod] == NULL )
			{
				return( E_OUTOFMEMORY );
			}
			hResult = m_apMethodInfo[iDestMethod]->Init( pTypeInfo, pFuncDesc );
			if( FAILED( hResult ) )
			{
				return( hResult );
			}
			iDestMethod++;
		}
		pFuncDesc.Release();
	}

	for( iVar = 0; iVar < pTypeAttr->cVars; iVar++ )
	{
		hResult = pTypeInfo->GetVarDesc( iVar, &pVarDesc );
		if( FAILED( hResult ) )
		{
			return( hResult );
		}

		if( (pVarDesc->varkind == VAR_DISPATCH) && !(pVarDesc->wVarFlags&(VARFLAG_FRESTRICTED ) ))
		{
			tFound = false;
			for( iMethod = 0; iMethod < iDestMethod; iMethod++ )
			{
				if( (m_apMethodInfo[iMethod]->GetID() == pVarDesc->memid) &&
					(m_apMethodInfo[iMethod]->GetInvokeKind() == INVOKE_PROPERTYGET) )
				{
					tFound = true;
				}
			}

			if( !tFound )
			{
				// We haven't already added this one as a method, so do it now.
				m_apMethodInfo[iDestMethod] = new CMethodInfo;
				if( m_apMethodInfo[iDestMethod] == NULL )
				{
					return( E_OUTOFMEMORY );
				}
				hResult = m_apMethodInfo[iDestMethod]->InitPropertyGet(pTypeInfo, pVarDesc);
				if( FAILED( hResult ) )
				{
					return( hResult );
				}
				iDestMethod++;
			}

			if( !(pVarDesc->wVarFlags&VARFLAG_FREADONLY) )
			{
				tFound = false;
				for( iMethod = 0; iMethod < iDestMethod; iMethod++ )
				{
					if( (m_apMethodInfo[iMethod]->GetID() == pVarDesc->memid) &&
						(m_apMethodInfo[iMethod]->GetInvokeKind() ==
					INVOKE_PROPERTYPUT) )
					{
						tFound = true;
					}
				}

				if( !tFound )
				{
					// We haven't already added this one as a method, so do it now.

					m_apMethodInfo[iDestMethod] = new CMethodInfo;
					if( m_apMethodInfo[iDestMethod] == NULL )
					{
						return( E_OUTOFMEMORY );
					}
					hResult = m_apMethodInfo[iDestMethod]->InitPropertyPut(
					pTypeInfo, pVarDesc );
					if( FAILED( hResult ) )
					{
						return( hResult );
					}
					iDestMethod++;
				}
			}
		}

		pVarDesc.Release();
	}

	// Trim the array down to the actual size.
	m_apMethodInfo.SetCount( iDestMethod );

	return S_OK;
}
