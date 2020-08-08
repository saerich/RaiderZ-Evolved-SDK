/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_graph */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_GRAPHCONSTRAINTS_H
#define KY_GRAPHCONSTRAINTS_H

#include <kyruntime/core/inextmovemanager.h>
#include <kyruntime/core/pointwrapper.h>

#include <kyruntime/graph/graphvertexptr.h>

namespace Kaim
{

class AstarContext;
class GraphVertex;


////////////////////////////////////////////////////////////////////////////////////////////////////////
//		ConstraintShortestPath
////////////////////////////////////////////////////////////////////////////////////////////////////////

/*!	This implementation of IConstraint sets the cost of the provided line segment to the straight-line distance between the
	points, in meters. 
	\pp An extra cost is then added to reflect the number of other \Bots whose bounding
	boxes intersect the segment, making \Bots prefer less crowded paths. This extra cost depends
	on an aversion factor set through a call to SetCrowdAversionFactor(KyFloat32). With a higher aversion factor, \Bots will go
	further out of their way in order to avoid crowded areas; with a lower aversion factor, \Bots
	will be more tolerant of crowding.
	\pp The PathFinder creates and uses an instance of this IConstraint by default, which it maintains in PathFinder::m_defaultConstraint.
	\ingroup kyruntime_graph */
class ConstraintShortestPath : public IConstraint
{
public:
	KY_DECLARE_CONSTRAINT(ConstraintShortestPath)


public:
	/*!	\label_constructor */
	ConstraintShortestPath(): m_crowdAversionFactor(0.0f) {}

	KyBool GetCost(Bot* entity, const PointWrapper& from, const PointWrapper& to, KyFloat32& cost);

	/*!	Sets the crowd aversion factor for the ConstraintShortestPath. Leave this value at \c 0.0f unless otherwise
		instructed by Autodesk Support.
		\acceptedvalues	any positive value
		\defaultvalue	\c 0.0f */
	KyBool SetCrowdAversionFactor(KyFloat32 factor)
	{
		if (factor >= 0.0f)
		{
			m_crowdAversionFactor = factor;
			return KY_TRUE;
		}
		return KY_FALSE;
	}
	
	/*!	Retrieves the value set through a call to SetCrowdAversionFactor(KyFloat32). */
	KyFloat32 GetCrowdAversionFactor() {return m_crowdAversionFactor;}


protected:
	KyFloat32 m_crowdAversionFactor;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////
//		SphereConstraint
////////////////////////////////////////////////////////////////////////////////////////////////////////

/*!	This implementation of IConstraint forbids the use of any edge that does not lie entirely within a sphere of a given radius 
	centered on a given 3D position (typically the current position of an Bot using the PathFinder). 
	You can use this constraint to forbid \Bots from planning paths that take them too far from their 
	current position, or from any other important place in the terrain.
	\pp Using this constraint can also improve the performance of the PathFinder in some cases. For example, 
	consider a Bot that attempts to find a path to a point that is accessible only by passing through a 
	locked door. In this case, the A* algorithm will propagate through each other vertex in the spatial graph 
	before concluding that a path to the destination does not exist. If that Bot uses the SphereConstraint, its 
	A* algorithm will instead stop propagating each time it encounters an edge that crosses the boundary of the 
	sphere. This helps the PathFinder determine more quickly that no path is available, freeing up CPU resources
	for use by other \SDKName components. 
	\section sconstusing Using the SphereConstraint
	To use the SphereConstraint, you must create an instance of the SphereConstraint class, call its SetSphere() method
	to set up its center and radius, and pass your instance to PathFinder::SetConstraint() to make the PathFinder.
	Do not create your object on the stack; see PathFinder::SetConstraint().
	\ingroup kyruntime_graph */
class SphereConstraint : public ConstraintShortestPath
{
public:
	KY_DECLARE_CONSTRAINT(SphereConstraint)


public:
	/*!	\label_constructor */
	SphereConstraint(): m_center(0.f,0.f,0.f), m_radius(0.f) {}

	/*!	Sets the position and radius of the sphere used by the SphereConstraint. 
		\param center			The center of the sphere.
		\param radius			The radius of the sphere. */
	void SetSphere(const Vec3f& center, KyFloat32 radius)
	{
		m_center = center;
		m_radius = radius;
	}
	
	/*!	Retrieves the position and radius of the sphere used by the SphereConstraint. 
		\param[out] center		Stores the center of the sphere.
		\param[out] radius		Stores the radius of the sphere. */
	void GetSphere(Vec3f& center, KyFloat32& radius) const
	{
		center = m_center;
		radius = m_radius;
	}

	virtual KyBool GetCost(Bot* entity, const PointWrapper& from, const PointWrapper& to, KyFloat32& cost);


protected:
	Vec3f m_center;
	KyFloat32 m_radius;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////
//		HeuristicEuclidianDistance
////////////////////////////////////////////////////////////////////////////////////////////////////////

/*!	This implementation of IHeuristic sets the cost of each tested line segment to match the linear distance 
	between its start point and end point. This heuristic value is guaranteed to be admissible with the constraint cost calculated 
	by any IConstraint class supplied with \SDKName: i.e. the cost it returns will not overestimate the cost calculated by the
	IConstraint class. 
	\pp This IHeuristic is suitable for use in most \gameOrSim environments, as long as you are not using a custom
	IConstraint class that can assign a cost to an edge that is less than the linear distance of that edge (e.g. a 
	teleporter that moves between two points with a zero cost).
	\pp The PathFinder creates and uses an instance of this IHeuristic by default, which it maintains in PathFinder::m_defaultHeuristic.
	\ingroup kyruntime_graph */
class HeuristicEuclidianDistance: public IHeuristic
{
public:
	KY_DECLARE_HEURISTIC(HeuristicEuclidianDistance)


public:
	/*!	\label_constructor */
	HeuristicEuclidianDistance() {}

	virtual KyBool GetCost(Bot* /*entity*/, const PointWrapper& from, const PointWrapper& to, KyFloat32& cost)
	{
		cost = Distance(from.GetPosition(), to.GetPosition()); 
		return KY_TRUE;
	}
};


////////////////////////////////////////////////////////////////////////////////////////////////////////
//		ZeroHeuristic
////////////////////////////////////////////////////////////////////////////////////////////////////////

/*!	This implementation of IHeuristic assigns a cost of 0 to all tested line segments, which makes the A* algorithm propagate 
	through the spatial graph equally in all directions. Although this IHeuristic will eventually find the 
	shortest path to the destination, the process is highly inefficient. You should therefore use this 
	class only for testing purposes.
	\ingroup kyruntime_graph */
class ZeroHeuristic: public IHeuristic
{
public:
	KY_DECLARE_HEURISTIC(ZeroHeuristic)


public:
	/*!	\label_constructor */
	ZeroHeuristic() {}

	virtual KyBool GetCost(Bot* /*entity*/, const PointWrapper& /*from*/, const PointWrapper& /*to*/,
		KyFloat32& cost)
	{
		cost=0; 
		return KY_TRUE;
	}
};

} // namespace Kaim

#endif // KY_GRAPHCONSTRAINTS_H
