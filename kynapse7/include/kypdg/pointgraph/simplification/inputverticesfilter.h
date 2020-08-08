/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* This computer source code and related instructions and comments are the
* unpublished confidential and proprietary information of Autodesk, Inc. and
* are protected under applicable copyright and trade secret law. They may
* not be disclosed to, copied or used by any third party without
* the prior written consent of Autodesk, Inc.
*/

// primary contact: LASI - secondary contact: JODA

#ifndef KyPdg_InputVerticesFilter_H
#define KyPdg_InputVerticesFilter_H

#include "kypathdata/generic/memory.h"
#include "kypdg/pointgraph/pdginputgraphvertex.h"
#include "kypdg/generator/squarepartitiontypes.h"

namespace Kaim
{

class PdgSystem;
class DynamicNavCell;
class PdgPointGraph;
class DynamicNavFloor;
class DynamicNavTriangle;
class DynamicNavHalfEdge;

// Generic enums and structs describing filtering results / status

// Preprocessing result
enum InputVertexPreProcessResult
{
	INPUT_VERTEX_SNAPPED, // The input vertex had to be snapped. The maximum snap 2D distance is pixelSize / 2
	INPUT_VERTEX_UNSNAPPED, // The input vertex is at a valid position. Its altitude might still be changed
	INPUT_VERTEX_DISCARDED_WRONG_PREPROCESS, // Should never happen
	INPUT_VERTEX_DISCARDED_OUSTIDE_NAVMESH, // The input vertex is out of the navmesh, we will ignore it
	INPUT_VERTEX_DISCARDED_NEAR_OBSTACLE, // The input vertex is too close from an obstacle (typically less then 1 cm) 
	INPUT_VERTEX_DISCARDED_IGNORED, // The input vertex is ignored 
	INPUT_VERTEX_DISCARDED_CORNER_AND_NEAR_OBSTACLE, // Edge case 1: The input vertex is in a cell corner and at less than pixelSize/2 from an obstacle, we will ignore it
	INPUT_VERTEX_DISCARDED_CELL_BORDER_AND_NEAR_OBSTACLE, // Edge case 2: The input vertex is in a cell border and at less than pixelSize/2 from an obstacle, we will ignore it
	INPUT_VERTEX_DISCARDED_CANT_SNAP_ON_CELL_BORDER  // Edge case 3: The input vertex is close to a cell border but there is no cell boundary edge on which we can project, we will ignore it
};

// Geometric status (does not rely on the navmesh, purely geometric using the cellBox and pixelSize)
enum PdgInputVertexGeometricStatus
{
	INPUT_VERTEX_IN_CELL_NE_CORNER,
	INPUT_VERTEX_IN_CELL_NW_CORNER,
	INPUT_VERTEX_IN_CELL_SW_CORNER,
	INPUT_VERTEX_IN_CELL_SE_CORNER,
	INPUT_VERTEX_INSIDE_CELL, // inside the cellBox shrunk by pixelSize/2
	INPUT_VERTEX_IRRELEVANT_FOR_THIS_CELL, // outside of cellbox + pixelSize/2 margin - outside corner bevels
	INPUT_VERTEX_UNKNOWN_GEOMETRIC_STATUS,
	INPUT_VERTEX_NEAR_CELL_BORDER_OUTSIDE, // within pixelSize/2 range of a cell border and outside
	INPUT_VERTEX_NEAR_CELL_BORDER_INSIDE,  // within pixelSize/2 range of a cell border and inside
};

// store a geometric status + an angular sector if we are in a corner, see below
struct PdgInputVertexGeometricInfo
{
	PdgInputVertexGeometricInfo() : m_status(INPUT_VERTEX_IRRELEVANT_FOR_THIS_CELL), m_angularSector(KyUInt32MAXVAL) {}
	PdgInputVertexGeometricStatus m_status;
	KyUInt32 m_angularSector; // within [0..7] U {KyUInt32MAXVAL(default)}
};

// Now the "real" stuff.
// This class will browse all the inputVertices and try to project their originalPosition into the associated navcell.
// Particular caution must be taken when the inputVertex happens to be close to a singular feature in the navcell. 
// Such features are namely: 
// - Cell borders
// - Corners of the cell
// - Navcell floor links
// In order to ensure stitching consistency between cells and floors, we have to anticipate float accuracy issues by snapping
// some positions to particular locations on the cell boundary edges keeping in mind that the snapping must be performed in 
// a perfectly symetric way. 
// At the end of the processing (Filter()),  the m_pdgInputVertices array will contain the original input vertices that were
// succesfully snapped into the navmesh. The snappedPos field of each inputVertex will be computed as well as the snapMethod that
// will need to be used in the DynamicTriangulation.
// 
//  == KNOWN LIMITATIONS ==
// - Input vertices snapped on a navfloor link might end up in a "cul de sac" ie there might be an edge missing from the vertex to the other floor
// - Very rare glitch in corner causing bad stitch
// - When snapped on a cell border edge if there are 2 floors within an altitude range of +/- EntityHeight
// we might project the input vertex in the wrong floor possibly causing stitch problem
class InputVerticesFilter
{
	KY_DEFINE_NEW_DELETE_OPERATORS

private:
	// some internal structures/enum we will need
	struct ContainingNavTriangle
	{
		ContainingNavTriangle() : m_floor(KY_NULL), m_triangle(KY_NULL), m_altitudeOffset(KyFloat32MAXVAL) {}
		const DynamicNavFloor* m_floor;
		const DynamicNavTriangle* m_triangle;
		KyFloat32 m_altitudeOffset;

