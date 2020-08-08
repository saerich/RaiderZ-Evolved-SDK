/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/


/*! \file
	\ingroup kypathdata_visualsystem */

// primary contact: GUAL - secondary contact: MAMU
#ifndef KyPathData_IVisualGeometry_H
#define KyPathData_IVisualGeometry_H


#include "kypathdata/generic/memory.h"
#include "kypathdata/math/vec3f.h"
#include "kypathdata/math/box3f.h"
#include "kypathdata/visualsystem/visualcolor.h"


namespace Kaim
{


/*! Represents a single triangle that can be rendered by a class that derives from IVisualGeometry.
	\ingroup kypathdata_visualsystem */
class VisualTriangle
{
public:
	Vec3f A; /*!< The first point in the triangle. */
	Vec3f B; /*!< The second point in the triangle. */
	Vec3f C; /*!< The third point in the triangle. */
	Vec3f m_normal; /*!< The normal of the front face of the triangle. */
	VisualColor m_color; /*!< The color that should be used to render the triangle. */
	KyUInt32 m_firstVertexIdx; /*!< The index of the first vertex of the triangle within the list of vertices passed by the VisualGeometryBuilder to the IVisualGeometry. */
	KyUInt32 m_triangleIdx; /*!< The index of this triangle within the list of triangles passed by the VisualGeometryBuilder to the IVisualGeometry. */
};


/*! The VisualGeometrySetupConfig class is used to configure an object that derives from IVisualGeometry. */
class VisualGeometrySetupConfig
{
public:
	/*! \label_constructor */
	VisualGeometrySetupConfig() : m_verticesCount(0), m_generateNormals(false) {}

public:
	KyUInt32 m_verticesCount; /*!< Sets the maximum number of vertices that need to be treated by the IVisualGeometry. Updated automatically by the VisualGeometryBuilder. */
	bool m_generateNormals; /*!< Indicates whether or not the IVisualGeometry should draw the normals of the faces of the triangles it draws. Updated automatically by the VisualGeometryBuilder. */
};


/*! IVisualGeometry is an abstract base class for an object that accepts triangles from a VisualGeometryBuilder
	(typically pushed to the VisualGeometryBuilder by a class that derives from VisualRepresentation), and that 
	manages the rendering of those triangles in the \gameOrSim engine.
	\pp This class cannot be used as-is. You must write a custom implementation of this class that implements
	the DoBeginTriangles(), DoPushTriangles() and DoEndTriangles() methods.
	\pp See also :USERGUIDE:"Rendering PathData".
	\ingroup kypathdata_visualsystem */
class IVisualGeometry
{
public:
	/*! \label_baseconstructor */
	IVisualGeometry() {}

	/*! Sets up this object according to the specified VisualGeometrySetupConfig. Typically called internally by
		the VisualGeometryBuilder. */
	void Setup(const VisualGeometrySetupConfig& setupConfig) { m_setupConfig = setupConfig; }

	/*! \label_virtualdestructor */
	virtual ~IVisualGeometry();

	/*! Initializes the process of pushing triangles to this object. Transparently calls DoBeginTriangles(). */
	void BeginTriangles(const VisualGeometrySetupConfig& setupConfig)
	{
		m_setupConfig = setupConfig;
		m_bbox.Clear();
		DoBeginTriangles();
	}

	/*! Pushes a single VisualTriangle to this object. Transparently calls DoPushTriangle(). */
	void PushTriangle(const VisualTriangle& triangle)
	{
		m_bbox.ExpandByVec3(triangle.A);
		m_bbox.ExpandByVec3(triangle.B);
		m_bbox.ExpandByVec3(triangle.C);

		DoPushTriangle(triangle);
	}

	/*! Closes the process of pushing triangles to this object. Transparently calls DoEndTriangles(). */
	void EndTriangles()
	{
		DoEndTriangles();
	}

private:
	/*! This method is called by BeginTriangles() when the VisualGeometryBuilder begins the process of passing
		triangles to this object. You can re-implement this method to carry out any initialization steps necessary
		in order to begin the process of managing the triangles that will be passed to DoPushTriangle(). */
	virtual void DoBeginTriangles() = 0;

	/*! This method is called by PushTriangle() each time the VisualGeometryBuilder passes a VisualTriangle
		to this object for rendering. You must re-implement this method to carry out the steps necessary
		in order to manage and render the triangle.
		\pp Note that the coordinates of the triangle are already in the coordinate space of the \gameOrSim engine. */
	virtual void DoPushTriangle(const VisualTriangle& triangle) = 0;

	/*! This method is called by EndTriangles() when the VisualGeometryBuilder is finished the process of passing
		triangles to this object. You can re-implement this method to carry out any steps necessary
		to close the process of managing the triangles passed to DoPushTriangle(). */
	virtual void DoEndTriangles() = 0;

public:
	VisualGeometrySetupConfig m_setupConfig; /*!< Stores the VisualGeometrySetupConfig used to configure this object. */
	Box3f m_bbox; /*!< A bounding box that encloses all geometry passed to this object. */
};


}


#endif
