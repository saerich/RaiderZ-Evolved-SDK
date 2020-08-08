/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/


// primary contact: GUAL - secondary contact: NOBODY
#ifndef KyPdg_DynamicPdgInputCell_H
#define KyPdg_DynamicPdgInputCell_H


#include <kypdg/generator/squarepartitiontypes.h>
#include <kypdg/pdginput/inputtriangle.h>
#include <kypdg/pdginput/terraintypevolume.h>
#include <kypdg/common/stlvector.h>
#include <kypathdata/generic/kyguid.h>


namespace Kaim
{

class PdgSystem;
class IDynamicPdgInputCellVisitor;
class SpatializedPdgInput;


class DynamicPdgInputCell
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	DynamicPdgInputCell();

	DynamicPdgInputCell(CellCoord x, CellCoord y);

	~DynamicPdgInputCell();

	void Clear();
	void PushTriangle(const InputTriangle& triangle);
	void PushTerrainTypeVolume(const KynapseTerrainTypeVolume& volume);

public:
	KyGuid m_sectorGuid;
	CellPos m_cellPos;
	KyInt32 m_minAltitude;
	KyInt32 m_maxAltitude;
	StlVector<InputTriangle> m_triangles;
	StlVector<KynapseTerrainTypeVolume> m_volumes;
};


}


#endif
