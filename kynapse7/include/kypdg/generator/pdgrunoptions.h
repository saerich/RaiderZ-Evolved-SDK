/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download,
* or which otherwise accompanies this software in either electronic or hard copy form.
*/


/*! \file
	\ingroup kypdg */

// primary contact: GUAL - secondary contact: LASI
#ifndef KyPdg_PdgRunOptions_H
#define KyPdg_PdgRunOptions_H


#include <kypdg/common/pathmap.h>
#include <kypdg/generator/pdgsector.h>


namespace Kaim
{

/*! Enumerates the different places that an intermediate data file can be placed. \ingroup kypdg */
enum InCellSubDir
{
	IN_CELL_SUB_DIR, /*!< Indicates that the data file should be placed in a sub-directory for data within that cell. */
	NOT_IN_CELL_SUB_DIR, /*!< Indicates that the data file should be placed outside of the cell sub-directory. */
	InCellSubDir_FORCE32 = KyUInt32MAXVAL
};

/*! Enumerates the different options for saving a .PathDataIndex file. \ingroup kypdg */
enum PathDataIndexSaveMode
{
	SAVE_GENERATOR_PATHDATA_INDEX, /*!< Indicates that the .PathDataIndex file should list all the sectors created by the Generator. */
	SAVE_SECTOR_PATHDATA_INDEX, /*!< Indicates that the .PathDataIndex file should be saved on a per sector basis. */
	DO_NOT_SAVE_PATHDATA_INDEX, /*!< Indicates that no .PathDataIndex file should be saved at all. */
	PathDataIndexSaveMode_FORCE32 = KyUInt32MAXVAL
};

/*! The IntermediateFileConfig class is used by IntermediateFilesConfig to store configuration parameters
	that control whether or not the Generator writes a single specific type of intermediate data to files on disk,
	where, and how.
	\ingroup kypdg */
class IntermediateFileConfig
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	/*! \label_constructor
		\param directory			Sets the value of #m_directory.
		\param inCellSubDir			Sets the value of #m_inCellSubDir.
		\param extension			Sets the value of #m_extension. */
	IntermediateFileConfig(const KyString& directory, InCellSubDir inCellSubDir, const KyString& extension);

	/*! Write accessor for #m_doWrite. */
	bool DoWrite() const { return m_doWrite; }

	/*! Returns the actual file name that should be used for the file that corresponds to the \c cellPos cell. */
	KyString FileName(const CellPos& cellPos) const;

public:
	/*! Determines whether or not this type of intermediate data should be written to files on disk.
		\defaultvalue		\c false */
	bool m_doWrite;

	/*! The directory in which the files for this type of intermediate data will be written, relative to the
		root directory set for the Generator. */
	KyString m_directory;

	/*! Determines whether or not the files for this type of intermediate data will be divided into 
		different subdirectories for different cells.
		\defaultvalue	#NOT_IN_CELL_SUB_DIR */
	InCellSubDir m_inCellSubDir;

	/*! Determines the extension to use for the files generated for this type of intermediate data. */
	KyString m_extension;
};

class IntermediateCellsConfig
{
public:
	typedef Array<CellPos>::ConstIterator CellPosConstIterator;
public:
	bool IsEmpty() const;
	bool ShouldBuild(const CellPos& pos) const;
	void AddCellToBuild(const CellPos& pos);
	void Clear();
	CellPosConstIterator CellPosBegin() const { return m_cellsToBuild.Begin(); }
	CellPosConstIterator CellPosEnd() const { return m_cellsToBuild.End(); }
private:
	bool Contain(const CellPos& pos) const;
private:
	/* Generate intermediate data for these cells. */
	Array<CellPos> m_cellsToBuild;
};

/*! The IntermediateFilesConfig class is used by PdgRunOptions to store configuration parameters that control
	whether or not the Generator writes different types of intermediate data to files on disk, where, and how.
	\ingroup kypdg */
