/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: GUAL - secondary contact: NOBODY
#ifndef KyPdg_BitArray_1024_H
#define KyPdg_BitArray_1024_H


#include <kypathdata/generic/memory.h>
#include <assert.h>


namespace Kaim
{


class BitArray_1024
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	BitArray_1024();

	~BitArray_1024();

	KyResult Init(void* words, KyInt32 wordsCount);

	KyResult Init(KyInt32 wordsCount);

	void ClearBits();

	void SetBit(KyInt32 bit_idx)
	{
		assert(bit_idx >= 0);

		KyUInt32 word_idx = bit_idx >> 5; // x / 32
		KyUInt32 bit_idx_in_word = bit_idx & 0x1F; // (1 << 5) - 1

		assert(word_idx < (KyUInt32)m_wordsCount);

		m_rootWord |= (1 << word_idx);
		m_words[word_idx] |= (1 << bit_idx_in_word);
	}

	KyInt32 GetBit(KyInt32 bit_idx) const
	{
		KyUInt32 word_idx = bit_idx >> 5; // x / 32
		KyUInt32 bit_idx_in_word = bit_idx & 0x1F; // (1 << 5) - 1
		return m_words[word_idx] & (1 << bit_idx_in_word);
	}

	KyInt32 GetFirstBitIdx() const;

	KyInt32 GetLastBitIdx() const;

private:
	KyInt32 m_wordsCount;
	KyUInt32 m_rootWord;
	KyUInt32* m_words;
	bool m_allocated;
};


}


#endif
