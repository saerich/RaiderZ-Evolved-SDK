/**********************************************************************
 *<
	FILE: XMLImp.h

	DESCRIPTION:	class XMLImp: A prototype for importing XML documents. 
	Conceptually, this importer implements the IXMLDocCoercion interface. see xmldocutil.h for 
	precise details. But the loose semantics of this interface are to specify some target 
	criteria and then to coerce the document toward those criteria.

	Its pseudo-implementation of CoerceDocument, delegates to a real component implementor.
	Upon return, we may get back a moniker to an ICoercionTransform-implementing component. If 
	so, this CoercionTransfrom was incompatible with the IXMLDomCoercion it was returned from.

	Aside: Probably, the incompatibility is implemented as follows. IXMLDocCoercion tries to bind 
	the moniker passing some BindContext data, for example the name of the process. 
	Certain ICoercionTransforms (like those which actually need to create nodes in VIZ)
	should refuse to bind unless they're being bound in the process space of VIZ.
	Alternatively, and, in fact this is how it works short-term, the moniker for the loader 
	doesn't even return the IXMLDocCoercion Interface 

	IN any event, if we're handed back a moniker, we try to bind and exercise it ourselves 
	(via some interface, IDOMElementLoader in teh short term, finally ICoercionTransform)

	CREATED BY: John Hutchinson, Discreet Development

	HISTORY: 12/11/99
			06/09/03 Deprecated, this class is not used anymore

 *>	Copyright (c) 1997, All Rights Reserved.
 **********************************************************************/

#ifndef __XMLIMP__H
#define __XMLIMP__H
#include "stdafx.h"
//#include "msxml.h"

#ifdef KAHN_OR_EARLIER // 06/10/03 this class is deprecated at Granite

#include "domloader.h"
#include "objidl.h"

extern HINSTANCE hInstance;

#define XMLIMP_CLASS_ID	Class_ID(0x3c38a645, 0x24f6eb39)

#define XMLIO_CONFIGDATA_VERSION     1

//--------------------< struct to hold configuration data >---------------------

struct XMLIO_ConfigData {
    XMLIO_ConfigData():pszRegistryURL(0), pszLogFile(0)
	{
        version = XMLIO_CONFIGDATA_VERSION;
		verbose = 0;
    }

	~XMLIO_ConfigData()
	{
	if(pszRegistryURL)
		SysFreeString(pszRegistryURL);
	if(pszLogFile)
		SysFreeString(pszLogFile);
	}
public:
	DWORD		version;        // the user data version
	LPOLESTR	pszRegistryURL;
	CLSID		clsidRegistry;
	CLSID		clsidXSLCoercer;

	int verbose;
	LPOLESTR pszLogFile;
};


class XMLImp : public SceneImport{
public:
	static HWND hParams;
	//Constructor/Destructor
	XMLImp();
	~XMLImp();

	//Methods of SceneImport
	virtual int				ExtCount(){ return  1;}
	virtual const TCHAR *	Ext(int n);					// Extension #n (i.e. "3DS")
	virtual const TCHAR *	LongDesc() {return GetString(IDS_LONG_DESC);}	// Long ASCII description (i.e. "Autodesk 3D Studio File")
	virtual const TCHAR *	ShortDesc() {return GetString(IDS_SHORT_DESC);}// Short ASCII description (i.e. "3D Studio")
	virtual const TCHAR *	AuthorName() {return GetString(IDS_AUTHOR);}	// ASCII Author name
	virtual const TCHAR *	CopyrightMessage() {return _T("");}				// ASCII Copyright message
	virtual const TCHAR *	OtherMessage1(){return _T("");}					// Other message #1
	virtual const TCHAR *	OtherMessage2(){return _T("");}					// Other message #2
	virtual unsigned int	Version(){return 100;}							// Version number * 100 (i.e. v3.01 = 301)
	virtual void			ShowAbout(HWND hWnd){};							// Show DLL's "About..." box
	virtual int				DoImport(const TCHAR *name,ImpInterface *ii,Interface *i, BOOL suppressPrompts=FALSE);	// Import file
	virtual int				ZoomExtents() { return ZOOMEXT_NO; }	// Override this for zoom extents control
private:

protected:
	HRESULT DelegatingCoerceDocument(/*[in]*/LPOLESTR pszURL, /*[out]*/IXMLDOMDocument **pRoot, /*[out]*/IMoniker *pXfrmMk, /*[in]*/int suppressPrompts);
	HRESULT ShowLog();
	IXMLLog * mpLog;
	void stop_log();
	void start_log();
	HRESULT StashClassActivator(IBindCtx *pbc);
	HRESULT BuildAtomicXfrmMoniker(IBindCtx *pbc, LPOLESTR pszDispName, IMoniker **pMk);
	HRESULT CreateExternalCoercer(IXMLDocCoercion2 **ppCoercion);
	HRESULT OpenRegistry(IXMLTransformRegistry **ppReg);
	HRESULT BuildXfrmMoniker(IXMLDOMDocument *pDoc, IMoniker **pmk);
	HRESULT NormalizeDocInfo(VARIANT& vinfo);
	HRESULT BindTransformMoniker(IBindCtx *pbc, IMoniker* pmk, ICoercionTransform **xfrm, /*[out]*/ IMoniker **ppMkNotBound);
	HRESULT LookupTransform(IBindCtx *pbc, VARIANT vDoc, IMoniker **ppCompositeMk);
	HRESULT BuildCompositeMoniker(IBindCtx *pbc, short nummk, BSTR *pBNames, IMoniker **ppCompositeMk);
	// These next two methods are essentially equivalent to delegating and non-delegating 
	// implementations of IDocCoercion::CoerceDocument respectively 
	// (even though this is not actually a component)
	HRESULT CoerceDocument(BSTR BURL, IXMLDOMDocument **pRoot, IMoniker **pPendingMk , BOOL suppressPrompts);
	HRESULT NonDelegatingCoerceDocument(IXMLDOMDocument * pXMLDoc, IMoniker *pXfrmMK, BOOL suppressPrompts);

	void GetCfgFilename(TCHAR* filename, TCHAR*filenamebase);
	void ReadINI();
	void WriteINI();

	VARIANT	m_impifx;
	VARIANT m_ifx;
	IUnknown *m_ClassActivator;
	XMLIO_ConfigData m_configdata;

};
#endif // KAHN_OR_EARLIER 

#endif // __XMLIMP__H
