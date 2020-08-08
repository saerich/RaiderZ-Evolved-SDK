/**********************************************************************
 *<
	FILE: XMLUtility.h

	DESCRIPTION:	XML utilites

	CREATED BY:		Neil Hazzard	

	HISTORY:		summer 2002

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/


#include "atlbase.h"
#include "msxml2.h"


bool CreateXMLNode(CComPtr<IXMLDOMDocument> doc, CComPtr<IXMLDOMNode> node, const TCHAR *nodeName, IXMLDOMNode **  newNode);
bool AddXMLAttribute(CComPtr<IXMLDOMNode> node, const TCHAR * name, const TCHAR * value);
bool AddXMLText(CComPtr<IXMLDOMDocument> doc, CComPtr<IXMLDOMNode> node, const TCHAR * text);
CComPtr<IXMLDOMText> CreateXMLText(CComPtr<IXMLDOMDocument>, CComPtr<IXMLDOMNode>, const TCHAR*);
void PrettyPrint(const TCHAR*, CComPtr<IXMLDOMDocument> );
