/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/


/*!	\file
	\ingroup kyruntime_pathfinding */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_CHECKDIRECTWAY_TRIVIAL_H
#define KY_CHECKDIRECTWAY_TRIVIAL_H

#include <kyruntime/pathfinding/icheckdirectway.h>

namespace Kaim
{
	/*!	This implementation of ICheckDirectWay always considers the direct way to the destination to be blocked,
		forcing the PathFinder to compute a path through the Graph using the A* algorithm.
		\ingroup kyruntime_pathfinding */
	class CheckDirectWay_Trivial: public ICheckDirectWay
	{
	public:
		KY_DECLARE_CHECKDIRECTWAY(CheckDirectWay_Trivial);

	public:
		/*! \label_constructor */
		CheckDirectWay_Trivial(): ICheckDirectWay() {}

		/*! \copydoc ICheckDirectWay::CheckDirectWay()
			This implementation of ICheckDirectWay::CheckDirectWay() returns #KY_FALSE in all cases. */
		virtual KyBool CheckDirectWay(const PointWrapper& start, const PointWrapper& dest, KyBool& ignoreAStar, KyFloat32& directWayCost);

	protected:
		virtual void OnSetPathFinder() {}
	};

} // namespace Kaim

#endif //KY_CHECKDIRECTWAY_TRIVIAL_H
