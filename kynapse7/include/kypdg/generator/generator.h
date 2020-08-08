/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

/*! \file
	\ingroup kypdg */

// primary contact: GUAL - secondary contact: NOBODY
#ifndef KyPdg_Generator_H
#define KyPdg_Generator_H


#include <kypdg/generator/pdgsystem.h>

#include <kypdg/generator/pdgsector.h>
#include <kypdg/pdginput/clientinputconsumer.h>
#include <kypdg/pdginput/pdginputproducer.h>

#include <kypdg/generator/generatorprofiling.h>
#include <kypdg/generator/pdgblobtyperegistry.h>
#include <kypdg/generator/globalcellgrid.h>
#include <kypdg/generator/pdgguidcompound.h>
#include <kypdg/generator/additionaldatamaker.h>
#include <kypathdata/containers/kystring.h>
#include <kypathdata/containers/array.h>


namespace Kaim
{

class PdgInput;
class Cell;
class IParallelForInterface;
class CollisionInterfaceFactory;
class NavMesh;
class GraphCell;
class ClientInputConsumer;
class PdgSectorGenerator;
class PdgPathDataElement;


/*! The Generator is the principal class in the PathData generation system. For more details, including a step-by-step 
	guide on how to use the Generator to create PathData for your own terrains, see 
	:USERGUIDE:"Integrating the PathData Generation Framework".
	\section generatoroutput Output files
	The Generator is capable of generating multiple kinds of data files:
	-	\c .ClientInput and .PdgInput files, which are records of the input triangles, terrain types and terrain type volumes that describe
		your terrain, passed as inputs to the PathData generation system.
	-	Intermediate data files, which record temporary data calculated during the generation process.
	-	Final \c .PathData files, which contain the final data that you will load into your \gameOrSim at runtime.
	\pp You can control which types of data files you want to generate each time you use the Generator.
	\section generatorsectors Sectors and multi-sectors
	The Generator can be set up with any number of sectors, each of which is responsible for a block of terrain geometry
	within a defined area of your \gameOrSim world. Separate input, intermediate and output data are created for each sector,
	so that the sectors can be streamed into memory independently at runtime. Adjacent sectors whose triangles lie close to
	each other will be linked automatically at runtime.
	\pp You can also configure the Generator to re-partition each sector that you provide into smaller sectors automatically.
	\pp See also :USERGUIDE:"Sectorizing the Terrain".
	\ingroup kypdg */
class Generator
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public: 
	/*! Enumerates the possible sources of configuration data when generating a sector from a .ClientInput file. Provided in a call
		to AddSectorFromClientInput(). */
	enum ClientInputConfigMode
	{
		USE_GENERATOR_CONFIG, /*!< Instructs the Generator to use its current configuration and coordinate system when generating data for the sector. */
		USE_CLIENT_INPUT_CONFIG /*!< Instructs the Generator to use the configuration and coordinate system contained in the .ClientInput file when generating data for the sector. */
	};

public:
	/*! \label_constructor
		\param parallelForInterface		A pointer to an object that the Generator will use to parallelize computations over multiple processors,
										or #KY_NULL in order to perform all computations sequentially on a single processor. This object is not 
										copied; you must ensure that you do not destroy it until after you are finished using the Generator. See also
										:USERGUIDE:"Parallelizing PathData Generation". */
	Generator(IParallelForInterface* parallelForInterface);

	/*! \label_destructor */
	~Generator();

public:
	// ------------------------------ Configuration ------------------------------
	
	/*!	Sets an absolute directory that the Generator will use as a root for all file and path names provided to it. */
	void SetRootDirectory(const char* rootDirectory);

	/*!	Sets a name used to identify this Generator. This name will be used to name any .PathDataIndex files created by the Generator. The default value is "Generator". */
	void SetName(const char* name);

	/*! Sets an instance of the PdgGlobalConfig class, which contains configuration parameters that control the characteristics
		of the PathData generated for all sectors. The instance you provide is copied internally, so you can destroy the
		object you pass at any time. */
	void SetGlobalConfig(const PdgGlobalConfig& config);
	
	/*! Retrieves the instance of the PdgGlobalConfig class that contains configuration parameters that control the characteristics
		of the PathData generated for all sectors. */
	const PdgGlobalConfig& GetGlobalConfig() const { return m_sys.Config(); }

