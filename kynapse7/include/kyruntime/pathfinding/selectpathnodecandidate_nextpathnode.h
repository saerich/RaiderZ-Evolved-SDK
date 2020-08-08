/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/




/*!	\file
	\ingroup kyruntime_pathfinding */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_SELECTPATHNODECANDIDATE_NEXTPATHNODE_H
#define KY_SELECTPATHNODECANDIDATE_NEXTPATHNODE_H

#include <kyruntime/pathfinding/iselectpathnodecandidate.h>


namespace Kaim
{
/*!	This implementation of the ISelectPathNodeCandidate interface suggests the next PathNode 
	in the Path as a candidate, until one of the following conditions is reached:
	-	The distance to the last candidate checked exceeds the maximum distance specified by
		the #m_maxDistPathNode member.
	-	It reaches the maximum number of candidate \PathNodes that it is allowed to process in a 
		single frame, set by the #m_maxCandidatePerFrame member.
	-	It considers any candidate PathNode that lies at the end of a spatial graph edge that is
		linked to a PathObject whose IPathObjectBehavior::CanByPassEntrance() method returns
		#KY_FALSE. This indicates that the Bot must follow the edge exactly, without skipping the
		PathObject. 
	\pp This process allows the PathFinder to look forward along the Path past a certain number of
	blocked \PathNodes in order to adopt a reachable PathNode farther along the Path. For example, 
	if an obstacle intervenes between a Bot and its upcoming \PathNodes, but the Bot can move directly 
	to a PathNode farther along the Path, this modifier can skip past the inaccessible candidates and 
	find the accessible candidate farther along the Path.
	\pp The maximum number of consecutive unreachable \PathNodes that this modifier will look ahead
	is set by a call to NodesToSkip(KyUInt32) (default value \c 4). After this many candidates
	are refused by the PathFinder as being unreachable, SelectPathNodeCandidate_NextPathNode
	returns to the PathNode that follows the current PathNode and begins to look ahead again
	in case one of the previously unreachable PathNodes have become reachable in the meantime.
	\ingroup kyruntime_pathfinding */
class SelectPathNodeCandidate_NextPathNode: public ISelectPathNodeCandidate
{
public:
	KY_DECLARE_SELECTPATHNODECANDIDATE(SelectPathNodeCandidate_NextPathNode)


public:
	/*!	\label_constructor */
	SelectPathNodeCandidate_NextPathNode();

	/*!	\label_virtualdestructor */
	virtual ~SelectPathNodeCandidate_NextPathNode();

	virtual void ReInit();

	/*!	\copydoc ISelectPathNodeCandidate::SelectPathNodeCandidate()
		For details on this implementation of ISelectPathNodeCandidate::SelectPathNodeCandidate(), see
		the class description. */
	virtual PathNodeIterator SelectPathNodeCandidate(Path& path,
		PathNodeIterator& currentPathNode);

	/*!	\copydoc ISelectPathNodeCandidate::EvaluateShortcut()
		\pp This implementation of ISelectPathNodeCandidate::EvaluateShortcut() evaluates the cost of moving 
		to the candidate point directly by calling the PathFinder EvaluateShortcut method. If
		the shortcut cost is higher than the cost of moving along the Path, it rejects the candidate. */
	virtual KyBool EvaluateShortcut(const PointWrapper& currentPosition, const PathNodeIterator& currentPathNode,
		const PathNodeIterator& candidatePathNode, const PointWrapper& candidatePosition);

	/*!	\copydoc ISelectPathNodeCandidate::ValidateCandidate()
		\pp This implementation of ISelectPathNodeCandidate::ValidateCandidate() evaluates whether or not the 
		candidate point is accessible directly by calling the ICanGo modifier currently in use by the 
		PathFinder. If the point is not reachable directly, the candidate is rejected. */
	virtual KyBool ValidateCandidate(const PointWrapper& currentPosition,
		const PointWrapper& currentPathNode, const PointWrapper& candidate);

	/*! Retrieves the value set through a call to MaxDistPathNode(KyFloat32). */
	KyFloat32 MaxDistPathNode() const { return m_maxDistPathNode; }

	/*! Determines the maximum distance a PathNnode may be from the current Botin order to be considered as a candidate.
		\units			meters
		\acceptedvalues	any positive value
		\defaultvalue	\c 50.0f */
	void MaxDistPathNode(KyFloat32 val)
	{
		KY_FUNCTION("SelectPathNodeCandidate_NextPathNode::MaxDistPathNode");
		KY_ASSERT(val > 0.f, ("Invalid value (%.2f), 'MaxDistPathNode' must be > 0.", val));
		m_maxDistPathNode = val;
	}
	
	/*! Retrieves the value set through a call to MaxCandidatePerFrame(KyUInt32). */
	KyUInt32 MaxCandidatePerFrame() const { return m_maxCandidatePerFrame; }

	/*! Determines the maximum number of PathNode candidates that may be checked each frame.
		\acceptedvalues	any positive value
		\defaultvalue	\c 2 */
	void MaxCandidatePerFrame(KyUInt32 val)
	{ 
		KY_FUNCTION("SelectPathNodeCandidate_NextPathNode::MaxCandidatePerFrame");
		KY_ASSERT(val > 0.f, ("Invalid value (%u), 'MaxCandidatePerFrame' must be > 0.", val));
		m_maxCandidatePerFrame = val;
	}
	
	KyUInt32 NodesToSkip() const { return m_nbNodesToSkip; }

	/*! Determines the maximum number of inaccessible path nodes that this modifier will check at each frame before 
		abandoning the search for a better candidate. Once this maximum value is reached, the modifier returns the
		last accessible PathNode found along the Path.
		\acceptedvalues	any positive value
		\defaultvalue	\c 4 */
	void NodesToSkip(KyUInt32 val) 
	{
		KY_FUNCTION("SelectPathNodeCandidate_NextPathNode::NodesToSkip");
		KY_ASSERT(val >= 0.f, ("Invalid value (%u), 'NodesToSkip' must be >= 0.", val));
		m_nbNodesToSkip = val; 
	}

protected:
	/* Tell if we can continue to propose candidate. manage m_maxDistPathNode and m_maxCandidatePerFrame. */
	virtual KyBool CanContinueToProcess(Path& path, PathNodeIterator& currentPathNode);
	/* Return the next candidate or KY_NULL. */
	virtual PathNodeIterator NextCandidate(Path& path, PathNodeIterator& currentPathNode);

	PathNodeIterator CheckDestination(Path& path, const PathNodeIterator& currentPathNode);

	virtual void OnSetPathFinder();

protected:
	KyFloat32 m_maxDistPathNode; /*!< \c MaxDistPathNode configuration parameter. */
	KyUInt32 m_maxCandidatePerFrame; /*!< \c MaxCandidatePerFrame configuration parameter. */
	KyUInt32 m_nbNodesToSkip; //< max number of invalid candidate to test.
	

	KyFloat32 m_frameTime; /*!< Marks the start time of the current frame. Do not modify. */
	KyUInt32 m_currentFrameCandidateCount; /*!< The number of candidates tested in the current frame. Do not modify. */

	KyUInt32 m_nthNodeToExplore; //< n th node to explore after the current.
	GraphVertexSafePtr m_nodeSafePtr; //< Last valid candidate uid.
};

} // namespace Kaim

#endif // KY_SELECTPATHNODECANDIDATE_NEXTPATHNODE_H
