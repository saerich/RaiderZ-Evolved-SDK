/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download,
* or which otherwise accompanies this software in either electronic or hard copy form.
*/




// primary contact: GUAL - secondary contact: NOBODY
#ifndef KyPdg_PdgPathDataFilter_H
#define KyPdg_PdgPathDataFilter_H


#include <kypathdata/navmesh/navmesh.h>
#include <kypathdata/graph/graph.h>
#include <kypdg/generator/pdgguidcompound.h>
#include <kypdg/pdginput/seedpoint.h>


#include "kypathdata/containers/kystring.h"
#include "kypdg/common/stlvector.h"

namespace Kaim
{

class PdgPathDataElement;
class NavMeshManager;

typedef KyUInt32 PdgConnectedComponentId;
static const PdgConnectedComponentId PdgConnectComponentTraversal_NEW = KyUInt32MAXVAL;
static const PdgConnectedComponentId PdgConnectComponentTraversal_OPEN = KyUInt32MAXVAL - 1;


class PdgPathDataFilter
{
	KY_DEFINE_NEW_DELETE_OPERATORS

	PdgPathDataFilter() : m_pathDataElement(KY_NULL) {}

public:
	PdgPathDataFilter(PdgPathDataElement& pathDataElement) : m_pathDataElement(&pathDataElement) {}

	~PdgPathDataFilter() { m_pathDataElement = KY_NULL; }


	KyResult PerformFilteringFromSeed(DynamicKynapseSeedPointArray& seedpoints);

	KyResult PerformFilteringFromSurfaces(const KyFloat32 minSurfaceOfConnectedComponent);

private:
	friend class PdgConnectComponentTraversal;

	enum ConnectedComponentStatus
	{
		ConnectedComponentStatus_NOSEED,
		ConnectedComponentStatus_SEED,
	};

	struct ConnectedComponentData
	{
		ConnectedComponentData(KyFloat32 surfaceInit) : m_status(ConnectedComponentStatus_NOSEED), m_surface(surfaceInit) {}

		ConnectedComponentStatus m_status;
		KyFloat32 m_surface;
	};

	typedef StlVector<ConnectedComponentData> ConnectedComponentDataTable;
	typedef StlVector<StlVector<PdgConnectedComponentId> > NavFloorConnectedComponentIdTable;

	void ResizeNavFloorConnectedComponentIdTable(NavFloorConnectedComponentIdTable& navFloorConnectedComponentIdTable);

	KyUInt32 ComputeConnectedComponent(NavMeshManager& navMeshManager, NavFloorConnectedComponentIdTable& navFloorConnectedComponentIdTable);

	void FilterConnectedComponentFromSeedPoints(NavMeshManager& navMeshManager, NavFloorConnectedComponentIdTable& navFloorConnectedComponentIdTable,
		ConnectedComponentDataTable& connectedComponentDataTable, DynamicKynapseSeedPointArray& seedpoints);

	void FilterConnectedComponentFromOtherNavMeshInvolvedInOverlap(NavMeshManager& navMeshManager, NavFloorConnectedComponentIdTable& navFloorConnectedComponentIdTable, ConnectedComponentDataTable& connectedComponentDataTable);
	void AddAllInvolvedNavMeshInOverlapToNavMeshManager(NavMeshManager& navMeshManager, StlVector<NavMesh*>& addedNavMesh);
	void RemoveAllInvolvedNavMeshInOverlapFromNavMeshManager(NavMeshManager& navMeshManager, StlVector<NavMesh*>& addedNavMesh);

	void ComputeConnectedComponentSurface(NavFloorConnectedComponentIdTable& navFloorConnectedComponentIdTable, ConnectedComponentDataTable& connectedComponentDataTable); 

	KyResult CleanNavMeshAndGraph(NavFloorConnectedComponentIdTable& navFloorConnectedComponentIdTable, ConnectedComponentDataTable& connectedComponentDataTable);

	KyResult CleanDynamicNavcells(NavFloorConnectedComponentIdTable& navFloorConnectedComponentIdTable, ConnectedComponentDataTable& connectedComponentDataTable);
	KyResult CleanDynamicGraph(NavFloorConnectedComponentIdTable& navFloorConnectedComponentIdTable, ConnectedComponentDataTable& connectedComponentDataTable);

	PdgPathDataElement* m_pathDataElement;
};


}


#endif

