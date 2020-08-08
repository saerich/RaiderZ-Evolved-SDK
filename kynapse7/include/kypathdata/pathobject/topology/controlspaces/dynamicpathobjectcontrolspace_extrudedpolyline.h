/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kypathdata_pathobject */

#ifndef KyPathData_DynamicPathObject_ControlSpace_ExtrudedPolyline_H
#define KyPathData_DynamicPathObject_ControlSpace_ExtrudedPolyline_H


#include "kypathdata/pathobject/pathobjectdatatypes.h"
#include "kypathdata/pathobject/topology/controlspaces/dynamicpathobjectcontrolspace.h"
#include "kypathdata/shapes/extrudedpolylinevolume.h"


namespace Kaim
{
class PathObjectControlSpaceData_ExtrudedPolyline;

/*! This class represents a PathObject control space shaped like a two-dimensional polyline extruded
	along the vertical Z axis.
	\ingroup kypathdata_pathobject */
class DynamicPathObjectControlSpace_ExtrudedPolyline : public DynamicPathObjectControlSpace
{
public:
	/*! \label_constructor If you use this constructor, you must set up the dimensions and behavior
		of the object by calling Setup() before you can use this object. */
	DynamicPathObjectControlSpace_ExtrudedPolyline();

	/*! \label_constructor
		\param data				Binary data of this object, typically read from a dump file. */
	DynamicPathObjectControlSpace_ExtrudedPolyline(const PathObjectControlSpaceData_ExtrudedPolyline& data);

	/*! \label_virtualdestructor */
	virtual ~DynamicPathObjectControlSpace_ExtrudedPolyline();

	virtual const IVolume* GetVolume() const;

	/*! Read accessor for #m_extrudedPolylineVolume. */
	const ExtrudedPolyline& GetExtrudedPolyline();

	/*! Sets the dimensions and behavior of the control space.
		\param polylinePoints							An array of points that represent the outline of the connection space.
		\param pointCount								The number of points in the \c polylinePoints array.
		\param minAltitude								The altitude of the bottom face of the volume.
		\param maxAltitude								The altitude of the top face of the volume.
		\param canLockEdges								Sets the value that will be returned by CanLockEdges().
		\param isAnObstacle								Sets the value that will be returned by IsAnObstacle().
		\param isASpaceToStartUsingThePathObject		Sets the value that will be returned by IsASpaceToStartUsingThePathObject(). */
	void SetUp(
		const Vec3f* polylinePoints,
		KyUInt32 pointCount,
		KyFloat32 minAltitude,
		KyFloat32 maxAltitude,
		bool canLockEdges,
		bool isAnObstacle,
		bool isASpaceToStartUsingThePathObject);

public:
	ExtrudedPolylineVolume m_extrudedPolylineVolume; /*!< Stores the extruded polyline that represents this control space. */
};

}

#endif // #ifndef KyPathData_DynamicPathObject_ControlSpace_ExtrudedPolyline_H
