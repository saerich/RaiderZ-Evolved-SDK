/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/


/*! \file
	\ingroup kypathdata_visualsystem */

// primary contact: MAMU - secondary contact: 
#ifndef KyPathData_ColorPalette_H
#define KyPathData_ColorPalette_H

#include "kypathdata/generic/memory.h"
#include "kypathdata/visualsystem/visualcolor.h"
#include "kypathdata/containers/array.h"

namespace Kaim
{

/*! Represents a palette of colors.
	\pp Each instance of this class stores a configurable array of VisualColor objects, and provides access to those
	colors from their indices within the array.
	\pp Use of this class is entirely optional. It is provided for your convenience in writing your own visual
	representations for \SDKName data.
	\ingroup kypathdata_visualsystem */
class ColorPalette : public VirtualMemObject
{

public:
	/*! \label_constructor
		\param maxColors			Sets the initial size of the #m_colors array.
		\param defaultColor			Sets the value of #m_defaultColor. */
	ColorPalette(KyUInt32 maxColors, VisualColor defaultColor = VisualColor::White);

	/*! \label_virtualdestructor */
	virtual ~ColorPalette();

	/*! \label_clear */
	void Clear();

	/*! \label_reinit
		\param maxColors			Sets the initial size of the #m_colors array.
		\param defaultColor			Sets the value of #m_defaultColor. */
	virtual void Init(KyUInt32 maxColors, VisualColor defaultColor = VisualColor::White);

	/*! Returns the color at the specified index within #m_colors.
		If the index is out of bounds, the default color is returned. */
	virtual VisualColor GetColor(KyUInt32 index) const;

	/*! Sets the color at the specified index within #m_colors. Returns #KY_ERROR if the specified index 
		is out of bounds. */
	KyResult RecordColor(KyUInt32 index, VisualColor color);

public:
	Array<VisualColor> m_colors;      /*!< Stores the list of colors in this palette. */
	VisualColor m_defaultColor;       /*!< Stores the default color for this palette, returned by GetColor() if there is an error or if no color is registered for the specified index. */
};




/*! Defines an abstract base interface for an object that provides access to a palette of colors based on a terrain type mask.
	\pp This class cannot be used directly. You must use a class that derives from ITerrainTypeColorPalette and
	that implements its virtual methods. You can use the TerrainTypeColorPalette class supplied with \SDKName, or
	a custom class that you write yourself.
	\pp This class is used internally to support the visual PathData representations supplied with \SDKName. 
	Use of this class in your own code is entirely optional.
	\ingroup kypathdata_visualsystem */
class ITerrainTypeColorPalette : public VirtualMemObject
{
public:
	/*! \label_virtualdestructor */
	virtual ~ITerrainTypeColorPalette() {}

	/*! Returns a color that can be used to represent the specified terrain type mask. */
	virtual VisualColor GetTerrainTypeColor(KyUInt32 terrainType) const = 0;

	/*! Returns a color that can be used to represent the non-walkable terrain type (#s_nonWalkable_terrainType). */
	virtual VisualColor GetNonWalkableTerrainTypeColor() const = 0;

	/*! Returns a color that can be used to represent the default terrain type (#s_allWalkable_terrainType). */
	virtual VisualColor GetAllWalkableTerrainTypeColor() const = 0;
};




/*! This class is used by the visual PathData representations supplied with \SDKName in order to 
	retrieve the colors that should be used to represent different terrain types.
	\pp By default this palette contains 32 different colors, one for each byte of the terrain type mask.
	\pp This class is used internally to support the visual PathData representations supplied with \SDKName. 
	Use of this class in your own code is entirely optional.
	\ingroup kypathdata_visualsystem */
class TerrainTypeColorPalette : public ITerrainTypeColorPalette
{
public:
	/*! \label_constructor
		\param nonWalkableColor			Sets the value of #m_nonWalkableColor.
		\param allWalkableColor			Sets the value of #m_allWalkableColor.
		\param saturation				Sets the value of #m_saturation in range [0, 1] (0 = white, 1 = fully saturated colors).
										Use this value to make different versions of the default color palette with same color tone but different aspect.
										Aspect can be washed aspect (low saturation) to very colorful (high saturation).
										Default value is 1.0f (Colorful palette).
		\param luminance				Sets the value of #m_luminance in range [0, 1] (0 = dark, 1 = sunny).
										Use this value to make different versions of the default color palette with same color tone but different luminance.
										Default value is 1.0f (Sunny palette).*/
	TerrainTypeColorPalette(VisualColor nonWalkableColor = VisualColor::Red, VisualColor allWalkableColor = VisualColor::Blue, KyFloat32 saturation = 1.0f, KyFloat32 luminance = 1.0f);

	/*! \label_virtualdestructor */
	virtual ~TerrainTypeColorPalette() {}

	/*! @copydoc ITerrainTypeColorPalette::GetTerrainTypeColor()
		\pp This implementation returns:
		-	A different color for each terrain type. If the \c terrainType is
			a mask of two or more terrain types, the colors are mixed.
		-	#m_allWalkableColor if the \c terrainType is 0xFFFFFFFF (#s_allWalkable_terrainType).
		-	#m_nonWalkableColor if the \c terrainType is 0x00000000 (#s_nonWalkable_terrainType). */
	virtual VisualColor GetTerrainTypeColor(KyUInt32 terrainType) const;

	/*! @copydoc ITerrainTypeColorPalette::GetNonWalkableTerrainTypeColor()
		\pp This implementation returns #m_nonWalkableColor. */
	virtual VisualColor GetNonWalkableTerrainTypeColor() const { return m_nonWalkableColor; }

	/*! @copydoc ITerrainTypeColorPalette::GetAllWalkableTerrainTypeColor()
		\pp This implementation returns #m_allWalkableColor. */
	virtual VisualColor GetAllWalkableTerrainTypeColor() const { return m_allWalkableColor; }

public:
	/*! Fills this palette with a predefined set of colors. */
	void SetDefaultColors();

	/*! Fills all elements in this palette with the specified color. */
	void SetAllColorsTheSame(VisualColor color);

public:
	VisualColor m_nonWalkableColor; /*!< Stores the color that will be used for the non-walkable terrain type (#s_nonWalkable_terrainType). */
	VisualColor m_allWalkableColor; /*!< Stores the color that will be used for the default terrain type (#s_allWalkable_terrainType). */
	KyFloat32 m_saturation;         /*!< Stores the saturation of the default color palette. Value is defined from 0 (not saturated = all white) to 1.0f (saturated colors). */
	KyFloat32 m_luminance;          /*!< Stores the luminance of the default color palette. Value is defined from 0 (dark) to 1.0f (sunny). */
	VisualColor m_colors[32]; /*!< Stores the list of colors in this palette. */
};

}

#endif
