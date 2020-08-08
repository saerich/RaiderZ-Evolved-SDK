/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: LASI - secondary contact: GUAL
#ifndef KyPdg_NavRasterVisualRepresentation_H
#define KyPdg_NavRasterVisualRepresentation_H


#include "kypathdata/visualsystem/visualrepresentation.h"
#include "navrastercell.h"


namespace Kaim
{

class VisualGeometryBuilder;
class NavRasterCell;

class NavRasterCellVisualRepresentation : public VisualRepresentation
{
public:
	NavRasterCellVisualRepresentation(NavRasterCell* navRasterCell) : VisualRepresentation(navRasterCell) {}

	virtual void BuildGeometry(VisualGeometryBuilder& geometryBuilder);

private:
	const NavRasterColumnElement* GetNeighbor(const NavRasterColumnElement& pixel, const PixelPos& pos, KyInt32 dir);
};


}


#endif
