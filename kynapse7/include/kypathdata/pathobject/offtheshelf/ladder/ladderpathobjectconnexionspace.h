/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: MAMU - secondary contact: None
#ifndef KYPATHDATA_LADDER_PATHOBJECT_CONNEXIONSPACE_H
#define KYPATHDATA_LADDER_PATHOBJECT_CONNEXIONSPACE_H

#include "kypathdata/pathobject/topology/connexionspaces/dynamicpathobjectconnexionspace_cylinder.h"
#include "kypathdata/shapes/draw.h"

namespace Kaim
{

/*	
*/
class LadderPathObjectConnexionSpace : public DynamicPathObjectConnexionSpace_Cylinder
{
public:
	LadderPathObjectConnexionSpace() : DynamicPathObjectConnexionSpace_Cylinder() {}

	LadderPathObjectConnexionSpace(const Vec3f& grabPosition, const Vec3f& grabOrientation) :
	 DynamicPathObjectConnexionSpace_Cylinder(), m_grabPosition(grabPosition), m_grabOrientation(grabOrientation) {}
	
public:
	void SetUp(
		const Vec3f& baseCenter,
		KyFloat32 height,
		KyFloat32 radius,
		const Vec3f& grabPosition,
		const Vec3f& grabOrientation)
	{
		DynamicPathObjectConnexionSpace_Cylinder::SetUp(baseCenter, height, radius);
		m_grabPosition = grabPosition;
		m_grabOrientation = grabOrientation;
	}

	virtual void Render(KyUInt32 renderFlags) const
	{
		DynamicPathObjectConnexionSpace_Cylinder::Render(renderFlags);
		Draw3DPoint(m_grabPosition, 200, 200, 0);
	}

public:
	Vec3f m_grabPosition;     /*!< The position where the entity must be to grab the ladder, coming from the current connexion space.*/
	Vec3f m_grabOrientation;  /*!< The orientation the entity must have to grab the ladder, coming from the current connexion space.*/
};

}

#endif //KYPATHDATA_LADDER_PATHOBJECT_CONNEXIONSPACE_H
