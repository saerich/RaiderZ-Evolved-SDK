/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download,
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

/*! \file
	\ingroup kypdg_generator */

// primary contact: GUAL - secondary contact: NOBODY
#ifndef KyPdg_AdditionalDataMaker_H
#define KyPdg_AdditionalDataMaker_H


#include <kypathdata/blob/blobarray.h>
#include <kypathdata/blob/baseblobbuilder.h>
#include <kypdg/generator/pdgguidcompound.h>
#include <kypathdata/generic/guidcompoundblobbuilder.h>
#include <kypathdata/graph/graph.h>
#include <kypathdata/pathdata/additionaldata.h>
#include <kypathdata/containers/array.h>
#include "kypathdata/graph/inputgraphvertexcell.h"


namespace Kaim
{


template<class AdditionalDataT>
class AdditionalDataBlobBuilder : public BaseBlobBuilder<AdditionalDataT>
{
public:
	typedef typename AdditionalDataT::DataType DataType;

	AdditionalDataBlobBuilder(const PdgGuidCompound* guidCompound, const Array<Array<DataType> >* dataCells) :
		m_guidCompound(guidCompound), m_dataCells(dataCells) {}

private:
	virtual void DoBuild()
	{
		KyUInt32 guidsCount = (KyUInt32)m_guidCompound->m_guids.GetCount();
		GuidCompoundBlobBuilder guidCompoundBuilder(guidsCount > 0 ? &m_guidCompound->m_guids[0] : KY_NULL, guidsCount, m_guidCompound->m_timeStamp);
		BLOB_BUILD(this->m_blob->m_guidCompound, guidCompoundBuilder);

		KyUInt32 cellsCount = (KyUInt32)m_dataCells->GetCount();
		BlobArray<DataType>* blobDataCells = BLOB_ARRAY(this->m_blob->m_dataCells, cellsCount);
		for (KyUInt32 cellIdx = 0; cellIdx < cellsCount; ++cellIdx)
		{
			const Array<DataType>& dynArray = (*m_dataCells)[cellIdx];
			KyUInt32 count = (KyUInt32)dynArray.GetCount();

			DataType* blobArray = BLOB_ARRAY(blobDataCells[cellIdx], count);
			if (this->IsWriteMode())
			{
				for (KyUInt32 i = 0; i < count; ++i)
					blobArray[i] = dynArray[i];
			}
		}
	}

private:
	const PdgGuidCompound* m_guidCompound;
	const Array<Array<DataType> >* m_dataCells;
};


//---------------------------------------------------------------------------------------------------------------
//                                           EdgeAdditionalData
//---------------------------------------------------------------------------------------------------------------

/*! Provides an abstract base interface for an object that the Generator can invoke to tag each edge in the \Graphs 
	that it creates with additional data.
	\pp This class cannot be used directly; use a class that derives from EdgeAdditionalDataMaker and that implements
	the ComputeEdgeData() method.
	\pp See also :USERGUIDE:"Using Additional Data".
	\ingroup kypdg_generator */
class IEdgeAdditionalDataMaker : public VirtualMemObject
{
public:
	/*! You must implement this method to generate the additional data that should be stored for the specified edge.
		\param A				The starting point of the edge.
		\param B				The ending point of the edge.
		\param[out] edgeData	The additional data computed for this edge. */
	virtual void ComputeEdgeData(const Vec3f& A, const Vec3f& B, void* edgeData) = 0;

