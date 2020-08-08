/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_graph */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_POINTINFO_VERTEXUID_H
#define KY_POINTINFO_VERTEXUID_H

#include <kyruntime/core/pointinfo.h>
#include <kyruntime/graph/graphvertexsafeptr.h>


namespace Kaim
{

/*! This class represents an item of additional information that can be attached
	to a PointWrapper object, which provides access to the vertex related to
	the position saved in the PointWrapper.
	It is used internally by the PathFinder to optimize CPU consumption.
	\ingroup kyruntime_graph */
class PointInfo_VertexSafePtr: public PointInfo
{
public:
	KY_DECLARE_POINTINFO(PointInfo_VertexSafePtr)

public:
	/*!	\label_constructor */
	PointInfo_VertexSafePtr() {}

	/*!	\label_constructor Copies the state of the specified GraphVertexSafePtr to the newly created object. */
	PointInfo_VertexSafePtr(const GraphVertexSafePtr& vertexSafePtr) : m_vertexSafePtr(vertexSafePtr) {}

	virtual void Reset() { m_vertexSafePtr.Invalidate(); }

	/*!	Sets the GraphVertexSafePtr maintained in this instance of PointInfo_VertexSafePtr. */
	void SetVertexSafePtr(const GraphVertexSafePtr& vertexSafePtr) { m_vertexSafePtr = vertexSafePtr; }

	/*!	Retrieves the GraphVertexSafePtr maintained in this instance of PointInfo_VertexSafePtr. */
	const GraphVertexSafePtr& GetVertexSafePtr() const { return m_vertexSafePtr; }

	/*!	Retrieves the GraphVertexSafePtr maintained in this instance of PointInfo_VertexSafePtr. */
	GraphVertexSafePtr& GetVertexSafePtr() { return m_vertexSafePtr; }

	void Invalidate() { m_vertexSafePtr.Invalidate(); }

protected:
	GraphVertexSafePtr m_vertexSafePtr;
};

} //namespace Kaim

#endif // KY_POINTINFO_VERTEXUID_H
