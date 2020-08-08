/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download,
* or which otherwise accompanies this software in either electronic or hard copy form.
*/


// primary contact: GUAL - secondary contact: NOBODY
#ifndef KyPdg_ObjFileReader_H
#define KyPdg_ObjFileReader_H



#include "kypathdata/math/vec3f.h"
#include "kypdg/pdginput/inputtriangle.h"
#include "kypathdata/visualsystem/visualrepresentation.h"
#include "kypathdata/basesystem/coordsystem.h"
#include "kypathdata/containers/kystring.h"
#include "kypathdata/containers/array.h"
#include <algorithm>


namespace Kaim
{

class PdgSystem;
class ColorPalette;

class ObjFileReader
{
public:
	ObjFileReader();

	~ObjFileReader();

	/*! Read Obj file directly from specified filename. Id sys != KY_NULL, read Obj file using PdgSystem root path.*/
	KyResult ReadObjFile(const char* objFileName, PdgSystem* sys = KY_NULL);

	KyUInt32 VerticesCount() const { return (KyUInt32)m_vertices.GetCount(); }
	const Vec3f* Vertices() const { return &m_vertices[0]; }

	KyUInt32 IndicesCount() const { return (KyUInt32)m_indices.GetCount(); }
	const KyUInt32* Indices() const { return &m_indices[0]; }

	KyUInt32 MaterialIndicesCount() const { return (KyUInt32)m_materialIndices.GetCount(); }
	const KyUInt32* MaterialIndices() const { return &m_materialIndices[0]; }

	void Clear();

public:
	Array<KyString> m_materialNames;
	KyUInt32 m_primitiveWithoutMaterialCount; //!< The number of primitives (eg: triangles) that are not associated to any material.

private:
	KyResult ReadFileContent(const char* objFileName, PdgSystem* sys);
	KyResult ReadPosition(char* str, Vec3f& pos);
	KyResult ReadFace(char* str, KyUInt32* indexes, KyUInt32 maxVertices, KyUInt32& readVertices);
	KyResult ParseMaterialNames(KyFilePtr file);

	/*! Return the materialName index in the array.*/
	KyUInt32 RecordMaterialName(const KyString& materialName);
	void RemoveMaterialName(const KyString& materialName);
	void ClearMaterialNames() { m_materialNames.Clear(); }

private:
	char* m_line;
	const KyUInt32 m_maxLineSize;

	KyUInt32 m_currentMaterialIndex;

	// current file data
	Array<Vec3f> m_vertices;
	Array<KyUInt32> m_indices;
	Array<KyUInt32> m_materialIndices;

private:
	ObjFileReader(const ObjFileReader&);
	ObjFileReader& operator=(const ObjFileReader&);
	KyResult ReadContent(KyFilePtr file);
};


/*
VisualRepresentation of all triangles of an obj file.
The color of the triangles is set depending on the associated material
according to the given color palette.
If counterClockWise is false, the visualRep will change the order of triangles vertices.
*/
class ObjVisualRepresentation : public VisualRepresentation
{
public:
	ObjVisualRepresentation(const ObjFileReader* obj, CoordSystem& coordSystem, ColorPalette* palette, bool counterClockWise)
		: VisualRepresentation(obj), m_coordSystem(coordSystem), m_palette(palette), m_counterClockWise(counterClockWise) {}

	virtual void BuildGeometry(VisualGeometryBuilder& geometryBuilder);

private:
	CoordSystem m_coordSystem;
	ColorPalette* m_palette;
	bool m_counterClockWise;
};


/*
VisualRepresentation of triangles associated to a given material index.
The color of the triangles is set according to the given color palette.
*/
class ObjMaterialVisualRepresentation : public VisualRepresentation
{
public:
	ObjMaterialVisualRepresentation(const ObjFileReader* obj, CoordSystem& coordSystem, ColorPalette* palette,
		KyUInt32 materialIndex, bool counterClockWise) : VisualRepresentation(obj), m_coordSystem(coordSystem),
		m_palette(palette), m_materialIndex(materialIndex), m_counterClockWise(counterClockWise) {}

	virtual void BuildGeometry(VisualGeometryBuilder& geometryBuilder);

private:
	CoordSystem m_coordSystem;
	ColorPalette* m_palette;
	KyUInt32 m_materialIndex;
	bool m_counterClockWise;
};


}


#endif