		bool operator<(const ContainingNavTriangle& other) const
		{
			return fabs(m_altitudeOffset) < fabs(other.m_altitudeOffset);
		}
	};

	struct NearestEdge
	{
		NearestEdge() : m_minDistance(KyFloat32MAXVAL), m_floor(KY_NULL), m_edge(KY_NULL) {}
		KyFloat32 m_minDistance;
		const DynamicNavFloor* m_floor;
		const DynamicNavHalfEdge* m_edge;
	};

	struct FilterHalfEdge
	{
		FilterHalfEdge() : m_floor(KY_NULL), m_edge(KY_NULL) {}
		KyUInt32 m_type;
		const DynamicNavFloor* m_floor;
		const DynamicNavHalfEdge* m_edge;
	};
	typedef FilterHalfEdge CellBorderHalfEdge;
	typedef FilterHalfEdge ObstacleHalfEdge;

	enum CellBorderHalfEdgeEndPoint
	{
		CELL_BORDER_EDGE_START, 
		CELL_BORDER_EDGE_END
	};

	enum CellBorderHalfDirection
	{
		CELL_BORDER_EDGE_DIRECTION_HORIZONTAL, 
		CELL_BORDER_EDGE_DIRECTION_VERTICAL, 
		CELL_BORDER_EDGE_DIRECTION_WHATEVER
	};

	enum CornerAngularSectorSnapMethod
	{
		NOT_A_CORNER_RELEVANT_SECTOR,
		OUTSIDE_CELL_HORIZONTAL_SNAP, 
		OUTSIDE_CELL_VERTICAL_SNAP, 
		INSIDE_CELL_HORIZONTAL_SNAP,
		INSIDE_CELL_VERTICAL_SNAP
	};

	enum NearBorderPreprocessMode
	{
		NEAR_BORDER_MODE_NORMAL,
		NEAR_BORDER_MODE_CORNER
	};

	// A static table to quickly lookup what needs to be done when we have an inputvertex in 
	// a particular angular sector of a particular corner.
	// used by GetCornerSnapMethod()
	static CornerAngularSectorSnapMethod s_snapMethodTable[];
	static CornerAngularSectorSnapMethod GetCornerSnapMethod(PdgInputVertexGeometricInfo& info);

public:
	InputVerticesFilter(const PdgSystem* sys, const DynamicNavCell* navCell, const PdgPointGraph* graph, Array<PdgInputGraphVertex>* pdgInputVertices, KyFloat32 minSqDistFromBorder = 0.0001f)
		: m_sys(sys), m_navCell(navCell), m_graph(graph), 
		  m_pdgInputVertices(pdgInputVertices), m_cellBorderEdgesArrayBuilt(false), 
		  m_minSqDistFromBorder(minSqDistFromBorder) {}

	// Main Function to be called
	KyResult Filter();

	// 1. Where is our inputVertex in the cellBox ? 
	PdgInputVertexGeometricInfo ComputeVertexGeometricInfo(const PdgInputGraphVertex& vertex);

	// 2. Where should we try to snap/project the vertex in the navmesh ? 
	InputVertexPreProcessResult PreProcessVertex(PdgInputGraphVertex& vertex, PdgInputVertexGeometricInfo& info);
	
	// 3. Well, it depends ... 
	// 3.1 Are we in a Corner ? 
	InputVertexPreProcessResult PreProcessVertexInCorner(PdgInputGraphVertex& vertex, PdgInputVertexGeometricInfo& info);
	// 3.2 ... Or near a cell border ? 
	InputVertexPreProcessResult PreProcessVertexNearCellBorder(PdgInputGraphVertex& vertex, NearBorderPreprocessMode mode = NEAR_BORDER_MODE_NORMAL, CellBorderHalfEdge precomputedEdge = CellBorderHalfEdge());
	// 3.3 ... Or stricly inside the cell box maybe ? 
	InputVertexPreProcessResult PreProcessVertexInsideCell(PdgInputGraphVertex& vertex);


private: 
	// utility private functions: 