	/*! Sets an instance of the PdgRunOptions class, which contains configuration parameters that control the way in which the
		Generator operates. The instance you provide is copied internally, so you can destroy the object you pass at any time. */
	void SetRunOptions(const PdgRunOptions& runOptions) { m_sys.RunOptions() = runOptions; }

	/*! Retrieves the instance of the PdgRunOptions class that contains configuration parameters that control the way in which the
		Generator operates. */
	const PdgRunOptions& GetRunOptions() const { return m_sys.RunOptions(); }

	/*! Sets the IParallelForInterface object the Generator will use to execute jobs in parallel. This object is not copied; you must ensure
		that you do not destroy it until after you are finished using the Generator.
		See :USERGUIDE:"Parallelizing PathData Generation". */
	void SetParallelForInterface(IParallelForInterface* parallelForInterface) { m_sys.m_parallelForInterface = parallelForInterface; }


	/*! Sets the CollisionInterfaceFactory used by the Generator to create and destroy objects that implement ICollisionInterface,
		used to generate PathData for flying characters and/or jump edge candidates in areas where the Bot can descend but not
		ascend. */
	void SetCollisionInterfaceFactory(CollisionInterfaceFactory* collisionInterfaceFactory) { m_sys.m_collisionInterfaceFactory = collisionInterfaceFactory; }

	/*! Adds a new sector that will be generated from a PdgInputProducer according to the settings in the specified PdgSector.
		\pp The PdgSector you provide is copied internally by the Generator, so that you do not have to retain it in memory. 
		However, do not destroy the PdgInputProducer set in the PdgSector until after your call to Generator::Generate().
		\return A #KyResult that indicates the success or failure of the request. */
	KyResult AddNewSector(const PdgSector& sector);

	/*! Adds a sector that will be generated from an existing \c .PdgInput file previously generated for the sector.
		\pre You must set the root directory for the Generator to the directory that contains the \c .PdgInput file for the sector.
		\param sectorName			The name of the sector. This name should match the name of the \c .PdgInput file for
									the sector (without the extension), and the value returned by the PdgSector::GetName() 
									method for the PdgSector originally used to create the sector.
		\param partitioner			An optional pointer to an object that will be invoked to partition the PathData for the sector into
									multiple smaller output sectors. See also :USERGUIDE:"Sectorizing the Terrain".
		\param postProcesses		An optional array of objects that implement the ISectorGeneratorPostProcess interface, which will be
									invoked to perform a post-processing phase after generating the Graph and NavMesh for this sector.
		\param doGeneratePathdata	Indicates whether or not final PathData will be generated for the sector. You might want to disable
									this if you are using the geometry of this sector solely as input for generating an adjacent sector,
									in order to generate data in the area of overlap between the two sectors.
		\return A #KyResult that indicates the success or failure of the request. */
	KyResult AddSectorFromPdgInput(const char* sectorName, PdgSectorPartitioner* partitioner = KY_NULL, Array<ISectorGeneratorPostProcess*>* postProcesses = KY_NULL, bool doGeneratePathdata = true);

	/*! Adds a sector that will be generated from an existing \c .ClientInput file previously generated for the sector.
		\pre You must set the root directory for the Generator to the directory that contains the \c .ClientInput file for the sector.
		\param inputName			The name of the sector. This name should match the name of the \c .ClientInput file for
									the sector (without the extension), and the value returned by the PdgSector::GetName() 
									method for the PdgSector originally used to create the sector.
		\param partitioner			An optional pointer to an object that will be invoked to partition the PathData for the sector into
									multiple smaller output sectors. See also :USERGUIDE:"Sectorizing the Terrain".
		\param postProcesses		An optional array of objects that implement the ISectorGeneratorPostProcess interface, which will be
									invoked to perform a post-processing phase after generating the Graph and NavMesh for this sector.
		\param doGeneratePathdata	Indicates whether or not final PathData will be generated for the sector. You might want to disable
									this if you are using the geometry of this sector solely as input for generating an adjacent sector,
									in order to generate data in the area of overlap between the two sectors.
		\param configMode			Indicates which PdgGlobalConfig and which CoordSystem the Generator should use. This can either be the
									current Generator configuration, or the one that was originally set up at the time the .ClientInput was
									generated. Can be any option from the #ClientInputConfigMode enumeration. The default value is #USE_GENERATOR_CONFIG.
		\return A #KyResult that indicates the success or failure of the request. */
	KyResult AddSectorFromClientInput(const char* inputName, PdgSectorPartitioner* partitioner = KY_NULL, Array<ISectorGeneratorPostProcess*>* postProcesses = KY_NULL,  bool doGeneratePathdata = true, ClientInputConfigMode configMode = USE_GENERATOR_CONFIG);


