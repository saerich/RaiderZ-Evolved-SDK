/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*! \file
	\ingroup kypathdata_visualsystem */

#ifndef KyPathData_AxisFrameVisualRepresentation_H
#define KyPathData_AxisFrameVisualRepresentation_H


#include "kypathdata/generic/memory.h"
#include "kypathdata/visualsystem/visualrepresentation.h"

namespace Kaim
{

class AxisFrame;

/*! This implementation of VisualRepresentation pushes geometry that represents an AxisFrame object.
	You can use this class to test your PathData rendering system.
	-	The right (X) axis is drawn using a red arrow.
	-	The front (Y) axis is drawn using a green arrow.
	-	The up (Z) axis is drawn using a blue arrow.
	\pp This is the same axis system visible in the standalone Kynapse Lab tool. You can also use this class yourself 
	to test your PathData rendering system. 
	\ingroup kypathdata_visualsystem */
class AxisFrameVisualRepresentation : public VisualRepresentation
{
public:
	/*! \label_constructor
		\param axisFrame			The AxisFrame instance whose geometry this object will construct. */
	AxisFrameVisualRepresentation(AxisFrame* axisFrame) : VisualRepresentation(axisFrame) {}

	/*! \label_virtualdestructor */
	virtual ~AxisFrameVisualRepresentation() {}

	virtual void BuildGeometry(VisualGeometryBuilder& geometryBuilder);
};


}

#endif // #KyPathData_AxisFrameVisualRepresentation_H
