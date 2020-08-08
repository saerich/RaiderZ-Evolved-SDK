/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*! \file
	\ingroup kypathdata_visualsystem */

#ifndef KyPathData_GridVisualRepresentation_H
#define KyPathData_GridVisualRepresentation_H


#include "kypathdata/generic/memory.h"
#include "kypathdata/visualsystem/visualrepresentation.h"

namespace Kaim
{

class Grid;

/*! This implementation of VisualRepresentation pushes geometry that represents a Grid object.
	\pp The grid is drawn using gray and dark gray segments.
	\pp This is the same grid visible in the standalone Kynapse Lab tool. You can also use this class yourself 
	to test your PathData rendering system. 
	\ingroup kypathdata_visualsystem */
class GridVisualRepresentation : public VisualRepresentation
{
public:
	/*! \label_constructor
		\param grid				The Grid instance whose geometry this object will construct. */
	GridVisualRepresentation(Grid* grid) : VisualRepresentation(grid) {}

	/*! \label_virtualdestructor */
	virtual ~GridVisualRepresentation() {}

	virtual void BuildGeometry(VisualGeometryBuilder& geometryBuilder);
};


}

#endif // #KyPathData_GridVisualRepresentation_H
