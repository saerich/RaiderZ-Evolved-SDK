/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// ---------- Primary contact: JUBA - secondary contact: NOBODY
#ifndef KyRuntime_VertexData_H
#define KyRuntime_VertexData_H

#include <kypathdata/graph/graphtypes.h>
#include <kypathdata/containers/list.h>

namespace Kaim
{

/* For internal use. */
class VirtualEdgesIndicesForVertex
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	SharedPoolList<VirtualEdgeIdx> m_listOfVirtualEdgeIndices;
	List<VirtualEdgesIndicesForVertex>::Iterator m_listIteratorForFastDelete;
};

/* For internal use. */
class VertexData
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	VertexData() : m_astarId(AstarId_Invalid), m_inVirtualEdgeIndices(KY_NULL), m_outVirtualEdgeIndices(KY_NULL) {}

	void Clear()
	{
		m_astarId = AstarId_Invalid;
		m_inVirtualEdgeIndices = KY_NULL;
		m_outVirtualEdgeIndices = KY_NULL;
	}

	KyUInt32 GetAstarId() const { return m_astarId; }
	void SetAstarId(KyUInt32 astarId) { m_astarId = astarId; }

	const SharedPoolList<VirtualEdgeIdx>* GetListOfInVirtualEdgeIndices() const { return m_inVirtualEdgeIndices == KY_NULL ? KY_NULL : &(m_inVirtualEdgeIndices->m_listOfVirtualEdgeIndices); }
	const SharedPoolList<VirtualEdgeIdx>* GetListOfOutVirtualEdgeIndices() const { return m_outVirtualEdgeIndices == KY_NULL ? KY_NULL : &(m_outVirtualEdgeIndices->m_listOfVirtualEdgeIndices); }

	SharedPoolList<VirtualEdgeIdx>* GetListOfInVirtualEdgeIndices() { return m_inVirtualEdgeIndices == KY_NULL ? KY_NULL : &(m_inVirtualEdgeIndices->m_listOfVirtualEdgeIndices); }
	SharedPoolList<VirtualEdgeIdx>* GetListOfOutVirtualEdgeIndices() { return m_outVirtualEdgeIndices == KY_NULL ? KY_NULL : &(m_outVirtualEdgeIndices->m_listOfVirtualEdgeIndices); }

	void SetListOfInVirtualEdgeIndices(VirtualEdgesIndicesForVertex* virtualEdgesIndicesForVertex) { m_inVirtualEdgeIndices = virtualEdgesIndicesForVertex; }
	void SetListOfOutVirtualEdgeIndices(VirtualEdgesIndicesForVertex* virtualEdgesIndicesForVertex) { m_outVirtualEdgeIndices = virtualEdgesIndicesForVertex; }

public:
	KyUInt32 m_astarId;                                     /*!< Used internally during path calculations. Do not modify. */
	VirtualEdgesIndicesForVertex* m_inVirtualEdgeIndices;
	VirtualEdgesIndicesForVertex* m_outVirtualEdgeIndices;
};

}

#endif //KyRuntime_VertexData_H

