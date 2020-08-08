/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/


// primary contact: JODA - secondary contact: GUAL
#ifndef KyPdg_CombinaisonCalculator_H
#define KyPdg_CombinaisonCalculator_H


#include <kypdg/common/stlvector.h>


namespace Kaim
{

class CombinaisonCalculator
{
public:
	CombinaisonCalculator(KyUInt32 maxElementCountInCombinaison)
	{
		m_maxBitIndex = maxElementCountInCombinaison - 1;
		m_maxBitMask = (1 << (m_maxBitIndex + 1)) - 1;
		m_bitMask = 1; // we dont consider the 
	}

	bool IsFinished() const
	{
		return m_bitMask > m_maxBitMask;
	}

	void Next()
	{
		++m_bitMask;
	}

	const StlVector<KyUInt32>& Compute()
	{
		m_currentCombinaison.clear();

		for (KyUInt32 bitIndex = 0; bitIndex <= m_maxBitIndex; ++bitIndex)
		{
			KyUInt32 isBitAtIndex = (m_bitMask >> bitIndex) & 0x1;
			if (isBitAtIndex)
				m_currentCombinaison.push_back(bitIndex);
		}

		return m_currentCombinaison;
	}

public:
	KyUInt32 m_maxBitIndex;
	KyUInt32 m_maxBitMask;
	KyUInt32 m_bitMask;
	StlVector<KyUInt32> m_currentCombinaison;
};


} // namespace Kaim


#endif
