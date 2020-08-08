/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/


/*!	\file
	\ingroup kyruntime_pathfinding */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_CHECKDIRECTWAY_DISTANCE_H
#define KY_CHECKDIRECTWAY_DISTANCE_H

#include <kyruntime/pathfinding/icheckdirectway.h>

namespace Kaim
{
	/*!	This implementation of ICheckDirectWay tests whether or not the direct way to the destination point is
		clear of obstacles only when the distance to the destination is less than the value set through a call
		to SetMaxDist().
		\pp When the direct path is clear of obstacles, the value set through a call to SetIgnoreAStar()
		determines whether the PathFinder will skip running the A* calculation and begin following the direct path
		immediately, or whether it will run the A* calculation and compare the cost with the cost of the direct way.
		\pp This implementation determines that the direct way is blocked if it crosses any boundary of the static 
		NavMesh, any blocking \ObstacleLayers set up for the CanGo_AiMesh modifier, or any impassable Local PathFinding 
		obstacles that lie in the direct way. If a Local PathFinding obstacle lies in the way, but the Bot can circle
		around it to get to the destination, the direct way is not considered to be blocked.
		\ingroup kyruntime_pathfinding */
	class CheckDirectWay_Distance: public ICheckDirectWay
	{
	public:
		KY_DECLARE_CHECKDIRECTWAY(CheckDirectWay_Distance);

	public:
		/*! \label_constructor */
		CheckDirectWay_Distance():
			ICheckDirectWay(),
			m_paramMaxDist(5.0f),
			m_paramIgnoreAStar(KY_FALSE)
		{}

		/*! \copydoc ICheckDirectWay::CheckDirectWay()
			For details on this impelementation, see the CheckDirectWay_Distance class description. */
		virtual KyBool CheckDirectWay(const PointWrapper& start, const PointWrapper& dest, KyBool& ignoreAStar, KyFloat32& directWayCost);

		// Parameter accessors
		/*! Sets the maximum distance that may exist between the starting point of the path calculation and
			the destination in order for this class to check whether or not the direct way is clear of obstacles.
			If the distance to the destination is greater than this value, CheckDirectWay() will always return
			#KY_FALSE.
			\units			meters
			\acceptedvalues	any positive value.
			\defaultvalue	\c 5.0f */
		void SetMaxDist(KyFloat32 distance);

		/*! Retrieves the value set through a call to SetMaxDist(). */
		KyFloat32 GetMaxDist() const { return m_paramMaxDist; }

		/*! When the CheckDirectWay() method determines that the direct way to the destination is clear of obstacles,
			this parameter determines whether or not the PathFinder will also run an A* and compare the cost of the two
			options.
			\acceptedvalues	-	#KY_TRUE: The PathFinder will skip the A* calculation and begin following the direct
								way to the destination.
							-	#KY_FALSE: The PathFinder will run an A* calculation to get a path through the Graph
								to the destination, compare the cost of the two options, and choose the option with the
								lower cost.
			\defaultvalue	#KY_FALSE */
		void SetIgnoreAStar(KyBool ignoreAStar) {m_paramIgnoreAStar = ignoreAStar; }

		/*! Retrieves the value set through a call to SetIgnoreAStar(). */
		KyBool GetIgnoreAStar() const { return m_paramIgnoreAStar; }

	protected:
		virtual void OnSetPathFinder() {}

	protected:
		KyFloat32 m_paramMaxDist; /* the distance below which we check the direct way to avoid doing small A stars. */
		KyBool m_paramIgnoreAStar;
	};

} // namespace Kaim

#endif //KY_CHECKDIRECTWAY_DISTANCE_H
