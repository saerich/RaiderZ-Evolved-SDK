/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: GUAL - secondary contact: MAMU
#ifndef KyPathData_VisualBoxVertices_H
#define KyPathData_VisualBoxVertices_H


#include "kypathdata/generic/memory.h"
#include "kypathdata/math/vec3f.h"


namespace Kaim
{


class VisualGeometryBuilder;
class Box3f;


//      H----------G
//     /|         /|
//    / |        / |
//   E----------F  |
//   |  |       |  |
//   |  D-------|--C
//   | /        | /
//   |/         |/
//   A----------B

/*
VisualSystem module internal class.
Allows to store oriented box vertices.
There are many ways to initialize the oriented box vertices positions : InitFrom...(...).
There is only one way to push them into a VisualGeometryBuilder : Fill(...).
*/
class VisualBoxVertices
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	void InitFromCenter(const Vec3f& center, KyFloat32 halfWidth);

	void InitFromBox(const Box3f& box);

	void InitFromSegment(const Vec3f& P, const Vec3f& Q, KyFloat32 halfWidth);

	void InitFromWall(const Vec3f& P, const Vec3f& Q, KyFloat32 upHeight, KyFloat32 downHeight, KyFloat32 halfWidth);

	void Fill(VisualGeometryBuilder& geometryBuilder);

private:
	/* Returns false when P == Q */
	bool CalculateFrontUpRight(const Vec3f& P, const Vec3f& Q, Vec3f& front, Vec3f& up, Vec3f& right);

public:
	Vec3f A, B, C, D, E, F, G, H;
};


}


#endif
