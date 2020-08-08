/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/


// ---------- Primary contact: JUBA - secondary contact: NOBODY
#ifndef KyPathData_ActiveGuids_H
#define KyPathData_ActiveGuids_H

#include "kypathdata/generic/kyguid.h"
#include "kypathdata/generic/guidcompound.h"
#include "kypathdata/containers/array.h"

namespace Kaim
{



class ActiveGuids
{
public:
	void AddActiveKyGuid(const KyGuid& kyGuid);
	void RemoveActiveKyGuid(const KyGuid& kyGuid);

	//activatedGuid must be sorted
	bool IsGuidCompoundCompatible(const GuidCompound& guidCompound) const
	{
		KyUInt32 currentIndexInArray = 0;
		const KyUInt32 numberOfKyGuid = guidCompound.GetKyGuidCount();
		const KyUInt32 arraySize = m_activeGuids.GetCount();

		for (KyUInt32 i = 0; i < numberOfKyGuid; ++i)
		{
			const KyGuid& currentKyGuid = guidCompound.GetKyGuid(i);

			bool foundCurrentGuid = false;

			for (; currentIndexInArray < arraySize; ++currentIndexInArray)
			{
				if (currentKyGuid == m_activeGuids[currentIndexInArray])
				{
					foundCurrentGuid = true;
					++currentIndexInArray;
					break;
				}
			}

			if (!foundCurrentGuid)
				return false;
		}

		return true;
	}

	void Clear()
	{
		m_activeGuids.ForceClear();
	}


	Array<KyGuid> m_activeGuids;
};

}


#endif //KyPathData_ActiveGuids_H

