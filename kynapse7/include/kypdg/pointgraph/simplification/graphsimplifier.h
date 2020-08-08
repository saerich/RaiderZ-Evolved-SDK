/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download,
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

// primary contact: JODA - secondary contact: NOBODY
#ifndef KyPdg_GraphSimplifier_H
#define KyPdg_GraphSimplifier_H

#include "kypathdata/generic/memory.h"

namespace Kaim
{

class CoverageInfo;
class DynamicNavCell;
class DynamicNavFloor;
class PdgPointGraph;
class PdgSystem;


class GraphSimplifier
{
	KY_DEFINE_NEW_DELETE_OPERATORS;
public:
	void Simplify(const PdgSystem& sys, const DynamicNavCell& navCell, PdgPointGraph& graph);

public: // only for test
	static void SimplifyInsideNavFloor(const PdgSystem& sys, PdgPointGraph& graph, const DynamicNavFloor& navFloor, CoverageInfo& coverageInfo);
private:
	static void SimplifyNavFloorLink(const DynamicNavCell& navCell, PdgPointGraph& graph);
};

} // namespace Kaim


#endif // KyPdg_GraphSimplifier_H
