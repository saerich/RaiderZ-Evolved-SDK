/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_pathfinding */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_GOTO_TRIVIAL_H
#define KY_GOTO_TRIVIAL_H

#include <kyruntime/pathfinding/igoto.h>


namespace Kaim
{
/*!	This implementation of the IGoto interface invokes the ISteering modifier currently in use
	by the PathFinder to generate an Action that moves the Bot directly to its
	current target point, without considering any other \Bots or \Bodies that may lie in its way.
	\pp Because this modifier does not perform any adjustments to the current target point of the Bot
	based on the surrounding traffic, it may generate
	trajectories that collide with other \Bots and \Bodies in the World. However, it does offer the 
	benefit of minimal CPU consumption.
	\ingroup kyruntime_pathfinding */
class Goto_Trivial: public IGoto
{
public:
	KY_DECLARE_GOTO(Goto_Trivial)


public:
	/*!	\label_constructor */
	Goto_Trivial():
		IGoto() {}

	virtual KyBool Goto(const PointWrapper& targetPoint, Action& action);

protected:
	virtual void OnSetPathFinder() {}

};

} // namespace Kaim

#endif // KY_GOTO_TRIVIAL_H
