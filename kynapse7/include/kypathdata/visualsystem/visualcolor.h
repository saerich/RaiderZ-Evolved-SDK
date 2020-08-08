/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/


/*! \file
	\ingroup kypathdata_visualsystem */

// primary contact: GUAL - secondary contact: MAMU
#ifndef KyPathData_VisualColor_H
#define KyPathData_VisualColor_H

#include "kypathdata/generic/memory.h"

namespace Kaim
{

/*! Represents a single RGBA color.
	\ingroup kypathdata_visualsystem */
class VisualColor
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	/*! \label_constructor Sets the color to opaque black. */
	VisualColor() : m_r(0), m_g(0), m_b(0), m_a(255) {}

	/*! \label_constructor
		\param r			\label_redval
		\param g			\label_greenval
		\param b			\label_blueval
		\param a			\label_alphaval */
	VisualColor(KyUInt8 r, KyUInt8 g, KyUInt8 b, KyUInt8 a = 255 ) { Set(r, g, b, a); }

	/*! \label_constructor Copies the state of the specified VisualColor to the newly created object. */
	VisualColor(const VisualColor& color) { Set(color.m_r, color.m_g, color.m_b, color.m_a); }

	/*! \label_constructor Copies the RGB values of the specified VisualColor to the newly created object, and sets
		the alpha value to the \c a parameter. */
	VisualColor(const VisualColor& color, KyFloat32 a) { Set(color.m_r, color.m_g, color.m_b, (KyUInt8)(a * 255.0f)); }

	/*! Returns #KY_TRUE if the RGBA values of the specified VisualColor are the same as for this object. */
	bool operator == (const VisualColor other) const
	{
		return m_r == other.m_r && m_g == other.m_g && m_b == other.m_b && m_a == other.m_a;
	}

	/*! Returns #KY_TRUE if at least one of the RGBA values of the specified VisualColor is different than for this object. */
	bool operator != (const VisualColor& other) const
	{
		return !(operator ==(other));
	}

	/*! Mix 2 colors. */
	VisualColor operator + (const VisualColor& other);

	/*! Mix with another colors. */
	void operator += (const VisualColor& other);

	/*! Sets the color value of this object.
		\param r			\label_redval
		\param g			\label_greenval
		\param b			\label_blueval
		\param a			\label_alphaval */
	void Set(KyUInt8 r, KyUInt8 g, KyUInt8 b, KyUInt8 a = 255)
	{
		m_r = r;
		m_g = g;
		m_b = b;
		m_a = a;
	}

