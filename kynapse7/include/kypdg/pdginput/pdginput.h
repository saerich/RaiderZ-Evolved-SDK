/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: GUAL - secondary contact: NOBODY
#ifndef KyPdg_PdgInput_H
#define KyPdg_PdgInput_H


#include <kypdg/generator/squarepartitiontypes.h>
#include <kypathdata/blob/blobaggregate.h>
#include <kypdg/generator/pdgglobalconfig.h>
#include <kypdg/pdginput/pdgsectorconfig.h>
#include <kypdg/pdginput/pdginputcell.h>
#include <kypathdata/basesystem/coordsystem.h>
#include <kypdg/pdginput/seedpoint.h>
#include <kypathdata/generic/basetypes.h>
#include <kypathdata/containers/array.h>
#include <kypathdata/graph/inputgraphvertex.h>


namespace Kaim
{

class PdgSystem;
class PdgSectorConfig;
class SpatializedPdgInput;
class PdgInputLoader;

class PdgInput
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	/*!	Constructor to only read a PdgInput.*/
	PdgInput() : m_sys(KY_NULL) {}

	/*!	---Internal--- Constructor used during PathData generation.*/
	PdgInput(PdgSystem* sys) : m_sys(sys) {}
	
	~PdgInput() { Clear(); }

	KyResult Save(const char* fullFileName);

	/*!	Load uses PdgFile if a PdgSystem has been specified in constructor.
		Otherwise it loads from specified filename.*/
	KyResult Load(const char* fullFileName);

	/* ! Called by previous method, whatever the case */
	KyResult Load(KyFilePtr& file, PdgInputLoader& loader);

	/*! Save to rootDirectory/sectorName.PdgInput */
	KyResult SaveToSectorName();

	/*! Load from rootDirectory/sectorName.PdgInput */
	KyResult LoadFromSectorName(const char* sectorName);

	KyResult InitFromSpatializedPdgInput(const PdgSectorConfig& sectorConfig, SpatializedPdgInput& spatializedPdgInput);

	void AddClientSeedPoints(const DynamicClientSeedPointArray& seedpoints);

	void AddClientInputGraphVertices(const DynamicInputVertexArray& vertices);


	void Clear();

public:
	PdgSystem* m_sys;

	PdgGlobalConfig m_globalConfig;

	CoordSystem m_coordSystem;

	PdgSectorConfig m_sectorConfig;

	struct InputCellData
	{
		PdgInputCell* m_inputCell;
		KyUInt32 m_size;
	};
	Array<InputCellData> m_inputCells;
	DynamicKynapseSeedPointArray m_seedPoints;
	DynamicInputVertexArray m_inputGraphVertices;

private:
	PdgInput& operator=(const PdgInput& pdgInputSector);
	PdgInput(const PdgInput& pdgInputSector);
};



}


#endif