	/*! Specifies the GUIDs of sectors that cover the same areas of terrain, but that will never be loaded into the same Database
		at the same time. This prevents the PathData generation system from generating alternative data in the area of overlap.
		This is primarily intended to support swappable sectors, which reflect multiple versions of the terrain in a single area --
		for example, one version of a sector that contains a building, and another version of a sector in which the building has
		been destroyed.
		\pp You can still use swappable sectors without calling this method. However, as the alternative data generated in the area
		of overlap between the sectors will never be used, you can reduce the size of your PathData by preventing the PathData generation
		system from generating the data in the first place.
		\pp You can call this method multiple times to set up multiple sets of swappable sectors.
		\param guids			An array of GUIDs that identify the sectors that will never be loaded into the same Database at the
								same time.
		\param guidsCount		The number of elements in the \c guids array. */
	void AddExclusiveGuids(KyGuid* guids, KyUInt32 guidsCount);

	/*! Specifies a combination of sectors for which the Generator will always generate overlap data normally, regardless of whether the
		sectors have been configured to restrict overlap data through a call to PdgSector::SetInputCellBox().
		\pp You can call this method multiple times to set up multiple sets of sectors.
		\pp You must call this method once for each set of overlapping sectors for which you want to generate overlap data normally.
		For example, if you have three sectors that overlap (sectors A, B and C), and you want to generate overlap data normally
		amongst all those sectors, you would call this method four times: once for [A,B], once for [A,C], once for [B,C], and once
		for [A,B,C].
		\param guids			An array of GUIDs that identify the sectors that will always have overlap data generated normally.
		\param guidsCount		The number of elements in the \c guids array. */
	void AddExplicitOverlapSectors(KyGuid* guids, KyUInt32 guidsCount);

	/*! Provides an instance of a class that derives from EdgeAdditionalDataMaker, which will be invoked to tag each edge
		in the \Graphs created by the Generator with custom data.
		\pp See also :USERGUIDE:"Using Additional Data".
		\tparam T						The class of data your EdgeAdditionalDataMaker creates for each edge.
		\param edgeAdditionalDataMaker	An instance of your EdgeAdditionalDataMaker class. Do not destroy this object until after
										you have finished using the Generator. */
	template <class T>
	void SetEdgeAdditionalDataMaker(EdgeAdditionalDataMaker<T>* edgeAdditionalDataMaker)
	{
		m_sys.m_blobRegistry->AddBlobType<EdgeAdditionalData<T> >();
		m_sys.m_edgeAdditionalDataMaker = edgeAdditionalDataMaker;
	}

	/*! Provides an instance of a class that derives from VertexAdditionalDataMaker, which will be invoked to tag each vertex
		in the \Graphs created by the Generator with custom data.
		\pp See also :USERGUIDE:"Using Additional Data".
		\tparam T							The class of data your VertexAdditionalDataMaker creates for each vertex.
		\param vertexAdditionalDataMaker	An instance of your VertexAdditionalDataMaker class. Do not destroy this object until after
											you have finished using the Generator. */
	template <class T>
	void SetVertexAdditionalDataMaker(VertexAdditionalDataMaker<T>* vertexAdditionalDataMaker)
	{
		m_sys.m_blobRegistry->AddBlobType<VertexAdditionalData<T> >();
		m_sys.m_vertexAdditionalDataMaker = vertexAdditionalDataMaker;
	}

	/*! When you set up the Generator with a VertexAdditionalDataMaker, by default the VertexAdditionalDataMaker is
		invoked only for vertices that are contained in the \Graphs created by the Generator, not vertices contained in
		any custom Additional Graphs that you create yourself and add to your sectors. If you want the Generator to 
		invoke your VertexAdditionalDataMaker to also generate additional data for the vertices in your Additional
		Graph, call this method before you call Generate(). */
	void EnablePostProcessVertexAdditionalData()
	{
		m_sys.m_postProcessAdditionalDataMask = m_sys.m_postProcessAdditionalDataMask | PdgSystem::USE_VERTEX_ADDITIONAL_DATA_MAKER;
	}

