/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


#ifndef KY_BITFIELDFIXEDSIZE
#define KY_BITFIELDFIXEDSIZE

#include <kypathdata/generic/types.h>
#include <string.h>

namespace Kaim
{

template <KyUInt32 N>
class BitFieldFixedSize
{
public:
	BitFieldFixedSize() { UnsetAllBits(); }

	void SetBit(KyUInt32 index) { GetByteByGlobalIndex(index) |= (KyUInt8) 1 << GetPosInElem(index); }
	void UnsetBit(KyUInt32 index) { GetByteByGlobalIndex(index) &= (KyUInt8) ~(1 << GetPosInElem(index)); }

	void UnsetAllBits() { memset(m_bytes, '\0', sizeof (m_bytes)); }
	void SetAllBits() { memset(m_bytes, '\xFF', sizeof (m_bytes)); }

	bool IsBitSet(KyUInt32 index) const { return ((GetByteByGlobalIndex(index) >> GetPosInElem(index)) & 0x01) != 0; }
protected:
	const KyUInt8& GetByteByGlobalIndex(KyUInt32 index) const { return m_bytes[index / nbBitPerByte]; }
	KyUInt8& GetByteByGlobalIndex(KyUInt32 index) { return m_bytes[index / nbBitPerByte]; }
	KyUInt32 GetPosInElem(KyUInt32 index) const { return (index & mask); }
protected:
	enum { mask = 0x07, nbBitPerByte = 8};
	enum { nbBytes = (N + 7) / 8 };
protected:
	KyUInt8 m_bytes[nbBytes];
};

} // namespace Kaim

#endif // KY_BITFIELDFIXEDSIZE
