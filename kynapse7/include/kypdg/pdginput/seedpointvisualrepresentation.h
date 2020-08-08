/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/


#ifndef KyPathData_SeedPointVisualRepresentation_H
#define KyPathData_SeedPointVisualRepresentation_H


#include "kypathdata/visualsystem/visualrepresentation.h"


namespace Kaim
{


class SeedPointVisualRepresentation : public VisualRepresentation
{
public:
	SeedPointVisualRepresentation(Vec3f* position, VisualColor color = VisualColor(0, 255, 0, 255))
		: VisualRepresentation(position), m_color(color) {}

	virtual void BuildGeometry(VisualGeometryBuilder& geometryBuilder);

public:
	VisualColor m_color;
};


}

#endif // KyPathData_SeedPointVisualRepresentation_H
