/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kypathdata_pathobject */

#ifndef KyPathData_DynamicPathObject_ControlSpace_Box_H
#define KyPathData_DynamicPathObject_ControlSpace_Box_H


#include "kypathdata/pathobject/pathobjectdatatypes.h"
#include "kypathdata/pathobject/topology/controlspaces/dynamicpathobjectcontrolspace.h"

#include "kypathdata/shapes/axisalignedboxvolume.h"


namespace Kaim
{
class PathObjectControlSpaceData_Box;

/*! This class represents a PathObject control space shaped like an axis-aligned bounding box.
	\ingroup kypathdata_pathobject */
class DynamicPathObjectControlSpace_Box : public DynamicPathObjectControlSpace
{
public:
	/*! \label_constructor If you use this constructor, you must set up the dimensions and behavior
		of the object by calling Setup() before you can use this object. */
	DynamicPathObjectControlSpace_Box();

	/*! \label_constructor
		\param data				Binary data of this object, typically read from a dump file. */
	DynamicPathObjectControlSpace_Box(const PathObjectControlSpaceData_Box& data);

	/*! \label_virtualdestructor */
	virtual ~DynamicPathObjectControlSpace_Box();

	virtual const IVolume* GetVolume() const;

	/*! Read accessor for #m_aabox. */
	const AxisAlignedBox& GetAxisAlignedBox();

	/*! Sets the dimensions and behavior of the control space.
		\param min										The minima of the bounding box: i.e. the corner of the box with the lowest values on the
														X, Y and Z axes.
		\param max										The maxima of the bounding box: i.e. the corner of the box with the highest values on the
														X, Y and Z axes.
		\param canLockEdges								Sets the value that will be returned by CanLockEdges().
		\param isAnObstacle								Sets the value that will be returned by IsAnObstacle().
		\param isASpaceToStartUsingThePathObject		Sets the value that will be returned by IsASpaceToStartUsingThePathObject(). */
	void SetUp(const Vec3f& min, const Vec3f& max, bool canLockEdges, bool isAnObstacle, bool isASpaceToStartUsingThePathObject);

public:
	AxisAlignedBoxVolume m_aabox; /*!< Stores the axis-aligned bounding box that represents this control space. */
};

}

#endif // #ifndef KyPathData_DynamicPathObject_ControlSpace_Box_H
