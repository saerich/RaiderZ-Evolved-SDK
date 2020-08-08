/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download,
* or which otherwise accompanies this software in either electronic or hard copy form.
*/


// ---------- Primary contact: JUBA - secondary contact: NOBODY
#ifndef KyPathData_GraphcellGrid_H
#define KyPathData_GraphcellGrid_H

#include "kypathdata/graph/graphtypes.h"
#include "kypathdata/navmesh/navmeshtypes.h"
#include "kyruntime/graph/graphcellstitcher.h"
#include <kyruntime/graph/additionalgraphstitcher.h>
#include "kypathdata/containers/array.h"
#include "kypathdata/containers/list.h"
#include "kypathdata/generic/kyguid.h"
#include "kypathdata/generic/guidcompound.h"
#include "kyruntime/graph/edgelocker.h"
#include "kyruntime/graph/edgelockervolume.h"
#include "kyruntime/graph/graphcellgridcallbacks.h"

namespace Kaim
{

class AdditionalStitchedGraph;
class GraphCell;
class StitchedGraphCell;
class StitchedGraph;
class GraphStitchDataManager;
class ActiveGuids;

/* For internal use. */
class StitchedGraphCellVersions
{
public:

	bool operator ==(const StitchedGraphCellVersions& stitchedGraphCellVersions)
	{
		return &m_sharedPoolListOfStitchedGraphCell == &stitchedGraphCellVersions.m_sharedPoolListOfStitchedGraphCell;
	}

	SharedPoolList<StitchedGraphCell*> m_sharedPoolListOfStitchedGraphCell;
};

/* For internal use. */
class DynamicDataForCellPos
{
public:

	bool operator ==(const DynamicDataForCellPos& additionalGraphLinkedToThisPos)
	{
		return &m_sharedPoolListOfAdditionalGraphVertex == &additionalGraphLinkedToThisPos.m_sharedPoolListOfAdditionalGraphVertex;
	}

	SharedPoolList<GraphVertexPtr> m_sharedPoolListOfAdditionalGraphVertex;
	SharedPoolList<GraphVertexPtr> m_sharedPoolListOfVertexConnection;
	SharedPoolList<EdgeLockerVolume*> m_sharedPoolListOfEdgeLockerVolume;
};

/* For internal use. */
class GraphCellPosInfo
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	GraphCellPosInfo() : m_activeStitchedGraphCell(KY_NULL), m_stitchedGraphCellVersions(KY_NULL),
		m_dynamicDataForCellPos(KY_NULL) {}
	void Clear()
	{
		m_activeStitchedGraphCell = KY_NULL;
		m_stitchedGraphCellVersions = KY_NULL;
		m_dynamicDataForCellPos = KY_NULL;
	}

	StitchedGraphCell* m_activeStitchedGraphCell;
	StitchedGraphCellVersions* m_stitchedGraphCellVersions;
	DynamicDataForCellPos* m_dynamicDataForCellPos;
};

/* For internal use. */
class GraphCellGrid
{
	KY_DEFINE_NEW_DELETE_OPERATORS

private:
	GraphCellGrid& operator=(const GraphCellGrid& graphCellGrid);
	bool operator==(const GraphCellGrid& graphCellGrid) const;
	bool operator==(const GraphCellGrid& graphCellGrid);
	GraphCellGrid(const GraphCellGrid& graphCellGrid);

public:
	GraphCellGrid();

	~GraphCellGrid();


	void SetGraphManager(GraphManager& graphManager)
	{
		m_graphManager = &graphManager;
		m_additionalGraphStitcher.SetGraphManager(graphManager);
		m_edgeLocker.SetGraphManager(graphManager);
	}

	void Clear();

	bool IsCellAvailiable(const CellPos& pos) const;
	const GraphCell* GetGraphCell(const CellPos& pos) const;

	KY_FORCE_INLINE SharedPoolList<GraphVertexPtr>* GetSharedPoolListtOfConnectionPtrToThisGraphCell(const CellPos& pos) const
	{
		KY_FUNCTION("GraphCellGrid::GetStitchedGraphCell");
		KY_ASSERTK(m_sizeOfBuffer != 0);

		if (m_cellBox.IsInside(pos))
		{
			DynamicDataForCellPos* dynamicDataForCellPos = m_bufferOfGraphCellPosInfo[m_cellBox.GetRowMajorIndex(pos)].m_dynamicDataForCellPos;
			return dynamicDataForCellPos == KY_NULL ? KY_NULL : &dynamicDataForCellPos->m_sharedPoolListOfVertexConnection;
		}

		return KY_NULL;
	}

	KY_FORCE_INLINE SharedPoolList<GraphVertexPtr>* GetSharedPoolListtOfDynamicVertexLinkedToThisGraphCell(const CellPos& pos) const
	{
		KY_FUNCTION("GraphCellGrid::GetStitchedGraphCell");
		KY_ASSERTK(m_sizeOfBuffer != 0);

		if (m_cellBox.IsInside(pos))
		{
			DynamicDataForCellPos* dynamicDataForCellPos = m_bufferOfGraphCellPosInfo[m_cellBox.GetRowMajorIndex(pos)].m_dynamicDataForCellPos;
			return dynamicDataForCellPos == KY_NULL ? KY_NULL : &dynamicDataForCellPos->m_sharedPoolListOfAdditionalGraphVertex;
		}

		return KY_NULL;
	}

