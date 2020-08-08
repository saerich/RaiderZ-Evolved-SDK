/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


#ifndef KyPdg_PdgGraphEdge_H
#define KyPdg_PdgGraphEdge_H


#include <kypathdata/generic/memory.h>
#include "kypdg/pointgraph/dynamicpointgraphvertex.h"
#include <kypdg/common/stlvector.h>

namespace Kaim
{

class PdgGraphEdgeCollapse;

/*
Contains all basic data of a graph edge, namely:
-The unique ID of the edge.
-The index of the start vertex.
-The index of the end vertex.
-The cost of the edge.
*/
class PdgGraphEdge
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	PdgGraphEdge();
	PdgGraphEdge(PdgGraphVertex& startVertex, PdgGraphVertex& endVertex);
	~PdgGraphEdge();

	enum Flags
	{
		Fixed = 1 << 1, // constant, can't be simplified / moved
		Simplifiable = 1 << 2, // variable flag during simplification process
		ModifiedThisLoop = 1 << 3, // variable flag during simplification process
		ToBeRemoved = 1 << 4, // For lazy remove.

		Dummy_Flag = KYFORCEENUMSIZEINT
	};

	void Clear();

	PdgGraphVertex* GetStartVertex() { return m_startVertex; }
	PdgGraphVertex* GetEndVertex() { return m_endVertex; }
	const PdgGraphVertex* GetStartVertex() const { return m_startVertex; }
	const PdgGraphVertex* GetEndVertex() const { return m_endVertex; }
	inline KyFloat32 GetSquareLength() const { return (m_startVertex->GetPosition() - m_endVertex->GetPosition()).GetSquareLength(); }
	inline KyFloat32 GetLength() const { return (m_startVertex->GetPosition() - m_endVertex->GetPosition()).GetLength(); }
	inline KyFloat32 GetSquareLength2d() const { return (m_startVertex->GetPosition() - m_endVertex->GetPosition()).GetSquareLength2d(); }
	inline KyFloat32 GetLength2d() const { return (m_startVertex->GetPosition() - m_endVertex->GetPosition()).GetLength2d(); }


	void ChangeStartVertex(PdgGraphVertex& newStart);
	void ChangeEndVertex(PdgGraphVertex& newEnd);

	inline void SetFixed(bool enable) { if (enable) SetFlag(Fixed); else UnsetFlag(Fixed); }
	inline bool IsFixed() const { return IsFlagSet(Fixed); }

	inline void SetSimplifiable(bool enable) { if (enable) SetFlag(Simplifiable); else UnsetFlag(Simplifiable); }
	inline bool IsSimplifiable() const { return IsFlagSet(Simplifiable); }

	inline void SetModifiedThisLoop(bool enable) { if (enable) SetFlag(ModifiedThisLoop); else UnsetFlag(ModifiedThisLoop); }
	inline bool IsModifiedThisLoop() const { return IsFlagSet(ModifiedThisLoop); }

	void SetRemoved() { SetFlag(ToBeRemoved); }
	bool IsRemoved() const { return IsFlagSet(ToBeRemoved); }

	bool IsInNavFloor(NavFloorIdx floorIdx) const;

	PdgGraphEdgeCollapse* GetCollapse();
	const PdgGraphEdgeCollapse* GetCollapse() const;

protected:
	inline void SetFlag(Flags flag) { m_flags |= flag; }
	inline void UnsetFlag(Flags flag) { m_flags &= (0xFFFFFFFF ^ flag); }
	inline bool IsFlagSet(Flags flag) const { return (m_flags & flag) != 0; }
public:
	PdgGraphVertex* m_startVertex;   //< start vertex.
	PdgGraphVertex* m_endVertex;     //< end vertex.
	KyInt32 m_index;
protected:
	KyInt32 m_flags; //< Internal use during simplification
	PdgGraphEdgeCollapse* m_collapse;

	friend class PdgPointGraph;
	friend class PdgGraphEdgeCollapse;
};

}

#endif // #ifndef KyPdg_PdgGraphEdge_H
