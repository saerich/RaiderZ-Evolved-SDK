/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kypathdata_pathobject */

#ifndef KyPathData_DynamicPathObject_ConnexionSpace_ExtrudedPolyline_H
#define KyPathData_DynamicPathObject_ConnexionSpace_ExtrudedPolyline_H


#include "kypathdata/pathobject/pathobjectdatatypes.h"
#include "kypathdata/pathobject/ipathobjectconnexionspace.h"
#include "kypathdata/shapes/extrudedpolylinevolume.h"


namespace Kaim
{
class PathObjectConnexionSpaceData_ExtrudedPolyline;

/*! This class represents a PathObject connection space shaped like a two-dimensional polyline extruded
	along the vertical Z axis.
	\ingroup kypathdata_pathobject */
class DynamicPathObjectConnexionSpace_ExtrudedPolyline : public IPathObjectConnexionSpace
{
public:
	/*! \label_constructor If you use this constructor, you must set up the dimensions of the object
		by calling Setup() before you can use this object. */
	DynamicPathObjectConnexionSpace_ExtrudedPolyline();

	/*! \label_constructor
		\param data				Binary data of this object, typically read from a dump file. */
	DynamicPathObjectConnexionSpace_ExtrudedPolyline(const PathObjectConnexionSpaceData_ExtrudedPolyline& data);

	/*! \label_virtualdestructor */
	virtual ~DynamicPathObjectConnexionSpace_ExtrudedPolyline();

	/*! \label_reinit */
	void Init();

	// Access
	virtual const IVolume* GetVolume() const;

	/*! \copydoc IPathObjectConnexionSpace::GetConnexionPosition()
		This implementation returns the value stored in #m_connexionPosition. */
	virtual const Vec3f& GetConnexionPosition() const;

	/*! Sets the dimensions of the connection space.
		\param polylinePoints			An array of points that represent the outline of the connection space.
		\param pointCount				The number of points in the \c polylinePoints array.
		\param minAltitude				The altitude of the bottom face of the volume.
		\param maxAltitude				The altitude of the top face of the volume.
		\param connexionPosition		The position that will be returned by calls to GetConnectionPosition(). */
	void SetUp(const Vec3f* polylinePoints, KyUInt32 pointCount, KyFloat32 minAltitude, KyFloat32 maxAltitude, const Vec3f& connexionPosition);

public:
	ExtrudedPolylineVolume m_volume;  /*!< Stores the volume that represents the connexion space. */
	Vec3f m_connexionPosition;        /*!< Stores the position returned by GetConnectionPosition(). Set by Setup() to the connection position specified in its \c connexionPosition parameter. */
};

}

#endif // #ifndef KyPathData_DynamicPathObject_ConnexionSpace_ExtrudedPolyline_H
