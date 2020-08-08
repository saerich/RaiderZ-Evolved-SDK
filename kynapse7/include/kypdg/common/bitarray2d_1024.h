/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: GUAL - secondary contact: NOBODY
#ifndef KyPdg_BitArray2d_1024_H
#define KyPdg_BitArray2d_1024_H


#include "kypdg/generator/squarepartitiontypes.h"
#include "kypdg/common/bitarray_1024.h"
#include <kypdg/common/stlvector.h>
#include <assert.h>


namespace Kaim
{


class BitArray2d_1024
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	BitArray2d_1024();

	KyResult InitMaxSize(KyInt32 maxCountX, KyInt32 maxCountY);

	KyResult SetSize(KyInt32 countX, KyInt32 countY);

	void SetPixel(KyInt32 x, KyInt32 y)
	{
		assert(x >= 0 && x < m_countX && y >= 0 && y < m_countY);
		m_lineBitArray.SetBit(y);
		m_pixelBitArrays[y].SetBit(x);
	}

	bool GetPixel(KyInt32 x, KyInt32 y) const
	{
		assert(x >= 0 && x < m_countX && y >= 0 && y < m_countY);
		return m_pixelBitArrays[y].GetBit(x) != 0;
	}

	KyInt32 GetFirstY() const { return m_lineBitArray.GetFirstBitIdx(); }
	KyInt32 GetLastY()  const { return m_lineBitArray.GetLastBitIdx();  }

	KyInt32 GetFirstX(KyInt32 y) const { return m_pixelBitArrays[y].GetFirstBitIdx(); }
	KyInt32 GetLastX(KyInt32 y)  const { return m_pixelBitArrays[y].GetLastBitIdx();  }

	void ClearBits();

private:
	KyInt32 m_maxCountX;
	KyInt32 m_maxCountY;

	KyInt32 m_countX;
	KyInt32 m_countY;

	KyInt32 m_wordCountX;
	KyInt32 m_wordCountY;

	BitArray_1024 m_lineBitArray;
	StlVector<BitArray_1024> m_pixelBitArrays;

	StlVector<KyUInt32> m_lineBitsMem;
	StlVector<KyUInt32> m_pixelBitsMem;
};


}


#endif
