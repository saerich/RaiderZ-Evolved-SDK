/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	

/*! \file
	\ingroup kypdg */

// primary contact: LASI - secondary contact: GUAL
#ifndef KyPdg_ClientInputTriangle_H
#define KyPdg_ClientInputTriangle_H


#include "kypathdata/blob/endianness.h"
#include "kypdg/pdginput/inputtriangle.h"
#include "kypathdata/containers/array.h"

namespace Kaim
{

struct ClientInputTriangle
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	ClientInputTriangle() {} // default Ctor needed

	ClientInputTriangle(const Vec3f& A, const Vec3f& B, const Vec3f& C, TerrainTypeMask terrainTypeMask)
	{
		m_triangleA = A;
		m_triangleB = B;
		m_triangleC = C;
		m_terrainTypeMask = terrainTypeMask;
	}

	Vec3f m_triangleA;
	Vec3f m_triangleB;
	Vec3f m_triangleC;
	TerrainTypeMask m_terrainTypeMask;
};

inline void SwapEndianness(Endianness::Target e, ClientInputTriangle& self)
{
	SwapEndianness(e, self.m_triangleA);
	SwapEndianness(e, self.m_triangleB);
	SwapEndianness(e, self.m_triangleC);
	SwapEndianness(e, self.m_terrainTypeMask);
}

typedef Array<ClientInputTriangle> DynamicClientInputTriangleArray;
}


#endif