	/*! Sets the color value of this object.
		\param r			\label_redval
		\param g			\label_greenval
		\param b			\label_blueval
		\param a			\label_alphaval */
	void SetFloat(KyFloat32 r, KyFloat32 g, KyFloat32 b, KyFloat32 a = 1.0f)
	{
		m_r = (KyUInt8)(r * 255.0f);
		m_g = (KyUInt8)(g * 255.0f);
		m_b = (KyUInt8)(b * 255.0f);
		m_a = (KyUInt8)(a * 255.0f);
	}

public:
	KyUInt8 m_r; /*!< \label_redval */
	KyUInt8 m_g; /*!< \label_greenval */
	KyUInt8 m_b; /*!< \label_blueval */
	KyUInt8 m_a; /*!< \label_alphaval */

public:
	static const VisualColor AliceBlue;         /*!< Represents the color with RGBA values (240, 248, 255, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor AntiqueWhite;      /*!< Represents the color with RGBA values (250, 235, 215, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor Aqua;              /*!< Represents the color with RGBA values (  0, 255, 255, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor Aquamarine;        /*!< Represents the color with RGBA values (127, 255, 212, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor Azure;             /*!< Represents the color with RGBA values (240, 255, 255, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor Beige;             /*!< Represents the color with RGBA values (245, 245, 220, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor Bisque;            /*!< Represents the color with RGBA values (255, 228, 196, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor Black;             /*!< Represents the color with RGBA values (  0,   0,   0, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor BlanchedAlmond;    /*!< Represents the color with RGBA values (255, 235, 205, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor Blue;              /*!< Represents the color with RGBA values (  0,   0, 255, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor BlueViolet;        /*!< Represents the color with RGBA values (138,  43, 226, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor Brown;             /*!< Represents the color with RGBA values (165,  42,  42, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor BurlyWood;         /*!< Represents the color with RGBA values (222, 184, 135, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor CadetBlue;         /*!< Represents the color with RGBA values ( 95, 158, 160, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor Chartreuse;        /*!< Represents the color with RGBA values (127, 255,   0, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor Chocolate;         /*!< Represents the color with RGBA values (210, 105,  30, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor Coral;             /*!< Represents the color with RGBA values (255, 127,  80, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor CornflowerBlue;    /*!< Represents the color with RGBA values (100, 149, 237, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor Cornsilk;          /*!< Represents the color with RGBA values (255, 248, 220, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor Crimson;           /*!< Represents the color with RGBA values (220,  20,  60, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor Cyan;              /*!< Represents the color with RGBA values (  0, 255, 255, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor DarkBlue;          /*!< Represents the color with RGBA values (  0,   0, 139, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor DarkCyan;          /*!< Represents the color with RGBA values (  0, 139, 139, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor DarkGoldenrod;     /*!< Represents the color with RGBA values (184, 134,  11, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor DarkGray;          /*!< Represents the color with RGBA values (169, 169, 169, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor DarkGreen;         /*!< Represents the color with RGBA values (  0, 100,   0, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor DarkKhaki;         /*!< Represents the color with RGBA values (189, 183, 107, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor DarkMagenta;       /*!< Represents the color with RGBA values (139,   0, 139, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor DarkOliveGreen;    /*!< Represents the color with RGBA values ( 85, 107,  47, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor DarkOrange;        /*!< Represents the color with RGBA values (255, 140,   0, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor DarkOrchid;        /*!< Represents the color with RGBA values (153,  50, 204, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor DarkRed;           /*!< Represents the color with RGBA values (139,   0,   0, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor DarkSalmon;        /*!< Represents the color with RGBA values (233, 150, 122, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor DarkSeaGreen;      /*!< Represents the color with RGBA values (143, 188, 139, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor DarkSlateBlue;     /*!< Represents the color with RGBA values ( 72,  61, 139, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor DarkSlateGray;     /*!< Represents the color with RGBA values ( 47,  79,  79, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor DarkTurquoise;     /*!< Represents the color with RGBA values (  0, 206, 209, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor DarkViolet;        /*!< Represents the color with RGBA values (148,   0, 211, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor DeepPink;          /*!< Represents the color with RGBA values (255,  20, 147, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor DeepSkyBlue;       /*!< Represents the color with RGBA values (  0, 191, 255, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor DimGray;           /*!< Represents the color with RGBA values (105, 105, 105, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor DodgerBlue;        /*!< Represents the color with RGBA values ( 30, 144, 255, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor Firebrick;         /*!< Represents the color with RGBA values (178,  34,  34, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor FloralWhite;       /*!< Represents the color with RGBA values (255, 250, 240, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor ForestGreen;       /*!< Represents the color with RGBA values ( 34, 139,  34, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor Fuchsia;           /*!< Represents the color with RGBA values (255,   0, 255, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor Gainsboro;         /*!< Represents the color with RGBA values (220, 220, 220, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor GhostWhite;        /*!< Represents the color with RGBA values (248, 248, 255, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor Gold;              /*!< Represents the color with RGBA values (255, 215,   0, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor Goldenrod;         /*!< Represents the color with RGBA values (218, 165,  32, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor Gray;              /*!< Represents the color with RGBA values (128, 128, 128, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor Green;             /*!< Represents the color with RGBA values (  0, 128,   0, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor GreenYellow;       /*!< Represents the color with RGBA values (173, 255,  47, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor Honeydew;          /*!< Represents the color with RGBA values (240, 255, 240, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor HotPink;           /*!< Represents the color with RGBA values (255, 105, 180, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor IndianRed;         /*!< Represents the color with RGBA values (205,  92,  92, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor Indigo;            /*!< Represents the color with RGBA values ( 75,   0, 130, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor Ivory;             /*!< Represents the color with RGBA values (255, 255, 240, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor Khaki;             /*!< Represents the color with RGBA values (240, 230, 140, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor Lavender;          /*!< Represents the color with RGBA values (230, 230, 250, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor LavenderBlush;     /*!< Represents the color with RGBA values (255, 240, 245, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor LawnGreen;         /*!< Represents the color with RGBA values (124, 252,   0, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor LemonChiffon;      /*!< Represents the color with RGBA values (255, 250, 205, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor LightBlue;         /*!< Represents the color with RGBA values (173, 216, 230, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor LightCoral;        /*!< Represents the color with RGBA values (240, 128, 128, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor LightCyan;         /*!< Represents the color with RGBA values (224, 255, 255, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor LightGoldenrodYellow; /*!< Represents the color with RGBA values (250, 250, 210, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor LightGray;         /*!< Represents the color with RGBA values (211, 211, 211, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor LightGreen;        /*!< Represents the color with RGBA values (144, 238, 144, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor LightPink;         /*!< Represents the color with RGBA values (255, 182, 193, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor LightSalmon;       /*!< Represents the color with RGBA values (255, 160, 122, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor LightSeaGreen;     /*!< Represents the color with RGBA values ( 32, 178, 170, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor LightSkyBlue;      /*!< Represents the color with RGBA values (135, 206, 250, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor LightSlateGray;    /*!< Represents the color with RGBA values (119, 136, 153, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor LightSteelBlue;    /*!< Represents the color with RGBA values (176, 196, 222, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor LightYellow;       /*!< Represents the color with RGBA values (255, 255, 224, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor Lime;              /*!< Represents the color with RGBA values (  0, 255,   0, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor LimeGreen;         /*!< Represents the color with RGBA values ( 50, 205,  50, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor Linen;             /*!< Represents the color with RGBA values (250, 240, 230, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor Magenta;           /*!< Represents the color with RGBA values (255,   0, 255, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor Maroon;            /*!< Represents the color with RGBA values (128,   0,   0, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor MediumAquamarine;  /*!< Represents the color with RGBA values (102, 205, 170, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor MediumBlue;        /*!< Represents the color with RGBA values (  0,   0, 205, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor MediumOrchid;      /*!< Represents the color with RGBA values (186,  85, 211, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor MediumPurple;      /*!< Represents the color with RGBA values (147, 112, 219, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor MediumSeaGreen;    /*!< Represents the color with RGBA values ( 60, 179, 113, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor MediumSlateBlue;   /*!< Represents the color with RGBA values (123, 104, 238, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor MediumSpringGreen; /*!< Represents the color with RGBA values (  0, 250, 154, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor MediumTurquoise;   /*!< Represents the color with RGBA values ( 72, 209, 204, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor MediumVioletRed;   /*!< Represents the color with RGBA values (199,  21, 133, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor MidnightBlue;      /*!< Represents the color with RGBA values ( 25,  25, 112, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor MintCream;         /*!< Represents the color with RGBA values (245, 255, 250, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor MistyRose;         /*!< Represents the color with RGBA values (255, 228, 225, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor Moccasin;          /*!< Represents the color with RGBA values (255, 228, 181, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor NavajoWhite;       /*!< Represents the color with RGBA values (255, 222, 173, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor Navy;              /*!< Represents the color with RGBA values (  0,   0, 128, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor OldLace;           /*!< Represents the color with RGBA values (253, 245, 230, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor Olive;             /*!< Represents the color with RGBA values (128, 128,   0, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor OliveDrab;         /*!< Represents the color with RGBA values (107, 142,  35, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor Orange;            /*!< Represents the color with RGBA values (255, 165,   0, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor OrangeRed;         /*!< Represents the color with RGBA values (255,  69,   0, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor Orchid;            /*!< Represents the color with RGBA values (218, 112, 214, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor PaleGoldenrod;     /*!< Represents the color with RGBA values (238, 232, 170, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor PaleGreen;         /*!< Represents the color with RGBA values (152, 251, 152, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor PaleTurquoise;     /*!< Represents the color with RGBA values (175, 238, 238, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor PaleVioletRed;     /*!< Represents the color with RGBA values (219, 112, 147, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor PapayaWhip;        /*!< Represents the color with RGBA values (255, 239, 213, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor PeachPuff;         /*!< Represents the color with RGBA values (255, 218, 185, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor Peru;              /*!< Represents the color with RGBA values (205, 133,  63, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor Pink;              /*!< Represents the color with RGBA values (255, 192, 203, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor Plum;              /*!< Represents the color with RGBA values (221, 160, 221, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor PowderBlue;        /*!< Represents the color with RGBA values (176, 224, 230, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor Purple;            /*!< Represents the color with RGBA values (128,   0, 128, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor Red;               /*!< Represents the color with RGBA values (255,   0,   0, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor RosyBrown;         /*!< Represents the color with RGBA values (188, 143, 143, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor RoyalBlue;         /*!< Represents the color with RGBA values ( 65, 105, 225, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor SaddleBrown;       /*!< Represents the color with RGBA values (139,  69,  19, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor Salmon;            /*!< Represents the color with RGBA values (250, 128, 114, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor SandyBrown;        /*!< Represents the color with RGBA values (244, 164,  96, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor SeaGreen;          /*!< Represents the color with RGBA values ( 46, 139,  87, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor SeaShell;          /*!< Represents the color with RGBA values (255, 245, 238, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor Sienna;            /*!< Represents the color with RGBA values (160,  82,  45, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor Silver;            /*!< Represents the color with RGBA values (192, 192, 192, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor SkyBlue;           /*!< Represents the color with RGBA values (135, 206, 235, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor SlateBlue;         /*!< Represents the color with RGBA values (106,  90, 205, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor SlateGray;         /*!< Represents the color with RGBA values (112, 128, 144, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor Snow;              /*!< Represents the color with RGBA values (255, 250, 250, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor SpringGreen;       /*!< Represents the color with RGBA values (  0, 255, 127, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor SteelBlue;         /*!< Represents the color with RGBA values ( 70, 130, 180, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor Tan;               /*!< Represents the color with RGBA values (210, 180, 140, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor Teal;              /*!< Represents the color with RGBA values (  0, 128, 128, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor Thistle;           /*!< Represents the color with RGBA values (216, 191, 216, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor Tomato;            /*!< Represents the color with RGBA values (255,  99,  71, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor TransparentBlack;  /*!< Represents the color with RGBA values (  0,   0,   0,   0). \ingroup kypathdata_visualsystem */
	static const VisualColor TransparentWhite;  /*!< Represents the color with RGBA values (255, 255, 255,   0). \ingroup kypathdata_visualsystem */
	static const VisualColor Turquoise;         /*!< Represents the color with RGBA values ( 64, 224, 208, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor Violet;            /*!< Represents the color with RGBA values (238, 130, 238, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor Wheat;             /*!< Represents the color with RGBA values (245, 222, 179, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor White;             /*!< Represents the color with RGBA values (255, 255, 255, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor WhiteSmoke;        /*!< Represents the color with RGBA values (245, 245, 245, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor Yellow;            /*!< Represents the color with RGBA values (255, 255,   0, 255). \ingroup kypathdata_visualsystem */
	static const VisualColor YellowGreen;       /*!< Represents the color with RGBA values (154, 205,  50, 255). \ingroup kypathdata_visualsystem */

public:
	/*! Retrieves the color with the specified index within a limited internal palette of 31 distinct colors. */
	static VisualColor GetDistinctColor(KyUInt32 index);

	/*! Retrieves the color with the specified index within a limited internal palette of 31 distinct colors.
		The palette can be limited further by the \c nbColorsInPalette parameter. */
	static VisualColor GetDistinctColor(KyUInt32 index, KyUInt32 nbColorsInPalette);

	/*! Retrieves the color with the specified index from the specified palette.
		\param index				The index of the color to retrieve.
		\param customPalette		An array of colors from which the specified index will be retrieved. 
		\param nbColorsInPalette	The number of elements in the \c customPalette array. */
	static VisualColor GetDistinctColor(const KyUInt32 index, const VisualColor* customPalette, KyUInt32 nbColorsInPalette);
};


}


#endif
