/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*! \file
	\ingroup kyruntime_obstaclemanagement */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_OUTLINEUPDATEPOLICY_MOVEMENTANALYSIS_H
#define KY_OUTLINEUPDATEPOLICY_MOVEMENTANALYSIS_H

#include "kyruntime/obstaclemanagement/ioutlineupdatepolicy.h"


namespace Kaim
{

/*! This implementation of IOutlineUpdatePolicy determines how to project each Obstacle based on the scalar
	velocity set for the Obstacle in the last call to Obstacle::Update().:
	-	If the Obstacle is at rest, its projected outline is set to match its exact outline, and it is projected into
		the Local PathFinding layer. An Obstacle is considered "at rest" if its speed has not exceeded 0.01 meters per
		second in the last time interval set through a call to SetAtRestPeriod().
	-	If the Obstacle is moving slower than the threshold value set through a call to SetHighSpeedThreshold(), its 
		projected outline is calculated by extending the exact outline in the direction of movement, and it is projected
		into the Local PathFinding layer. Although the resulting projection will be less exact, it will stay valid longer
		before needing to be re-projected, which may improve performance. The projected outline is only recomputed once
		the exact outline of the Obstacle is no longer contained entirely within the extended outline.
	-	If the Obstacle is moving faster than the threshold value set through a call to SetHighSpeedThreshold(), its
		projected outline is set to match its exact outline, and it is projected into the blocking layer.
	\ingroup kyruntime_obstaclemanagement */
class OutlineUpdatePolicy_MovementAnalysis: public IOutlineUpdatePolicy
{
public:
	/*! \label_constructor */
	OutlineUpdatePolicy_MovementAnalysis() :
		IOutlineUpdatePolicy(),
		m_paramAtRestPeriod(1.0f),
		m_paramHighSpeedThreshold_sq(1.0f) {}

	//inherit from IOutlineUpdatePolicy::Clone (changing only the return type which doesn't hide the function)
	virtual OutlineUpdatePolicy_MovementAnalysis* Clone() const
	{
		KY_LOG_MEMORY("ObstacleManager");
		return KY_NEW_CLASS(OutlineUpdatePolicy_MovementAnalysis)(*this);
	}

	/*! \label_virtualdestructor */
	virtual ~OutlineUpdatePolicy_MovementAnalysis() {}

	/*! A time threshold for considering an Obstacle to be at rest. If the Obstacle does not move faster than
		0.01 meters per second over the length of time set by this threshold, it is considered to be at rest. In this
		case, its exact outline will be projected into the Local PathFinding layer.
		\units			seconds
		\acceptedvalues	any positive value.
		\defaultvalue	\c 1.0f */
	KyResult SetAtRestPeriod(KyFloat32 value);

	/*! A speed threshold for considering an Obstacle to be moving at a high speed. Obstacles whose scalar velocity
		is greater than this value are projected into the blocking layer.
		\units			meters per second
		\acceptedvalues	any positive value.
		\defaultvalue	\c 1.0f */
	KyResult SetHighSpeedThreshold(KyFloat32 value);

	/*! See the class description above for details on this implementation of IOutlineUpdatePolicy::UpdateProjectedOutline(). */
	virtual ProjectionLayer UpdateProjectedOutline(Obstacle* obstacle);

	/*! For internal use by UpdateProjectedOutline(). */
	KyResult UpdateProjectedOutline_Exact(Obstacle* obstacle);

	/*! For internal use by UpdateProjectedOutline(). */
	KyResult UpdateProjectedOutline_Adaptative(Obstacle* obstacle);


public:
	/*! Stores the value set through a call to SetAtRestPeriod(). */
	KyFloat32 m_paramAtRestPeriod;

	/*! Stores the square of the value set through a call to SetHighSpeedThreshold(). */
	KyFloat32 m_paramHighSpeedThreshold_sq;
};

} // Kaim

#endif // KY_OUTLINEUPDATEPOLICY_MOVEMENTANALYSIS_H
