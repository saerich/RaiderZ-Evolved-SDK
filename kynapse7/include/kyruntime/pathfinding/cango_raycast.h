/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_pathfinding */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_CANGO_RAYCAST_H
#define KY_CANGO_RAYCAST_H

#include <kyruntime/pathfinding/icango.h>


namespace Kaim
{
/*!	This implementation of the ICanGo interface tests whether or not a Bot can follow the 
	direct line between two points by invoking the collision testing methods provided
	in the ICollisionBridge interface.
	\pp CanGo_RayCast is not recommended for general use:
	-	Because the implementation of the ICollisionBridge interface typically conducts collision 
		tests directly against the world geometry in the \gameOrSim engine using a physics subsystem,
		and these tests frequently consume relatively large amounts of CPU time, using CanGo_RayCast 
		for all \Bots can consume excessive CPU resources.
	-	Because CanGo_RayCast tests only against the obstacles in the \gameOrSim engine, it does not
		take into account the presence of dynamic \Obstacles or blocking \PathObjects that exist within the
		World. It therefore cannot be used in conjunction with the Local PathFinding system.
	\pp For most \Bots, it is recommended to use the CanGo_NavMesh modifier instead, which takes into
	account obstacles that lie within the static NavMesh and dynamic \ObstacleLayers. The CanGo_RayCast modifier is 
	typically used only for flying \Bots, as those \Bots cannot use a NavMesh. See also :USERGUIDE:"Flying".
	\ingroup kyruntime_pathfinding */
class CanGo_RayCast: public ICanGo
{
public:
	KY_DECLARE_CANGO(CanGo_RayCast)


public:
	/*! \label_constructor */
	CanGo_RayCast():
		ICanGo() {}

	/*! \copydoc ICanGo::CanGo()
		\pp Note that this implementation of ICanGo::CanGo() does not necessarily indicate whether or not a path exists
		between the \c start point and the \c dest point. It is simply a collision test that determines whether or not the 
		straight line between the two points is blocked by any solid obstacle in the \gameOrSim engine. */
	virtual KyBool CanGo(const PointWrapper& start, const PointWrapper& dest, KyFloat32 radius);

	/*! \copydoc ICanGo::TraceLine()
		\pp Note that this implementation of ICanGo::TraceLine() does not necessarily indicate whether or not a path exists
		between the \c start point and the \c dest point. It is simply a collision test that determines whether or not the 
		straight line between the two points is blocked by any solid obstacle in the \gameOrSim engine. */
	virtual void TraceLine(const PointWrapper& startPoint, const PointWrapper& destPoint,
		KyFloat32 radius, PointWrapper& lastValidPoint);

	// Remove warning 4266: no override available for virtual member function from base; function is hidden.
	using ICanGo::CanGo;
	using ICanGo::TraceLine;

protected:
	virtual void OnSetPathFinder() {}
};

} // namespace Kaim

#endif // KY_CANGO_RAYCAST_H
