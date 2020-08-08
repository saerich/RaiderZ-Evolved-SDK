/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_pathfinding */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_FINDNODESFROMPOSITIONS_ASYNCNEARESTREACHABLE_ASYNCMODULE_ST_H
#define KY_FINDNODESFROMPOSITIONS_ASYNCNEARESTREACHABLE_ASYNCMODULE_ST_H

#include <kyruntime/pathfinding/findnodesfrompositions_asyncnearestreachable_asyncmodule.h>


namespace Kaim
{
//
// Asynchroneous CFindNodesFromPositions, running in the Single Thread (ST) created by the AsyncManager service
//
/*!	This implementation of the FindNodesFromPositions_AsyncNearestReachable_AsyncModule class manages
	the calculations performed by the FindNodesFromPositions_AsyncNearestReachable pathfinding 
	modifier on Windows and Xbox 360 platforms, using a secondary thread.
	\ingroup kyruntime_pathfinding */
class FindNodesFromPositions_AsyncNearestReachable_AsyncModule_ST:
				public FindNodesFromPositions_AsyncNearestReachable_AsyncModule
{
public:

	/*!	\label_baseconstructor */
	FindNodesFromPositions_AsyncNearestReachable_AsyncModule_ST(AsyncManager& asyncMgr);

	/*!	\label_virtualdestructor */
	virtual ~FindNodesFromPositions_AsyncNearestReachable_AsyncModule_ST(); 
	KY_DECLARE_ASYNC_MODULE(FindNodesFromPositions_AsyncNearestReachable_AsyncModule_ST)

	/*!	label_init
		\param maxSimultaneousRequests			Determines the maximum number of simultaneous requests this AsyncModule can accept. */
	KyResult Initialize(KyUInt32 maxSimultaneousRequests);

	virtual void BeginAsyncProcessing();

	virtual KyBool AddRequest_Start(FindNodesFromPositions_AsyncNearestReachable& owner);
	virtual KyBool AddRequest_Dest(FindNodesFromPositions_AsyncNearestReachable& owner);

protected:
	//
	// Configuration settings
	//

	/* Request queue size */
	KyUInt32 m_maxSimultaneousRequests;

	//
	// Work variables
	//

	FindNodesFromPositions_AsyncNearestReachable** m_startRequests;
	KyUInt32 m_startRequestCount;

	FindNodesFromPositions_AsyncNearestReachable** m_destRequests;
	KyUInt32 m_destRequestCount;
};

} // namespace Kaim

#endif // KY_FINDNODESFROMPOSITIONS_ASYNCNEARESTREACHABLE_ASYNCMODULE_ST_H
