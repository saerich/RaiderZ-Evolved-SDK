/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/


// Primary contact: LASI - secondary contact: NONE
#ifndef KyPathData_InputGraphVertexBlobBuilder_H
#define KyPathData_InputGraphVertexBlobBuilder_H

#include "kypathdata/graph/inputgraphvertexcell.h"
#include "kypathdata/blob/baseblobbuilder.h"
#include "kypathdata/containers/array.h"
#include "kypdg/generator/pdgguidcompound.h"
#include "kypathdata/generic/guidcompoundblobbuilder.h"


namespace Kaim
{

class InputGraphVertexCellBlobBuilder: public BaseBlobBuilder<InputGraphVertexCell>
{
public:
	InputGraphVertexCellBlobBuilder(const DynamicInputGraphVertexCell* vertices): m_vertices(vertices) {};

private:
	virtual void DoBuild();

private:
	const DynamicInputGraphVertexCell* m_vertices;
};

class InputGraphVerticesBlobBuilder : public BaseBlobBuilder<InputGraphVertices>
{
public:
	InputGraphVerticesBlobBuilder(const Array<const DynamicInputGraphVertexCell*>* vertexCells, const PdgGuidCompound* guidCompound)
	{
		m_vertexCells = vertexCells;
		m_guidCompound = guidCompound;
	};

private:
	virtual void DoBuild()
	{
		// m_blob->m_guidCompound
		const KyUInt32 guidsCount = KyUInt32(m_guidCompound->m_guids.GetCount());
		GuidCompoundBlobBuilder guidCompoundBuilder(guidsCount > 0 ? &m_guidCompound->m_guids[0] : KY_NULL, guidsCount, m_guidCompound->m_timeStamp);
		BLOB_BUILD(m_blob->m_guidCompound, guidCompoundBuilder);

		// m_blob->m_cells
		KyUInt32 cellsCount = m_vertexCells->GetCount();
		InputGraphVertexCell* cells = BLOB_ARRAY(m_blob->m_cells, cellsCount);
		for (KyUInt32 i = 0 ; i < cellsCount; ++i)
		{
			BLOB_BUILD(cells[i], InputGraphVertexCellBlobBuilder((*m_vertexCells)[i]));

		}
	}
private:
	const Array<const DynamicInputGraphVertexCell*>* m_vertexCells;
	const PdgGuidCompound* m_guidCompound;
};


}

#endif // KyPathData_InputGraphVertexBlobBuilder_H
