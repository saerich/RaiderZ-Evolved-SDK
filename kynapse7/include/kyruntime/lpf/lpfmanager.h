/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_lpf */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_LPFMANAGER_H
#define KY_LPFMANAGER_H

#include <kyruntime/lpf/ilpfmanager.h>


namespace Kaim
{

class IConstraint;


/*!	This class provides a concrete implementation of ILpfManager that you can
	configure in your World.
	\pp See also :USERGUIDE:"Using Obstacles".
	\ingroup kyruntime_lpf */
class LpfManager: public ILpfManager
{
public:
	KY_DECLARE_WORLDSERVICE(LpfManager)

	/*!	\label_baseconstructor */
	explicit LpfManager(World& world);
	
	/*!	\label_virtualdestructor */
	~LpfManager();

	/*!	\label_init
		\param maxContexts			The maximum number of \LpfContexts that this LpfManager will be able to manage.
		\param maxAreaPoint			Determines the size of the internal memory buffers reserved by the LpfManager for storing points in the polylines
									it generates around \LpfAreas during the process of merging together two overlapping \LpfAreas. 
									\pp This parameter has a direct and significant effect on the memory requirements of \SDKName. Although this parameter
									is optional, its default value is set high enough to support very large numbers of intersecting obstacles within each
									LpfContext. You may therefore be able to realize significant savings in memory by providing a value that more accurately
									reflects the expected use of dynamic obstacles in your game. 
									\pp For best performance, set the value of this parameter as follows:
									-	Determine the maximum total number of points you expect to be contained in all polylines that surround any LpfArea.
										This value depends on the number of obstacles that you expect to intersect at runtime, and also on the geometrical
										complexity of those intersections. 
										For example, in the image below, a collection of obstacles of different sizes intersecting in free rotation produces
										a complex outline containing a total of 34 points for only seven square obstacles: 24 in the outer polyline, and 10
										in the inner polyline. 
										\image html "graphics/lpf_maxareapointcount1.png"
									-	However, if the size, rotation and position of the dynamic obstacles are restricted by the \gameOrSim engine, the
										value of this parameter may be set much lower. For example, in the image below, the obstacles are of only two sizes,
										and must align to a grid; this produces a much simpler compound polyline requiring only nine points for four obstacles. 
										\image html "graphics/lpf_maxareapointcount2.png"
										Note that the points in the middle of the straight sides of the LpfArea are not simplified further. As each obstacle
										is merged, the existing polyline is clipped or extended by the outline of the added obstacle. For example, in the
										LpfArea shown above, the obstacles were added in clockwise order. 
									-	Because the process of merging obstacle polylines is iterative, it is also recommended that you slightly increase
										the value arrived at above, in order to support intermediate states that can arise during the merging process. The
										size of the point array should have enough "headroom" to accommodate the addition of a new obstacle, even if the
										final polyline is once again simplified to fewer points. Typically, this headroom should equal the number of points
										contained in your most complex obstacle outline. 
										For example, in the image below, the polyline surrounding the LpfArea contains nine points both before and after
										merging the large block shown in red. However, during the merge, the polyline contains those nine points plus the
										four points that delineate the added large block, until the inner points are removed when the polyline is simplified.
										\image html "graphics/lpf_maxareapointcount3.png"
		\param lpfAsynModule			The instance of LpfManager_AsyncModule set up for the AsyncManager that will be used to manage the asynchronous
										processing of the LpfContext update cycle, or #KY_NULL if none.
		\param tmpPathPointMax			Sizes an internal buffer used to store temporary paths during the path calculation process.
		\return KyResult::Success if the initialization completed successfully. */
	KyResult Initialize(KyUInt32 maxContexts=1, KyUInt32 maxAreaPoint=40, LpfManager_AsyncModule* lpfAsynModule=KY_NULL, KyInt32 tmpPathPointMax=32);

	IConstraint* m_lpfConstraint; /*!< An instance of LpfConstraint used for A* computations and path following. For internal use. Do not modify. */ 
};

} // namespace Kaim

#endif // KY_LPFMANAGER_H
