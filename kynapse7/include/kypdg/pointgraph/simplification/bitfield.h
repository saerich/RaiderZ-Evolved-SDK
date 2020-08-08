/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* This computer source code and related instructions and comments are the
* unpublished confidential and proprietary information of Autodesk, Inc. and
* are protected under applicable copyright and trade secret law. They may
* not be disclosed to, copied or used by any third party without
* the prior written consent of Autodesk, Inc.
*/

#ifndef KyPdg_BitField_H
#define KyPdg_BitField_H

#include "kypathdata/generic/memory.h"
#include <string.h>

namespace Kaim
{

class BitField
{
public:
	BitField(KyUInt32 nbBits) : m_nbBits(nbBits), m_bytes(KY_MALLOC(KyUInt8, GetNbBytes())) { UnsetAllBits(); }
	~BitField() { KY_FREE(m_bytes); }

	void SetBit(KyUInt32 index) { GetByteByGlobalIndex(index) |= (KyUInt8) 1 << GetPosInElem(index); }
	void UnsetBit(KyUInt32 index) { GetByteByGlobalIndex(index) &= (KyUInt8) ~(1 << GetPosInElem(index)); }

	void UnsetAllBits() { memset(m_bytes, '\0', GetNbBytes() * sizeof (*m_bytes)); }
	void SetAllBits() { memset(m_bytes, '\xFF', GetNbBytes() * sizeof (*m_bytes)); }

	bool IsBitSet(KyUInt32 index) const { return ((GetByteByGlobalIndex(index) >> GetPosInElem(index)) & 0x01) != 0; }
protected:
	const KyUInt8& GetByteByGlobalIndex(KyUInt32 index) const { return m_bytes[index / nbBitPerByte]; }
	KyUInt8& GetByteByGlobalIndex(KyUInt32 index) { return m_bytes[index / nbBitPerByte]; }
	KyUInt32 GetPosInElem(KyUInt32 index) const { return (index & mask); }
	KyUInt32 GetNbBytes() const { return (m_nbBits + 7) / 8; }
protected:
	enum { mask = 0x07, nbBitPerByte = 8};
protected:
	KyUInt32 m_nbBits;
	KyUInt8* m_bytes;
};

}

#endif // KyPdg_BitField_H
