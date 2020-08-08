/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/


#ifndef KyPathData_PathObjectConnexionSpaceVisualRepresentation_Cylinder_H
#define KyPathData_PathObjectConnexionSpaceVisualRepresentation_Cylinder_H


#include "kypathdata/visualsystem/visualrepresentation.h"

namespace Kaim
{

class DynamicPathObjectConnexionSpace_Cylinder;

/* 
Implements the visual representation of the pathObject connexionSpace.
*/
class PathObjectConnexionSpaceVisualRepresentation_Cylinder : public VisualRepresentation
{
public:
	PathObjectConnexionSpaceVisualRepresentation_Cylinder(DynamicPathObjectConnexionSpace_Cylinder* poConnexionSpace) : VisualRepresentation(poConnexionSpace) {}
	virtual ~PathObjectConnexionSpaceVisualRepresentation_Cylinder() {}
	virtual void BuildGeometry(VisualGeometryBuilder& geometryBuilder);
};


}

#endif // #KyPathData_PathObjectConnexionSpaceVisualRepresentation_Cylinder_H
