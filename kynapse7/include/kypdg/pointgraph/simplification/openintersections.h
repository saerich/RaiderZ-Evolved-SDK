/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* This computer source code and related instructions and comments are the
* unpublished confidential and proprietary information of Autodesk, Inc. and
* are protected under applicable copyright and trade secret law. They may
* not be disclosed to, copied or used by any third party without
* the prior written consent of Autodesk, Inc.
*/

// primary contact: JODA - secondary contact: NOBODY

#ifndef KyPdg_OpenIntersections_H
#define KyPdg_OpenIntersections_H


namespace Kaim
{

class Vec2f;

bool SegmentVsOpenTriangle2d(const Vec2f& a, const Vec2f& b, const Vec2f& v1, const Vec2f& v2, const Vec2f& v3);

bool SegmentVsOpenSegment(const Vec2f& close1, const Vec2f& close2, const Vec2f& open1, const Vec2f& open2);

}

#endif // KyPdg_OpenIntersections_H