class IntermediateFilesConfig
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	/*! \label_constructor */
	IntermediateFilesConfig();

	/*! Const read accessor for #m_rasters. */
	const IntermediateFileConfig& Rasters() const { return m_rasters; }

	/*! Const read accessor for #m_navRasters. */
	const IntermediateFileConfig& NavRasters() const { return m_navRasters; }

	/*! Const read accessor for #m_boundaryGraphs. */
	const IntermediateFileConfig& BoundaryGraphs() const { return m_boundaryGraphs; }

	/*! Const read accessor for #m_navCells. */
	const IntermediateFileConfig& NavCells() const { return m_navCells; }

	/*! Read accessor for #m_triangulation. */
	const IntermediateFileConfig& Triangulation() const { return m_triangulation; }

	/*! Const read accessor for #m_pointGraphs. */
	const IntermediateFileConfig& PointGraphs() const { return m_pointGraphs; }

	/*! Const read accessor for #m_flyRasters. */
	const IntermediateFileConfig& FlyRasters() const { return m_flyRasters; }

	/*! Read accessor for #m_rasters. */
	IntermediateFileConfig& Rasters() { return m_rasters; }

	/*! Read accessor for #m_navRasters. */
	IntermediateFileConfig& NavRasters() { return m_navRasters; }

	/*! Read accessor for #m_boundaryGraphs. */
	IntermediateFileConfig& BoundaryGraphs() { return m_boundaryGraphs; }

	/*! Read accessor for #m_navCells. */
	IntermediateFileConfig& NavCells() { return m_navCells; }

	/*! Read accessor for #m_triangulation. */
	IntermediateFileConfig& Triangulation() { return m_triangulation; }

	/*! Read accessor for #m_pointGraphs. */
	IntermediateFileConfig& PointGraphs() { return m_pointGraphs; }

	/*! Read accessor for #m_flyRasters. */
	IntermediateFileConfig& FlyRasters() { return m_flyRasters; }

	/*! Sets the Generator to write or not write all possible types of intermediate data files.
		\param doWrite		\c true makes the Generator write all intermediate data files, \c false disables
							the Generator from writing any intermediate files. */
	void SetAllDoWrites(bool doWrite);

	/*! Returns true if at least one intermediate file will be created. */
	bool DoWriteSomeIntermediateFile() const;
public:
	/*! Contains the configuration data that controls the way the Generator writes raster data to files on disk. */
	IntermediateFileConfig m_rasters;

	/*! Contains the configuration data that controls the way the Generator writes navRaster data to files on disk. */
	IntermediateFileConfig m_navRasters;

	/*! Contains the configuration data that controls the way the Generator writes boundary graph data to files on disk. */
	IntermediateFileConfig m_boundaryGraphs;

	/*! Contains the configuration data that controls the way the Generator writes NavMesh cell data to files on disk. */
	IntermediateFileConfig m_navCells;

	/*! Contains the configuration data that controls the way the Generator writes triangulation data to files on disk. */
	IntermediateFileConfig m_triangulation;

	/*! Contains the configuration data that controls the way the Generator writes point graph data to files on disk. */
	IntermediateFileConfig m_pointGraphs;

	/*! Contains the configuration data that controls the way the Generator writes fly raster data to files on disk. */
	IntermediateFileConfig m_flyRasters;

	IntermediateCellsConfig m_cellsConfig;
};

/*! The PdgRunOptions class contains configuration parameters that control the way in which the Generator
	carries out the process of generating PathData for its sectors. You can create an instance of this class,
	set up its data members as desired, and pass it in a call to Generator::SetRunOptions().
	\ingroup kypdg */
class PdgRunOptions
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	/*! Enumerates the possible ways that the Generator can decide whether or not to create \c .PdgInput and
		\c .PathData files for each sector. */
	enum GlobalMode { FORCE_NO,			/*!< Instructs the Generator to never generate a kind of data file, regardless of the setting in the PdgSector. */
					  FORCE_YES,		/*!< Instructs the Generator to always generate a kind of data file, regardless of the setting in the PdgSector. */
					  USE_SECTOR_FLAG	/*!< Instructs the Generator to use the value set in the PdgSector. */
					  };

public:
	/*! \label_constructor */
	PdgRunOptions() { Clear(); }

	/*! \label_clear Resets all members to their default values. */
	void Clear();

	/*! Writes a record of the parameter settings in this object to the specified file stream.
		Called internally by the Generator. */
	void DumpParameters(KyFilePtr outFile);

	/*! Read accessor for #m_intermediateFiles. */
	const IntermediateFilesConfig& Intermediates() const { return m_intermediateFiles; }

	/*! Const read accessor for #m_intermediateFiles. */
	IntermediateFilesConfig& Intermediates() { return m_intermediateFiles; }

	/*! Restricts PathData generation to the specified cell, and generates intermediate data for that cell. 
		Used to troubleshoot localized problems with PathData generation. */
	void AddCellToBuild(const CellPos& pos) { m_intermediateFiles.m_cellsConfig.AddCellToBuild(pos); }

	/*! Indicates whether or not the Generator will actually use parallel processing. When the Generator is configured 
		to write intermediate data, multi-core generation is deactivated. */
	bool DoReallyUseMultiCore() const { return m_doMultiCore && !m_intermediateFiles.DoWriteSomeIntermediateFile(); }

