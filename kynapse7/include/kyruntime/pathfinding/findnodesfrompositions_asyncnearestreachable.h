/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_pathfinding */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_FINDNODESFROMPOSITIONS_ASYNCNEAREASTREACHABLE_H
#define KY_FINDNODESFROMPOSITIONS_ASYNCNEAREASTREACHABLE_H

#include <kyruntime/pathfinding/findnodesfrompositions_nearestreachable.h>
#include <kyruntime/pathfinding/findnodesfrompositions_asyncnearestreachable_asyncmodule.h>


namespace Kaim
{

class FindNodesFromPositions_AsyncNearestReachable;
class FindNodesFromPositions_AsyncNearestReachable_AsyncModule;


/*!	FindNodesFromPositions_AsyncNearestReachable derives from FindNodesFromPositions_NearestReachable,
	and uses the same process to calculate the nearest reachable vertices to the starting
	point and destination point for the path computation. However, it uses the asynchronous processing
	framework to distribute calculations to a secondary thread or subordinate processing unit.
	\pp See also :USERGUIDE:"Asynchronous Processing".
	\section asyncnearest Setting Up FindNodesFromPositions_AsyncNearestReachable
	Setting up this modifier involves the following considerations.
	\subsection requirements Requirements
	-	You must activate the AsyncManager in your World through a call to World::ActivateService().
	-	You must call AsyncManager::AddModule() in order to set up an instance of an AsyncModule class that derives from 
		FindNodesFromPositions_AsyncNearestReachable_AsyncModule, and you must call the Initialize() method of the instance
		returned. You can provide this instance in the call to Initialize(), but if you do not, OnInitialize() will attempt
		to retrieve the instance automatically from the AsyncManager.
	\ingroup kyruntime_pathfinding */
class FindNodesFromPositions_AsyncNearestReachable: public FindNodesFromPositions_NearestReachable
{
public:
	KY_DECLARE_FINDNODESFROMPOSITIONS(FindNodesFromPositions_AsyncNearestReachable)


public:
	/*!	\label_constructor
		\param asyncFindNodesModule			An instance of FindNodesFromPositions_AsyncNearestReachable_AsyncModule that you have set up for the AsyncManager.
											If you do not provide it here, OnInitialize() will attempt to retrieve it automatically from the AsyncManager. */
	FindNodesFromPositions_AsyncNearestReachable(FindNodesFromPositions_AsyncNearestReachable_AsyncModule* asyncFindNodesModule =  KY_NULL);

	/*!	\label_virtualdestructor */
	virtual ~FindNodesFromPositions_AsyncNearestReachable();

	virtual void ReInit();

	virtual KyBool FindStartNode(const PointWrapper& start, GraphVertexSafePtr& startNodeSafePtr);
	virtual KyBool FindDestNode(const PointWrapper& dest, GraphVertexSafePtr& destNodeSafePtr);

	/*!	Used by the PlayStation 3 implementation of the AsyncModule corresponding to this modifier, in order
		to calculate the cost of each candidate for the starting vertex. The default implementation invokes
		the IGoto modifier currently in use by the PathFinder. */
	virtual KyFloat32 GetCostToNode(const PointWrapper& node);
	
	/*!	Used by the PlayStation 3 implementation of the AsyncModule corresponding to this modifier, in order
		to calculate the cost of each candidate for the destination vertex. The default implementation invokes
		the IGoto modifier currently in use by the PathFinder. */
	virtual KyFloat32 GetCostFromNode(const PointWrapper& node);

	// async process

	/*!	Called by the AsyncModule corresponding to this modifier in order to begin the asynchronous
		calculations. */
	void AsyncFindStartNode();

	/*!	Called by the AsyncModule corresponding to this modifier in order to begin the asynchronous
		calculations. */
	void AsyncFindDestNode();

	// interface to async module
	/*!	Retrieves the last starting position passed to FindStartNode() by the PathFinder. */
	const Vec3f& GetStart() const { return m_start.GetPosition(); }
	
	/*!	Retrieves the last destination position passed to FindDestNode() by the PathFinder. */
	const Vec3f& GetDest() const { return m_dest.GetPosition(); }
	
	/*!	Retrieves the UID of the last starting vertex calculated by this modifier. */
	GraphVertexSafePtr GetStartUID() const { return m_startSafePtrResult; }
	
	/*!	Retrieves the UID of the last destination vertex calculated by this modifier. */
	GraphVertexSafePtr GetDestUID() const { return m_destSafePtrResult; }

	/*!	Sets the UID of the last starting vertex calculated by this modifier. For internal use. */
	void SetStartUIDResult(const GraphVertexSafePtr& startUIDResult, KyBool noNavFloorFound) { m_startSafePtrResult = startUIDResult; m_startHasNoLM = noNavFloorFound; m_asyncStartResult = KY_TRUE; }

	/*!	Sets the UID of the last destination vertex calculated by this modifier. For internal use. */
	void SetDestUIDResult(const GraphVertexSafePtr& destUIDResult) { m_destSafePtrResult = destUIDResult; m_asyncDestResult = KY_TRUE; }


protected:
	virtual void OnSetPathFinder();

	FindNodesFromPositions_AsyncNearestReachable_AsyncModule* m_asyncFindNodesModule;

	// inputs stored for latter processing
	PointWrapper m_start;
	PointWrapper m_dest;
	PointInfo_VertexSafePtr m_start_VertexSafePtrInfo;
	PointInfo_VertexSafePtr m_dest_VertexSafePtrInfo;
	Bot* m_bot;	


	// state flags
	KyBool m_asyncStartResult;
	KyBool m_asyncDestResult;

	// output returned by async processing
	GraphVertexSafePtr m_startSafePtrResult;
	GraphVertexSafePtr m_destSafePtrResult;
	KyBool m_startHasNoLM;
};

} // namespace Kaim

#endif // KY_FINDNODESFROMPOSITIONS_ASYNCNEAREASTREACHABLE_H
