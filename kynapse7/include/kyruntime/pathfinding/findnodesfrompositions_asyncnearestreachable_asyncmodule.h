/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_pathfinding */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_FINDNODESFROMPOSITIONS_ASYNCNEARESTREACHABLE_ASYNCMODULE_H
#define KY_FINDNODESFROMPOSITIONS_ASYNCNEARESTREACHABLE_ASYNCMODULE_H

#include <kyruntime/asyncmanagement/asyncmodule.h>
#include <kyruntime/pathfinding/findnodesfrompositions_asyncnearestreachable.h>


namespace Kaim
{

class FindNodesFromPositions_AsyncNearestReachable;


/*!	FindNodesFromPositions_AsyncNearestReachable_AsyncModule provides an abstract base class for
	\AsyncModules that manage the calculations performed by the FindNodesFromPositions_AsyncNearestReachable 
	pathfinding modifier. This base class is specialized for use on different platforms.
	\ingroup kyruntime_pathfinding */
class FindNodesFromPositions_AsyncNearestReachable_AsyncModule: public AsyncModule
{
public:

	/*!	\label_baseconstructor */
	FindNodesFromPositions_AsyncNearestReachable_AsyncModule(AsyncManager& asyncMgr): AsyncModule(asyncMgr) {}; 

	/*!	Registers a new request for an asynchronous calculation. Called automatically by
		FindNodesFromPositions_AsyncNearestReachable when an instance of the PathFinder 
		needs to calculate a new path.
		\return #KY_TRUE if the new request was successfully added, or #KY_FALSE if the request could
				not be added to the list maintained by the AsyncModule. #KY_FALSE may indicate that
				the size of the list has been exhausted. */
	virtual KyBool AddRequest_Start(FindNodesFromPositions_AsyncNearestReachable& owner) = 0;

	/*!	Registers a new request for an asynchronous calculation. Called automatically by
		FindNodesFromPositions_AsyncNearestReachable when an instance of the PathFinder 
		needs to calculate a new path.
		\return #KY_TRUE if the new request was successfully added, or #KY_FALSE if the request could
				not be added to the list maintained by the AsyncModule. #KY_FALSE may indicate that
				the size of the list has been exhausted. */
	virtual KyBool AddRequest_Dest(FindNodesFromPositions_AsyncNearestReachable& owner) = 0;

public:
	static AsyncModuleClass Class; /*!< The MetaClass of this AsyncModule. */ // Replaces the KY_DECLARE... for virtual classes
};

} // namespace Kaim

#endif // KY_FINDNODESFROMPOSITIONS_ASYNCNEARESTREACHABLE_ASYNCMODULE_H
