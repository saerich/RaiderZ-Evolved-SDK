/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download,
* or which otherwise accompanies this software in either electronic or hard copy form.
*/



// primary contact: GUAL - secondary contact: NOBODY
#ifndef KyPdg_PdgSectorGenerator_H
#define KyPdg_PdgSectorGenerator_H


#include <kypdg/generator/pdgsystem.h>
#include <kypdg/generator/pdgsector.h>
#include <kypdg/generator/pdgblobtyperegistry.h>
#include <kypathdata/navmesh/navmesh.h>
#include <kypathdata/graph/graphcell.h>
#include <kypathdata/blob/blobhandler.h>
#include <kypathdata/blob/blobaggregate.h>
#include <kypdg/pdginput/pdginput.h>
#include <kypdg/pdginput/clientinputconsumer.h>
#include <kypdg/generator/pdgguidcompound.h>
#include <kypdg/generator/pathdataelementglobalmap.h>
#include <kypdg/generator/ipdgsectorgeneratorpostprocess.h>
#include <kypathdata/graph/additionalgraph.h>


namespace Kaim
{

class Cell;
class ClientInputConsumer;
class PdgSector;
class PdgPathDataElement;
class ClientInput;
class PathDataIndex;


/* internal */
class PdgSectorGenerator
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public: 
	enum ProducerOwnershipStatus
	{
		PRODUCER_BELONGS_TO_SECTOR,
		PRODUCER_BELONGS_TO_CLIENT
	};

	// determines whether or not the sector is overlapping another one
	enum SectorOverlapStatus
	{
		SECTOR_IS_OVERLAPPING_ANOTHER_SECTOR,
		SECTOR_IS_ISOLATED
	};

public:
	PdgSectorGenerator(PdgSystem* sys, const PdgSector& sector);

	~PdgSectorGenerator();

	// client interface
	BlobAggregate* GetPathDataAggregate() { return m_pathDataAggregate; }

	const Array<BlobAggregate*>& GetPartitionnedPathDataAggregates() { return m_partitionnedPathDataAggregates; }

	PathDataIndex* GetPathDataIndex() { return m_pathDataIndexHandler.Blob(); }

public: // internal
	void ClearPathDataElements();

	void ClearCells();

	KyResult LoadPdgInput();

	KyResult ProducePdgInput();

	KyResult CheckIntegerPrecisionConsistencyFromPdgInput();

	KyResult ResolveGlobalConfigFromPdgInput();

	KyResult ResolveGlobalConfigFromClientInput(const ClientInput& input);

	PdgPathDataElement* GetOrCreatePathDataElement(const PdgGuidCompound& guidCompound);

	KyResult GenerateFlyGraphAndMakePathData();
	KyResult GenerateRawCells();
	KyResult FilterCells();
	KyResult GenerateGraph();
	KyResult GenerateNavMesh();

	KyResult PostProcessGeneratedPathData();

	KyResult MakePathData();

	void ClearPathData();

	KyResult MakePathDataIndex();

	void SetProducerOwnership(ProducerOwnershipStatus status) { m_producerOwnershipStatus = status; }

	const PdgPathDataElementMap& GetPathDataElements() const { return m_pathDataElements; }

	const KyGuid& GetGuid() { return m_sector.Guid(); }

	const char* GetName() { return m_sector.GetName(); }

	void AddPostProcess(ISectorGeneratorPostProcess* process) { m_postProcesses.PushBack(process); }

private:
	void Init();
	bool GetFinalGenerateMode(PdgRunOptions::GlobalMode globalMode, bool sectorMode);
	void ComputeGenerationFlags();
	KyResult AddPathDataElementBlobs(BlobAggregate& aggregate, PdgPathDataElement& pathDataElement);
	KyResult AddPathDataAdditionalGraphs(BlobAggregate& aggregate);
	KyResult WriteIndividualElements(PdgPathDataElement& pathDataElement);
	KyResult AddOldPathDataBlobs(BlobAggregate& aggregate);
	KyResult MakePlainSectorPathdata();
	KyResult MakePartitionedSectorPathdata();

public:
	PdgSystem* m_sys;
	PdgSector m_sector;
	ClientInputConsumer m_inputConsumer;
	PdgInput m_pdgInput; // aggregate file that contains globalConfig, sectorConfig, pdgInputCells
	PdgPathDataElementMap m_pathDataElements;
	bool m_doGeneratePdgInput;
	bool m_doGenerateClientInput;
	bool m_doGeneratePathData;
	bool m_doIterativePathData;
	ProducerOwnershipStatus m_producerOwnershipStatus;
	SectorOverlapStatus m_overlapStatus;
	Array<ISectorGeneratorPostProcess*> m_postProcesses;

	// TODO
	// class AdditionalGraphElement { m_additionalGraph; m_edgeData; m_vertexData; }
	// class AdditionalGraphElementMap { Array<AdditionalGraphElement> }
	Array< BlobHandler<AdditionalGraph> > m_additionalGraphHandlers;
	Array<BaseBlobHandler> m_additionalGraphEdgeDataHandlers;
	Array<BaseBlobHandler> m_additionalGraphVertexDataHandlers;

	BlobAggregate* m_pathDataAggregate;
	Array<BlobAggregate*> m_partitionnedPathDataAggregates;
	BlobHandler<PathDataIndex> m_pathDataIndexHandler;
};

} // namespace Kaim

#endif

