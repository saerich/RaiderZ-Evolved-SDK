/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*! \file
	\ingroup kypathdata_navmesh */

// ---------- Primary contact: JUBA - secondary contact: NOBODY
#ifndef KyPathData_NavMeshTypes_H
#define KyPathData_NavMeshTypes_H

#include "kypathdata/generic/memory.h"
#include "kypathdata/basesystem/logger.h"
#include "kypathdata/math/vec3f.h"
#include "kypathdata/math/vec2i.h"
#include "kypathdata/math/box2i.h"
#include "kypathdata/math/cardinaldir.h"


namespace Kaim
{

typedef KyInt32 CellCoord; /*!< A type that represents the placement of a cell on one axis of a 2D grid. \ingroup kypathdata_navmesh */
typedef Vec2i CellPos; /*!< A type that represents the position of a cell within a 2D grid. \ingroup kypathdata_navmesh */
typedef Box2i CellBox; /*!< A type that represents a bounding box around cells in a 2D grid. \ingroup kypathdata_navmesh */
static const KyInt32 InvalidCellCoord = KyInt32MAXVAL; /*!< Represents an invalid #CellCoord object. \ingroup kypathdata_navmesh */


typedef Vec3f NavVertex; /*!< A type that represents a single vertex of a triangle within a NavMesh. \ingroup kypathdata_navmesh */

// TerrainType in the NavMesh
typedef KyUInt32 NavTerrainType; /*!< A type that represents a single type of terrain. These types of terrain can be combined into bitmasks that use the #NavTerrainTypeMask type. \ingroup kypathdata_navmesh */
static const NavTerrainType NavTerrainType_All = KyUInt32MAXVAL; /*!< Represents a terrain type that includes all terrain types. \ingroup kypathdata_navmesh */
static const NavTerrainType NavTerrainType_None = 0; /*!< Represents no terrain type, or an invalid terrain type. \ingroup kypathdata_navmesh */

typedef KyUInt32 NavTerrainTypeMask; /*!< A type that represents a combination of multiple different #NavTerrainType values. \ingroup kypathdata_navmesh */
static const NavTerrainTypeMask NavTerrainTypeMask_All = KyUInt32MAXVAL; /*!< Represents a terrain type that includes all terrain types. \ingroup kypathdata_navmesh */
static const NavTerrainTypeMask NavTerrainTypeMask_None = 0; /*!< Represents no terrain type, or an invalid terrain type. \ingroup kypathdata_navmesh */

/*! Indicates whether or not the specified #NavTerrainTypeMask contains the specified #NavTerrainType. \ingroup kypathdata_navmesh */
static KY_FORCE_INLINE bool IsTerrainTypeCompliant(NavTerrainTypeMask mask, NavTerrainType terrainType) { return (mask & terrainType) != 0; }

// vertex Index in the Floor
typedef KyUInt32 NavVertexIdx; /*!< An index that uniquely identifies a single vertex of a triangle within the set of vertices owned by a NavFloor. \ingroup kypathdata_navmesh */
static const NavVertexIdx NavVertexIdx_Invalid = KyUInt32MAXVAL; /*!< Represents an invalid #NavVertexIdx. \ingroup kypathdata_navmesh */
static const NavVertexIdx NavVertexIdx_Unset   = KyUInt32MAXVAL - 1; /*!< Represents an unspecified #NavVertexIdx. \ingroup kypathdata_navmesh */
static const NavVertexIdx NavVertexIdx_Outside = KyUInt32MAXVAL - 2; /*!< Represents space outside of the valid NavMesh. \ingroup kypathdata_navmesh */
/*! Indicates whether or not the specified NavVertexIdx is valid. \ingroup kypathdata_navmesh */
static KY_FORCE_INLINE bool IsNavVertexIdxValid(const NavVertexIdx idx) { return idx < NavVertexIdx_Outside; }

// HalfEdge Index in the Floor
typedef KyUInt32 NavHalfEdgeIdx; /*! An index that uniquely identifies a single edge of a triangle within the set of edges owned by a NavFloor. \ingroup kypathdata_navmesh */
static const NavHalfEdgeIdx NavHalfEdgeIdx_Invalid = KyUInt32MAXVAL; /*!< Represents an invalid #NavHalfEdgeIdx. \ingroup kypathdata_navmesh */
static const NavHalfEdgeIdx NavHalfEdgeIdx_Unset   = KyUInt32MAXVAL - 1; /*!< Represents an unspecified #NavHalfEdgeIdx. \ingroup kypathdata_navmesh */
static const NavHalfEdgeIdx NavHalfEdgeIdx_Outside = KyUInt32MAXVAL - 2; /*!< Represents space outside of the valid NavMesh. \ingroup kypathdata_navmesh */
/*! Indicates whether or not the specified NavHalfEdgeIdx is valid. \ingroup kypathdata_navmesh */
static KY_FORCE_INLINE bool IsNavHalfEdgeIdxValid(const NavHalfEdgeIdx idx) { return idx < NavHalfEdgeIdx_Outside; }

// Triangle Index in the Floor
//WARNING, we call it Idxx opposed to NavTriangleIdx, but it is not an Idxx in an array
typedef KyUInt32 NavTriangleIdx; /*! An index that uniquely identifies a single triangle within the set of triangles owned by a NavFloor. \ingroup kypathdata_navmesh */
static const NavTriangleIdx NavTriangleIdx_Invalid = KyUInt32MAXVAL; /*!< Represents an invalid #NavTriangleIdx. \ingroup kypathdata_navmesh */
static const NavTriangleIdx NavTriangleIdx_Unset   = KyUInt32MAXVAL - 1; /*!< Represents an unspecified #NavTriangleIdx. \ingroup kypathdata_navmesh */
static const NavTriangleIdx NavTriangleIdx_Outside = KyUInt32MAXVAL - 2; /*!< Represents space outside of the valid NavMesh. \ingroup kypathdata_navmesh */
/*! Indicates whether or not the specified NavTriangleIdx is valid. \ingroup kypathdata_navmesh */
static KY_FORCE_INLINE bool IsNavTriangleIdxValid(const NavTriangleIdx idx) { return idx < NavTriangleIdx_Outside; }

// Floor Index in the Cell
typedef KyUInt32 NavFloorIdx; /*! An index that uniquely identifies a single NavFloor within the set of \NavFloors owned by a NavCell. \ingroup kypathdata_navmesh */
static const NavFloorIdx NavFloorIdx_Invalid = KyUInt32MAXVAL; /*!< Represents an invalid #NavFloorIdx. \ingroup kypathdata_navmesh */
static const NavFloorIdx NavFloorIdx_Unset   = KyUInt32MAXVAL - 1; /*!< Represents an unspecified #NavFloorIdx. \ingroup kypathdata_navmesh */
static const NavFloorIdx NavFloorIdx_Outside = KyUInt32MAXVAL - 2; /*!< Represents space outside of the valid NavMesh. \ingroup kypathdata_navmesh */
/*! Indicates whether or not the specified NavFloorIdx is valid. \ingroup kypathdata_navmesh */
static KY_FORCE_INLINE bool IsNavFloorIdxValid(const NavFloorIdx idx) { return idx < NavFloorIdx_Outside; }

// NavCell Index in a NavMesh
typedef KyUInt32 NavCellIdx; /*! An index that uniquely identifies a single NavCell within the set of \NavCells owned by a NavMesh. \ingroup kypathdata_navmesh */
static const NavCellIdx NavCellIdx_Invalid = KyUInt32MAXVAL; /*!< Represents an invalid #NavCellIdx. \ingroup kypathdata_navmesh */
/*! Indicates whether or not the specified NavCellIdx is valid. \ingroup kypathdata_navmesh */
static KY_FORCE_INLINE bool IsNavCellIdxValid(const NavCellIdx idx) { return idx < NavCellIdx_Invalid; }

// StitchedNavCell Index in a StitchedNavMesh
typedef KyUInt32 StitchedNavCellIdx;
static const StitchedNavCellIdx StitchedNavCellIdx_Invalid = KyUInt32MAXVAL;
static KY_FORCE_INLINE bool IsStitchedNavCellIdxValid(const StitchedNavCellIdx idx) { return idx < StitchedNavCellIdx_Invalid; }

}

#endif

