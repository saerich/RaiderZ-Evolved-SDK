/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*! \file
	\ingroup kyruntime_bodyinfomanagement */

#ifndef KY_CONCRETEENTITYINFO_INL
#define KY_CONCRETEENTITYINFO_INL

#include <kyruntime/core/timemanager.h>

namespace Kaim
{

/////////////////////////////////////////////////////////////////////////////////
// ConcreteBodyInfo
/////////////////////////////////////////////////////////////////////////////////

template <typename T>
ConcreteBodyInfo<T>::ConcreteBodyInfo(const char* name, BodyInfoManager& bodyInfoManager, BodyProfile& profile) :
	BodyInfo(name, bodyInfoManager, profile)
{
	const KyUInt32 bits = bodyInfoManager.GetDateInfo().m_bitsPerDateId;
	m_datesId = KY_NEW_CLASS(BitArray)(bits);
}

template <typename T>
ConcreteBodyInfo<T>::~ConcreteBodyInfo()
{
	KY_DELETE_CLASS(m_datesId);
}

template <typename T>
void ConcreteBodyInfo<T>::ReInit()
{
	if (m_datesId)
	{
		const KyUInt32 invalid = m_bodyInfoManager.GetInvalidDateId();

		for (KyUInt32 i = 0; i < m_datesId->GetCount(); ++i)
			m_datesId->Set(i, invalid);
	}
}

template <typename T>
KyFloat32 ConcreteBodyInfo<T>::GetDate(KyUInt32 index) const
{
	KY_FUNCTION("ConcreteBodyInfo<T>::GetDate");
	KY_ASSERTK(index < m_datesId->GetCount());

	const KyUInt32 dateId = m_datesId->Get(index);
	const KyFloat32 date = m_bodyInfoManager.GetDate(dateId);
	return date;
}

template <typename T>
void ConcreteBodyInfo<T>::Refresh(KyUInt32 index)
{
	KY_FUNCTION("ConcreteBodyInfo<T>::Refresh");
	KY_ASSERT(index < m_datesId->GetCount(), ("Invalid index, index should be entity.GetPosInProfile()"));

	const KyUInt32 curDateId = m_bodyInfoManager.GetCurrentDateId();
	m_datesId->Set(index, curDateId);
}

template <typename T>
void ConcreteBodyInfo<T>::MoveData(KyUInt32 indexFrom, KyUInt32 indexTo)
{
	m_data[indexTo] = m_data[indexFrom];
	m_datesId->Set(indexTo, m_datesId->Get(indexFrom));
}

template <typename T>
void ConcreteBodyInfo<T>::Resize(KyUInt32 newSize)
{
	m_data.Resize(newSize);
	m_datesId->Resize(newSize);
}

template <typename T>
BodyInfo::ValidityStatus ConcreteBodyInfo<T>::GetValue(KyUInt32 index, T& value,
															BodyInfo::ValidityStatus requestedStatus, BodyInfo::AccessMode accessMode)
{
	// 1- check whether the info is up-to-date
	const KyFloat32 date = GetDate(index);
	BodyInfo::ValidityStatus status = BodyInfo::INVALID;
	KyBool upToDate = KY_TRUE;
	const KyFloat32 age = ((date == KyFloat32MAXVAL) ? KyFloat32MAXVAL : (GetWorld().GetAccumulatedTimeInUpdate() - date));

	if (age < m_validTime) 
	{
		status = BodyInfo::VALID;
	}
	else if (age < m_unsafeTime)
	{
		status = BodyInfo::UNSAFE;
		if (requestedStatus == BodyInfo::VALID)
			upToDate = KY_FALSE;
	}
	else if (requestedStatus > BodyInfo::INVALID)
		upToDate = KY_FALSE;

	// 2- recompute if needed
	if (upToDate == KY_FALSE)
	{
		if (accessMode == BodyInfo::RECOMPUTE_IF_TIME)
		{
			TimeManager& timeManager = GetWorld().GetTimeManager();
			for (;;)
			{
				timeManager.UpdateNoMoreTime();
				if (timeManager.NoMoreTime() == KY_TRUE)
					break;
				KyBool finished = KY_TRUE;
				const KyBool valid = ComputeInfoCell(index, finished);

				if (finished)
				{
					if (valid)
						status = BodyInfo::VALID;
					break;
				}
			}
		}
		else if (accessMode == BodyInfo::FORCE_RECOMPUTATION)
		{
			for (;;)
			{
				KyBool finished = KY_TRUE;
				const KyBool valid = ComputeInfoCell(index, finished);
				if (finished)
				{
					if (valid)
						status = BodyInfo::VALID;
					break;
				}
			}
		}
	}
	// 3- return the info
	value = m_data[index];
	return status;
}

} // namespace Kaim

#endif // KY_CONCRETEENTITYINFO_INL
