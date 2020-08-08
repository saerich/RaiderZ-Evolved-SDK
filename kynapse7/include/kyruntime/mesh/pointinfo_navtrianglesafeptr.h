/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*! \file
	\ingroup kyruntime_mesh */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_POINTINFO_NAVMESHLOCATION_H
#define KY_POINTINFO_NAVMESHLOCATION_H

#include <kyruntime/core/pointinfo.h>
#include <kypathdata/navmesh/navtrianglesafeptr.h>


namespace Kaim
{

/*!	This implementation of PointInfo allows a NavTriangleSafePtr object to be
	added to a PointWrapper, in order to associate a NavMesh triangle with the
	position in 3D space contained in the PointWrapper.
	\ingroup kyruntime_mesh */
class PointInfo_NavTriangleSafePtr: public PointInfo
{
public:
	KY_DECLARE_POINTINFO(PointInfo_NavTriangleSafePtr)

	virtual void Reset() { m_navTriangleSafePtr.Invalidate(); }


public:
	NavTriangleSafePtr m_navTriangleSafePtr; /*!< A NavTriangleSafePtr that identifies the NavMesh triangle to be associated with the PointWrapper. */
};


} // namespace Kaim

#endif // KY_POINTINFO_NAVMESHLOCATION_H