public:
	/*! Determines whether or not the PathData generation system spreads computations across all available CPUs on your computer.
		See also :USERGUIDE:"Parallelizing PathData Generation".
		\defaultvalue		\c true */
	bool m_doMultiCore;

	/*! Contains configuration parameters that relate to the generation of intermediate data files. */
	IntermediateFilesConfig m_intermediateFiles;

	/*! Determines whether or not the Generator writes new \c .PdgInput files to disk. 
		\defaultvalue		\c true */
	bool m_doWritePdgInputFiles;

	/*! Determines the way in which the Generator decides whether or not to generate new \c .PdgInput data for each sector. 
		The default value instructs the Generator to look in the sector or multi-sector configuration. You can also force
		it to always generate data for all sectors, or never generate data for any sectors.
		\acceptedvalues	Any value from the #GlobalMode enumeration. 
		\defaultvalue	#USE_SECTOR_FLAG */
	GlobalMode m_generatePdgInputMode;

	/*! Determines the way in which the Generator decides whether or not to generate new \c .ClientInput data for each sector. 
		The default value instructs the Generator to look in the sector or multi-sector configuration. You can also force
		it to always generate data for all sectors, or never generate data for any sectors.
		\acceptedvalues	Any value from the #GlobalMode enumeration. 
		\defaultvalue	#USE_SECTOR_FLAG */
	GlobalMode m_generateClientInputMode;

	/*! Determines the way in which the Generator decides whether or not to generate new \c .PathData data for each sector. 
		The default value instructs the Generator to look in the sector or multi-sector configuration. You can also force
		it to always generate data for all sectors, or never generate data for any sectors.
		\acceptedvalues	Any value from the #GlobalMode enumeration. 
		\defaultvalue	#USE_SECTOR_FLAG */
	GlobalMode m_generatePathDataMode;

	/*! Determines how the Generator behaves when it attempts to write a \c .PathData file and finds that one already exists
		in the same directory.
		-	When this value is set to \c true, the Generator opens the existing file, and compares its unique ID to the
			ID of the sector it is trying to save. If the values are the same, the existing file is overwritten. If the values
			are different, both sets of PathData are saved into the same file. This allows the iterative building up of sectors
			from multiple different types of static meshes that coexist in the same area of the terrain.
		-	When this value is set to \c false, existing \c .PathData files are always overwritten by newer ones.
		\pp \defaultvalue	\c false */
	bool m_iterativeGeneration;

	/*! Determines whether or not areas of overlap between adjacent sectors are treated separately.
		Consider two sectors, A and B, that overlap significantly. When the geometry of A or B is loaded into the \gameOrSim by 
		itself, the corresponding PathData needs to reflect only the terrain of that sector. However, when both sectors are loaded
		into the \gameOrSim at the same time, the PathData in the overlapping area needs to reflect the combination of triangles in
		the two sectors. 
		\pp When this parameter is set to \c true, the PathData generation system generates an alternate chunk of
		PathData for the area of the overlap that represents the combination of the two geometries. This alternate chunk is stored
		in either one of the two sectors, and identified by a GUID that is the combination of the GUIDs for both overlapping sectors.
		The runtime stitching system automatically detects when both sectors are loaded at the same time, and uses that alternate chunk
		for the area of overlap instead of the PathData in each sector that reflects only the geometry of that individual sector.
		\defaultvalue	\c true */
	bool m_doGenerateOverlaps;

	/*! Determines whether or not the Generator writes separate files to contain the NavMesh, the Graph, and the jump edge
		candidates for each sector.
		\defaultvalue	\c false */
	bool m_doWriteIndividualElements;

	/*! Determines whether or not the Generator clears the final PathData from memory upon completing its generation run.
		\defaultvalue	\c true */
	bool m_doClearFinalPathDataElements;

	/*! Determines wheter or not .PathData and .PathDataIndex files are written.
		\defaultvalue	\c true */
	bool m_doWritePathDataFiles;

	/*! Determines whether or not the Generator saves .PathDataIndex files to record the PathData it creates, and whether it creates a
		single .PathDataIndex for all data, or a separate .PathDataIndex file for each sector.
		\acceptedvalues	Any element from the Kaim::PathDataIndexSaveMode enumeration.
		\defaultvalue	\c #SAVE_GENERATOR_PATHDATA_INDEX */
	PathDataIndexSaveMode m_pathDataIndexSaveMode;

	/*! Determines the endianness of the final output files. This allows you to avoid swapping the byte order of the data when loading
		PathData on a machine with a different endianness. See also :USERGUIDE:"PathData Endianness".
		Note that .PdgInput and intermediate files are always little-endian.
		\acceptedvalues	Any element from the Endianness::Type enumeration.
		\defaultvalue	\c Endianness::KY_BIG_ENDIAN */
	KyUInt32 m_outputFilesEndianness;

	/*! Determines whether or not the generation process should write a file that contains a record of all generation
		parameter settings, and the name of the file to create. 
		\acceptedvalues	Any valid path and file name, or #KY_NULL to disable.
		\defaultvalue	#KY_NULL */
	char* m_paramDumpFilename;
};

} // namespace Kaim

#endif // KyPdg_PdgRunOptions_H

