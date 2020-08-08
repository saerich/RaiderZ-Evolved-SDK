/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_pathfinding */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_SELECTPATHNODE_ASYNCNEXTPATHNODE_ASYNCMODULE_H
#define KY_SELECTPATHNODE_ASYNCNEXTPATHNODE_ASYNCMODULE_H

#include <kyruntime/asyncmanagement/asyncmodule.h>


namespace Kaim
{

class SelectPathNodeCandidate_AsyncNextPathNode;


//
// SelectPathNodeCandidate Async Module, root class for platform-specific implementations
//

/*!	SelectPathNodeCandidate_AsyncNextPathNode_AsyncModule provides an abstract base class for
	\AsyncModules that manage the calculations performed by the 
	SelectPathNodeCandidate_AsyncNextPathNode pathfinding modifier. This base class is
	specialized for use on different platforms.
	\ingroup kyruntime_pathfinding */
class SelectPathNodeCandidate_AsyncNextPathNode_AsyncModule: public AsyncModule
{
public:
	/*!	\label_baseconstructor */
	SelectPathNodeCandidate_AsyncNextPathNode_AsyncModule(AsyncManager&);
	
	/*!	\label_virtualdestructor */
	virtual ~SelectPathNodeCandidate_AsyncNextPathNode_AsyncModule() = 0;

	/*!	label_init
		\param maxSimultaneousRequests			Determines the maximum number of simultaneous requests this AsyncModule can accept. */
	KyResult Initialize(KyUInt32 maxSimultaneousRequests);


	/**********************************************************************
	* Requests
	*
	* An periodic-aperiodic time-slicing is used to better spread the async
	* SelectPathNode calls over all the frames. Typical use from the modifier is :
	* 1- Start with timer = 0
	* 2- if timer>PeriodicTime, call a GrantAperiodicTask until served
	* 3- reset timer to 0
	**********************************************************************/

	/*!	Registers a new request for an asynchronous calculation. Called automatically by the
		SelectPathNodeCandidate_AsyncNextPathNode modifier when an instance of the PathFinder 
		needs to select a new target PathNode.
		\return #KY_TRUE if the new request was successfully added, or #KY_FALSE if the request could
				not be added to the list maintained by the async module. #KY_FALSE may indicate that
				the size of the list has been exhausted. */
	KyBool AddRequest(SelectPathNodeCandidate_AsyncNextPathNode& owner);

protected:
	//
	// Configuration settings
	//

	/* Request queue size - default to MAX_ENTITIES */
	KyUInt32 m_maxSimultaneousRequests;

	//
	// Work variables
	//

	SelectPathNodeCandidate_AsyncNextPathNode** m_requests;
	KyUInt32 m_requestCount;


public:
	static AsyncModuleClass Class; /*!< The metaclass of this AsyncModule. */ // Replaces the KY_DECLARE... for virtual classes
};

} // namespace Kaim

#endif // KY_SELECTPATHNODE_ASYNCNEXTPATHNODE_ASYNCMODULE_H
