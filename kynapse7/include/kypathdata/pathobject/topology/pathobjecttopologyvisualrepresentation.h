/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


#ifndef KyPathData_PathObjectTopologyVisualRepresentation_H
#define KyPathData_PathObjectTopologyVisualRepresentation_H


#include "kypathdata/visualsystem/visualrepresentation.h"

namespace Kaim
{

class IPathObjectTopology;

/*
Implements the visual representation of the pathObject topology.
*/
class PathObjectTopologyVisualRepresentation : public VisualRepresentation
{
public:
	PathObjectTopologyVisualRepresentation(IPathObjectTopology* poTopology) : VisualRepresentation(poTopology) {}
	virtual ~PathObjectTopologyVisualRepresentation() {}
	virtual void BuildGeometry(VisualGeometryBuilder& geometryBuilder);
};


}

#endif // #KyPathData_PathObjectTopologyVisualRepresentation_H
