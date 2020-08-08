/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kypathdata_pathobject */

#ifndef KyPathData_DynamicPathObject_ControlSpace_ExtrudedRect_H
#define KyPathData_DynamicPathObject_ControlSpace_ExtrudedRect_H


#include "kypathdata/pathobject/pathobjectdatatypes.h"
#include "kypathdata/pathobject/topology/controlspaces/dynamicpathobjectcontrolspace.h"
#include "kypathdata/shapes/extrudedrectanglevolume.h"


namespace Kaim
{
class PathObjectControlSpaceData_ExtrudedRectangle;

/*! This class represents a PathObject control space shaped like a two-dimensional rectangle extruded
	along the vertical Z axis.
	\ingroup kypathdata_pathobject */
class DynamicPathObjectControlSpace_ExtrudedRectangle : public DynamicPathObjectControlSpace
{
public:
	/*! \label_constructor If you use this constructor, you must set up the dimensions and behavior
		of the object by calling Setup() before you can use this object. */
	DynamicPathObjectControlSpace_ExtrudedRectangle();

	/*! \label_constructor
		\param data				Binary data of this object, typically read from a dump file. */
	DynamicPathObjectControlSpace_ExtrudedRectangle(const PathObjectControlSpaceData_ExtrudedRectangle& data);

	/*! \label_virtualdestructor */
	virtual ~DynamicPathObjectControlSpace_ExtrudedRectangle();

	virtual const IVolume* GetVolume() const;

	/*! Read accessor for #m_extrudedRectangleVolume. */
	const ExtrudedRectangle& GetExtrudedRectangle();

	/*! Sets the dimensions and behavior of the control space.
		\param baseCenter								The center of the bottom face of the volume.
		\param mainAxis									The orientation of one axis of the rectangle along the (X,Y) plane.
		\param length									The extent of the rectangle in the direction specified by \c mainAxis.
		\param width									The extent of the rectangle in the direction orthogonal to \c mainAxis.
		\param height									The height of the volume.
		\param canLockEdges								Sets the value that will be returned by CanLockEdges().
		\param isAnObstacle								Sets the value that will be returned by IsAnObstacle().
		\param isASpaceToStartUsingThePathObject		Sets the value that will be returned by IsASpaceToStartUsingThePathObject(). */
	void SetUp(
		const Vec3f& baseCenter,
		Vec3f mainAxis,
		const KyFloat32 length,
		const KyFloat32 width,
		const KyFloat32 height,
		bool canLockEdges,
		bool isAnObstacle,
		bool isASpaceToStartUsingThePathObject);

public:
	ExtrudedRectangleVolume m_extrudedRectangleVolume; /*!< Stores the extruded rectangle that represents this control space. */
};

}

#endif // #ifndef KyPathData_DynamicPathObject_ControlSpace_ExtrudedRect_H
