/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	

/*! \file
	\ingroup kypdg */

// primary contact: LASI - secondary contact: GUAL
#ifndef KyPdg_ClientInputTriangleSoup_H
#define KyPdg_ClientInputTriangleSoup_H

#include "kypdg/pdginput/clientinputtriangle.h"

namespace Kaim
{

struct ClientInputTriangleSoup
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	static const char* GetBlobType() { return "ClientInputTriangleSoup"; }
	// history: 
	// 0: initial version (obviously)
	static KyUInt32 GetBlobVersion() { return 0; }

public:
	ClientInputTriangleSoup() {}

public:
	BlobArray<ClientInputTriangle> m_triangles;

private:
	ClientInputTriangleSoup& operator=(const ClientInputTriangleSoup& other);
	ClientInputTriangleSoup(const ClientInputTriangleSoup& other);
};

inline void SwapEndianness(Endianness::Target e, ClientInputTriangleSoup& self)
{
	SwapEndianness(e, self.m_triangles);
}

}


#endif

