/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: GUAL - secondary contact: LASI
#ifndef KyPdg_TickCount_H
#define KyPdg_TickCount_H

#include "kypathdata/containers/kystring.h"
#include <kypathdata/generic/memory.h>


namespace Kaim
{


class PerformanceTickCount
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	PerformanceTickCount() : m_value(0) {}
	PerformanceTickCount(KyInt64 value) : m_value(value) {}

	static PerformanceTickCount Now();

	static KyFloat32 Milliseconds(PerformanceTickCount t0, PerformanceTickCount t1);

	static KyFloat32 Seconds(PerformanceTickCount t0, PerformanceTickCount t1);

private:
	KyInt64 m_value; // we assume 64 bits is enough to store the tick count value
};


} // namespace Kaim


#endif
