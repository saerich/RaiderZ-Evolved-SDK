/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


#ifndef KyPathData_PathObject_ConnexionLink_Data_H
#define KyPathData_PathObject_ConnexionLink_Data_H


#include "kypathdata/pathobject/pathobjectdatatypes.h"
#include <kypathdata/blob/endianness.h>

namespace Kaim
{

/*
Links a connexion space to another one.
The link can be directional or not.
Serializable structure (Blob) containing data of a pathObject connection link, namely:
-The start connexion space of the link
-The end connexion space of the link
-A flag saying if the link is bidirectional.
*/
class PathObjectConnexionLinkData
{
public:
	static const char* GetBlobType() { return "PathObjectConnexionLinkData"; }
	static KyUInt32 GetBlobVersion() { return 0; }

public:
	PathObjectConnexionLinkData() {}

	
public:
	PathObjectConnexionSpaceIdx m_startConnexionSpaceIdx;   //!< ID of the start connexion space.
	PathObjectConnexionSpaceIdx m_endConnexionSpaceIdx;     //!< ID of the end connexion space.
};

/*
Implements the endianness swap for all members of PathObjectConnexionLinkData.
*/
inline void SwapEndianness(Endianness::Target e, PathObjectConnexionLinkData& self)
{
	SwapEndianness(e, self.m_startConnexionSpaceIdx);
	SwapEndianness(e, self.m_endConnexionSpaceIdx);
}

}

#endif // #ifndef KyPathData_PathObject_ConnexionLink_Data_H
