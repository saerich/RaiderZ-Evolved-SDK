/**********************************************************************
 *<
	FILE:			MethInfo.h

	DESCRIPTION:	Classed for holding type library information

	CREATED BY:		Ravi Karra

	HISTORY:		Started on Nov.3rd 1999

 *>	Copyright (c) 1997, All Rights Reserved.
 **********************************************************************/
 
#ifndef __METHINFO__H
#define __METHINFO__H

#include "utillib.h"
#include "TypeAttr.h"

class CParamInfo
{
public:
	TSTR			GetName()		const	{ return m_strName;	}
	HRESULT			Init( LPCOLESTR pszName );
	TSTR			m_strName;
	TSTR			m_strType;
	CComPtr<ITypeInfo>	m_spTI;
	unsigned short	m_flags;
};

const int REQUESTEDIT_ALWAYS = 0;
const int REQUESTEDIT_NEVER = 1;
const int REQUESTEDIT_PROMPT = 2;

class CMethodInfo
{
public:
	CMethodInfo();
	~CMethodInfo();

	DISPID			GetID()			const	{ return m_dispid;		}
	ITypeInfo*		GetTypeInfo()	const	{ return m_spTypeInfo;	}
	INVOKEKIND		GetInvokeKind() const	{ return m_invkind;		}
	TSTR			GetName()		const	{ return m_strName;		}
	BOOL			IsBindable()	const	{ return m_tBindable;	}
	BOOL			IsReadOnly()	const	{ return m_readOnly;	}
	BOOL			IsHidden()		const	{ return m_isHidden;	}
	BOOL			RequestsEdit()	const	{ return m_tRequestEdit;}
	VARTYPE			GetReturnType() const	{ return m_returnType;	}	
	int				GetNumParams()	const	{ return m_apParamInfo.Count(); }
	
	CParamInfo*		GetParam(int iParam)		const { return m_apParamInfo[iParam]; }
	int				GetRequestEditResponse()	const { return m_eRequestEditResponse; }	
	HRESULT			Init( ITypeInfo* pTypeInfo, const FUNCDESC* pFuncDesc );
	HRESULT			InitPropertyGet( ITypeInfo* pTypeInfo, const VARDESC* pVarDesc );
	HRESULT			InitPropertyPut( ITypeInfo* pTypeInfo, const VARDESC* pVarDesc );
	void			SetRequestEditResponse( int eResponse );
	HRESULT			GetValue(VARIANTARG* var, Value** pval, int iParam=-1);
	HRESULT			GetVariant(Value* pval, VARIANTARG* var, int iParam=-1);

	void			GetTypeString(const TYPEDESC* desc, TSTR& str, bool expand=false, ITypeInfo** ppEnumInfo=NULL);
	void			GetReturnString(TSTR& str) { str = m_strReturn;	}
	void			GetParamString(TSTR& str);

	static VARTYPE	GetUserDefinedType(ITypeInfo *pTI, HREFTYPE hrt);
	static HRESULT	GetEnumTypeInfo(ITypeInfo *pTI, HREFTYPE hrt, ITypeInfo** ppEnumInfo);
	static HRESULT	GetEnumString(ITypeInfo* ppEnumInfo, TSTR& str);

public:
	TSTR			m_strName;
	TSTR			m_strReturn;
	VARTYPE			m_returnType;
	DISPID			m_dispid;
	INVOKEKIND		m_invkind;
	BOOL			m_readOnly;
	BOOL			m_isHidden;
	BOOL			m_tBindable;
	BOOL			m_tRequestEdit;
	int				m_eRequestEditResponse;
	CComPtr<ITypeInfo>	m_spTypeInfo;
	CComPtr<ITypeInfo>	m_pIntTypeInfo;
	Tab<CParamInfo*>	m_apParamInfo;
};

class CInterfaceInfo
{
public:
	CInterfaceInfo();
	~CInterfaceInfo();

	CMethodInfo*	FindMethod( DISPID dispid ) const;
	CMethodInfo*	FindMethod( const TCHAR* name ) const; // case-insensitive
	CMethodInfo*	FindPropertyGet( DISPID dispid ) const;
	CMethodInfo*	FindPropertySet( DISPID dispid ) const;
	CMethodInfo*	FindPropertyGet( const TCHAR* name ) const; // case-insensitive
	CMethodInfo*	FindPropertySet( const TCHAR* name ) const; // case-insensitive
	IID				GetIID()				const { return m_iid; }
	CMethodInfo*	GetMethod(int iMethod)	const { return m_apMethodInfo[iMethod]; }
	int				GetNumMethods()			const { return m_apMethodInfo.Count(); }
	HRESULT			Init( ITypeInfo* pTypeInfo );

public:
	Tab<CMethodInfo*> m_apMethodInfo;
	IID				m_iid;
};
#endif // __METHINFO__H
