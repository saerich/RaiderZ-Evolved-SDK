/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_pathfinding */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_FINDNODESFROMPOSITIONS_NEAREASTREACHABLE_H
#define KY_FINDNODESFROMPOSITIONS_NEAREASTREACHABLE_H

#include <kyruntime/pathfinding/ifindnodesfrompositions.h>

#include <kyruntime/graph/pointinfo_vertexsafeptr.h>
#include <kyruntime/graph/graphvertexptr.h>


namespace Kaim
{
/*!	This implementation of IFindNodesFromPositions selects as the starting vertex for the next
	path calculation the nearest reachable vertex to the current position of the Bot. As the ending vertex,
	it selects the nearest reachable vertex to the final destination point. 
	\pp This modifier invokes GraphManager::FindNearbySortedVertexSafePtrs() to find a list of possible candidate vertices. 
	This list is sorted according to the cost calculated by the GetCostFunc_Start() and GetCostFunc_Dest() methods. 
	By default, these methods compute the cost of each candidate vertex by invoking the current IConstraint used by 
	the PathFinder. Once the list is sorted by cost, the reachability of each candidate is tested in order
	by calling the CanGo_Start() and CanGo_Dest() methods. By default, these methods compute the reachability of 
	the candidate by invoking the ICanGo modifier in use by the PathFinder. The first reachable candidate in
	the list is chosen.
	\ingroup kyruntime_pathfinding */
class FindNodesFromPositions_NearestReachable: public IFindNodesFromPositions
{
public:
	enum Status
	{
		Idle = 0,
		SearchingStartNodes,
		SearchingDestNodes,
		dummy_forceenumsize = KYFORCEENUMSIZEINT
	};

public:
	KY_DECLARE_FINDNODESFROMPOSITIONS(FindNodesFromPositions_NearestReachable)


public:
	/*!	\label_constructor
		\param searchSpaceSize			Determines the amount of memory allocated by FindNodesFromPositions_NearestReachable
										to store candidate vertices retrieved from the GraphManager.
										\units			bytes
										\acceptedvalues	any positive value
										\defaultvalue	\c 128 */
	FindNodesFromPositions_NearestReachable(KyUInt32 searchSpaceSize=128);
	
	/*!	\label_virtualdestructor */
	virtual ~FindNodesFromPositions_NearestReachable();

	void ReInit();

	virtual KyBool FindStartNode(const PointWrapper& start, GraphVertexSafePtr& startNodeSafePtr);
	virtual KyBool FindDestNode(const PointWrapper& dest, GraphVertexSafePtr& destNodeSafePtr);

	/*!	FindStartNode() passes this method to GraphManager::FindNearbySortedVertexSafePtrs() in order to sort
		the vertices found near the starting point in order of increasing cost. The default implementation
		transparently invokes the IConstraint currently in use by the PathFinder. You 
		can redefine this method in a derived class in order to prioritize a different set of criteria. 
		\param pos			The starting position for the path calculation.
		\param vec			A candidate vertex near the starting position.
		\param[out] cost	Stores the cost computed by this method for the specified vertex.
		\return #KY_TRUE if the \c cost parameter was successfully updated, or #KY_FALSE otherwise.*/
	virtual KyBool GetCostFunc_Start(const Vec3f& pos, const GraphVertexPtr& vec, KyFloat32& cost);
	
	/*!	FindDestNode() passes this method to GraphManager::FindNearbySortedVertexSafePtrs() in order to sort
		the vertices found near the destination point in order of increasing cost. The default implementation
		transparently invokes the IConstraint currently in use by the PathFinder. You 
		can redefine this method in a derived class in order to prioritize a different set of criteria. 
		\param pos			The final destination for the path calculation.
		\param vec			A candidate vertex near the destination.
		\param[out] cost	Stores the cost computed by this method for the specified vertex.
		\return #KY_TRUE if the \c cost parameter was successfully updated, or #KY_FALSE otherwise. */
	virtual KyBool GetCostFunc_Dest(const Vec3f& pos, const GraphVertexPtr& vec, KyFloat32& cost);
	
	/*!	FindStartNode() invokes this method to test each candidate starting vertex in order of cost, to verify 
		that the candidate is directly reachable from the starting position. The default implementation invokes
		the ICanGo modifier in use by the PathFinder. You can redefine this method in a derived 
		class in order to use a different set of criteria.
		\param pos			The starting position for the path calculation.
		\param node			The candidate vertex being tested.
		\return #KY_TRUE if the vertex is accessible from the starting position, or #KY_FALSE otherwise. */
	virtual KyBool CanGo_Start(const PointWrapper& pos, const GraphVertexPtr& node);
	
	/*!	FindDestNode() invokes this method to test each candidate destination vertex in order of cost, to verify 
		that the candidate is directly reachable from the final destination position. The default implementation invokes
		the ICanGo modifier in use by the PathFinder. You can redefine this method in a derived 
		class in order to use a different set of criteria.
		\param pos			The destination position for the path calculation.
		\param node			The candidate vertex being tested.
		\return #KY_TRUE if the vertex is accessible from the final destination position, or #KY_FALSE otherwise. */
	virtual KyBool CanGo_Dest(const GraphVertexPtr& node, const PointWrapper& pos);

protected:
	virtual void OnSetPathFinder();

	KyUInt32 m_searchSpaceSize; /*< For internal use. Do not modify. */

	Status m_status; /*< For internal use. Do not modify. */
	KyUInt32 m_searchSpaceId; /*< For internal use. Do not modify. */
	KyUInt32 m_searchSpaceIdEnd; /*< For internal use. Do not modify. */
	GraphVertexSafePtr* m_searchSpace; /*< For internal use. Do not modify. */
	Vec3f m_start; /*< For internal use. Do not modify. */
	Vec3f m_dest; /*< For internal use. Do not modify. */
	PointWrapper m_pw1; /*< For internal use. Do not modify. */
	PointWrapper m_pw2; /*< For internal use. Do not modify. */
	PointInfo_VertexSafePtr m_vertexSafePtrInfo; /*< For internal use. Do not modify. */
	KyBool m_limitAltitudeRange;

private :
	void InvalidateSearchSpace();
};

} // namespace Kaim

#endif // KY_FINDNODESFROMPOSITIONS_NEAREASTREACHABLE_H
