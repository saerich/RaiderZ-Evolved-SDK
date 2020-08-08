/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: LASI - secondary contact: NONE
#ifndef KyPdg_InputTriangleClipper_H
#define KyPdg_InputTriangleClipper_H

#include "kypdg/pdginput/inputtriangle.h"
#include "kypdg/common/stlvector.h"

namespace Kaim
{

//forward decl
class Box2i; 

class InputTriangleClipper
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	
	// Clip a triangle into, 0..n triangles using a clipping box.
	// output triangle(s) is(are) stored in outputTriangles if inputTriangle is relevant.
	// We use the box to define 4 clipping half planes.
	// If triangle does not overlap the clipping box, outputTriangles will be empty
	// if a triangle require clipping, the result after clipping is usually a n-sided polygon.
	// We triangulate this polygon by using a pivot point to build a fan of triangles which are
	// then stored in outputTriangles list. 
	// Output triangles have the same winding as input triangle.
	KyResult ClipTriangle(const InputTriangle& triangle, const Box2i& clippingBox, StlVector<InputTriangle>& outputTriangles);
};

}


#endif //KyPdg_InputTriangleClipper_H

