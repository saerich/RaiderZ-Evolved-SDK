/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: GUAL - secondary contact: NOBODY
#ifndef KyPdg_SquarePartitionTypes_H
#define KyPdg_SquarePartitionTypes_H


#include "kypathdata/math/vec2i.h"
#include "kypathdata/math/box2i.h"
#include "kypathdata/navmesh/navmeshtypes.h"


namespace Kaim
{
	typedef KyInt32 Coord;
	typedef Vec2i CoordPos;
	typedef Box2i CoordBox;
	static const KyInt32 InvalidCoord = KyInt32MAXVAL;

	typedef KyInt32 PixelCoord;
	typedef Vec2i PixelPos;
	typedef Box2i PixelBox;
	static const KyInt32 InvalidPixelCoord = KyInt32MAXVAL;

}


#endif
