/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/


/*! \file
	\ingroup kypathdata_pathdata */

// primary contact: GUAL - secondary contact: NOBODY
#ifndef KyPathData_AdditionalData_H
#define KyPathData_AdditionalData_H


#include <kypathdata/generic/memory.h>
#include <kypathdata/blob/blobtyperegistry.h>


namespace Kaim
{

// --------------------------- EdgeAdditionalData ---------------------------
class EdgeAdditionalDataType
{
public:
	static const char* GetBlobType() { return "EdgeAdditionalData"; }
	static KyUInt32 GetBlobVersion() { return 0; }
};


template<class T>
class EdgeAdditionalData
{
public:
	typedef T DataType;
	static const char* GetBlobType() { return "EdgeAdditionalData"; }
	static KyUInt32 GetBlobVersion() { return 0; }

public:
	GuidCompound m_guidCompound;
	BlobArray<BlobArray<T> > m_dataCells;
};


template<class T>
inline void SwapEndianness(Endianness::Target e, EdgeAdditionalData<T>& self)
{
	SwapEndianness(e, self.m_guidCompound);
	SwapEndianness(e, self.m_dataCells);
}


// --------------------------- VertexAdditionalData ---------------------------
class VertexAdditionalDataType
{
public:
	static const char* GetBlobType() { return "VertexAdditionalData"; }
	static KyUInt32 GetBlobVersion() { return 0; }
};


template<class T>
class VertexAdditionalData
{
public:
	typedef T DataType;
	static const char* GetBlobType() { return "VertexAdditionalData"; }
	static KyUInt32 GetBlobVersion() { return 0; }

public:
	GuidCompound m_guidCompound;
	BlobArray<BlobArray<T> > m_dataCells;
};


template<class T>
inline void SwapEndianness(Endianness::Target e, VertexAdditionalData<T>& self)
{
	SwapEndianness(e, self.m_guidCompound);
	SwapEndianness(e, self.m_dataCells);
}


// --------------------------- AdditionalDataRegistry ---------------------------
/*! This class registers types of additional data that you have generated for the Graph vertices and edges of
	a sector. It is used to inform the PathData class how to interpret the binary additional data generated
	for those vertices and edges and stored in the .pathdata file.
	\pp When you need to load the PathData for a sector that contains additional data, you need to create
	an instance of this class, set it up with the classes of additional data that you created for the
	vertices and edges in the sector, and pass it to the PathData object in a call to PathData::LoadWithAdditionalData()
	or PathData::LoadFromMemoryWithAdditionalData().
	\pp See also :USERGUIDE:"Using Additional Data".
	\ingroup kypathdata_pathdata */
class AdditionalDataRegistry
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	/*! Registers a type of additional data that has been generated for the Graph vertices in a sector.
		\tparam T		The class name of the additional data to load for the vertices. */
	template<class T> void AddVertexData()
	{
		m_blobRegistry.AddBlobType<VertexAdditionalData<T> >();
	}

	/*! Registers a type of additional data that has been generated for the Graph edges in a sector.
		\tparam T		The class name of the additional data to load for the edges. */
	template<class T> void AddEdgeData()
	{
		m_blobRegistry.AddBlobType<EdgeAdditionalData<T> >();
	}

public:
	BlobTypeRegistry m_blobRegistry; /*!< For internal use. Do not modify. */
};


}


#endif

