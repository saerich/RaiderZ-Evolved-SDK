/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

// primary contact: LASI - secondary contact: NONE
#ifndef KyPdg_TriangleSplitter_H
#define KyPdg_TriangleSplitter_H

#include "kypathdata/math/vec3f.h"
#include "kypathdata/generic/memory.h"

namespace Kaim
{

class CoordSystem;
template <typename T>
class Array;


// This class essentially split a triangle into multiple sub triangles using the "IsTriangleTooBig" criteria from the coordSystem. 
// Children will have the same winding (CW or CCW) as their parent.
// If the input triangles already satisfies the criteria, children list will be empty
class TriangleSplitter 
{

	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	KyResult SplitTriangleAccordingToCoordSystem(const Vec3f& A, const Vec3f& B, const Vec3f& C, const CoordSystem& coordSystem, Array<Vec3f>& childrenLegitTriangles) const;

private: // internal usage
	KyResult StepSubDivide(const Vec3f& A, const Vec3f& B, const Vec3f& C, bool isCCW, Array<Vec3f>& children) const;

};


}


#endif

