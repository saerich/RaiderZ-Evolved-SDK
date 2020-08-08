/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*! \file
	\ingroup kypathdata_navmesh */

// Primary contact: JUBA - secondary contact: NOBODY
#ifndef KyPathData_NavMesh_H
#define KyPathData_NavMesh_H

#include "kypathdata/navmesh/navmeshtypes.h"
#include "kypathdata/generic/memory.h"
#include "kypathdata/navmesh/navcell.h"
#include "kypathdata/blob/blobarray.h"
#include "kypathdata/blob/endianness.h"
#include <kypathdata/basesystem/coordsystem.h>
#include "kypathdata/generic/guidcompound.h"

namespace Kaim
{

/*! The NavMesh is one of the main data types that represent the terrain of your \gameOrSim world from the point of
	view of your characters. Each instance of the NavMesh class represents the navigable space within a sector of your 
	terrain using a mesh of composed of linked triangles.
	\pp Each NavMesh is made up of a regular grid of NavCell objects, each of which divides the triangles within a localized,
	square area of the sector into \NavFloors according to their connectivity and their terrain types.
	\pp For more details on \NavMeshes, including how to create them from your terrain, how to stream them at runtime, and
	how to access them, see the :USERGUIDE:"PathData" chapter.
	\ingroup kypathdata_navmesh */
class NavMesh
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	/*! Each NavMesh creates and maintains an instance of this class, accessible through its #m_entityParameters member.
		This object indicates the settings used to generate the NavMesh, and thereby the physical characteristics and
		movement model of the characters expected to use the NavMesh at runtime.
		\ingroup kypathdata_navmesh */
	struct EntityParameters 
	{
		/*! \label_constructor For internal use. */
		EntityParameters() 
		{
			m_entityRadius = 0.f;
			m_entityHeight = 0.f;
			m_stepMax = 0.f;
			m_slopeMax = 0.f;
		}
		/*! The radius of the characters that can use this NavMesh, in meters. */
		KyFloat32 m_entityRadius; //in meters
		/*! The height of the characters that can use this NavMesh, in meters. */
		KyFloat32 m_entityHeight; //in meters
		/*! The maximum difference in altitude, in meters, that is considered navigable between neighboring pixels in the
			raster created from the terrain mesh. */
		KyFloat32 m_stepMax; //in meters
		/*! The maximum slope, in degrees, for a triangle in the terrain mesh to be considered navigable. */
		KyFloat32 m_slopeMax; //in degrees
	};
public: 
	enum NavMeshFlags
	{
		NAVMESH_NO_FLAG		     = 0,
		NAVMESH_FROM_PARTITIONER = 1, 
		// NAVMESH_ADDITIONAL_FLAG = 1<<1; etc

		NavMeshFlags_FORCE32 = 0xFFFFFFFF
	};

public:
	/*! \label_blobtype */
	static const char* GetBlobType() { return "NavMesh"; }

	// version history: 
	// -----------------
	// 0- Initial version.
	// 1- Added entity parameters (LASI)
	// 2- change the order of attributes + change the BBox of floor from Box3f to Box2f
	// 3- remove some attributes of the NavCell
	// 4- Added m_guidCompound
	// 5- m_guidCompound now has a timeStamp
	// 6- CellBoundary navHalfEdgee know their CardinalDir
	// 7: remove member m_cellBoundaryEdgeCount in navCell
	// 8: added NavMeshFlag
	/*! \label_blobversion */
	static KyUInt32 GetBlobVersion() { return 8; } 

public:
	/*! \label_constructor For internal use. */
	NavMesh() {}

	/*! Returns the number of NavCell objects maintained by this NavMesh. */
	KY_FORCE_INLINE KyUInt32 GetCellCount() const { return m_cells.m_count; }

	/*! Returns the NavCell with the specified index within this NavMesh. */
	KY_FORCE_INLINE const NavCell& GetNavCell(KyUInt32 idx) const { return m_cells.Values()[idx]; }

	/*! Read accessor for #m_cellBox. */
	KY_FORCE_INLINE const CellBox& GetCellBox() const { return m_cellBox; }

	/*! Indicates whether or not the NavMesh is valid. */
	bool IsValid() const
	{
		for (KyUInt32 i= 0; i < m_cells.m_count; ++i)
			if (!GetNavCell(i).IsValid())
				return false;

		return true;
	}

	/*! Indicates whether or not the specified NavMesh was created with the same generation parameters
		as this object. */
	bool IsCompatibleWith(const NavMesh& navMesh) const;

public:
	CoordSystem m_coordSystem; /*!< The coordinate system used by the NavMesh. Do not modify. */

	Box3f m_aabbox; /*!< An axis-aligned bounding box that encloses all triangles in the NavMesh. Do not modify. */
	CellBox m_cellBox; /*!< The box of \NavCells maintained by this NavMesh. Do not modify. */
	KyFloat32 m_cellSize; /*!< The length and width of each NavCell. For internal use. Do not modify. */
	GuidCompound m_guidCompound; /*!< For internal use. Do not modify. */

	EntityParameters m_entityParameters; /*! The physical characteristics and movemement model of the character for which the NavMesh was generated. */
	BlobArray<NavCell> m_cells; /*!< The list of \NavCells maintained by the NavMesh. For internal use. Do not modify. */
	KyUInt32 m_flags; /*!< Maintains extra information about this NavMesh, set at generation time. For internal use. Do not modify. */

private:
	NavMesh& operator=(const NavMesh& navMesh);
	NavMesh(const NavMesh& navMesh);
};

/*! \label_swapendianness
	\ingroup kypathdata_navmesh */
inline void SwapEndianness(Endianness::Target e, NavMesh::EntityParameters& self)
{
	SwapEndianness(e, self.m_entityRadius);
	SwapEndianness(e, self.m_entityHeight);
	SwapEndianness(e, self.m_stepMax);
	SwapEndianness(e, self.m_slopeMax);
}

/*! \label_swapendianness
	\ingroup kypathdata_navmesh */
inline void SwapEndianness(Endianness::Target e, NavMesh& self)
{
	SwapEndianness(e, self.m_coordSystem);

	SwapEndianness(e, self.m_aabbox);
	SwapEndianness(e, self.m_cellBox);
	SwapEndianness(e, self.m_cellSize);
	SwapEndianness(e, self.m_guidCompound);

	SwapEndianness(e, self.m_entityParameters);
	SwapEndianness(e, self.m_cells);
	SwapEndianness(e, self.m_flags);
}

}


#endif //KyPathData_NavMesh_H

