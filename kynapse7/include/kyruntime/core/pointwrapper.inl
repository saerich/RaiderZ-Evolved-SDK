/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_core */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_POINTWRAPPER_INL
#define KY_POINTWRAPPER_INL

#include <kyruntime/core/pointinfo.h>


namespace Kaim
{

PointWrapper& PointWrapper::operator=(const PointWrapper& pw)
{
	m_position = pw.m_position;

	for (KyUInt32 i=0; i<KY_MAX_POINT_INFO; ++i)
		m_info[i] = pw.m_info[i];

	return *this;
}

void PointWrapper::ClearPointInfo()
{
	for (KyUInt32 i=0; i<KY_MAX_POINT_INFO; ++i)
		m_info[i] = KY_NULL;
}

void PointWrapper::ResetPointInfos()
{
	for (KyUInt32 i=0; i<KY_MAX_POINT_INFO; ++i)
	{
		if (m_info[i])
			m_info[i]->Reset();
	}
}

} // namespace Kaim

#endif // KY_POINTWRAPPER_INL
