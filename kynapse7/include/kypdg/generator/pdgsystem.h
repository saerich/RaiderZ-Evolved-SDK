/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: GUAL - secondary contact: NOBODY
#ifndef KyPdg_PdgSystem_H
#define KyPdg_PdgSystem_H


#include <kypathdata/basesystem/coordsystem.h>
#include <kypathdata/blob/blobhandler.h>
#include <kypathdata/basesystem/basesystem.h>
#include <kypdg/common/pdgfilesystem.h>
#include <kypdg/common/pathmap.h>
#include <kypdg/common/pdgfile.h>
#include <kypdg/generator/squarepartition.h>
#include <kypdg/generator/pdgglobalconfig.h>
#include <kypdg/generator/pdgrunoptions.h>
#include <kypdg/generator/pathdataelementglobalmap.h>


namespace Kaim
{

class PdgSectorConfig;
class BlobTypeRegistry;
class BlobAggregate;
class BlobAggregateLoader;
class IParallelForInterface;
class CollisionInterfaceFactory;
class IEdgeAdditionalDataMaker;
class IVertexAdditionalDataMaker;


/* PdgSystem gathers everything that is global across a generation: configuration, options... */
class PdgSystem
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	enum GlobalConfigSource { GLOBAL_CONFIG_DEFAULT, GLOBAL_CONFIG_FROM_USER, GLOBAL_CONFIG_FROM_DATA, GlobalConfigSource_Force32 = 0xFFFFFFFF };
	
	/*! Enumerates what kind of additional data will be built on additional graph from post processes. \ingroup kypdg */
	enum PostProcessAdditionalDataGenerationMask
	{
		USE_NO_ADDITIONAL_DATA_MAKER = 0, 
		USE_VERTEX_ADDITIONAL_DATA_MAKER = 1, 
		USE_EDGE_ADDITIONAL_DATA_MAKER = 2, 
		USE_ALL_ADDITIONAL_DATA_MAKERS = 0xFFFFFFFF
	};


public:
	PdgSystem() :
		m_globalConfigSource(GLOBAL_CONFIG_DEFAULT),
		m_blobRegistry(KY_NULL),
		m_parallelForInterface(KY_NULL),
		m_collisionInterfaceFactory(KY_NULL),
		m_edgeAdditionalDataMaker(KY_NULL),
		m_vertexAdditionalDataMaker(KY_NULL), 
		m_postProcessAdditionalDataMask(USE_NO_ADDITIONAL_DATA_MAKER)
	{
		// call SetGlobalConfig on default config to ensure the partition is 
		// set even when SetGlobalConfig is not explicitly called to avoid div by 0
		// when using SquarePartition math functions later in the pdg
		SetGlobalConfig(m_globalConfig, GLOBAL_CONFIG_DEFAULT);
	}

	KyResult SetGlobalConfig(const PdgGlobalConfig& globalConfig, GlobalConfigSource globalConfigSource);

	const PdgGlobalConfig& Config() const { return m_globalConfig; }
	PdgGlobalConfig& Config()             { return m_globalConfig; }

	const PdgRunOptions& RunOptions() const { return m_runOptions; }
	PdgRunOptions& RunOptions()             { return m_runOptions; }

	const PdgFileSystem& File() const { return m_fileSystem; }
	PdgFileSystem& File()             { return m_fileSystem; }

	const SquarePartition& Partition() const { return m_partition; }
	SquarePartition& Partition()             { return m_partition; }

	const PathMap& Paths() const { return m_paths; }
	PathMap& Paths()             { return m_paths; }

	BlobTypeRegistry* GetBlobRegistry() const { return m_blobRegistry; }
	void SetBlobRegistry(BlobTypeRegistry* blobRegistry) { m_blobRegistry = blobRegistry; }

	const CoordSystem& Coord() const { return Kaim::GetCoordSystem(); }
	CoordSystem& Coord()             { return Kaim::GetCoordSystem(); }

	KyFloat32 GetActualCellSizeInMeter() { return m_partition.m_nbPixelsOnCellSide * m_globalConfig.m_rasterPrecision; }

	/*! Load Blob from fileName */
	template<class T>
	T* LoadBlob(const char* fileName, BlobHandler<T>& blobHandler);

	/*! Save Blob to fileName */
	template<class T>
	KyResult SaveBlob(const char* fileName, const BlobHandler<T>& blobHandler, Endianness::Type endianness = Endianness::KY_BIG_ENDIAN);

	/*! Save Blob to fileName */
	template<class T>
	KyResult SaveBlob(const KyString& fileName, const BlobHandler<T>& blobHandler, Endianness::Type endianness = Endianness::KY_BIG_ENDIAN);

	/*! Load BlobAggregate from fileName */
	KyResult LoadAggregate(const char* fileName, BlobAggregate& aggregate);

	/*! Save Blob to fileName */
	KyResult SaveAggregate(const char* fileName, BlobAggregate& aggregate, Endianness::Type endianness = Endianness::KY_BIG_ENDIAN);

	bool IsEdgeAdditionalDataEnabled()
	{
		return (m_postProcessAdditionalDataMask & USE_EDGE_ADDITIONAL_DATA_MAKER) != 0;
	}

	bool IsVertexAdditionalDataEnabled()
	{
		return (m_postProcessAdditionalDataMask & USE_VERTEX_ADDITIONAL_DATA_MAKER) != 0;
	}

public:
	PdgGlobalConfig m_globalConfig;
	GlobalConfigSource m_globalConfigSource;
	PdgRunOptions m_runOptions;
	PdgFileSystem m_fileSystem;
	SquarePartition m_partition;
	PathMap m_paths;
	PdgPathDataElementMap m_pathDataElementGlobalMap;
	BlobTypeRegistry* m_blobRegistry;
	IParallelForInterface* m_parallelForInterface;
	CollisionInterfaceFactory* m_collisionInterfaceFactory; //!< Used in Jump/Fly for collision tests.
	IEdgeAdditionalDataMaker* m_edgeAdditionalDataMaker;
	IVertexAdditionalDataMaker* m_vertexAdditionalDataMaker;
	/*! Determines whether or not the AdditionalData makers from the PdgSystem will be called on additional graph
	from PostProcesses or not. 
	\acceptedvalues	Any combination of elements from the Kaim::PdgSystem::PostProcessAdditionalDataGenerationMask enumeration.
	\defaultvalue	\c #USE_NO_ADDITIONAL_DATA_MAKER */
	KyUInt32 m_postProcessAdditionalDataMask;
};

template<class T>
T* PdgSystem::LoadBlob(const char* fileName, BlobHandler<T>& blobHandler)
{
	PdgFile pdgFile(this, fileName, "rb");
	KyFilePtr file = pdgFile.GetFile();
	if (file == KY_NULL)
		return KY_NULL;
	return blobHandler.Load(file);
}


template<class T>
KyResult PdgSystem::SaveBlob(const char* fileName, const BlobHandler<T>& blobHandler, Endianness::Type endianness)
{
	PdgFile pdgFile(this, fileName, "wb");
	KyFilePtr file = pdgFile.GetFile();
	if (file == KY_NULL)
		return KY_ERROR;
	return BlobFileWriter::Write(file, blobHandler.Blob(), blobHandler.BlobSize(), endianness);
}

template<class T>
KyResult PdgSystem::SaveBlob(const KyString& fileName, const BlobHandler<T>& blobHandler, Endianness::Type endianness)
{
	return SaveBlob(fileName.CStr(), blobHandler, endianness);
}

}


#endif

