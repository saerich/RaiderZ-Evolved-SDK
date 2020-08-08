/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: MAMU - secondary contact: LASI
#ifndef KyPdg_PdgInputProducerFromClientInputGeometry_H
#define KyPdg_PdgInputProducerFromClientInputGeometry_H

#include "kypathdata/containers/kystring.h"
#include "kypdg/pdginput/pdginputproducer.h"
#include "kypdg/pdginput/clientinputconsumer.h"


namespace Kaim
{


/*	Used internally by the Generator when you add a new sector to be generated from an existing .ClientInput file. */
class PdgInputProducerFromClientInputGeometry : public PdgInputProducer
{
public:
	enum TriangleWinding { Clockwise, CounterClockwise };

public:
	PdgInputProducerFromClientInputGeometry() {}

	virtual ~PdgInputProducerFromClientInputGeometry();

	/* Adds the specified file to the list of files that will be read by Produce().
		\param objFileName						Path to the obj file (including extension ".obj").
		\param trianglesAreCounterClockwise		True if the triangles of the clientInput file are counter clockwise. 
												If false, triangles will be transformed to be fed to generation as counterClockwise. */
	ClientInput* AddClientInputFileName(const char* clientInputFileName, TriangleWinding triangleWinding);

	virtual KyResult Produce(ClientInputConsumer& inputConsumer);

	/* Adds a terrain type volume that will be passed to the ClientInputConsumer.
		\param points			An array of points that make up the exterior polyline of the volume. This list must be closed: i.e.
								its last element must be exactly the same as its first member. The points must be specified in
								clockwise order, seen from above. The polyline may not be twisted: i.e. its edges may not intersect.
		\param pointsCount		The number of elements in the \c points array.
		\param altMin			The altitude of the bottom face of the volume.
		\param altMax			The altitude of the top face of the volume.
		\param terrainTypeMask	A bitmask that sets the terrain type value that will be set for all triangles in this volume. */
	KyResult AddTerrainTypeVolume(const Vec3f* points, const KyUInt32 pointsCount, const KyFloat32 altMin, const KyFloat32 altMax, KyUInt32 terrainTypeMask);

	void AddSeedPoint(const Vec3f& position);

	void AddInputVertex(const Vec3f& position, KyUInt32 tag, InputGraphVertex::VerticalRelativePosition relativePosition = InputGraphVertex::INPUT_VERTEX_ON_GROUND);

public:
	struct Entry
	{
		Entry() : m_clientInput(KY_NULL), m_triangleWinding(CounterClockwise) {}
		KyString m_fileName;
		ClientInput* m_clientInput;
		TriangleWinding m_triangleWinding;
	};
	Array<Entry> m_clientInputs;

	Array<ClientTerrainTypeVolume> m_volumes;
	Array<SeedPoint> m_seedPoints;
	Array<InputGraphVertex> m_inputGraphVertices;
};


}


#endif // KyPdg_PdgInputProducerFromClientInput_H

