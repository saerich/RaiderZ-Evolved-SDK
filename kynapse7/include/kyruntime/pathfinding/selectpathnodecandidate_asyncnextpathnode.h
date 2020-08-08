/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_pathfinding */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_SELECTPATHNODECANDIDATE_ASYNCNEXTPATHNODE_H
#define KY_SELECTPATHNODECANDIDATE_ASYNCNEXTPATHNODE_H

#include <kyruntime/pathfinding/selectpathnodecandidate_nextpathnode.h>


namespace Kaim
{

class SelectPathNodeCandidate_AsyncNextPathNode_AsyncModule;


/*!	SelectPathNodeCandidate_AsyncNextPathNode derives from SelectPathNodeCandidate_NextPathNode,
	and uses the same process to choose the next PathNode the Bot will target. However, it 
	uses the asynchronous processing framework to distribute calculations to a secondary thread 
	or subordinate processing unit.
	\pp See also :USERGUIDE:"Asynchronous Processing".
	\section asyncnpndasetup Setting Up SelectPathNodeCandidate_AsyncNextPathNode
	Setting up this modifier involves the following considerations.
	\subsection requirements Requirements
	-	You must activate the AsyncManager in your World through a call to World::ActivateService().
	-	You must call AsyncManager::AddModule() in order to set up an instance of an AsyncModule class that derives from 
		SelectPathNodeCandidate_AsyncNextPathNode_AsyncModule, and you must call the Initialize() method of the instance
		returned. You can provide this instance in the call to Initialize(), but if you do not, OnInitialize() will attempt
		to retrieve the instance automatically from the AsyncManager.
	\ingroup kyruntime_pathfinding*/
class SelectPathNodeCandidate_AsyncNextPathNode: public SelectPathNodeCandidate_NextPathNode
{
public:
	KY_DECLARE_SELECTPATHNODECANDIDATE(SelectPathNodeCandidate_AsyncNextPathNode)


public:
	/*!	\label_constructor
		\param asyncModule					An instance of SelectPathNodeCandidate_AsyncNextPathNode_AsyncModule that you have set up for the AsyncManager.
											If you do not provide it here, OnInitialize() will attempt to retrieve it automatically from the AsyncManager. */
	SelectPathNodeCandidate_AsyncNextPathNode(SelectPathNodeCandidate_AsyncNextPathNode_AsyncModule* asyncModule=KY_NULL);

	virtual void ReInit();

	virtual KyBool ValidateCandidate(const PointWrapper& currentPosition,
		const PointWrapper& currentPathNode, const PointWrapper& candidate);

	virtual PathNodeIterator CheckLastPathNodeCandidate(const Path& path,
		const PathNodeIterator& currentPathNode);

	// async process

	/*!	Called by the AsyncModule corresponding to this modifier in order to begin the asynchronous
		calculations. */
	void AsyncValidateCandidate();

	// interface for the async module

	/*!	Retrieves the position of the Bot specified in the last call to ValidateCandidate(). */
	const PointWrapper& GetPosition() const { return m_position; }
	
	/*!	Retrieves the position of the last candidate PathNode tested by ValidateCandidate(). */
	const PointWrapper& GetCandidate() const { return m_candidate; }
	
	/*!	Called internally to indicate the results of the asynchronous calculations. */
	void SetAsyncCandidateResult(KyBool result);


protected:
	virtual void OnSetPathFinder();

	SelectPathNodeCandidate_AsyncNextPathNode_AsyncModule* m_asyncModule;

	// state flags
	KyBool m_asyncRequestSent;
	KyBool m_asyncResultReceived;

	// input stored for later processing
	PointWrapper m_position;
	PointWrapper m_candidate;
	Bot* m_bot;	

	// output returned by the async processing
	KyBool m_result;
};

} // namespace Kaim

#endif // KY_SELECTPATHNODECANDIDATE_ASYNCNEXTPATHNODE_H
