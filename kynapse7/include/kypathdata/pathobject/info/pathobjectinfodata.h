/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


#ifndef KyPathData_PathObjectInfoData_H
#define KyPathData_PathObjectInfoData_H


#include "kypathdata/blob/blobarray.h"
#include <kypathdata/basesystem/coordsystem.h>
#include "kypathdata/pathobject/pathobjectdatatypes.h"

namespace Kaim
{

/*
Serializable structure (Blob) containing all basic information of a pathObject, namely:
-The unique ID of the pathObject
-The Type of the PathObject:
   eg: ladder, door, lift,...
-The name of the Database associated with the PathObject
*/
class PathObjectInfoData
{
public:
	static const char* GetBlobType() { return "PathObjectInfoData"; }
	static KyUInt32 GetBlobVersion() { return 0; }

public:
	PathObjectInfoData() {}
	
public:
	PathObjectId m_uid;                                         //< Unique ID of the PathObject in a database.
	BlobArray<char> m_type;                                     //< Type of the PathObject.
	BlobArray<char> m_databaseName;                             //< Name of the database associated with the PathObject.
	KyFloat32 m_averageEntityHeight;                            //< Average height of entities that will use the PathObject
	
private:
	PathObjectInfoData& operator=(const PathObjectInfoData& pathObjectInfoData);
	PathObjectInfoData(const PathObjectInfoData& pathObjectInfoData);
};

/*
Endianness swap
*/
inline void SwapEndianness(Endianness::Target e, PathObjectInfoData& self)
{
	SwapEndianness(e, self.m_uid);
	SwapEndianness(e, self.m_type);
	SwapEndianness(e, self.m_databaseName);
	SwapEndianness(e, self.m_averageEntityHeight);
}

}

#endif // #ifndef KyPathData_PathObjectInfoData_H
