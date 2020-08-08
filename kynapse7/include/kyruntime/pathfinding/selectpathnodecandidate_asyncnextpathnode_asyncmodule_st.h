/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_pathfinding */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_SELECTPATHNODECANDIDATE_ASYNCNEXTPATHNODE_ASYNCMODULE_ST_H
#define KY_SELECTPATHNODECANDIDATE_ASYNCNEXTPATHNODE_ASYNCMODULE_ST_H

#include <kyruntime/pathfinding/selectpathnodecandidate_asyncnextpathnode_asyncmodule.h>

#include <kyruntime/asyncmanagement/asyncmodule.h>


namespace Kaim
{
//
// Asynchroneous SelectPathNodeCandidate, running in the Single Thread (ST) created by the AsyncManager service
//

/*!	This implementation of SelectPathNodeCandidate_AsyncNextPathNode_AsyncModule manages
	the calculations performed by the SelectPathNodeCandidate_AsyncNextPathNode pathfinding 
	modifier on Windows and Xbox 360 platforms, using a secondary thread. */
class SelectPathNodeCandidate_AsyncNextPathNode_AsyncModule_ST:
				public SelectPathNodeCandidate_AsyncNextPathNode_AsyncModule
{
public:
	/*!	\label_baseconstructor */
	SelectPathNodeCandidate_AsyncNextPathNode_AsyncModule_ST(AsyncManager& asyncMgr);
	
	/*!	\label_virtualdestructor */
	virtual ~SelectPathNodeCandidate_AsyncNextPathNode_AsyncModule_ST();
	KY_DECLARE_ASYNC_MODULE(SelectPathNodeCandidate_AsyncNextPathNode_AsyncModule_ST)

	virtual void BeginAsyncProcessing();
};

} // namespace Kaim

#endif // KY_SELECTPATHNODECANDIDATE_ASYNCNEXTPATHNODE_ASYNCMODULE_ST_H