	// pretty straightforward, call IsInsideTriangle on every triangle in the navcell
	ContainingNavTriangle FindContainingTriangle(const Vec3f& pos);

	// self-explanatory
	bool IsOutsideNavmesh(const Vec3f& pos);

	// find the closest floor link edge
	NearestEdge FindNearestFloorLinkEdgeInTriangle(const DynamicNavFloor& floor, KyUInt32 firstEdgeIdx, const Vec3f pos);

	// find the closest obstacle edge
	NearestEdge FindNearestObstacleEdgeInTriangle(const DynamicNavFloor& floor, KyUInt32 firstEdgeIdx, const Vec3f pos);

	// find  the closest Cell border edge
	CellBorderHalfEdge FindNearestCellBorder(const Vec3f& pos, CellBorderHalfDirection dir = CELL_BORDER_EDGE_DIRECTION_WHATEVER);

	// try to find a cell border edge that start or end at a certain endpoint of an other border edge
	CellBorderHalfEdge FindNextCellBorder(const CellBorderHalfEdge& edge, CellBorderHalfEdgeEndPoint endpoint);



	// returns a value between 0..7 if inside the box of radius, centered on C
	// returns KyUInt32MaxVal if outside
	//
	//			   0xFFFFFFFF = OUTSIDE
	//				 <---radius--->
	//				 --------------- -
	//				 | \  2 | 1  / |  |
	//				 |   \  |  /   |  Radius
	//				 |  3  \|/  0  |  |
	//  0xFFFFFFFF	 |------C------|  |	0xFFFFFFFF = OUTSIDE
	//				 |  4  /|\  7  |  |
	//			     |   /  |  \   |  | 
	//				 | /  5 | 6  \ |  |
	//				 --------------- - 
	//					0xFFFFFFFF

	// A vertex is deemed relevant when it is inside the exclusiveCoordBox
	// extended by pixelSize/ 2 AND if it not on the outside bevel of a corner
	//
	//	  <-pixelSize->		
	//	 ---------------------------------
	//	 |			 / |   | \           |
	//	 |  NW     /   |p/2|   \    NE   |
	//	 |       /  0  |   |  3  \       |
	//	 |      C------|...|------C      |
	//	 |     /|\  7  |   |  4  /|\     |
	//	 |   /  |  \   |   |   /  |  \   |
	//	 | /  5 | 6  \ |   | /  5 | 6  \ |
	//	 ---------------   ---------------
	//	 |		.				  .      |
	//	 |	p/2	.  Cell interior  .	p/2  |
	//	 |		.				  .	     |
	//	 ---------------   --------------
	//	 | \  2 | 1  / |   | \  2 | 1  / |
	//	 |   \  |  /   |   |   \  |  /   |
	//	 |     \|/  0  |   |  3  \|/     |
	//	 |      C------|...|------C      |
	//	 |       \  7  |   |  4  /       |
	//	 |  SW     \   |p/2|   /    SE   |
	//	 |           \ |   | /           |
	//	 ---------------------------------

	KyUInt32 FindAngularSector(const CoordPos& pos, const CoordPos& center, KyUInt32 radius);

	// to optimize FindNearestCellBorder. Not always necessary.
	void BuildBorderEdgesArray();

	Vec3f GetPosOnGround(const PdgInputGraphVertex& vertex) const
	{
		if(vertex.IsOnGround())
		{
			return vertex.GetInitialPos();
		}
		return vertex.GetInitialPos() - Vec3f(0.0f, 0.0f, m_sys->Config().m_entityHeight * 0.5f);
	}

	PdgInputGraphVertex GetComplementaryFloorLinkVertex(const PdgInputGraphVertex& vertex);

public:
	// input data
	const PdgSystem* m_sys;
	const DynamicNavCell* m_navCell;
	const PdgPointGraph* m_graph;
	KyFloat32 m_minSqDistFromBorder;

	// input/output data
	Array<PdgInputGraphVertex>* m_pdgInputVertices;

private:
	Array<CellBorderHalfEdge> m_cellBordersHalfEdges; // built on demand via BuildBorderEdgesArray
	bool m_cellBorderEdgesArrayBuilt; 

};

} // namespace Kaim

#endif // KyPdg_InputVerticesFilter_H
