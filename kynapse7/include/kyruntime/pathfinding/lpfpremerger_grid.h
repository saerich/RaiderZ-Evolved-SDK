/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_pathfinding */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_LPFPREMERGER_GRID_H
#define KY_LPFPREMERGER_GRID_H

#include <kyruntime/pathfinding/lpfpremerger_grid_config.h>
#include <kyruntime/mesh/polygonmerger_grid.h>

namespace Kaim
{

/*!	LpfPreMerger_Grid is an implementation of ILpfPreMerger that merges \LpfObstacleRecords into \LpfPreAggregates
	by invoking an instance of the PlgMrg_Grid_Merger class. 
	\pp Each time the obstacle records in a NavFloor change, the LpfPreMerger_Grid is called to build 
	new \LpfPreAggregates that match the new layout of the obstacles. In this implementation, the 
	PlgMrg_Grid_Merger constructs a grid of sampling points and projects the extended outlines of all obstacle 
	records in the NavFloor into that grid. For each \LpfPolylineNode contained in the obstacle 
	outlines, the PlgMrg_Grid_Merger determines the point in the grid that best represents the position of that 
	\LpfPolylineNode based on the angle of its incoming and outgoing line segments, and marks that point. 
	It then uses these marked points to create a new set of output contours that approximates the merged 
	extended outlines of the input obstacles. A new \LpfPreAggregate is created for each of these contours. 
	\pp LpfPreMerger_Grid is typically more efficient than LpfPreMerger_Polygon in situations where many 
	\LpfObstacleRecords overlap within the same NavFloor. In addition, it offers the ability to configure 
	the precision of the grid used to project the obstacle outlines: when the distance between sampling 
	points is very small, LpfPreMerger_Grid produces more accurate results but consumes more CPU and memory 
	resources; when the distance between sampling points is larger, it offers better performance, but produces 
	less accurate results. You can take advantage of this to implement some custom Levels of Detail (LoD) in
	your pre-merging; see \ref lods below. 
	\pp \section gridconstruction Constructing the grid 
	The PlgMrg_Grid_Merger class allocates a set amount of memory for its grid when the World is initialized, 
	sized according to the LpfPreMerger_Grid_Config::m_maxGridWidth and LpfPreMerger_Grid_Config::m_maxPrecision
	values set in the LpfPreMerger_Grid_Config object used to create the LpfPreMerger_Grid. 
	\pp At initialization time, the PlgMrg_Grid_Merger calculates how many grid points would be needed in order to 
	fill a grid of the specified width with sampling points separated at the specified distance, and allocates
	enough memory to hold a pool of grid points of that size. 
	\pp Each time the PlgMrg_Grid_Merger treats a NavFloor, it re-computes the layout of its grid to optimize the 
	usage of the pool of grid points for the actual area covered by the input polylines. It calculates 
	the actual coverage of the set of input polylines along the X and Y axes, and calculates how many 
	grid points would be needed to cover an axis-aligned grid of that size with the current preferred 
	precision setting. By default, this preferred precision level is equal to the value set by 
	LpfPreMerger_Grid_Config::m_maxPrecision, but you can change the preferred precision at any time by calling 
	LpfPremerger_Grid::SetPrecision(). If the pool of grid points is large enough, the grid is laid out 
	using that desired precision as the spacing interval between the grid points. If, however, the pool 
	of grid points is not sufficient, the PlgMrg_Grid_Merger uses the closest possible spacing for its grid 
	points that will not exhaust the pool. 
	\pp Note that SetPrecision() only accepts values greater than or equal to the value you set for
	LpfPreMerger_Grid_Config::m_maxPrecision. It also accepts \c 0.0f, which instructs the PlgMrg_Grid_Merger to
	use the LpfPreMerger_Grid_Config::m_maxPrecision value as the current preferred precision. 
	\pp \section lods Levels of Detail 
	In general, there is a direct correlation between the spacing of the points in the grid used by the 
	PlgMrg_Grid_Merger and the performance of the pre-merging operation. You can take advantage of this in order 
	to prioritize accuracy over performance when pre-merging some \NavFloors over others (such as those close to 
	a player character), and to prioritize performance over accuracy when pre-merging other \NavFloors 
	(such as those farther away from a player character). In order to do this, you must change the 
	preferred precision used in the pre-merging operation depending on the NavFloors that contain the 
	obstacles. 
	\pp Because this kind of control over Levels of Detail is very specific to the runtime considerations of 
	your game, it is not implemented directly in the LpfPreMerger_Grid class. However, you can implement such a 
	system by deriving your own class from LpfPremerger_Grid to specialize its behavior. 
	\pp The LpfPreMerger_Grid class maintains the current preferred precision for the grid 
	in its #m_currentPrecision member. Each time the ComputePreAggregates() method treats a new NavFloor, 
	it sets up the PlgMrg_Grid_Merger with the current preferred precision by calling 
	PlgMrg_Grid_Merger::SetupBeforeAddInputs(). In order to support different levels of precision for 
	pre-merging different NavFloors, you need to override ComputePreAggregates() to evaluate the NavFloor 
	passed in the request, determine the preferred precision appropriate for that 
	NavFloor, and call LpfPreMerger_Grid::SetPrecision() to store the new preferred precision. Then, 
	call the implementation of ComputePreAggregates() in the parent LpfPreMerger_Grid class. 
	\ingroup kyruntime_pathfinding */
class LpfPreMerger_Grid: public ILpfPreMerger
{
public:
	KY_DECLARE_LPFPREMERGER(LpfPreMerger_Grid)


public:
	/*!	Used internally to manage the current state of the computations carried out by LpfPreMerger_Grid. */
	enum Status
	{
		PMGS_Idle = 0, /*!< Indicates that all computations have been finished, and the LpfPreMerger_Grid is ready to accept new work. */
		PMGS_WriteInputs, /*!< Indicates that the LpfPreMerger_Grid is updating the PlgMrg_Grid_Merger with the inputs required for it to merge the polylines. */
		PMGS_Merge, /*!< Indicates that the LpfPreMerger_Grid is invoking the PlgMrg_Grid_Merger to merge obstacle polylines. */
		PMGS_ReadOutputs, /*!< Indicates that the LpfPreMerger_Grid is retrieving the merged polylines generated by PlgMrg_Grid_Merger. */
		PMGS_Error_Merge, /*!< Indicates that the LpfPreMerger_Grid is attempting to handle an error that occurred during the merging process. */
		PMGS_DUNMMY__ = KYFORCEENUMSIZEINT
	};


public:
	// Methods inherited from ILpfPreMerger
	/*!	\label_baseconstructor
		\param context				The \LpfContext that will own this object. */
	LpfPreMerger_Grid(ILpfManager::LpfContext& context):
		ILpfPreMerger(context),
		m_status(PMGS_Idle), m_orIt1(), m_orIt2() {}

