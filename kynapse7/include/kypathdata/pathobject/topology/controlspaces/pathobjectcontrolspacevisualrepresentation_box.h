/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


#ifndef KyPathData_PathObjectControlSpaceVisualRepresentation_Box_H
#define KyPathData_PathObjectControlSpaceVisualRepresentation_Box_H


#include "kypathdata/visualsystem/visualrepresentation.h"

namespace Kaim
{

class DynamicPathObjectControlSpace_Box;

/*
Implements the visual representation of the pathObject ControlSpace.
*/
class PathObjectControlSpaceVisualRepresentation_Box : public VisualRepresentation
{
public:
	PathObjectControlSpaceVisualRepresentation_Box(DynamicPathObjectControlSpace_Box* poControlSpace) : VisualRepresentation(poControlSpace) {}
	virtual ~PathObjectControlSpaceVisualRepresentation_Box() {}
	virtual void BuildGeometry(VisualGeometryBuilder& geometryBuilder);
};


}

#endif // #KyPathData_PathObjectControlSpaceVisualRepresentation_Box_H
