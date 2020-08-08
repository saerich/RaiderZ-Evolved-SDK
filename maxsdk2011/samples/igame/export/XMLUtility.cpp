/**********************************************************************
 *<
	FILE: XMLUtility.cpp

	DESCRIPTION:	Handy functions for XML, to keep the code a bit cleaner

	CREATED BY:		Neil Hazzard

	HISTORY:		Summer 2002

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/

#include "IGameExporter.h"
#include "atlbase.h"
#include "msxml2.h"

bool CreateXMLNode(CComPtr<IXMLDOMDocument> doc, CComPtr<IXMLDOMNode> node, const TCHAR *nodeName, IXMLDOMNode ** newNode)
{

	CComPtr <IXMLDOMNode> sceneNode;
	doc->createNode(CComVariant(NODE_ELEMENT), CComBSTR(nodeName), NULL, &sceneNode);
	node->appendChild(sceneNode, newNode);
	return true;
}

bool AddXMLAttribute(CComPtr<IXMLDOMNode> node, const TCHAR * name, const TCHAR * value)
{
	CComQIPtr<IXMLDOMElement> element;
	element = node;
	element->setAttribute(CComBSTR(name), CComVariant(value));
	return true;
}

bool AddXMLText(CComPtr<IXMLDOMDocument> doc, CComPtr<IXMLDOMNode> node, const TCHAR * text)
{
	CComPtr <IXMLDOMText> keydata;
	doc->createTextNode(CComBSTR(text), &keydata);
	node->appendChild(keydata,NULL);
	return true;

}
CComPtr<IXMLDOMText> CreateXMLText(CComPtr<IXMLDOMDocument> doc, CComPtr<IXMLDOMNode> parentNode, const TCHAR* name)
{
	CComPtr<IXMLDOMNode> node;
	CComPtr<IXMLDOMText> textNode;

	if( CreateXMLNode(doc,parentNode, name,&node) == true) {
		doc->createTextNode(CComBSTR(""), &textNode);
		node->appendChild(textNode,NULL);
	}

	return textNode;
}

void PrettyPrint(const TCHAR* name, CComPtr<IXMLDOMDocument> pXMLDoc)
{
	// perform formatting XSLT transform to get indented XML output
	CComPtr<IXMLDOMDocument> pXSLDoc;
	BSTR outputXML = NULL;
	HRESULT hr = CoCreateInstance(CLSID_DOMDocument, NULL, CLSCTX_INPROC_SERVER,  IID_IXMLDOMDocument, (void**)&pXSLDoc);
	if (SUCCEEDED(hr)) {
		// load indenting XSL doc 
		VARIANT_BOOL result;
		CComBSTR indentXSL(
			"<xsl:stylesheet version=\"1.0\""
			"      xmlns:xsl=\"http://www.w3.org/1999/XSL/Transform\">"
			"   <xsl:output method=\"xml\"/>"
			"   <xsl:param name=\"indent-increment\" select=\"'\t'\" />"
			"   <xsl:template match=\"node()\">"
			"      <xsl:param name=\"indent\" select=\"'&#xA;'\"/>"
			"      <xsl:value-of select=\"$indent\"/>"
			"      <xsl:copy>"
			"        <xsl:copy-of select=\"@*\" />"
			"        <xsl:apply-templates>"
			"          <xsl:with-param name=\"indent\""
			"               select=\"concat($indent, $indent-increment)\"/>"
			"        </xsl:apply-templates>"
			"        <xsl:if test=\"node()\">"
			"          <xsl:value-of select=\"$indent\"/>"
			"        </xsl:if>"
			"      </xsl:copy>"
			"   </xsl:template>"
//			"   <xsl:template match=\"comment()|processing-instruction()\">"
//			"      <xsl:copy />"
//			"   </xsl:template>"
//			"   <!-- WARNING: this is dangerous. Handle with care -->"
//			"   <xsl:template match=\"text()[normalize-space(.)='']\"/>"
			"</xsl:stylesheet>"
			);
		hr = pXSLDoc->loadXML(indentXSL, &result);
		if (SUCCEEDED(hr)) {
			// perform transform
			hr = pXMLDoc->transformNode(pXSLDoc, &outputXML);
		}
	}

	// output transformed XML if previous sequence succeeded, else normal XMLDoc save
	if (SUCCEEDED(hr)) {
		FILE* out = _tfopen(name, _T("w"));
		if (out != NULL) {
			// hack the UTF-16 back to UTF-8 (there probably is a way to mod the stylesheet to do this)
			wchar_t* enc = wcsstr(outputXML, L"\"UTF-16\"");
			if (enc != NULL) memcpy(enc, L"\"utf-8\" ", 8 * sizeof(wchar_t));
			// convert BSTR to MBCS for output
			int len = WideCharToMultiByte(CP_ACP, 0, outputXML, -1, 0, 0, NULL, NULL);
			TCHAR* buf = (TCHAR*)malloc(len * sizeof(WCHAR));
			WideCharToMultiByte(CP_ACP, 0, outputXML, -1, buf, len, NULL, NULL);
			// write the XML
			_fputts(buf, out);   
			fclose(out);
			free(buf);
		}
		SysFreeString(outputXML);
	}
	else
		// save the XML graph out to the export file
		pXMLDoc->save(CComVariant(name));

	
}
