/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// Primary contact: JUBA - secondary contact: NOBODY
#ifndef KyPdg_ObjToIndexedTriangleSoup_H
#define KyPdg_ObjToIndexedTriangleSoup_H



#include "kypathdata/containers/kystring.h"


namespace Kaim
{
class IndexedTriangleSoup;
class PdgSystem;

class ObjToIndexedTriangleSoup
{
public:
	ObjToIndexedTriangleSoup(PdgSystem* sys = KY_NULL) : m_sys(sys) {}

	KyResult FillIndexedTriangleSoupFromObjFile(const char* fileName, IndexedTriangleSoup& indexedTriangle);

public:
	PdgSystem* m_sys;
};

}


#endif //KyPdg_ObjToIndexedTriangleSoup_H

