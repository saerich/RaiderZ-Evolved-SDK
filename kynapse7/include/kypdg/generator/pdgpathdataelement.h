/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download,
* or which otherwise accompanies this software in either electronic or hard copy form.
*/




// primary contact: GUAL - secondary contact: NOBODY
#ifndef KyPdg_PdgPathDataElement_H
#define KyPdg_PdgPathDataElement_H


#include <kypathdata/navmesh/navmesh.h>
#include <kypathdata/graph/graph.h>
#include <kypdg/generator/pdgguidcompound.h>
#include <kypathdata/graph/jumpedgecandidatearray.h>

#include "kypathdata/containers/kystring.h"
#include "kypathdata/containers/array.h"
#include "kypathdata/graph/inputgraphvertexcell.h"

namespace Kaim
{

class PdgSystem;
class PdgInput;
class Cell;
class FlyCell;
class NavMesh;
class GraphCell;
class PdgSectorGenerator;

class PdgPathDataElement
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public: 
	enum PdgPathDataElementOrigin
	{
		PATHDATA_ELEMENT_FROM_GENERATOR, 
		PATHDATA_ELEMENT_FROM_PARTITIONER
	};
public:
	PdgPathDataElement(PdgSystem* sys, PdgPathDataElementOrigin origin = PATHDATA_ELEMENT_FROM_GENERATOR) : m_sys(sys), m_mainSectorGenerator(KY_NULL), m_origin(origin) {}

	// DeAllocate the Cells and FlyCells pointed in m_cells and m_flyCells and clear the arrays.
	// NB. ClearCells is NOT called automatically when PdgPathDataElement is destroyed !
	// see note below
	void ClearCells();

	NavMesh* GetNavMesh() { return m_navMeshHandler.Blob(); }
	Graph* GetGraph() { return m_graphHandler.Blob(); }
	JumpEdgeCandidateArray* GetJumpCandidates() { return m_jumpHandler.Blob(); }
	InputGraphVertices* GetInputVertices() { return m_inputVerticesHandler.Blob(); }

	KyResult GenerateRawCells();
	KyResult FilterCells();
	KyResult GenerateGraphAndGraphRelatedData();

	KyResult GenerateFly();

	KyResult GenerateNavMesh();
	KyResult GenerateGraph();
	
	void ComputeCellBBox();

	KyResult SaveIndividualElements(Endianness::Type endianness);

	bool operator<(const PdgPathDataElement& other);

private:
	KyResult GenerateCells();
	KyResult GenerateFlyCells();

	KyString GetIndividualElementFileName();
	
	KyResult GenerateFlyGraph();
	KyResult GenerateJumpCandidates();

	KyResult GenerateEdgeAdditionalData();
	KyResult GenerateVertexAdditionalData();
	KyResult GenerateInputGraphVertices();

public:
	PdgSystem* m_sys;
	PdgGuidCompound m_guidCompound;
	PdgSectorGenerator* m_mainSectorGenerator;

	// Please note that cells allocated and stored in m_cells and m_flyCells
	// must be deallocated manually (not done in dtor). 
	// DeAllocation is performed by ClearCells() method which is called from PdgSectorGenerator. 
	// The reason for this is that a partitioner might create an aliasing
	// where 2 different pathdata elements store pointers to the same cells
	// If we were to call ClearCells in the dtor, we would risk a 2x DeAlloc. 
	Array<Cell*> m_cells;
	Array<FlyCell*> m_flyCells;
	CellBox m_cellBox;

	BlobHandler<NavMesh> m_navMeshHandler;
	BlobHandler<Graph> m_graphHandler;
	BlobHandler<JumpEdgeCandidateArray> m_jumpHandler;
	BlobHandler<InputGraphVertices> m_inputVerticesHandler;
	BaseBlobHandler m_edgeDataHandler;
	BaseBlobHandler m_vertexDataHandler;

	PdgPathDataElementOrigin m_origin;
};


}


#endif

