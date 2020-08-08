/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*! \file
	\ingroup kypathdata_pathdata */

// primary contact: GUAL - secondary contact: NOBODY
#ifndef KyPathData_PathData_H
#define KyPathData_PathData_H


#include <kypathdata/generic/memory.h>
#include <kypathdata/containers/array.h>
#include <kypathdata/pathdata/pathdatablobtyperegistry.h>
#include <kypathdata/pathdata/additionaldata.h>

namespace Kaim
{

class NavMesh;
class Graph;
class JumpEdgeCandidateArray;
class InputGraphVertices;
class AdditionalGraph;

/*! PathDataAllocator is an abstract base class for an object that the PathData class can use to allocate
	and free memory. 
	\pp You cannot use this class as-is; you must create your own class that derives from PathDataAllocator.
	You create an instance of your custom class, and assign it to a PathData instance in the PathData constructor.
	\ingroup kypathdata_pathdata */
class PathDataAllocator : public VirtualMemObject
{
public:
	virtual void* Malloc(KyUInt32 byteSize) = 0;
	virtual void Free(void* mem) = 0;
	virtual ~PathDataAllocator();
};


/*! The PathData class aggregates the data that your Bots and other AI components use to understand the terrain
	and their movement opportunities. This comprises a Graph (an abstract representation of the topology
	of the terrain), and a NavMesh (an abstract representation of the areas that are considered navigable).
	For details, see :USERGUIDE:"PathData".
	The PathData class also offers a mechanism for loading data from a \c .pathdata file created by the \SDKName PathData
	generation framework. For a step-by-step guide, see :USERGUIDE:"Integration Phase 5: Loading PathData at Runtime".
	\ingroup kypathdata_pathdata */
class PathData
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	/*! Enumerates the possible data that can be loaded from a \c .pathdata file. */
	enum LoadFilter { LOAD_NAVMESHS        = 1, /*!< Loads all \NavMeshes contained in the \c pathdata file. */
					  LOAD_SUBGRAPHS       = 2, /*!< Loads all \Graphs contained in the \c pathdata file. */
					  LOAD_JUMPCANDIDATES  = 4, /*!< Loads jump edge candidates contained in the \c pathdata file. */
					  LOAD_ADDITIONALDATA  = 8, /*!< Loads all VertexAdditionalData and EdgeAdditionalData contained in the \c pathdata file. */
					  LOAD_ADDITIONALGRAPH = 16, /*!< Loads all additional graphs contained in the \c pathdata file. */
					  LOAD_INPUTVERTICES   = 32, /*!< Loads all input vertices contained in the \c pathdata file. */
					  LOAD_ALL = 0xFFFFFFFF    /*!< Loads all data contained in the \c pathdata file. */
					};
	/*! Enumerates the possible sources from which a PathData object can load binary data. */
	enum LoadMode { UNSET, /*!< Indicates a data source that has not been specified. */ 
					FROM_FILE, /*!< Indicates that binary data should be read from a specified file on disk. */ 
					FROM_MEMORY /*!< Indicates that binary data should be read from a memory buffer. */ 
					};

public:
	/*! \label_constructor
		\param allocator			A class that derives from PathDataAllocator, which the PathData object will use to allocate
									and de-allocate memory. This parameter is optional. If you do not specify a PathDataAllocator,
									#KY_MALLOC and #KY_FREE will be used. */
	PathData(PathDataAllocator* allocator = KY_NULL) : m_loadMode(UNSET), m_allocator(allocator) {}
	
	/*! \label_destructor Clears all data and de-allocates all memory. */
	~PathData() { Clear(); }

	/*! Loads data from the specified \c .pathdata file, and adds it to this object.
		\pp If this sector contains any additional data, use LoadWithAdditionalData() instead.
		\param fullFileName			The absolute path and file name of the file to load.
		\param loadFilter			A bitmask composed of elements of the #LoadFilter enumeration that indicates what data
									should be loaded from the specified file.
		\return Result::Success if the PathData was successfully loaded, or Result::Failure otherwise. */
	KyResult Load(const char* fullFileName, KyUInt32 loadFilter = LOAD_ALL);

	/*! Loads data from the specified memory buffer, and adds it to this object.
		\pp If this sector contains any additional data, use LoadFromMemoryWithAdditionalData() instead.
		\param memory				The memory buffer that contains the binary data for this object, read from the .pathdata file.
		\param loadFilter			A bitmask composed of elements of the #LoadFilter enumeration that indicates what data
									should be loaded from the buffer.
		\return Result::Success if the PathData was successfully loaded, or Result::Failure otherwise. */
	KyResult LoadFromMemory(void* memory, KyUInt32 loadFilter = LOAD_ALL);

