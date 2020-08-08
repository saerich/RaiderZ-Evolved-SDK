/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kypathdata_basesystem */

#ifndef KY_DRAW_DEBUG_H
#define KY_DRAW_DEBUG_H

namespace Kaim
{

/*!	Draws the specified text at the specified position.
	\param p			The position of the text, in three-dimensional space.
	\param r			\label_redval
	\param g			\label_greenval
	\param b			\label_blueval
	\param a			\label_alphaval
	\param text			The text to be rendered. */

inline void Draw3DText(const Vec3f& p, KyUInt32 r, KyUInt32 g, KyUInt32 b, KyUInt32 a, const char* text)
{
	IDrawBridge* drawBridge = GetBaseSystem().GetBridge().GetDrawBridge();
	if( drawBridge != KY_NULL )
		drawBridge->Text(p, r, g, b, a, text);
}

}

#endif // #define KY_DRAW_DEBUG_H
