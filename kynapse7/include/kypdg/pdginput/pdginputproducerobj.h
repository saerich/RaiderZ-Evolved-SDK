/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/


/*! \file
	\ingroup kypdg */

// primary contact: GUAL - secondary contact: NOBODY
#ifndef KyPdg_PdgInputProducerOBJ_H
#define KyPdg_PdgInputProducerOBJ_H


#include <kypathdata/generic/types.h>
#include <kypathdata/containers/kystring.h>
#include <kypdg/pdginput/pdginputproducer.h>
#include <kypdg/pdginput/objfilereader.h>
#include <kypathdata/containers/array.h>
#include <kypathdata/graph/inputgraphvertex.h>


namespace Kaim
{

class ObjFileReader;
class ClientInputConsumer;
class GeometryByString;
class TerrainTypeByString;


/*! Used internally by the PdgInputProducerOBJ class to represent each input .obj file.
	\ingroup kypdg */
class GeometryForPdgInputProducer
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	/*! \label_constructor For internal use. */
	GeometryForPdgInputProducer(bool generateTerrainTypesPerMaterial, TerrainTypeMask defaultTerrainType);

	/*! \label_destructor For internal use. */
	~GeometryForPdgInputProducer();

	/*! Sets the terrain type that will be tagged to all faces that have no material, and to faces that have
		a material that has not been mapped to any other terrain type. Default value is "s_allWalkable_terrainType". */
	void SetDefaultTerrainType(TerrainTypeMask terrainType) { m_defaultTerrainType = terrainType; }

	/*! Creates a mapping between the specified material and the specified terrain type. Any faces that have the
		specified material will be tagged with that terrain type.
		\pp Faces with materials that are not associated to any terrain type will be automatically given 
		a terrain type if that mechanism is enabled (see #m_generateTerrainTypesPerMaterial), or else they
		will be given the default terrain type (see SetDefaultTerrainType()). */
	void AddTerrainTypeMaterial(const char* materialName, const TerrainTypeMask terrainType);

	/*! Creates a mapping between the specified material and the special non-walkable terrain type. Any faces that have the
		specified material will be tagged as non-walkable, and no PathData will be generated for those faces. */
	void AddForbiddenMaterial(const char* materialName); 

	/*! Enables or disables the automatic mapping of materials to terrain types (enabled by default).
		\pp When enabled, each registered material in the .obj gets a terrain type according to the following rule:
		the TerrainType will be a mask with 1 bit set at position i+1 % 32, where i is the index in the TerrainType list.
		For example: m_terrainTypes[0] -> 0..010, m_terrainTypes[1] -> 0..100  etc.
		\pp 0xFFFFFFFF is reserved for plain triangles without any materials.
		\pp Terrain type assignations are circular. If there are more then 30 materials, multiple materials will get
		mapped to the same terrain type. */
	void GenerateTerrainTypesForMaterials(bool enable) { m_generateTerrainTypesPerMaterial = enable; }

	/*! Initializes the association between materials (material index) and terrain types. 
		If #m_generateTerrainTypesPerMaterial is enabled, a terrain type is automatically generated
		for each material. Otherwise, the default terrain type is associated to each material. */
	void InitAssociationBetweenMaterialIndexAndTerrainType();

	/*! Initializes the association between material indices and terrain types, and fills it from the map of 
		associations between terrain types and material names created by the customer (using 
		SetMaterialTerrainType()). */
	void CreateAssociationBetweenMaterialIndexAndTerrainType();

	/*! Returns the terrain type associated to a material index. */
	TerrainTypeMask GetTerrainTypeFromMaterialIndex(KyUInt32 materialIndex);

public:
	ObjFileReader m_objFileReader;
	bool m_trianglesAreCounterClockwise;  /*!< Indicates whether or not the vertices of the triangles in the input .obj file are specified in counter-clockwise order (true) or in clockwise order (false). */
	TerrainTypeByString* m_terrainTypeByMaterialName;  /*!< Stores the mapping (material names => terrain types). Do not modify. */
	TerrainTypeMask m_defaultTerrainType;  /*!< The defalt terrain type used to tag each face that has a material for which no terrain type was specified. Do not modify. */
	bool m_generateTerrainTypesPerMaterial;  /*!< If true, terrainTypes are automatically generated for each material. Do not modify directly; see GenerateTerrainTypesForMaterials(). */

private:
	/*!< Temporary data created when the obj has been loaded: 
	Association of a terrain type for each material index from the obj.*/
	Array<TerrainTypeMask> m_terrainTypeByMaterialIndex;
};




