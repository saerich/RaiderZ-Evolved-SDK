/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/


#ifndef KyPathData_PathObjectConnexionSpaceVisualRepresentation_H
#define KyPathData_PathObjectConnexionSpaceVisualRepresentation_H


#include "kypathdata/visualsystem/visualrepresentation.h"

namespace Kaim
{

class IPathObjectConnexionSpace;

/*
Implements the visual representation of the pathObject connexionSpace.
*/
class PathObjectConnexionSpaceVisualRepresentation : public VisualRepresentation
{
public:
	PathObjectConnexionSpaceVisualRepresentation(IPathObjectConnexionSpace* poConnexionSpace) : VisualRepresentation(poConnexionSpace) {}
	virtual ~PathObjectConnexionSpaceVisualRepresentation() {}
	virtual void BuildGeometry(VisualGeometryBuilder& geometryBuilder);
};


}

#endif // #KyPathData_PathObjectConnexionSpaceVisualRepresentation_H