	// internal
	/*! For internal use. */
	virtual void Compute(const PdgGuidCompound* pdgGuidCompound, const Graph* graph, const InputGraphVertices* inputVertices, BaseBlobHandler* blobHandler)= 0;
};

/*! The EdgeAdditionalDataMaker is a base class for an object that the Generator can invoke to tag each each edge in the
	\Graphs that it creates with an instance of a specific class.
	\tparam T			The class of additional data that this object will create.
	\pp This class cannot be used directly. You must create a class that derives from EdgeAdditionalDataMaker, providing the class of
	additional data it will create in a template parameter. In your class, you must implement the ComputeEdgeData() method to 
	generate an instance of class \em T for each edge.
	\pp See also :USERGUIDE:"Using Additional Data".
	\ingroup kypdg_generator */
template<class T>
class EdgeAdditionalDataMaker : public IEdgeAdditionalDataMaker
{
	// internal
	virtual void Compute(const PdgGuidCompound* pdgGuidCompound, const Graph* graph, const InputGraphVertices* inputVertices, BaseBlobHandler* baseBlobHandler)
	{
		// compute dataCells
		Array<Array<T> > dataCells;

		KyUInt32 cellCount = graph->GetCellCount();
		dataCells.Resize(cellCount);

		for (KyUInt32 cellIdx = 0; cellIdx < cellCount; ++cellIdx)
		{
			const GraphCell& graphCell = graph->GetGraphCell(cellIdx);
			Array<T>& dataCell = dataCells[cellIdx];

			KyUInt32 edgeCount = graphCell.GetEdgeCount();
			dataCell.Resize(edgeCount);

			for (KyUInt32 edgeIdx = 0; edgeIdx < edgeCount; ++edgeIdx)
			{
				ComputeEdgeData(
					graphCell.GetEndVertexPosition(edgeIdx),
					graphCell.GetStartVertexPosition(edgeIdx),
					&dataCell[edgeIdx]);
			}
		}

		// Build the blob from dataCells
		BlobHandler<EdgeAdditionalData<T> >* blobHandler = (BlobHandler<EdgeAdditionalData<T> >*)baseBlobHandler;
		AdditionalDataBlobBuilder<EdgeAdditionalData<T> > builder(pdgGuidCompound, &dataCells);
		builder.Build(*blobHandler);
	}
};



//---------------------------------------------------------------------------------------------------------------
//                                           VertexAdditionalData
//---------------------------------------------------------------------------------------------------------------


/*! Provides an abstract base interface for an object that the Generator can invoke to tag each vertex in the \Graphs 
	that it creates with additional data.
	\pp This class cannot be used directly; use a class that derives from VertexAdditionalDataMaker and that implements
	the ComputeVertexData() method.
	\pp See also :USERGUIDE:"Using Additional Data".
	\ingroup kypdg_generator */
class IVertexAdditionalDataMaker : public VirtualMemObject
{
public:
	/*! You must implement this method to generate the additional data that should be stored for the specified vertex.
		\param P				The position of the vertex.
		\param[out] vertexData	The additional data computed for this vertex. */
	virtual void ComputeVertexData(const Vec3f& P, void* vertexData) = 0;

	// internal
	/*! For internal use. */
	virtual void Compute(const PdgGuidCompound* pdgGuidCompound, const Graph* graph, const InputGraphVertices* inputVertices, BaseBlobHandler* blobHandler)= 0;
};


/*! The VertexAdditionalDataMaker is a base class for an object that the Generator can invoke to tag each each vertex in the
	\Graphs that it creates with an instance of a specific class.
	\tparam T			The class of additional data that this object will create.
	\pp This class cannot be used directly. You must create a class that derives from VertexAdditionalDataMaker, providing the class of
	additional data it will create in a template parameter. In your class, you must implement the ComputeEdgeData() method to 
	generate an instance of class \em T for each vertex.
	\pp See also :USERGUIDE:"Using Additional Data".
	\ingroup kypdg_generator */
template<class T>
class VertexAdditionalDataMaker : public IVertexAdditionalDataMaker
{
	// internal
	virtual void Compute(const PdgGuidCompound* pdgGuidCompound, const Graph* graph, const InputGraphVertices* inputVertices, BaseBlobHandler* baseBlobHandler)
	{
		// compute dataCells
		Array<Array<T> > dataCells;

		KyUInt32 cellCount = graph->GetCellCount();
		dataCells.Resize(cellCount);

		for (KyUInt32 cellIdx = 0; cellIdx < cellCount; ++cellIdx)
		{
			const GraphCell& graphCell = graph->GetGraphCell(cellIdx);
			Array<T>& dataCell = dataCells[cellIdx];

			KyUInt32 vertexCount = graphCell.GetVertexCount();
			dataCell.Resize(vertexCount);

			for (KyUInt32 vertexIdx = 0; vertexIdx < vertexCount; ++vertexIdx)
			{
				ComputeVertexData(graphCell.GetVertexPosition(vertexIdx), &dataCell[vertexIdx]);
			}
		}

		// Build the blob from dataCells
		BlobHandler<VertexAdditionalData<T> >* blobHandler = (BlobHandler<VertexAdditionalData<T> >*)baseBlobHandler;
		AdditionalDataBlobBuilder<VertexAdditionalData<T> > builder(pdgGuidCompound, &dataCells);
		builder.Build(*blobHandler);
	}
};


}


#endif
