/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

// primary contact: GUAL - secondary contact: LASI
#ifndef KyPdg_ClientInputConsumer_H
#define KyPdg_ClientInputConsumer_H

#include <kypdg/pdginput/inputtriangle.h>
#include <kypdg/pdginput/terraintypevolume.h>
#include <kypathdata/math/box3f.h>
#include <kypdg/pdginput/seedpoint.h>
#include <kypdg/pdginput/clientinput.h>
#include <kypathdata/graph/inputgraphvertex.h>

namespace Kaim
{

class PdgSystem;
class SpatializedPdgInput;
class PdgInput;
class PdgSectorConfig;
class PdgSector;


/* interesting statistics, can be used to check if there was a OneMeter problem */
class ClientInputConsumerStatistics
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	ClientInputConsumerStatistics() { Clear(); }

	void Clear()
	{
		m_nbConsumedTriangles = 0;
		m_consumedSurface2d = 0;
		m_nbConsumedVolumes = 0;
	}

	KyUInt32 GetNbConsumedTriangles()               { return m_nbConsumedTriangles; }
	KyFloat32 GetConsumedSurface2d()                { return m_consumedSurface2d; }
	KyFloat32 GetAverageConsumedTriangleSurface2d() { return m_consumedSurface2d / (KyFloat32)m_consumedSurface2d; }
	void GetConsumedTrianglesBoundingBox(Box3f& box) { box = m_consumedTrianglesBoundingBox; }

public:
	KyInt32 m_nbConsumedTriangles;
	KyFloat32 m_consumedSurface2d;
	Box3f m_consumedTrianglesBoundingBox;
	KyUInt32 m_nbConsumedVolumes;
};

/*! The ClientInputConsumer class is one of the primary components of the PathData generation system. Its role
	is to accept data that describes your terrain from an instance of your PdgInputProducer class. This data is then 
	used as input for the PathData generation process. This data includes:
	-	The triangles that make up the terrain mesh, along with an optional color for each that defines its
		material: the type of terrain it represents.
	-	Optional 3D coloration volumes that tag all triangles within their volumes with a particular terrain type.
	-	Optional seed points that identify walkable areas.
	\ingroup kypdg */
class ClientInputConsumer
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public: 
	enum ClientInputLogMode
	{
		DO_NOT_LOG_CLIENT_INPUT,
		DO_LOG_CLIENT_INPUT
	};

