/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download,
* or which otherwise accompanies this software in either electronic or hard copy form.
*/


#ifndef KyPathData_ClientInputVisualRepresentation_H
#define KyPathData_ClientInputVisualRepresentation_H


#include "kypathdata/visualsystem/visualrepresentation.h"
#include "kypathdata/basesystem/coordsystem.h"


namespace Kaim
{


class ClientInput;
class ITerrainTypeColorPalette;
class ColorPalette;

class ClientInputVisualRepresentation : public VisualRepresentation
{

public:
	enum ClockWiseMode
	{
		CLOCK_WISE,
		COUNTER_CLOCK_WISE
	};

	enum DrawSeedPointMode
	{
		DO_NOT_DRAW_SEEDPOINTS,
		DRAW_SEEDPOINTS
	};

	enum DrawInputGraphVerticesMode
	{
		DO_NOT_DRAW_INPUT_GRAPH_VERTICES,
		DRAW_INPUT_GRAPH_VERTICES
	};

	enum DrawInternalTerrainTypesMode
	{
		DO_NOT_DRAW_TERRAIN_TYPES,
		DRAW_TERRAIN_TYPES
	};

public:
	ClientInputVisualRepresentation(ClientInput* clientInput, CoordSystem& coordSystem, ITerrainTypeColorPalette* terrainTypePalette,
		ColorPalette* palette, ClockWiseMode counterClockWise, DrawSeedPointMode drawSeedPoints, DrawInputGraphVerticesMode drawInputGraphVertices, DrawInternalTerrainTypesMode drawInternalTerrainTypes)
		: VisualRepresentation(clientInput), m_coordSystem(coordSystem), m_terrainTypePalette(terrainTypePalette), m_palette(palette),
		m_counterClockWise(counterClockWise), m_drawSeedPoints(drawSeedPoints), m_drawInputGraphVertices(drawInputGraphVertices), m_drawInternalTerrainTypes(drawInternalTerrainTypes) {}

	virtual void BuildGeometry(VisualGeometryBuilder& geometryBuilder);

public:
	CoordSystem m_coordSystem;
	ITerrainTypeColorPalette* m_terrainTypePalette;
	ColorPalette* m_palette;
	ClockWiseMode m_counterClockWise;
	DrawSeedPointMode m_drawSeedPoints;
	DrawInputGraphVerticesMode m_drawInputGraphVertices;
	DrawInternalTerrainTypesMode m_drawInternalTerrainTypes;
};


}

#endif // KyPathData_ClientInputVisualRepresentation_H
