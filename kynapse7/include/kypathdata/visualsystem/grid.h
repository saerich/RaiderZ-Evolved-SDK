/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*! \file
	\ingroup kypathdata_visualsystem */

#ifndef KyPathData_Grid_H
#define KyPathData_Grid_H


namespace Kaim
{

/*! This class represents an axis-aligned horizontal grid. By default, the grid has 10 squares on each side,
	and each square is 10 meters in length and width.
	\pp The Grid class, in conjunction with the GridVisualRepresentation class that represents its geometry, 
	can help you test your PathData rendering system. See :USERGUIDE:"Rendering PathData".
	\ingroup kypathdata_visualsystem */
class Grid
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	/*! \label_constructor */
	Grid() : m_nbCellsX(10), m_nbCellsY(10), m_cellSizeX(10.f), m_cellSizeY(10.f) {}

	/*! \label_virtualdestructor */
	virtual ~Grid() {}

public:
	KyInt32 m_nbCellsX;    /*!< The number of grid cells along the X axis. */
	KyInt32 m_nbCellsY;    /*!< The number of grid cells along the Y axis. */
	KyFloat32 m_cellSizeX; /*!< The length of each grid cell along the X axis. */
	KyFloat32 m_cellSizeY; /*!< The length of each grid cell along the Y axis. */
};

}

#endif // #KyPathData_Grid_H
