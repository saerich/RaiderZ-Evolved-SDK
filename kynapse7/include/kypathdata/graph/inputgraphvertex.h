/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

/*! \file
	\ingroup kypathdata_graph */

// primary contact: LASI - secondary contact: NOBODY
#ifndef KyPdg_InputGraphVertexPosition_H
#define KyPdg_InputGraphVertexPosition_H

#include "kypathdata/math/vec3f.h"
#include "kypathdata/blob/blobarray.h"
#include "kypathdata/blob/baseblobbuilder.h"
#include "kypathdata/containers/array.h"

namespace Kaim
{

/*! Represents a single Graph vertex that was specified in advance to the PathData generation system.
	\ingroup kypathdata_runtime */
class InputGraphVertex
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public: 
	/*! Enumerates the different possible vertical altitudes at which an input vertex position may be specified. */
	enum VerticalRelativePosition
	{
		INPUT_VERTEX_ON_GROUND, /*!< Indicates that the input vertex is specified at the altitude of the ground. */
		INPUT_VERTEX_AT_HALF_ENTITY_HEIGHT /*!< Indicates that the input vertex is specified at the waist height of the character using the PathData. */
	};
public:
	/*! \label_constructor For internal use. */
	InputGraphVertex() : m_tag(0), m_vertexIndexInCell(KyUInt32MAXVAL), m_verticalRelativePosition(INPUT_VERTEX_ON_GROUND) {}
	/*! \label_constructor For internal use. */
	explicit InputGraphVertex(const Vec3f& initialPosition, KyUInt32 tag, KyUInt32 verticalRelativePosition) : m_initialPosition(initialPosition), m_tag(tag), m_vertexIndexInCell(KyUInt32MAXVAL),  m_verticalRelativePosition(verticalRelativePosition) {}
public:
	/*! The position in 3D space originally provided to the PathData generation system for this input vertex.
		In the final PathData, this value is stored in the \SDKName coordinate system. In the ClientInput, this value
		is stored in the coordinate system used in the \gameOrSim. */
	Vec3f m_initialPosition;
	KyUInt32 m_tag; /*!< Stores the value passed to ClientInputConsumer::AddInputGraphVertex() in the \c tag parameter. */
	KyUInt32 m_vertexIndexInCell; /*! For internal use. Set during Graph generation. */
	KyUInt32 m_verticalRelativePosition; /*!< Stores the value passed to ClientInputConsumer::AddInputGraphVertex() in the \c relativePosition parameter. */
};

inline void SwapEndianness(Endianness::Target e, InputGraphVertex& self)
{
	SwapEndianness(e, self.m_initialPosition);
	SwapEndianness(e, self.m_tag);
	SwapEndianness(e, self.m_vertexIndexInCell);
	SwapEndianness(e, self.m_verticalRelativePosition);
}

typedef Array<InputGraphVertex> DynamicInputVertexArray;


class InputVertexArray
{
public:
	static const char* GetBlobType() { return "InputVertexArray"; }
	// history:
	// 0: first iteration
	// 1: renamed to InputVertexArray
	// 2: added m_verticalRelativePosition
	static KyUInt32 GetBlobVersion() { return 2; }

	BlobArray<InputGraphVertex> m_vertices; 
};

inline void SwapEndianness(Endianness::Target e, InputVertexArray& self)
{
	SwapEndianness(e, self.m_vertices);
}

class InputGraphClientVertexArrayBlobBuilder : public BaseBlobBuilder<InputVertexArray>
{
public:
	InputGraphClientVertexArrayBlobBuilder(const DynamicInputVertexArray* inputSeedpoints) :
	  m_inputVertices(inputSeedpoints)
	  {}
private:
	virtual void DoBuild()
	{
		InputGraphVertex* blob = BLOB_ARRAY(m_blob->m_vertices, m_inputVertices->GetCount());
		if (IsWriteMode() == true)
		{
			for(KyUInt32 i = 0 ; i < m_inputVertices->GetCount(); ++i)
			{
				blob[i] = (*m_inputVertices)[i];
			}
		}
	}
private:
	const DynamicInputVertexArray* m_inputVertices;
};

}


#endif

