/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// Primary contact: JUBA - secondary contact: NOBODY
#ifndef KyPathData_PropagationNode_H
#define KyPathData_PropagationNode_H

#include "kypathdata/navmesh/navmeshtypes.h"
#include "kypathdata/generic/memory.h"

namespace Kaim
{

class PropagationNode
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	PropagationNode() : m_trianglePtr(), m_cost(0.f) {}
	PropagationNode(const NavTrianglePtr& id) : m_trianglePtr(id), m_cost(0.f) {}

	KY_FORCE_INLINE bool operator< (const PropagationNode& node) const { return m_cost < node.m_cost; }
	KY_FORCE_INLINE bool operator< (const PropagationNode& node) { return m_cost < node.m_cost; }

public:
	NavTrianglePtr m_trianglePtr;

	KyFloat32 m_cost;
};

}


#endif //KyPathData_PropagationNode_H

