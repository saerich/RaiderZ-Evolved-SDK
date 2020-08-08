/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*! \file
	\ingroup kypathdata_visualsystem */

#ifndef KyPathData_AxisFrame_H
#define KyPathData_AxisFrame_H


namespace Kaim
{

/*! This class represents a set of three-dimensional axes. By default, each axis line is 20 meters in length.
	\pp The AxisFrame class, in conjunction with the AxisFrameVisualRepresentation class that represents its geometry, 
	can help you test your PathData rendering system. See :USERGUIDE:"Rendering PathData".
	\ingroup kypathdata_visualsystem */
class AxisFrame
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	/*! \label_constructor */
	AxisFrame() : m_size(20.f) {}

	/*! \label_virtualdestructor */
	virtual ~AxisFrame() {}

public:
	KyFloat32 m_size; /*!< The length of each axis line. */
};

}

#endif // #KyPathData_AxisFrame_H
