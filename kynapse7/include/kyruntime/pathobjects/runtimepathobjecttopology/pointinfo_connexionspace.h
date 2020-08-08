/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_pathobjects */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_POINTINFO_CONNEXIONSPACE_H
#define KY_POINTINFO_CONNEXIONSPACE_H

#include <kyruntime/core/pointinfo.h>
#include <kypathdata/containers/list.h>


namespace Kaim
{

class RuntimePathObjectConnexionSpace;


/*!	This implementation of PointInfo associates a RuntimePathObjectConnexionSpace
	with the position in 3D space contained in the PointWrapper.
	It is used internally by the PathFinder to maintain up-to-date information about its current PathObject.
	\ingroup kyruntime_pathobjects
*/
class PointInfo_ConnexionSpace: public PointInfo
{
public:
	KY_DECLARE_POINTINFO(PointInfo_ConnexionSpace)

public:
	PointInfo_ConnexionSpace(): m_connexionSpaces(KY_NULL) {}
	PointInfo_ConnexionSpace(SharedPoolList<RuntimePathObjectConnexionSpace*>::Pool* pool): m_connexionSpaces(pool) {}

	KyResult SetListPool(SharedPoolList<RuntimePathObjectConnexionSpace*>::Pool* pool);

	virtual void Reset() { m_connexionSpaces.Clear(); }

	const SharedPoolList<RuntimePathObjectConnexionSpace*>& GetConnexionSpaceList() const { return m_connexionSpaces; }

	void AddConnexionSpace(RuntimePathObjectConnexionSpace* connexionSpace);
	void RemoveConnexionSpace(RuntimePathObjectConnexionSpace* connexionSpace) { m_connexionSpaces.RemoveAllOccurrences(connexionSpace); }
	void RemoveOutsidesSpaces(const Vec3f& position);


protected:
	SharedPoolList<RuntimePathObjectConnexionSpace*> m_connexionSpaces;
};

} //namespace Kaim

#endif // KY_POINTINFO_CONNEXIONSPACE_H
