/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kypathdata_pathobject */

#ifndef KyPathData_DynamicPathObject_ConnexionSpace_ExtrudedRectangle_H
#define KyPathData_DynamicPathObject_ConnexionSpace_ExtrudedRectangle_H


#include "kypathdata/pathobject/pathobjectdatatypes.h"
#include "kypathdata/pathobject/ipathobjectconnexionspace.h"
#include "kypathdata/shapes/extrudedrectanglevolume.h"


namespace Kaim
{
class PathObjectConnexionSpaceData_ExtrudedRectangle;

/*! This class represents a PathObject connection space shaped like a two-dimensional rectangle extruded
	along the vertical Z axis.
	\ingroup kypathdata_pathobject */
class DynamicPathObjectConnexionSpace_ExtrudedRectangle : public IPathObjectConnexionSpace
{
public:
	/*! \label_constructor If you use this constructor, you must set up the dimensions of the object
		by calling Setup() before you can use this object. */
	DynamicPathObjectConnexionSpace_ExtrudedRectangle();

	/*! \label_constructor
		\param data				Binary data of this object, typically read from a dump file. */
	DynamicPathObjectConnexionSpace_ExtrudedRectangle(const PathObjectConnexionSpaceData_ExtrudedRectangle& data);

	/*! \label_virtualdestructor */
	virtual ~DynamicPathObjectConnexionSpace_ExtrudedRectangle();

	/*! \label_reinit */
	void Init();

	virtual const IVolume* GetVolume() const;

	/*! \copydoc IPathObjectConnexionSpace::GetConnexionPosition()
		This implementation returns the value stored in #m_connexionPosition. */
	virtual const Vec3f& GetConnexionPosition() const;

	/*! Sets the dimensions of the connection space.
		\param baseCenter		The center of the bottom face of the volume.
		\param mainAxis			The orientation of one axis of the rectangle along the (X,Y) plane.
		\param length			The extent of the rectangle in the direction specified by \c mainAxis.
		\param width			The extent of the rectangle in the direction orthogonal to \c mainAxis.
		\param height			The height of the volume. */
		void SetUp(
		const Vec3f& baseCenter,
		Vec3f mainAxis,
		const KyFloat32 length,
		const KyFloat32 width,
		const KyFloat32 height);

public:
	ExtrudedRectangleVolume m_volume;  /*!< Stores the volume that represents the connexion space. */
	Vec3f m_connexionPosition;         /*!< Stores the position returned by GetConnectionPosition(). Set by Setup() to the center of the bottom face of the volume. */
};

}

#endif // #ifndef KyPathData_DynamicPathObject_ConnexionSpace_ExtrudedRectangle_H
