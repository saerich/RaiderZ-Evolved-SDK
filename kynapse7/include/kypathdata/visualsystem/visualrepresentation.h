/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/


/*! \file
	\ingroup kypathdata_visualsystem */

// primary contact: GUAL - secondary contact: MAMU
#ifndef KyPathData_VisualRepresentation_H
#define KyPathData_VisualRepresentation_H


#include "kypathdata/generic/memory.h"
#include <kypathdata/containers/array.h>
#include <kypathdata/visualsystem/ivisualgeometry.h>


namespace Kaim
{

class VisualSystem;
class VisualGeometryBuilder;
class VisualGeometry;
class CoordSystem;


/*! Enumerates the possible levels of detail that can be set for a VisualRepresentation. Not all classes of
	VisualRepresentation support different levels of detail; whether or not this is supported and how it is
	supported depends on the implementation of VisualRepresentation::BuildGeometry() in each derived class.
	\ingroup kypathdata_visualsystem*/
enum VisualRepresentationDetailLevel { KY_VISUAL_DETAIL_LOW, /*!< Identifies a lower level of detail. */
										KY_VISUAL_DETAIL_HIGH /*!< Identifies a higher level of detail. */
										};


/*! VisualRepresentation is an abstract base class that expresses how an object or a set of data should be
	represented using the primitives exposed by the VisualGeometryBuilder, in order for the object or data
	to be rendered by an object that derives from IVisualGeometry.
	\pp This class cannot be used as-is. You must use a class that derives from VisualRepresentation, and that
	implements the virtual BuildGeometry() method to render a specific type of object or data. 
	\pp For example, specialized implementations of VisualRepresentation are included in the \SDKName SDK for most
	of the different classes of PathData and internal data involved in the PathData generation system. For 
	example, the NavCellVisualRepresentation class is an implementation of VisualRepresentation that pushes a
	triangle to the VisualGeometryBuilder to represent each triangle in its NavCell.
	\pp You can use the supplied classes of VisualRepresentation as-is. If you want to change the way a certain
	kind of data is represented, you can write your own class of VisualRepresentation specialized for the kind
	of data you want to render.
	\ingroup kypathdata_visualsystem */
class VisualRepresentation : public VirtualMemObject
{
public:
	/*! \label_baseconstructor
		\param representedData		The object or data that should be represented by the VisualRepresentation.
		\param geometry				An object that derives from IVisualGeometry that will be used to render the represented data. */
	VisualRepresentation(const void* representedData = KY_NULL, IVisualGeometry* geometry = KY_NULL) :
		m_representedData(representedData), m_geometry(geometry), m_detailLevel(KY_VISUAL_DETAIL_LOW)
	{}

	/*! \label_destructor */
	virtual ~VisualRepresentation();

	/*! Transparently calls BuildGeometry() to push the geometry for the object or data represented by this object to the
		IVisualGeometry object maintained in #m_geometry. */
	void ComputeGeometry() { ComputeGeometry(m_geometry); }

	/*! Transparently calls BuildGeometry() to push the geometry for the object or data represented by this object to the
		IVisualGeometry object provided in the \c geometry parameter. */
	void ComputeGeometry(IVisualGeometry* geometry);

	/*! Retrieves the bounding box around the geometry contained in #m_geometry, if any.
		\param[out] bbox			Updated to store the bounding box around the geometry contained in #m_geometry.
		\return \c true if #m_geometry has been set, or \c false otherwise. */
	bool GetBBox(Box3f& bbox) const
	{
		if (m_geometry == KY_NULL)
			return false;

		bbox = m_geometry->m_bbox;
		return true;
	}

public:
	/*! This virtual method must be implemented by every class that derives from VisualGeometry. It must take the data
		or object to be represented (stored in #m_representedData), and push a set of geometrical shapes to the VisualGeometryBuilder
		provided in the \c geometryBuilder object using the methods of the VisualGeometryBuilder class.
		\pp This method is typically invoked transparently through a call to ComputeGeometry(). However, you can call this
		method directly if you want multiple different VisualRepresentation objects to push their geometry to the same
		VisualGeometryBuilder. If you do so, you must call IVisualGeometry::BeginTriangles() once to start off the process. Then call
		BuildGeometry() once for each VisualRepresentation to set up the total number of triangles that will be
		required by the IVisualGeometry. Call VisualGeometryBuilder::SwitchToFillMode() to switch from count mode to fill mode,
		then call BuildGeometry() again for each VisualRepresentation in order to actually push the triangles. Finally, close the
		process by calling IVisualGeometry::EndTriangles(). */
	virtual void BuildGeometry(VisualGeometryBuilder& geometryBuilder) = 0;

public:
	const void* m_representedData; /*!< Stores the object or data that is represented by this instance of VisualRepresentation. */
	IVisualGeometry* m_geometry; /*!< Stores an object derived from IVisualGeometry that will render the geometry constructed by this instance of VisualRepresentation. */
	VisualRepresentationDetailLevel m_detailLevel; /*!< Indicates the level of detail that should be drawn. */
};


} // namespace Kaim


#endif
