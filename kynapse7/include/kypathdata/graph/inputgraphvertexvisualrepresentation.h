/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/


#ifndef KyPathData_InputGraphVertexVisualRepresentation_H
#define KyPathData_InputGraphVertexVisualRepresentation_H


#include "kypathdata/visualsystem/visualrepresentation.h"


namespace Kaim
{


class InputGraphVertexVisualRepresentation : public VisualRepresentation
{
public:
	InputGraphVertexVisualRepresentation(Vec3f* position, VisualColor color = VisualColor(0, 0, 255, 255))
		: VisualRepresentation(position), m_color(color) {}

	virtual void BuildGeometry(VisualGeometryBuilder& geometryBuilder);

public:
	VisualColor m_color;
};


}

#endif // KyPathData_InputGraphVertexVisualRepresentation_H