	/*! Loads data from the specified \c .pathdata file, and adds it to this object. Always use this function instead of Load() when
		your sector contains additional data.
		\pp See also :USERGUIDE:"Using Additional Data".
		\param fullFileName				The absolute path and file name of the file to load.
		\param additionalDataRegistry	An instance of AdditionalDataRegistry that you have configured with the types of additional data
										that you have generated for this sector.
		\param loadFilter				A bitmask composed of elements of the #LoadFilter enumeration that indicates what data
										should be loaded from the specified file.
		\return Result::Success if the PathData was successfully loaded, or Result::Failure otherwise. */
	KyResult LoadWithAdditionalData(const char* fullFileName, const AdditionalDataRegistry& additionalDataRegistry, KyUInt32 loadFilter = LOAD_ALL);

	/*! Loads data from the specified memory buffer, and adds it to this object. Always use this function instead of LoadFromMemory() when
		your sector contains additional data.
		\pp See also :USERGUIDE:"Using Additional Data".
		\param memory					The memory buffer that contains the binary data for this object, read from the .pathdata file.
		\param additionalDataRegistry	An instance of AdditionalDataRegistry that you have configured with the types of additional data
										that you have generated for this sector.
		\param loadFilter				A bitmask composed of elements of the #LoadFilter enumeration that indicates what data
										should be loaded from the buffer.
		\return Result::Success if the PathData was successfully loaded, or Result::Failure otherwise. */
	KyResult LoadFromMemoryWithAdditionalData(void* memory, const AdditionalDataRegistry& additionalDataRegistry, KyUInt32 loadFilter = LOAD_ALL);

	/*! \label_clear */
	void Clear();

	/*! Allocates memory when loading PathData from a file. Uses the PathDataAllocator passed to the constructor if any,
		otherwise uses #KY_MALLOC. */
	void* Malloc(KyUInt32 byteSize);

	/*! De-allocates memory when clearing PathData previously loaded from a file. Uses the PathDataAllocator passed to the constructor if any,
		otherwise uses #KY_FREE. */
	void Free(void* mem);

	/*! Indicates whether or not the specified PathData was created with the same generation parameters
		as this object. If this method returns #KY_TRUE, the two PathData objects can be loaded into the
		same Database at the same time. */
	bool IsCompatibleWith(const PathData& pathData) const;

private:
	KyResult Load(const BlobTypeRegistry& blobTypeRegistry, const char* fullFileName, KyUInt32 loadFilter);

	KyResult LoadFromMemory(const BlobTypeRegistry& blobTypeRegistry, void* memory, KyUInt32 loadFilter);

public:
	LoadMode m_loadMode; /*!< Stores the source from which the data in this object was loaded. Do not modify. */
	PathDataAllocator* m_allocator; /*!< Stores the PathDataAllocator passed to the constructor, if any. Do not modify. */
	Array<NavMesh*> m_navMeshs; /*!< The list of \NavMeshes maintained by this object. Do not modify. */
	Array<Graph*> m_graphs; /*!< The list of \Graphs maintained by this object. Do not modify. */
	Array<JumpEdgeCandidateArray*> m_jumpEdgeCandidates; /*!< The list of jump edge candidates maintained by this object. Do not modify. */

	// The 2 arrays below will have a count of either 0 , n OR n + m where n = m_graphs.GetCount() and m = m_additionalGraphs.GetCount()
	// -> count == 0 if the pdgSystem had no AdditionalDataMaker
	// -> count == n if the pdgSystem had additionalDataMakers but the additional data generation is disabled on additionalGraphs
	// -> count == (n + m) if the pdgSystem had additionalDataMakers and the additional data generation is enabled on additionalGraphs
	// m_edgeAdditionalDatas[0..n-1] are additionalData for graphs[0..n - 1] if relevant
	// m_edgeAdditionalDatas[n .. n + m - 1] are additionalData for additionalGraphs[0..m - 1] if relevant
	Array<void*> m_edgeAdditionalDatas; /*! The additional data stored for the edges in this sector. Do not modify. */
	Array<void*> m_vertexAdditionalDatas; /*! The additional data stored for the edges in this sector. Do not modify. */
	Array<InputGraphVertices*> m_inputGraphVertices; /*! The list of input vertices specified for this sector. Do not modify. */
	Array<AdditionalGraph*> m_additionalGraphs; /*! The list of additional graphs added to this sector. Do not modify. */

private:
	PathData& operator=(const PathData& pathData);
	PathData(const PathData& pathData);
};



}


#endif

