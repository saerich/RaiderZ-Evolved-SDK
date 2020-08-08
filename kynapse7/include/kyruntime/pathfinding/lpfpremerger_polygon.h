/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_pathfinding */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_LPFPREMERGER_POLYGON_H
#define KY_LPFPREMERGER_POLYGON_H

#include <kyruntime/pathfinding/lpfpremerger_polygon_config.h>


namespace Kaim
{


/*!	LpfPreMerger_Polygon is an implementation of ILpfPreMerger that merges \LpfObstacleRecords into \LpfPreAggregates
	by retrieving an instance of the PolygonMerger_Polygon class maintained by the LpfManager. 
	\pp Each time the \LpfObstacleRecords in a NavFloor change, the LpfPreMerger_Polygon is called to build 
	new \LpfPreAggregates that match the new layout of the obstacles. This implementation detects whether or 
	not the outline of each obstacle intersects the outline around any previously computed \LpfPreAggregate. If so, the two 
	are merged together:
	-	If the outline of either \LpfPreAggregate lies entirely within the area covered by the other, only the 
		larger \LpfPreAggregate is kept. The other is discarded. 
	-	If the outlines of the two \LpfPreAggregates cross, a new outline is computed that merges together the 
		outlines of both, discarding any points that are contained entirely within the outer 
		perimeter of the outline. This new merged polyline is assigned to one of the \LpfPreAggregate, and the 
		other \LpfPreAggregate is discarded. 
	\pp To conduct the actual polyline merging operation, this modifier calls 
	ILpfManager::LpfContext::MergePolygons(), which in turn invokes the instance 
	of the PolygonMerger_Polygon class that is maintained in the ILpfManager::m_polygonMerger class member. This is the 
	same class that each \LpfContext invokes in order to merge \LpfPreAggregate outlines into \LpfAreas after 
	the pre-merging process has been completed for each NavFloor.
	\pp This is the default implementation of ILpfPreMerger used by \LpfContexts.
	\ingroup kyruntime_pathfinding */
class LpfPreMerger_Polygon: public ILpfPreMerger
{
public:
	KY_DECLARE_LPFPREMERGER(LpfPreMerger_Polygon)


public:
	/*!	For internal use. */
	LpfPreMerger_Polygon(ILpfManager::LpfContext& context):
		ILpfPreMerger(context),
		m_orPolylines(KY_NULL), m_mergedPolylines(KY_NULL),
		m_orIt1(), m_orIt2(), m_preAggregate() {}

	/*!	For internal use. */
	~LpfPreMerger_Polygon();

	/*!	\label_init For internal use.
		\param config			The instance of LpfPreMerger_Polygon_Config used to initialize the \LpfContext. */
	void Initialize(const LpfPreMerger_Polygon_Config& config);

	virtual void ReInit();

	virtual KyBool ComputePreAggregates(List<ILpfManager::LpfPreAggregateComputationRequest*>& preAggregrateComputationRequests);

	KyBool ComputePreAggregate(ILpfManager::LpfNavFloorRecord* navFloorRecord, 
		SharedPoolList<ILpfManager::LpfObstacleRecord*>* obstacles,
		SharedPoolList<ILpfManager::LpfPreAggregatePtr>* preAggregates);

	/*!	Called internally by ComputePreAggregates() to merge the specified \LpfPreAggregates together. 
		For internal use. */
	KyBool AggregateObstacles(SharedPoolList<ILpfManager::LpfPreAggregatePtr>* preAggregates);

	/*!	\label_getworld */
	const World& GetWorld() const
	{
		return m_context.GetWorld();
	}

	/*!	\label_getworld */
	World& GetWorld()
	{
		return m_context.GetWorld();
	}

	/*!	\label_getengine */
	Engine& GetEngine()
	{
		return GetWorld().GetEngine();
	}

	/*!	\label_getengine */
	const Engine& GetEngine() const
	{
		return GetWorld().GetEngine();
	}

public:
	/*!	A list of polylines used to store the original polylines passed to AggregateObstacles(). Do not modify. */
	SharedPoolList<ILpfManager::LpfPolylinePtr> m_orPolylines;
	/*!	A list of polylines used to store the merged polylines computed by AggregateObstacles(). Do not modify. */
	SharedPoolList<ILpfManager::LpfPolylinePtr> m_mergedPolylines;
	/*!	For internal use. Do not modify. */
	SharedPoolList<ILpfManager::LpfObstacleRecord*>::Iterator m_orIt1;
	/*!	For internal use. Do not modify. */
	SharedPoolList<ILpfManager::LpfObstacleRecord*>::Iterator m_orIt2;
	/*!	Stores each new \LpfPreAggregate created by ComputePreAggregates() while it is checked for other overlapping 
		\LpfPreAggregates. Do not modify. */
	ILpfManager::LpfPreAggregatePtr m_preAggregate;
};

} // namespace Kaim

#endif // KY_LPFPREMERGER_POLYGON_H
