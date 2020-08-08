/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_graph */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_POINTINFO_ASTARID_H
#define KY_POINTINFO_ASTARID_H

#include <kyruntime/core/pointinfo.h>


namespace Kaim
{

/*!	An object of this class can be assigned to an instance of PointWrapper to
	store an A* ID related to the point maintained in the PointWrapper.
	This ID is set and retrieved internally by the \SDKName pathfinding process,
	and by \GraphTraversals that derive from AstarTraversal.
	\ingroup kyruntime_graph */
class PointInfo_AstarId: public PointInfo
{
public:
	KY_DECLARE_POINTINFO(PointInfo_AstarId)

public:
	PointInfo_AstarId(): m_astarId(KyUInt32MAXVAL) {}
	PointInfo_AstarId(KyUInt32 astarId) : m_astarId(astarId) {}

	virtual void Reset() { m_astarId=KyUInt32MAXVAL; }

	void SetAstarId(KyUInt32 astarId) {m_astarId = astarId;}
	const KyUInt32& GetAstarId() const {return m_astarId;}


protected:
	KyUInt32 m_astarId;
};

} //namespace Kaim

#endif // KY_POINTINFO_ASTARID_H
