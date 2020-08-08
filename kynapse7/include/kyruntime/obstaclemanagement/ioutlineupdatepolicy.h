/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*! \file
	\ingroup kyruntime_obstaclemanagement */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_IOUTLINEUPDATEPOLICY_H
#define KY_IOUTLINEUPDATEPOLICY_H

#include "kyruntime/obstaclemanagement/obstacle.h"

#include "kypathdata/generic/memory.h"


namespace Kaim
{

class World;
class Obstacle;
class ObstacleManager;


/*! IOutlineUpdatePolicy provides an abstract base class for an object that the ObstacleManager uses
	to determine how it should project each Obstacle into the \ObstacleLayers configured for it.
	\pp This class cannot be used as-is. You must use a derived class supplied with the \SDKName SDK
	(typically OutlineUpdatePolicy_MovementAnalysis), or write your own custom class that derives from
	IOutlineUpdatePolicy.
	\pp If you choose to write your own class, you must implement the virtual UpdateProjectedOutline()
	method. See the method description below.
	\pp See also :USERGUIDE:"Using Obstacles".
	\ingroup kyruntime_obstaclemanagement */
class IOutlineUpdatePolicy
{
	KY_DEFINE_NEW_DELETE_OPERATORS


public:
	/*! Enumerates the possible different types of ObstacleLayer in which the IOutlineUpdatePolicy can instruct the
		ObstacleManager to project an Obstacle outline. */
	enum ProjectionLayer
	{
		ProjectionLayer_LPF = 0, /*!< Indicates the Local PathFinding layer set up for the ObstacleManager. */
		ProjectionLayer_Blocking, /*!< Indicates the blocking layer set up for the ObstacleManager. */
		ProjectionLayer_None, /*!< Indicates no ObstacleLayer at all. */
		ProjectionLayer__dummy__forceenumsizeint = KYFORCEENUMSIZEINT
	};


public:
	/*! \label_baseconstructor */
	IOutlineUpdatePolicy() : m_obstacleManager(KY_NULL) {}

	/*! \label_baseconstructor Copies the state of the specified IOutlineUpdatePolicy to the newly created object. */
	IOutlineUpdatePolicy( const IOutlineUpdatePolicy& ) : m_obstacleManager(KY_NULL) {} //< not copying the obstacleManager, it is set when the IOutlineUpdatePolicy is set to an ObstacleManager

	/*! \label_virtualdestructor */
	virtual ~IOutlineUpdatePolicy() {}

	/*! This method is called by the ObstacleManager for each Obstacle that needs to be updated (i.e. each obstacle
		registered with the ObstacleManager that has had its Obstacle::Update() method called since the last time
		it was projected). 
		\pp This method is responsible for calculating an outline around the Obstacle that consists of a two-dimensional
		polyline with a vertical thickness, and storing that outline in the Obstacle::m_projectedOutline member. This 
		outline has the following requirements:
		-	The points in its polyline must be in clockwise order, seen from above.
		-	The list of points must be closed: i.e. the last point in the outline must be the same as the first.
		\pp The easiest way to calculate this outline is to simply copy the outline stored in the Obstacle::m_exactOutline
		member, which is automatically calculated by Obstacle::Update() to match the exact dimensions and shape of the
		Obstacle. However, you may want to simplify or alter the points in the exact outline. For example, the
		OutlineUpdatePolicy_MovementAnalysis class supplied with \SDKName enlarges the outlines of Obstacles that are
		moving slowly, so that the outlines will stay valid longer even though they are slightly less exact.
		\pp In your decision, you may choose to take into account the Obstacle::m_updatePolicy value, which indicates the
		fidelity expected by the Obstacle (set in the call to Obstacle::Update()). For example, the 
		OutlineUpdatePolicy_MovementAnalysis class supplied with \SDKName only permits the outline to be enlarged if
		the value of Obstacle::m_updatePolicy is #OutlineUpdatePolicy_Approximate. If the value is #OutlineUpdatePolicy_Exact,
		the Obstacle::m_exactOutline will always be used as-is.
		\pp Once you have calculated the outline, you must call the ProjectedOutline::SetupPointArray() method of
		Obstacle::m_projectedOutline in order to set the number of points in the array, then add each point in the
		outline to the ProjectedOutline::m_points array in turn, and set ProjectedOutline::m_pointCount to reflect the
		number of points you add to the outline. Set ProjectedOutline::m_thickness to reflect the height of the Obstacle.
		Set ProjectedOutline::m_activated to match the value set in m_exactOutline.
		\pp The return value of this method indicates to the ObstacleManager which of its \ObstacleLayers it should project
		the Obstacle into: the ObstacleLayer used for Local PathFinding, the blocking layer, or neither. The ObstacleManager 
		internally takes care of projecting the outline into the layer indicated by this method. For performance reasons, you
		should avoid projecting Obstacles that are likely to be updated frequently over a short period of time into the
		Local PathFinding layer, as re-projecting the Obstacle outline in that layer will entail re-computing the Local 
		PathFinding information computed from that layer at each frame. For example, the OutlineUpdatePolicy_MovementAnalysis 
		class supplied with \SDKName projects \Obstacles that move faster than a configurable speed threshold into the blocking
		layer rather than the Local PathFinding layer.
		\param obstacle				The Obstacle whose outline needs to be computed.
		\return An element from the #ProjectionLayer enumeration that indicates which type of ObstacleLayer the Obstacle
		should be projected into. */
	virtual ProjectionLayer UpdateProjectedOutline(Obstacle* obstacle) = 0;

	/*! Computes the exact outline around a specified oriented bounding box, and stores that outline in the
		Obstacle::m_exactOutline member of the specified Obstacle. This method is called transparently by 
		Obstacle::Update(const ShapeOBB&, const Vec3f&, OutlineUpdatePolicy, ObstacleUpdateDelay).
		\param obstacle			The Obstacle whose m_exactOutline member will be updated.
		\param shapeObb			The oriented bounding box around the Obstacle.
		\return A #KyResult that indicates the success or failure of the request. */
	KyResult ComputeExactOutline_OBB(Obstacle* obstacle, const ShapeOBB& shapeObb);

	/*! Computes the exact outline around a specified polyline shape, and stores that outline in the
		Obstacle::m_exactOutline member of the specified Obstacle. This method is called transparently by 
		Obstacle::Update(const ShapePolyline&, const Vec3f&, OutlineUpdatePolicy, ObstacleUpdateDelay).
		\param obstacle			The Obstacle whose m_exactOutline member will be updated.
		\param shapePolyline	The polyline shape around the Obstacle.
		\return A #KyResult that indicates the success or failure of the request. */
	KyResult ComputeExactOutline_Polyline(Obstacle* obstacle, const ShapePolyline& shapePolyline);


	/*! Returns a new copy of this object. */
	virtual IOutlineUpdatePolicy* Clone() const = 0;

protected:
	friend class ObstacleManager;
	ObstacleManager* m_obstacleManager; /*!< A pointer to the ObstacleManager using this IOutlineUpdatePolicy. Do not modify. */
};

} // Kaim

#endif // KY_IOUTLINEUPDATEPOLICY_H
