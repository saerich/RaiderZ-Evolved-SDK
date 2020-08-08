/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kypathdata_pathobject */

#ifndef KyPathData_DynamicPathObject_ConnexionSpace_Cylinder_H
#define KyPathData_DynamicPathObject_ConnexionSpace_Cylinder_H


#include "kypathdata/pathobject/pathobjectdatatypes.h"
#include "kypathdata/pathobject/ipathobjectconnexionspace.h"
#include "kypathdata/shapes/verticalcylindervolume.h"

namespace Kaim
{
class PathObjectConnexionSpaceData_Cylinder;

/*! This class represents a PathObject connection space shaped like a cylinder.
	\ingroup kypathdata_pathobject */
class DynamicPathObjectConnexionSpace_Cylinder : public IPathObjectConnexionSpace
{
public:
	/*! \label_constructor If you use this constructor, you must set up the dimensions of the object
		by calling Setup() before you can use this object. */
	DynamicPathObjectConnexionSpace_Cylinder();

	/*! \label_constructor
		\param data				Binary data of this object, typically read from a dump file. */
	DynamicPathObjectConnexionSpace_Cylinder(const PathObjectConnexionSpaceData_Cylinder& data);

	/*! \label_virtualdestructor */
	virtual ~DynamicPathObjectConnexionSpace_Cylinder();

	/*! \label_reinit */
	void Init();

	// Access
	/*! \copydoc IPathObjectConnexionSpace::GetConnexionPosition()
		This implementation returns the value stored in #m_connexionPosition. */
	virtual const Vec3f& GetConnexionPosition() const;

	/*! Retrieves the position at the center of the base of the cylinder. */
	virtual const Vec3f& GetBaseCenter() const;

	/*! Retrieves the height of the cylinder. */
	virtual KyFloat32 GetHeight() const;

	/*! Retrieves the radius of the cylinder. */
	virtual KyFloat32 GetRadius() const;
	virtual const IVolume* GetVolume() const;

	/*! Sets the dimensions of the connection space.
		\param baseCenter		The position at the center of the base of the cylinder.
		\param height			The height of the cylinder.
		\param radius			The radius of the cylinder. */
	void SetUp(const Vec3f& baseCenter, KyFloat32 height, KyFloat32 radius);

public:
	VerticalCylinderVolume m_volume;  /*!< Stores the volume that represents the connexion space. */
	Vec3f m_connexionPosition;        /*!< Stores the position returned by GetConnectionPosition(). Set by Setup() to the center of the base of the cylinder. */
};

}

#endif // #ifndef KyPathData_DynamicPathObject_ConnexionSpace_Cylinder_H