	/*!	\label_virtualdestructor */
	~LpfPreMerger_Grid();

	/*!	\label_init For internal use.
		\param config			The instance of LpfPreMerger_Grid_Config used to initialize the \LpfContext. */
	KyResult Initialize(const LpfPreMerger_Grid_Config& config);

	virtual void ReInit();

	virtual KyBool ComputePreAggregates(List<ILpfManager::LpfPreAggregateComputationRequest*>& preAggregrateComputationRequests);

	KyBool ComputePreAggregate(ILpfManager::LpfNavFloorRecord* navFloorRecord, 
		SharedPoolList<ILpfManager::LpfObstacleRecord*>* obstacles, 
		SharedPoolList<ILpfManager::LpfPreAggregatePtr>* preAggregates);

	// Specific methods
	/*!	Sets the precision of the grid used to merge obstacles. */
	void SetPrecision(KyFloat32 newValue);

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

protected:
	PlgMrg_Grid_Merger m_merger;
	Status m_status;
	SharedPoolList<ILpfManager::LpfObstacleRecord*>::Iterator m_orIt1;
	SharedPoolList<ILpfManager::LpfObstacleRecord*>::Iterator m_orIt2;
	KyUInt32 m_contourId;

	LpfPreMerger_Grid_Config m_config;

	KyFloat32 m_currentPrecision;
	KyUInt32 m_currentMaxContours;
};

} // namespace Kaim

#endif // KY_LPFPREMERGER_GRID_H
