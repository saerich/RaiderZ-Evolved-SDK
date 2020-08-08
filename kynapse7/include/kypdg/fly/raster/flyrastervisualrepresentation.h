/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/


// primary contact: BEHE - secondary contact: NONE
#ifndef KyFlyPdg_FlyRasterVisualRepresentation_H
#define KyFlyPdg_FlyRasterVisualRepresentation_H

#include "kypathdata/visualsystem/visualrepresentation.h"


namespace Kaim
{

class VisualGeometryBuilder;
class FlyRasterBlob;

class FlyRasterVisualRepresentation : public VisualRepresentation
{
public:
	FlyRasterVisualRepresentation(FlyRasterBlob* flyRasterBlob) : VisualRepresentation(flyRasterBlob) {}

	virtual void BuildGeometry(VisualGeometryBuilder& geometryBuilder);
};


} // namespace Kaim


#endif
