/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

// primary contact: LASI - secondary contact: NOBODY
#ifndef KyPdg_PdgInputGraphVertex_H
#define KyPdg_PdgInputGraphVertex_H

#include "kypathdata/generic/memory.h"
#include "kypathdata/graph/inputgraphvertex.h"
#include "kypdg/navmesh/dynamicnavfloor.h"

namespace Kaim
{

class InputGraphVertex;
class PdgGraphVertex;

class PdgInputGraphVertex
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	enum SnapMethod
	{
		SnapMethod_INSIDE,
		SnapMethod_ON_FLOOR_BORDER,
		SnapMethod_ON_CELL_BORDER,
		SnapMethod_FORCE32 = 0xFFFFFFFF
	};
public:
	PdgInputGraphVertex():
		m_associatedFloor(KY_NULL),
		m_vertex(KY_NULL),
		m_snapMethod(SnapMethod_FORCE32)
	{} // To make Array happy
	explicit PdgInputGraphVertex(InputGraphVertex& inputGraphVertex) :
		m_associatedFloor(KY_NULL),
		m_inputGraphVertex(inputGraphVertex),
		m_vertex(KY_NULL),
		m_snapMethod(SnapMethod_FORCE32)
	{}

	InputGraphVertex& GetInputGraphVertex() { return m_inputGraphVertex; }

	void SnapPos(const Vec3f& pos, SnapMethod snapMethod)
	{
		m_snappedPos = pos;
		m_snapMethod = snapMethod;
	}

	bool IsOnGround() const { return m_inputGraphVertex.m_verticalRelativePosition == InputGraphVertex::INPUT_VERTEX_ON_GROUND; }
	const Vec3f& GetInitialPos() const { return m_inputGraphVertex.m_initialPosition; } 
	const Vec3f& GetSnappedPos() const { return m_snappedPos; }
	SnapMethod GetSnapMethod() const { return m_snapMethod; }

	const DynamicNavFloor* GetAssociatedNavFloor() const { return m_associatedFloor; }
	void SetAssociatedNavFloor(const DynamicNavFloor* floor) { m_associatedFloor = floor; }

	bool SetVertex(PdgGraphVertex& vertex)
	{
		if (m_vertex != KY_NULL && m_vertex != &vertex)
			return false;
		m_vertex = &vertex;
		return true;
	}
	void RemoveAssociatedVertex() { m_vertex = KY_NULL; }
	PdgGraphVertex* GetVertex() const { return m_vertex; }

private:
	const DynamicNavFloor* m_associatedFloor;
	InputGraphVertex m_inputGraphVertex;
	PdgGraphVertex* m_vertex; // to feed vertexIdx
	Vec3f m_snappedPos;
	SnapMethod m_snapMethod;
};

}

#endif // KyPdg_PdgInputGraphVertex_H
