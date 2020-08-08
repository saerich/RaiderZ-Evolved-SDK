/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*! \file
	\ingroup kypdg */

// primary contact: GUAL - secondary contact: LASI
#ifndef KyPdg_PdgSector_H
#define KyPdg_PdgSector_H


#include <kypdg/pdginput/pdgsectorconfig.h>
#include <kypathdata/generic/kyguid.h>
#include <kypdg/common/pdgfilesystem.h>
#include <kypdg/pdginput/pdginputproducer.h>
#include <kypathdata/containers/array.h>
#include <kypdg/generator/ipdgsectorgeneratorpostprocess.h>


namespace Kaim
{

class PdgSectorPartitioner;

/*! The PdgSector class provides a definition for a single sector to be treated by the Generator.
	\ingroup kypdg */
class PdgSector
{
	KY_DEFINE_NEW_DELETE_OPERATORS

friend class PdgSectorGenerator;

public:
	/*! \label_constructor
		\param inputProducer		The instance of PdgInputProducer that will be invoked to generate input
									triangles for this sector. Do not destroy this PdgInputProducer until you have finished using the Generator.
		\param sectorGuid			A globally unique ID for this sector. On Windows platforms, you can get a GUID value that is guaranteed to
									be unique by calling KyGuid::Generate(). If you set this parameter to KyGuid::s_defaultGuid, KyGuid::Generate()
									will be called automatically.
		\param sectorName			A descriptive name for this sector. If #KY_NULL, it will be generated automatically
									from the \c sectorGuid.
		\param partitionner			An optional pointer to an instance of a class that derives from PdgSectorPartitioner. If you provide a valid pointer,
									the Generator will invoke your object to re-partition the PathData generated for this sector into multiple smaller sectors.
									The instance is not copied; if you provide a valid pointer, ensure that you do not destroy the object until after you
									have finished using the Generator. If you do not want to re-partition this sector, provide #KY_NULL instead (the default). */
	PdgSector(
		PdgInputProducer* inputProducer,
		KyGuid sectorGuid,
		const char* sectorName = KY_NULL, 
		PdgSectorPartitioner* partitionner = KY_NULL)
	{
		m_generatePdgInput = true;
		m_generatePathData = true;
		m_generateClientInput = false;

		m_inputProducer = inputProducer;
		m_partitionner = partitionner;

		if (sectorGuid == KyGuid::s_defaultGuid)
			sectorGuid = KyGuid::Generate();

		if (sectorName == KY_NULL)
		{
			char generatedSectorName[37];
			sectorGuid.ToString(generatedSectorName);
			m_sectorConfig.Init(sectorGuid, generatedSectorName);
		}
		else
		{
			m_sectorConfig.Init(sectorGuid, sectorName);
		}
	}

	/*! Write accessor for #m_generatePdgInput. */
	void SetGeneratePdgInput(bool generatePdgInput) { m_generatePdgInput = generatePdgInput; }
	/*! Write accessor for #m_generateClientInput. */
	void SetGenerateClientInput(bool generateClientInput) { m_generateClientInput = generateClientInput; }
	/*! Write accessor for #m_generatePathData. */
	void SetGeneratePathData(bool generatePathData) { m_generatePathData = generatePathData; }