public:
	/*! \label_constructor For internal use. */
	ClientInputConsumer(PdgSystem* sys);

	/*! \label_virtualdestructor For internal use. */
	~ClientInputConsumer() { Clear(); }

	/*! \label_clear */
	void Clear();

	/*! \label_init Called automatically by the Generator. For internal use. */
	void Init(const PdgSector* sector, ClientInputLogMode clientInputLogMode, const TerrainTypeMask& defaultTerrainTypeMask = s_allWalkable_terrainType);

	/*! Defines whether or not to filter out back-facing triangles: i.e. faces whose normals point downwards (negative Z values).
		If filtering is enabled (the default), all downward facing triangles will be considered non-walkable.
		If you disable filtering, downward-facing triangles will be considered as stable, walkable terrain, and PathData will be generated for them. */
	void ToggleBackfaceTriangleFiltering(bool doTag) { m_tagBackfaceTriangleAsNonWalkable = doTag; }

	/*! Indicates whether or not the this object contains valid input data. */
	bool IsValid() const;

	/*! Provides multiple triangles to the ClientInputConsumer using a single array of floating-point numbers. 
		Every three elements are interpreted as the X, Y and Z coordinates of a point in a triangle, and every three
		points are interpreted as a single triangle. In other words:
		-	<tt>client_vertices[0..2]</tt> should contain the X, Y and Z coordinates of the first point in the first triangle,
		-	<tt>client_vertices[3..5]</tt> should contain the X, Y and Z coordinates of the second point in the first triangle,
		-	<tt>client_vertices[6..8]</tt> should contain the X, Y and Z coordinates of the third point in the first triangle,
		-	<tt>client_vertices[9..11]</tt> should contain the X, Y and Z coordinates of the first point in the second triangle, etc.
		\pp All coordinates and altitudes must be expressed in the coordinate system of the \gameOrSim, NOT the \SDKName coordinate system. 
		They will be transformed automatically according to the CoordSystem set up for the BaseSystem. 
		\pp All triangles are assigned the default terrain type. */
	KyResult ConsumeTriangleFromFloatArray(KyFloat32* client_vertices)
	{
		return ConsumePlainTriangleFromPos(*(Vec3f*)&client_vertices[0], *(Vec3f*)&client_vertices[3], *(Vec3f*)&client_vertices[6]);
	}

	/*! Provides a single triangle to the ClientInputConsumer. The normal of the face of the triangle is computed internally, 
		interpreting the points provided in clockwise order.
		\param client_A				The coordinates of the first point in the triangle.
		\param client_B				The coordinates of the second point in the triangle.
		\param client_C				The coordinates of the third point in the triangle.
		\pp All coordinates and altitudes must be expressed in the coordinate system of the \gameOrSim, NOT the \SDKName coordinate system. 
		They will be transformed automatically according to the CoordSystem set up for the BaseSystem. */
	KyResult ConsumePlainTriangleFromPos(const Vec3f& client_A, const Vec3f& client_B, const Vec3f& client_C)
	{
		return ConsumeTriangleFromPos(client_A, client_B, client_C, m_defaultTerrainType);
	}

	/*! Provides a single triangle to the ClientInputConsumer. The normal of the face of the triangle is computed internally, 
		interpreting the points provided in clockwise order.
		\param client_A				The coordinates of the first point in the triangle.
		\param client_B				The coordinates of the second point in the triangle.
		\param client_C				The coordinates of the third point in the triangle.
		\param terrainType			A bitmask that sets the terrain type that will be assigned to the triangle. You can also use the
									special value #s_nonWalkable_terrainType to tag the triangle as being non-walkable; this prevents
									generating PathData for this triangle.
		\pp All coordinates and altitudes must be expressed in the coordinate system of the \gameOrSim, NOT the \SDKName coordinate system. 
		They will be transformed automatically according to the CoordSystem set up for the PdgSystem. */
	KyResult ConsumeTriangleFromPos(const Vec3f& client_A, const Vec3f& client_B, const Vec3f& client_C, TerrainTypeMask terrainType)
	{
		Vec3f client_triangleNormal;
		client_triangleNormal.Cross((client_B - client_A), (client_C - client_B));
		client_triangleNormal.Normalize();

		return ConsumeTriangle(client_A, client_B, client_C, client_triangleNormal, terrainType);
	}

	/*! Provides a single triangle to the ClientInputConsumer.
		\param client_A					The coordinates of the first point in the triangle.
		\param client_B					The coordinates of the second point in the triangle.
		\param client_C					The coordinates of the third point in the triangle.
		\param client_triangleNormal	The normal of the front face of the triangle.
		\param terraintTypeMask			A bitmask that sets the terrain type that will be assigned to the triangle. You can also use the
										special values #s_nonWalkable_terrainType and #s_allWalkable_terrainType.
		\pp All coordinates and altitudes must be expressed in the coordinate system of the \gameOrSim, NOT the \SDKName coordinate system. 
		They will be transformed automatically according to the CoordSystem set up for the BaseSystem. */
	KyResult ConsumeTriangle(const Vec3f& client_A, const Vec3f& client_B, const Vec3f& client_C, const Vec3f& client_triangleNormal, TerrainTypeMask terraintTypeMask);

	/*! Provides a single terrain type volume to the ClientInputConsumer.
		\pp All coordinates and altitudes must be expressed in the coordinate system of the \gameOrSim, NOT the \SDKName coordinate system. 
		They will be transformed automatically according to the CoordSystem set up for the BaseSystem. */
	KyResult ConsumeTerrainTypeVolume(const ClientTerrainTypeVolume& terrainTypeVolume);

	/*! Retrieves information about the triangles consumed by this object. */
	ClientInputConsumerStatistics& GetStats() { return m_stats; }

	/*! Sets the default terrain type that will be assigned to triangles when no other terrain type is specified explicitly. */
	void SetDefaultTerrainTypeMask(const TerrainTypeMask& mask) { m_defaultTerrainType = mask; }

	/*! Provides a seed point that identifies a walkable area of the terrain. If you provide a seed point, any areas of PathData
		that are not reachable from that seed point are automatically discarded during a post-processing phase. You can provide as
		many seed points as necessary for each sector.
		\param client_position			The position of the seed point. */
	KyResult AddSeedPoint(const Vec3f& client_position);

	/*! Specifies a position in 3D space at which the Graph will be guaranteed to have a vertex. This 
		\param client_position		The position of the vertex, in the coordinate system used by the \gameOrSim engine.  
		\param tag					A numeric tag that can be used to filter or identify this vertex during a post-processing phase,
									and when stitching the Graph at runtime.
		\param relativePosition     Indicates whether \c client_position is at the altitude of the ground or of the character's waist. */
	KyResult AddInputGraphVertex(const Vec3f& client_position, KyUInt32 tag, InputGraphVertex::VerticalRelativePosition relativePosition = InputGraphVertex::INPUT_VERTEX_ON_GROUND);

public: // internal usage
	/*! For internal use. */
	void FinalizePdgInput(PdgInput& pdgInput);

	/*! For internal use. */
	KyResult SaveClientInputUsingSectorName();

public:

	ClientInputConsumerStatistics m_stats; /*!< Maintains statistics about the data added to this object. Do not modify. */
	TerrainTypeMask m_defaultTerrainType; /*!< Stores the default terrain type set by SetDefaultTerrainTypeMask(). Set to #s_allWalkable_terrainType by default. */
	/*! @{ */
	/*! For internal use. Do not modify. */
	DynamicClientSeedPointArray m_clientSeedPoints;
	DynamicInputVertexArray m_clientInputGraphVertices;

	PdgSystem* m_sys;

	const PdgSector* m_sector;
	const PdgSectorConfig* m_sectorConfig;

	KyFloat32 m_cosSlopeMax;
	bool m_tagBackfaceTriangleAsNonWalkable;

	SpatializedPdgInput* m_spatializedPdgInput;

	ClientInput m_clientInput; 
	ClientInputLogMode m_clientInputLogMode;
	/*! @} */
};


}


#endif

