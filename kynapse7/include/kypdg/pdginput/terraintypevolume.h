/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*! \file
	\ingroup kypdg */

// primary contact: LASI - secondary contact: NOBODY
#ifndef KyPdg_TerrainTypeVolume_H
#define KyPdg_TerrainTypeVolume_H

#include "kypathdata/math/vec3f.h"
#include "kypathdata/containers/array.h"

namespace Kaim
{

/*! Represents a terrain type volume using a two-dimensional polyline extruded vertically along the "up" axis. These volumes
	may be provided in calls to ClientInputConsumer::ConsumeTerrainTypeVolume().
	This class assumes that the coordinates of the polyline are in the \gameOrSim engine coordinate system.
	\ingroup kypdg */
class ClientTerrainTypeVolume
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	/*! \label_constructor \label_mustinit */
	ClientTerrainTypeVolume();

	/*! \label_init Sets up the dimensions of the volume.
		\param points			An array of points that make up the exterior polyline of the volume. This list must be closed: i.e.
								its last element must be exactly the same as its first member. The points must be specified in
								clockwise order, seen from above. The polyline may not be twisted: i.e. its edges may not intersect.
		\param pointsCount		The number of elements in the \c points array.
		\param minZ				The altitude of the bottom face of the volume.
		\param maxZ				The altitude of the top face of the volume.
		\param terrainTypeMask	A bitmask that sets the terrain type value that will be set for all triangles in this volume.
	*/
	KyResult Init(const Vec3f* points, KyUInt32 pointsCount, KyFloat32 minZ, KyFloat32 maxZ, KyUInt32 terrainTypeMask);

public:
	/*! @{ */
	/*! Do not modify directly. */
	// Internally, data are stored in Client coordinates
	Array<Vec3f> m_points;
	KyFloat32 m_altitudeMin;
	KyFloat32 m_altitudeMax;
	KyUInt32 m_terrainTypeMask;
	KyUInt32 m_index; // internal use
	/*! @} */
};

typedef Array<ClientTerrainTypeVolume> DynamicClientTerrainTypeVolumeArray;

/* Represents a terrain type volume using a two-dimensional polyline extruded vertically along the Z axis. 
	This class essentially contains the same data as ClientTerrainTypeVolume, but in the \SDKName coordinate system. 
	This class also provides a conversion method to initialize itself from a ClientTerrainTypeVolume. */
class KynapseTerrainTypeVolume
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	/* \label_constructor \label_mustinit */
	KynapseTerrainTypeVolume();

	/* \label_init Sets up the dimensions of the volume.
		\param points			An array of points that make up the exterior polyline of the volume. This list must be closed: i.e.
								its last element must be exactly the same as its first member. The points must be specified in
								clockwise order, seen from above. The polyline may not be twisted: i.e. its edges may not intersect.
		\param pointsCount		The number of elements in the \c points array.
		\param minZ				The altitude of the bottom face of the volume.
		\param maxZ				The altitude of the top face of the volume.
		\param terrainTypeMask	A bitmask that sets the terrain type value that will be set for all triangles in this volume.
	 */
	KyResult Init(const Vec3f* points, KyUInt32 pointsCount, KyFloat32 minZ, KyFloat32 maxZ, KyUInt32 terrainTypeMask);

	KyResult InitFromClientTerrainTypeVolume(const ClientTerrainTypeVolume& clientPolyline);

public:
	/* @{ */
	/* Do not modify directly. */
	// Internally, data are stored in Kynapse coordinates
	Array<Vec3f> m_points;
	KyFloat32 m_altitudeMin;
	KyFloat32 m_altitudeMax;
	KyUInt32 m_terrainTypeMask;
	KyUInt32 m_index; // internal use
	/* @} */
};
}


#endif

