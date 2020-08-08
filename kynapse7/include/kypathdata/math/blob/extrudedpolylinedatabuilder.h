/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: LASI - secondary contact: NOBODY

#ifndef KY_EXTRUDEDPOLYLINEDATABUILDER_H
#define KY_EXTRUDEDPOLYLINEDATABUILDER_H

#include "kypathdata/blob/baseblobbuilder.h"

namespace Kaim
{

class ExtrudedPolylineData;

class ExtrudedPolylineDataBuilder : public BaseBlobBuilder<ExtrudedPolylineData>
{
public:
	ExtrudedPolylineDataBuilder(const Vec3f* polylinePoints, const KyUInt32 nbPoints, const KyFloat32 altMin, const KyFloat32 altMax)
	{
		m_altMin = altMin;
		m_altMax = altMax;
		m_polylinePoints = polylinePoints;
		m_polylinePointsSize = nbPoints;
	}
	~ExtrudedPolylineDataBuilder() {}

private:
	virtual void DoBuild();

	const Vec3f* m_polylinePoints;
	KyUInt32 m_polylinePointsSize;
	KyFloat32 m_altMin;
	KyFloat32 m_altMax;
};

} //namespace Kaim

#endif // KY_EXTRUDEDPOLYLINEDATABUILDER_H