	KY_FORCE_INLINE SharedPoolList<EdgeLockerVolume*>* GetSharedPoolListtOfEdgeLockerVolumeAtCellPos(const CellPos& pos) const
	{
		KY_FUNCTION("GraphCellGrid::GetStitchedGraphCell");
		KY_ASSERTK(m_sizeOfBuffer != 0);

		if (m_cellBox.IsInside(pos))
		{
			DynamicDataForCellPos* dynamicDataForCellPos = m_bufferOfGraphCellPosInfo[m_cellBox.GetRowMajorIndex(pos)].m_dynamicDataForCellPos;
			return dynamicDataForCellPos == KY_NULL ? KY_NULL : &dynamicDataForCellPos->m_sharedPoolListOfEdgeLockerVolume;
		}

		return KY_NULL;
	}

	KY_FORCE_INLINE StitchedGraphCell* GetStitchedGraphCell(const CellPos& pos) const
	{
		KY_FUNCTION("GraphCellGrid::GetStitchedGraphCell");
		KY_ASSERTK(m_sizeOfBuffer != 0);

		return m_cellBox.IsInside(pos) ? m_bufferOfGraphCellPosInfo[m_cellBox.GetRowMajorIndex(pos)].m_activeStitchedGraphCell : KY_NULL;
	}

	void EnlargeGrid(const CellBox& cellBox);

	KyResult ResetGrid(const GraphStitchDataManager& stitchDataManager);

public:
	void InsertGraph(StitchedGraph& stitchedGraph);
	void RemoveGraph(StitchedGraph& stitchedGraph);

	void InsertAdditionalGraph(AdditionalStitchedGraph& additionalStitchedGraph);
	void RemoveAdditionalGraph(AdditionalStitchedGraph& additionalStitchedGraph);

	void AddVertexConnectionAtCellPos(const GraphVertexPtr& connectedVertex, const CellPos& cellPos);
	void RemoveVertexConnectionFromCellPos(const GraphVertexPtr& connectedVertex, const CellPos& cellPos);

	KyResult AddEdgeLockerVolumeAtCellPos(EdgeLockerVolume& edgeLockerVolume, const CellPos& cellPos);
	void RemoveEdgeLockerVolumeFromCellPos(EdgeLockerVolume& edgeLockerVolume, const CellPos& cellPos);

	KyUInt32 GetMaxVertexCount() const { return m_maxVertexCount; }

	void UpdateCellStitchingInBox(const CellBox& cellBox);

protected:
	void UpdateAdditionalGraphLinking();

	void AddAdditionalGraphVertexAtCellPos(const GraphVertexPtr& vertexPtr, const CellPos& cellPos);
	void RemoveAdditionalGraphVertexFromCellPos(const GraphVertexPtr& vertexPtr, const CellPos& cellPos);

	void AssignAstarIds(StitchedGraphCell& stitchedGraphCell);
	void RecycleAstarIds(StitchedGraphCell& stitchedGraphCell);

	KyResult InsertGraphCell(StitchedGraphCell& stitchedGraphCell, bool doesNewStitchedGraphCellCanBeActivated);
	KyResult RemoveGraphCell(StitchedGraphCell& stitchedGraphCell);

	void UnStitchedCellFromTheGrid(GraphCellPosInfo& graphCellPosInfo);
	void StitchedCellInTheGrid(GraphCellPosInfo& graphCellPosInfo);

	void UnLinkAllAdditionalGraphVertexFromCellPos(SharedPoolList<GraphVertexPtr>& sharedPoolListOfVertexConnection);

	void UnLockAllEdgesOfStitchedGraphCellInEdgeLockerVolumes(SharedPoolList<EdgeLockerVolume*>& sharedPoolListOfEdgeLockerVolume, StitchedGraphCell* stitchedGraphCell);
	void LockAllEdgesOfStitchedGraphCellInEdgeLockerVolumes(SharedPoolList<EdgeLockerVolume*>& sharedPoolListOfEdgeLockerVolume, StitchedGraphCell* stitchedGraphCell);

	DynamicDataForCellPos* CreateAndInitializeDynamicDataForCellPos();

	List<KyUInt32> m_freeAstarIds;
	KyUInt32 m_maxVertexCount;
public:
	CellBox m_cellBox;
	GraphManager* m_graphManager;

	SharedPoolList<StitchedGraphCell*>::Pool m_poolForSharedPoolListOfStitchedGraphCell;
	SharedPoolList<GraphVertexPtr>::Pool m_poolForSharedPoolListOfGraphVertexPtr;
	SharedPoolList<EdgeLockerVolume*>::Pool m_poolForSharedPoolListOfEdgeLockerVolume;

	GraphCellPosInfo* m_bufferOfGraphCellPosInfo;
	KyUInt32 m_sizeOfBuffer;
	GraphCellStitcher m_staticGraphStitcher;
	AdditionalGraphStitcher m_additionalGraphStitcher;
	EdgeLocker m_edgeLocker;

	List<StitchedGraphCellVersions> m_listOfStitchedGraphCellVersions;
	List<DynamicDataForCellPos> m_listOfDynamicDataForCellPos;
	List<GraphVertexPtr> m_listOfAdditionalGraphVertexToUpadte;

	GraphCellGridCallBacks m_callBacks;
public:
	ActiveGuids* m_activatedGuids;
};


}

#endif //KyPathData_GraphcellGrid_H

