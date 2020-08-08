/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


#ifndef KyPathData_GuidCompound_H
#define KyPathData_GuidCompound_H

#include "kypathdata/generic/kyguid.h"
#include "kypathdata/blob/blobarray.h"

namespace Kaim
{

class GuidCompound
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	KY_FORCE_INLINE const KyGuid& GetMainGuid() const { return m_guids.m_count == 0 ?  KyGuid::s_defaultGuid : m_guids.Values()[0]; }

	bool operator<(const GuidCompound& other) const;

	bool AreGuidsEqual(const GuidCompound& other) const;

	bool operator==(const GuidCompound& other) const { return AreGuidsEqual(other) && m_timeStamp == other.m_timeStamp; }

	bool operator!=(const GuidCompound& other) const { return !operator==(other); }

	void Sort();

	KY_FORCE_INLINE KyUInt32 GetKyGuidCount() const { return m_guids.m_count; }

	KY_FORCE_INLINE const KyGuid& GetKyGuid(KyUInt32 index) const { return m_guids.Values()[index]; }

	bool IsCompoundedOf(const Kaim::GuidCompound& other) const;

public:
	BlobArray<KyGuid> m_guids;
	KyUInt32 m_timeStamp;
};

inline void SwapEndianness(Endianness::Target e, GuidCompound& self)
{
	SwapEndianness(e, self.m_guids);
	SwapEndianness(e, self.m_timeStamp);
}

} // namespace Kaim

#endif
