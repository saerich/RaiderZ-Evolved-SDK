/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

// primary contact: JODA - secondary contact: LASI

#ifndef KyPdg_GraphAltitudeFixer_H
#define KyPdg_GraphAltitudeFixer_H

//#include "kypathdata/generic/memory.h"
#include "kypathdata/generic/basetypes.h"

namespace Kaim
{

class DynamicNavCell;
class DynamicNavFloor;
class DynamicNavHalfEdge;
class PdgGraphVertex;
class PdgPointGraph;
class PdgSystem;
class Vec3f;

class GraphAltitudeFixer
{
public:
	static void FixAltitude(const PdgSystem& sys, const DynamicNavCell& navCell, PdgPointGraph& graph);
	static bool FixAltitude(const DynamicNavFloor& navFloor, Vec3f& pos, KyFloat32 halfHeight);

private:
	static void FixAltitude(const DynamicNavFloor& navFloor, const DynamicNavHalfEdge& halfEdge, Vec3f& pos, KyFloat32 halfHeight);
	static void FixAltitude(const DynamicNavCell& navCell, PdgGraphVertex& vertex, KyFloat32 halfHeight);
};


} // Kaim

#endif // KyPdg_GraphAltitudeFixer_H