	/*! When you set up the Generator with an EdgeAdditionalDataMaker, by default the EdgeAdditionalDataMaker is
		invoked only for edges that are contained in the \Graphs created by the Generator, not edges contained in
		any custom Additional Graphs that you create yourself and add to your sectors. If you want the Generator to 
		invoke your EdgeAdditionalDataMaker to also generate additional data for the edges in your Additional
		Graph, call this method before you call Generate(). */
	void EnablePostProcessEdgeAdditionalData()
	{
		m_sys.m_postProcessAdditionalDataMask = m_sys.m_postProcessAdditionalDataMask | PdgSystem::USE_EDGE_ADDITIONAL_DATA_MAKER;
	}
	// ---------------------------------- Generation ----------------------------------

	enum { LicenseError = -1000 };
	/*! Once you have configured the Generator the way you want it, and you have added all of the sectors and/or
		multi-sectors for which you want to generate PathData, call Generate() to carry out the data generation process. */
	KyResult Generate();

	/*! Creates a file that that contains a record of all generation parameter settings. The name of the file to create
		is set by PdgRunOptions::m_paramDumpFilename. This method is called internally by Generate(). */
	void DumpGenerationParameters();

	// ------------------------------- Browse Sectors -------------------------------
	/*! Returns the number of sectors currently set up for the Generator. */
	KyUInt32 GetSectorCount() { return (KyUInt32)m_sectorGenerators.GetCount(); }

	/*! Retrieves the PdgSector with the specified index in the list of sectors maintained by the Generator. */
	const PdgSector& GetSector(KyUInt32 index) { return m_sectorGenerators[index]->m_sector; }

	// ---------------------------------- Internals ----------------------------------

	/*! @{ */
	/*! For internal use only. */
	PdgSystem& Sys() { return m_sys; }
	const Array<PdgSectorGenerator*>& GetSectorGenerators() const { return m_sectorGenerators; }
	PdgSectorGenerator* AddInternalSector(const PdgSector& sector);
	PdgSectorGenerator* AddInternalSectorFromPdgInput(const char* sectorName, PdgSectorPartitioner* partitioner, Array<ISectorGeneratorPostProcess*>* postProcesses, bool doGeneratePathdata);
	PdgSectorGenerator* AddInternalSectorFromClientInput(const char* sectorName, PdgSectorPartitioner* partitioner, Array<ISectorGeneratorPostProcess*>* postProcesses, bool doGeneratePathdata, ClientInputConfigMode configMode);
	/*! @} */

private:
	enum FindInputCellMode
	{
		KEEP_EXPLICIT_OVERLAP_CELLS,
		IGNORE_EXPLICIT_OVERLAP_CELLS, 
	};
	bool CheckConfig() const;
	bool AreGuidsAndNamesUnique(bool pdgInputLoaded) const;
	bool IsGlobalConfigConsistent() const;
	KyResult LoadOrProducePdgInputs();
	KyResult MakeGlobalGrid();
	KyResult MakePathDataElements();
	KyResult GenerateSectors();
	void FindMainContributingInputCells(KyUInt32 mainIdx, const PdgInputCellsAtPos* cellsAtPos, Array<const PdgInputCell*>& inputCells, FindInputCellMode mode);
	KyResult FeedInputCells(const Array<const PdgInputCell*>& inputCells, const CellPos& cellPos, PdgPathDataElement* pathDataElement, bool isOnOverlap);
	bool IsGuidCompoundExclusive(const PdgGuidCompound& guidCompound) const;
	bool IsGuidCompoundAnExplicitOverlap(const PdgGuidCompound& guidCompound) const;
	KyResult SavePathDataIndex(Endianness::Type endianness);
	void UpdateSectorOverlapStatus();

private:
	PdgSystem m_sys;
	PdgRunOptions m_runOptions;
	PdgBlobTypeRegistry m_blobTypeRegistry;
	Array<PdgSectorGenerator*> m_sectorGenerators;
	GlobalCellGrid m_globalGrid;
	GeneratorProfiling m_profiling;
	Array< Array<KyGuid> > m_exclusiveGuids;
	Array< Array<KyGuid> > m_explicitOverlapSectors;
	KyUInt32 m_timeStamp;
	KyString m_name;
};


}


#endif