	/*! Specifies a bounding box around the PathData cells that should be considered to be part of this sector.
		\pp Using this method may allow you to reduce the size of your PathData by reducing the amount of overlap data
		created between adjacent sectors, at the cost of potential inconsistencies between the sector geometry and the
		PathData in areas overlapped by or adjacent to other sectors. You should only use this method once you have
		PathData generation working for your terrain, and only if you notice that the overlap data between adjacent
		sectors is causing your PathData to take up an excessive amount of memory.
		\pp This approach is best suited to worlds in which the sector geometry is laid out on a regular, axis-aligned
		grid whose cells are a multiple of the PathData cell size set by PdgGlobalConfig::m_cellSize.
		\pp This method allows you to explicitly associate PathData cells with a given sector. Wherever the input geometry
		of two or more sectors overlaps, <b>and</b> both sectors have a #CellBox set up by a call to this method, the PathData
		generated for each cell will always reflect the combination of all overlapping geometries. After generation, each
		PathData cell is stored in whichever sector (or sectors) include that cell in the #CellBox set up by this method.
		\pp For best results:
		-	If you want to use this method, set up a #CellBox for all your sectors.
		-	Ensure that there are no gaps in the cell coverage between sectors. Each PathData cell must be contained in the
			#CellBox that is assigned to at least one sector.
		-	You will typically want to avoid overlaps in the boxes that you assign to different sectors, which would result
			in the same PathData cell being included in both sectors. This is not a problem, except for the memory used by the
			duplicates.
		\pp Because each PathData cell always reflects the combination of all overlapping geometries, but each cell is
		stored in only one of the sectors, you may see localized inconsistencies between the PathData and the geometry
		within the area of overlap if you load only one of the overlapping sectors into the game.
		\pp To determine the extents of the cell box to use for each sector, you can:
		-	Calculate the values yourself. The origin of the 3CellBox is always at the same origin as the 3D world space, and
			the width and length of each cell in the CellBox is determined by the value of PdgGlobalConfig::m_cellSize.
			The CellBox is always axis-aligned.
		-	Do a first generation pass for your sector, open the PathData in the Kynapse Lab, and view the layout of the
			cells in the generated PathData for the sector.
		\pp Note that the use of this #CellBox may be overridden if this sector has been specified in a call to
		Generator::AddExplicitOverlapSectors(). */
	void SetInputCellBox(const CellBox& cellBox) { m_sectorConfig.SetInputCellBox(cellBox); }

	/*! Indicates whether or not the generation of overlap data with adjacent sectors has been restricted through a call to
		SetInputCellBox(). */
	bool IsInputCellBoxSet() { return m_sectorConfig.IsInputCellBoxSet(); }

	/*! Retrieves the value set through a call to SetInputCellBox(). */
		CellBox GetInputCellBox() 
	{
		KyInt32 minX = m_sectorConfig.m_inputCellBoxMinX;
		KyInt32 minY = m_sectorConfig.m_inputCellBoxMinY;
		KyInt32 maxX = m_sectorConfig.m_inputCellBoxMaxX;
		KyInt32 maxY = m_sectorConfig.m_inputCellBoxMaxY;
		return CellBox(CellPos(minX,minY), CellPos(maxX, maxY));
	}

	/*! Retrieves the name set for this sector. */
	const char* GetName() const { return m_sectorConfig.m_sectorName.CStr(); }

	/*! Retrieves the GUID set for this sector. */
	const KyGuid& Guid() const { return m_sectorConfig.m_sectorGuid; } 

	/*! Retrieves the PdgInputProducer object set for this sector, or #KY_NULL if none. */
	PdgInputProducer* Producer() const { return m_inputProducer; }

	/*! Specifies an instance of a class that implements ISectorGeneratorPostProcess, which will be invoked automatically
		after generating PathData for the sector. This object is not copied; you must ensure that you do not destroy it 
		until after you are finished using the Generator.
		\pp You can call this method multiple times in order to perform multiple post-process phases. The ISectorGeneratorPostProcess
		instances are invoked in the same order in which you provide them. */
	void AddPostProcess(ISectorGeneratorPostProcess* process) { m_postProcesses.PushBack(process); }

public:
	PdgSectorConfig m_sectorConfig; /*!< For internal use. Do not modify. */
	PdgInputProducer* m_inputProducer; /*!< Stores the PdgInputProducer object set for this sector. Do not modify. */
	PdgSectorPartitioner* m_partitionner; /*!< Stores a pointer to the PdgSectorPartionner object set for this sector, if any. Do not modify. */

	Array<ISectorGeneratorPostProcess*> m_postProcesses; /*!< Stores a list of pointers to the ISectorGeneratorPostProcess objects set for this sector, if any. Do not modify. */

	bool m_generatePdgInput; /*!< Determines whether or not new \c .PdgInput files will be generated for this sector. */
	bool m_generateClientInput; /*!< Determines whether or not new \c .ClientInput files will be generated for this sector. */
	bool m_generatePathData; /*!< Determines whether or not final \c .PathData files will be generated for this sector. */
	
};


}


#endif

