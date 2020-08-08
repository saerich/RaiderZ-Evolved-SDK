/**********************************************************************
 *<
	FILE: XMLImp.h

	DESCRIPTION:	class XMLImp2: A much simpler xml importer 
	
	load document
	Locate the root node of a known type
	Loop through children
		switch on element type
		call loader (use private ini file to map element types to module names)


	CREATED BY: John Hutchinson, Discreet Development

	HISTORY: 06/09/03 created to replace XMLImp

 *>	Copyright (c) 1997, All Rights Reserved.
 **********************************************************************/

#ifndef __XMLIMP2__H
#define __XMLIMP2__H
#include "stdafx.h"

#define XMLIMP2_CLASS_ID	Class_ID(0x3c38a645, 0x24f6eb40)

class XMLImp2 : public SceneImport{
public:
	//Constructor/Destructor
	XMLImp2(){};
	~XMLImp2(){};

	//Methods of SceneImport
	virtual int				ExtCount()	{ return  1;}
	virtual const TCHAR *	Ext(int n);					// Extension #n (i.e. "3DS")
	virtual const TCHAR *	LongDesc()	{return GetString(IDS_LONG_DESC_GRANITE);}	// Long ASCII description (i.e. "Autodesk 3D Studio File")
	virtual const TCHAR *	ShortDesc() {return GetString(IDS_SHORT_DESC_GRANITE);}// Short ASCII description (i.e. "3D Studio")
	virtual const TCHAR *	AuthorName() {return GetString(IDS_AUTHOR);}	// ASCII Author name
	virtual const TCHAR *	CopyrightMessage() {return _T("");}				// ASCII Copyright message
	virtual const TCHAR *	OtherMessage1(){return _T("");}					// Other message #1
	virtual const TCHAR *	OtherMessage2(){return _T("");}					// Other message #2
	virtual unsigned int	Version(){return 100;}							// Version number * 100 (i.e. v3.01 = 301)
	virtual void			ShowAbout(HWND hWnd){};							// Show DLL's "About..." box
	virtual int				DoImport(const TCHAR *name,ImpInterface *ii,Interface *i, BOOL suppressPrompts=FALSE);	// Import file
	virtual int				ZoomExtents() { return ZOOMEXT_NO; }	// Override this for zoom extents control
private:
	HRESULT CreatePluglet(REFCLSID rclsid, REFIID riid, void **ppPluglet, LPCTSTR sModule);

};
 
#endif __XMLIMP2__H
