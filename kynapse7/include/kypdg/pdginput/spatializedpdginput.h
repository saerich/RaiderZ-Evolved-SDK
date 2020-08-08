/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: GUAL - secondary contact: NOBODY
#ifndef KyPdg_SpatializedPdgInput_H
#define KyPdg_SpatializedPdgInput_H


#include <kypdg/generator/squarepartitiontypes.h>
#include <kypdg/common/stlvector.h>
#include <kypdg/pdginput/dynamicgrid.h>
#include <kypdg/pdginput/dynamicpdginputcell.h>
#include "kypathdata/generic/kyguid.h"


namespace Kaim
{

class PdgSystem;
class Triangle3i;
class TerrainTypeVolume;
struct InputTriangle;
class PdgSectorConfig;
class PdgInput;


/*
cells are indexed this way
  |     |     |     |     |
--+-----+-----+-----+-----+--
  |-2, 1|-1, 1| 0, 1| 1, 1|
  |     |     |     |     |
--+-----+-----+-----+-----+--
  |-2, 0|-1, 0| 0, 0| 1, 0|
  |     |     |     |     |
--+-----+-----#-----+-----+--
  |-2,-1|-1,-1| 0,-1| 1,-1|
  |     |     |     |     |
--+-----+-----+-----+---- +--
  |-2,-2|-1,-2| 0,-2| 1,-2|
  |     |     |     |     |
--+-----+-----+-----+-----+--
  |     |     |     |     |

a cell contains at least all triangles that intersect with
[cell_min - enlargement, cell_max + enlargement]
  |     |     |     |
  |   +---------+   |
--+---|-+-----+-|---+--
  |   | |     | |   |  
  |   | |     | |   |  
--+---|-+-----+-|---+--
  |   +---------+   |
  |     |     |     |
              >-<
               enlargement
*/
class SpatializedPdgInput
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	SpatializedPdgInput(PdgSystem* sys);

	~SpatializedPdgInput() { Clear(); }

	void Clear();

	void PushTriangle(const InputTriangle& triangle);

	void PushTerrainTypeVolume(const KynapseTerrainTypeVolume& volume);

	DynamicPdgInputCell* GetCell(CellCoord x, CellCoord y) { return m_grid.GetElement(x, y); }

	DynamicPdgInputCell** GetCells() { return m_grid.GetElements(); }

	KyUInt32 GetCellsCount()  { return m_grid.GetElementsCount(); }

public:
	PdgSystem* m_sys;

	KyUInt32 m_nbTrianglesPushed;
	KyUInt32 m_nbVolumesPushed;

	DynamicGrid<DynamicPdgInputCell> m_grid;

	KyGuid m_sectorGuid;
};


}


#endif
