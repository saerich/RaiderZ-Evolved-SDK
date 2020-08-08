/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: BEHE - secondary contact: GUAL
#ifndef KyFlyPdg_FlyCell_H
#define KyFlyPdg_FlyCell_H

#include "kypathdata/blob/blobhandler.h"
#include "kypdg/generator/celldesc.h"
#include "kypathdata/containers/array.h"

namespace Kaim
{

class DynamicNavRasterCell;
class DynamicPointGraphCell;
class DynamicPointGraphSimplifier;
class DynamicRasterCell;
class FlyRaster;
class PdgSystem;
class PointGraphCell;
class PdgPathDataElement;
class PdgInputCell;

/*
FlyCellDesc is a CellDesc extended with min/max altitude info
*/
class FlyCellDesc
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	void Init(const SquarePartition& partition, const CellPos& cellPos);
	bool operator == (const FlyCellDesc& other) const;
	void SetBoundingAltitudes(Coord minAltitude, Coord maxAltitude);

public:
	CellDesc m_cellDesc;
	Coord m_coordMinAltitude; // extended coords are the triangle max/min altitude plus an arbitrary extent to allow for flying above/under the geometry
	Coord m_coordMaxAltitude;
};
inline void SwapEndianness(Endianness::Target e, FlyCellDesc& self)
{
	SwapEndianness(e, self.m_cellDesc);
	SwapEndianness(e, self.m_coordMinAltitude);
	SwapEndianness(e, self.m_coordMaxAltitude);
}

/*
FlyCell is where most of the effective Fly-PDG algorithms are run.
*/
class FlyCell
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	FlyCell(PdgSystem* sys);
	~FlyCell();

	KyResult Init(const Array<const PdgInputCell*>& inputCells, const CellPos& cellPos, PdgPathDataElement* pathDataElement);

	KyResult Generate();

	const DynamicPointGraphCell* GetDynamicPointGraphCell() const { return m_graph; }

	const FlyCellDesc& Desc() const { return m_flyCellDesc; }

private:
	void AddInputCell(const PdgInputCell* inputCell) { m_inputCells.PushBack(inputCell); }

	KyResult GenerateRaster(DynamicRasterCell& raster);
	KyResult GenerateFaceVertice(const DynamicRasterCell& raster, DynamicPointGraphCell& graph);
	KyResult GenerateFlyRaster(FlyRaster& flyRaster);
	KyResult SimplifyGraph(DynamicPointGraphCell& graph, DynamicPointGraphSimplifier& graphSimplifier);

private:
	PdgSystem* m_sys;
	Array<const PdgInputCell*> m_inputCells;
	PdgPathDataElement* m_pathDataElement;
	DynamicPointGraphCell* m_graph;
	FlyCellDesc m_flyCellDesc;
	bool m_isEmpty;
};

} // namespace Kaim

#endif
