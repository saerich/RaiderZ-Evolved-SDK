/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: GUAL - secondary contact: NOBODY
#ifndef KyPdg_ObjFileToRawTriangleSoup_H
#define KyPdg_ObjFileToRawTriangleSoup_H


#include <kypathdata/generic/types.h>
#include "kypathdata/containers/kystring.h"


namespace Kaim
{

class PdgSystem;


class ObjFileToRawTriangleSoup
{
public:
	explicit ObjFileToRawTriangleSoup(PdgSystem* sys) : m_sys(sys) {}

	KyResult Convert(const char* objFileName, const KyString& rawTriangleSoupFileName);

private:
	PdgSystem* m_sys;

private:
	ObjFileToRawTriangleSoup(const ObjFileToRawTriangleSoup&);
	ObjFileToRawTriangleSoup& operator=(const ObjFileToRawTriangleSoup&);
};


}


#endif
