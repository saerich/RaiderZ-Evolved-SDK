/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: GUAL - secondary contact: NOBODY
#ifndef KyPdg_Cell_H
#define KyPdg_Cell_H


#include <kypdg/generator/celldesc.h>
#include <kypdg/generator/cellgeneratorprofiling.h>
#include <kypathdata/blob/blobhandler.h>
#include <kypdg/generator/pdgguidcompound.h>
#include <kypathdata/containers/array.h>
#include <kypathdata/graph/inputgraphvertex.h>
#include "kypathdata/graph/inputgraphvertexcell.h"


namespace Kaim
{

class PdgInput;
class PdgSystem;
class DynamicRasterCell;
class DynamicNavRasterCell;
class DynamicNavCellHeightField;
class BoundaryGraph;
class DynamicNavCell;
class PdgInputCell;
class DynamicPointGraphCell;
class PdgPathDataElement;
class PdgSectorGenerator;

class NavCell;
class PointGraphCell;

template <class T>
class BlobHandler;

/* Cell is where most of the effective PDG algorithms are run. */
class Cell
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	Cell(PdgSystem* sys);

	~Cell();

	void Clear();

	void Init(const CellPos& cellPos, PdgPathDataElement* pathDataElement, bool isOnOverlap);

	void ExtractRelevantInputGraphVertices(const PdgInput& input);

	void AddInputCell(const PdgInputCell* inputCell);

	KyResult Generate();

	const CellGeneratorProfiling& Profiling() const { return m_profiling; }

	const CellDesc& Desc() const { return m_cellDesc; }

	bool IsEmpty() const { return m_isEmpty; }

	const DynamicNavCell* GetDynamicNavCell() const  { return m_dynamicNavCell; }
	DynamicNavCell* GetDynamicNavCell() { return m_dynamicNavCell; }
	void SetDynamicNavCell(DynamicNavCell* cell) { m_dynamicNavCell = cell; }

	const NavCell* GetNavCell() const { return m_navCellHandler.Blob(); }
	NavCell* GetNavCell() { return m_navCellHandler.Blob(); }
	

	const DynamicPointGraphCell* GetDynamicPointGraphCell() const { return m_dynamicPointGraphCell; }
	DynamicPointGraphCell* GetDynamicPointGraphCell() { return m_dynamicPointGraphCell; }
	Array<const PdgInputCell*>& GetInputCells() { return m_inputCells; }
	bool IsOnOverlap() const { return m_isOnOverlap; }

	const DynamicInputGraphVertexCell* GetInputGraphVertexCell() const { return &m_inputGraphVertexCell; }

private:
	KyResult GenerateRaster();
	KyResult GenerateNavRaster();
	KyResult GenerateHeightField();
	KyResult GenerateNavCell();
	KyResult GenerateBoundaries();
	KyResult GeneratePointGraphCell();

private:
	PdgSystem* m_sys;
	CellDesc m_cellDesc;
	Array<const PdgInputCell*> m_inputCells;
	PdgPathDataElement* m_pathDataElement;

	bool m_isEmpty;
	bool m_isOnOverlap;

	// dynamic (pdg) data
	DynamicRasterCell* m_dynamicRaster;
	DynamicNavRasterCell* m_dynamicNavRaster;
	DynamicNavCellHeightField* m_navcellHeightField;
	BoundaryGraph* m_boundaryGraph;
	DynamicNavCell* m_dynamicNavCell;
	DynamicPointGraphCell* m_dynamicPointGraphCell;
	Array<KyUInt32> m_terrainTypeTable;
	DynamicInputGraphVertexCell m_inputGraphVertexCell;

	// final (runtime) data
	BlobHandler<NavCell> m_navCellHandler;
	BlobHandler<PointGraphCell> m_pointGraphCellHandler;

	CellGeneratorProfiling m_profiling;
};


}


#endif
