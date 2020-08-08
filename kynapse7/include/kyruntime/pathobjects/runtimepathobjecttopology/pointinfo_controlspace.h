/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_pathobjects */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_POINTINFO_CONTROLSPACE_H
#define KY_POINTINFO_CONTROLSPACE_H

#include <kyruntime/core/pointinfo.h>
#include <kypathdata/containers/list.h>


namespace Kaim
{

class RuntimePathObjectControlSpace;


/*!	This implementation of PointInfo associates a RuntimePathObjectControlSpace
	with the position in 3D space contained in the PointWrapper.
	It is used internally by the PathFinder to maintain up-to-date information about its current PathObject.
	\ingroup kyruntime_pathobjects */
class PointInfo_ControlSpace: public PointInfo
{
public:
	KY_DECLARE_POINTINFO(PointInfo_ControlSpace)

public:
	PointInfo_ControlSpace(): m_controlSpaces(KY_NULL) {}
	PointInfo_ControlSpace(SharedPoolList<RuntimePathObjectControlSpace*>::Pool* pool): m_controlSpaces(pool) {}

	KyResult SetListPool(SharedPoolList<RuntimePathObjectControlSpace*>::Pool* pool);

	virtual void Reset() { m_controlSpaces.Clear(); }

	const SharedPoolList<RuntimePathObjectControlSpace*>& GetControlSpaceList() const { return m_controlSpaces; }

	void AddControlSpace(RuntimePathObjectControlSpace* controlSpace);
	void RemoveControlSpace(RuntimePathObjectControlSpace* controlSpace) { m_controlSpaces.RemoveAllOccurrences(controlSpace); }
	void RemoveOutsidesSpaces(const Vec3f& position);


protected:
	SharedPoolList<RuntimePathObjectControlSpace*> m_controlSpaces;
};

} //namespace Kaim

#endif // KY_POINTINFO_CONTROLSPACE_H