/*!	The PdgInputProducerOBJ class is a concrete implementation of PdgInputProducer that can read the triangle and material
	data contained in a \c .obj file, and pass it to the ClientInputConsumer.
	\pp This class is used internally by the standalone PathData generation tools supplied with \SDKName. If you save your
	terrain geometry in \c .obj files, you can use this class yourself to create PathData from the triangles in your files. 
	Create an instance, set it up with the name of one or more \c .obj files it is to read by calling AddObjFileName(), 
	and configure its terrain types if necessary. Then pass it to the Generator for a sector.
	\ingroup kypdg */
class PdgInputProducerOBJ : public PdgInputProducer
{
public:
	/*! \label_constructor */
	PdgInputProducerOBJ(bool generateTerrainTypesPerMaterial = false, TerrainTypeMask defaultTerrainType = s_allWalkable_terrainType);

	/*! \label_virtualdestructor */
	~PdgInputProducerOBJ();

	/*! Adds the specified file to the list of files that will be read by Produce().
		\param objFileName						Path to the obj file (including extension ".obj").
		\param trianglesAreCounterClockwise		True if the triangles of the obj file are counter clockwise. 
												If false, triangles will be transformed to be fed generation as counterClockwise.
		\return A pointer to an instance of GeometryForPdgInputProducer, which you can use to set up materials. */
	GeometryForPdgInputProducer* AddObjFileName(const char* objFileName, bool trianglesAreCounterClockwise);

	/*! Sets the terrain type that will be tagged to all faces that have the specified material in ALL input .obj files.
		\pp The materials which will be not associated to any terrain type, will be automatically affected 
		a terrain type if the mechanism is enabled (see EnableAutomaticGenerationOfTerrainTypesForMaterials)
		else they will be affected the default terrainType (see SetDefaultTerrainType).
		\pp NOTE: To assign terrain types to materials differently for each input .obj file, use 
		the GeometryForPdgInputProducer* returned by AddObjFileName(). */
	void AddTerrainTypeMaterial(const char* materialName, const TerrainTypeMask terrainType);

	void AddForbiddenMaterial(const char* materialName);

	/*! Adds a terrain type volume that will be passed to the ClientInputConsumer.
		\param points			An array of points that make up the exterior polyline of the volume. This list must be closed: i.e.
								its last element must be exactly the same as its first member. The points must be specified in
								clockwise order, seen from above. The polyline may not be twisted: i.e. its edges may not intersect.
		\param pointsCount		The number of elements in the \c points array.
		\param altMin			The altitude of the bottom face of the volume.
		\param altMax			The altitude of the top face of the volume.
		\param terrainTypeMask	A bitmask that sets the terrain type value that will be set for all triangles in this volume. */
	KyResult AddTerrainTypeVolume(const Vec3f* points, const KyUInt32 pointsCount, const KyFloat32 altMin, const KyFloat32 altMax, KyUInt32 terrainTypeMask);

	void AddSeedPoint(const Vec3f& position);

	void AddInputGraphVertex(const Vec3f& position, KyUInt32 tag);

	virtual KyResult Produce(ClientInputConsumer& inputConsumer);

private:
	void SetupGlobalTerrainTypeMaterial(GeometryForPdgInputProducer* geometry);

private:
	class InputVertexData
	{
	public:
		InputVertexData() {} // To make Array happy
		InputVertexData(const Vec3f& position, KyUInt32 tag) : m_position(position), m_tag(tag) {}
	public:
		Vec3f m_position;
		KyUInt32 m_tag;
	};
public:
	Array<ClientTerrainTypeVolume> m_volumes; /*!< The list of terrain type volumes. Do not modify directly. */
	Array<SeedPoint> m_seedPoints; /*!< The list of seed points. Do not modify directly. */
	Array<InputVertexData> m_inputGraphVertices; /*!< The list of input graph points. Do not modify directly. */
	GeometryByString* m_geometryByFileName;  /*!< mapping from .obj file names (complete path) to the GeometryForPdgInputProducer instances that represent those input files. Do not modify. */

	bool m_generateTerrainTypesPerMaterial;  /*!< If true, each material is automatically mapped to a terrain type. Do not modify. */
	TerrainTypeMask m_defaultTerrainType;  /*!< The default terrain type used to tag each face that has a material for which no terrain type was specified. Do not modify. */
	TerrainTypeByString* m_terrainTypeByMaterialName;  /*!< Stores the mapping between materials and terrain types. Do not modify. */

};


}


#endif

