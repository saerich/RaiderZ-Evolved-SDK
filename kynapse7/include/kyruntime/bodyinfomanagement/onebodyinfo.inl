/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_bodyinfomanagement */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_ONEENTITYINFO_INL
#define KY_ONEENTITYINFO_INL

#include <kyruntime/bodyinfomanagement/bodyinfofilter.h>

namespace Kaim
{

/////////////////////////////////////////////////////////////////////////////////
// OneBodyInfo
/////////////////////////////////////////////////////////////////////////////////

template <typename T> void OneBodyInfo<T>::AddExploredProfile(BodyProfile* profile)
{
	KY_FUNCTION("OneBodyInfo<T>::AddExploredProfile");

#ifdef KYDEBUG
	for (KyUInt32 i = 0; i < m_exploredProfiles.GetCount(); ++i)
	{
		if (m_exploredProfiles[i]==profile)
		{
			KY_MESSAGE(KY_LOG_WARNING, ("This profile [%s] is already taken into account for this BodyInfo [%s]",
				profile->GetName(), this->GetName()));
			return;
		}
	}
#endif

	m_exploredProfiles.PushBack(profile);	
}


template <typename T> KyUInt32 OneBodyInfo<T>::GetCellCount() const
{
	KyUInt32 n = this->m_profile.GetBodies().GetCount();
	return n;
}

template <typename T> void OneBodyInfo<T>::SetValue(const Body& body, const T& value)
{
	KY_FUNCTION("OneBodyInfo<T>::SetValue");

	KY_ASSERT(body.GetProfile() != KY_NULL,
		("Please refer to the 'Using the BodyInfoManager Service in C++' section of the user guide"));
	const KyUInt32 index = body.GetPosInProfile();
	KY_ASSERTK(index < body.GetProfile()->GetBodies().GetCount());
	if (index < this->m_data.GetCount())
	{
		this->m_data[index] = value;
		this->Refresh(index);
	}
	else
	{
		KY_ASSERTK(index < this->m_data.GetCount());
	}
}

template <typename T> KyBool OneBodyInfo<T>::IsManaged(const Body& body) const
{
	KY_FUNCTION("OneBodyInfo<T>::IsManaged")

	// Check if the body profile is the same than the EntiyInfo
	if (body.GetProfile() != &(this->GetProfile()))
		return KY_FALSE;

	// Check if the body is managed by the profile (an body removed from the world give wrong position)
	const KyUInt32 entityPos = body.GetPosInProfile();
	if (entityPos >= this->m_profile.GetBodies().GetCount())
	{
		KY_MESSAGE(KY_LOG_WARNING, ("Cannot manage the body '%s' in BodyInfo '%s'. Did you remove this body from the world ?", body.GetName(), this->GetName()));
		return KY_FALSE;
	}
	return KY_TRUE;
}

template <typename T> BodyInfo::ValidityStatus OneBodyInfo<T>::GetValue(
	const Body& body, T& value,
	BodyInfo::ValidityStatus requestedStatus,
	BodyInfo::AccessMode accessMode)
{
	KY_FUNCTION("OneBodyInfo<T>::GetValue");

	if (requestedStatus == BodyInfo::INVALID)
		return BodyInfo::INVALID;

	// immediate mode
	if (this->m_immediateMode || !IsManaged(body))
	{
		if (accessMode == BodyInfo::RECOMPUTE_IF_TIME
			|| (this->m_immediateMode && accessMode == BodyInfo::READ_ONLY))
		{
			TimeManager& timeManager = this->GetWorld().GetTimeManager();
			for (;;)
			{
				timeManager.UpdateNoMoreTime();
				if (timeManager.NoMoreTime() == KY_TRUE)
					return BodyInfo::INVALID;
				OneBodyInfoExplorationIteratorReset(this->m_explorationIterator, this->m_exploredProfiles, KyUInt32MAXVAL);
				KyBool finished = KY_TRUE;
				const KyBool valid = ComputeInfo(body, value, finished);
				if (finished)
				{
					if (valid)
						return BodyInfo::VALID;
					return BodyInfo::INVALID;
				}
			}
		}
		else if (accessMode == BodyInfo::FORCE_RECOMPUTATION)
		{
			for (;;)
			{
				OneBodyInfoExplorationIteratorReset(this->m_explorationIterator, this->m_exploredProfiles, KyUInt32MAXVAL);
				KyBool finished = KY_TRUE;
				const KyBool valid = ComputeInfo(body, value, finished);
				if (finished)
				{
					if (valid)
						return BodyInfo::VALID;
					return BodyInfo::INVALID;
				}
			}
		}
		KY_ASSERTK(accessMode == BodyInfo::READ_ONLY);
		return BodyInfo::INVALID;
	}

	// standard mode
	KY_ASSERTK(body.GetProfile() != KY_NULL);
	const KyUInt32 index = body.GetPosInProfile();
	KY_ASSERTK(index < body.GetProfile()->GetBodies().GetCount());
	KY_ASSERTK(index < this->m_data.GetCount());
	const BodyInfo::ValidityStatus status = ConcreteBodyInfo<T>::GetValue(index, value, requestedStatus, accessMode);
	return status;
}

template <typename T> KyFloat32 OneBodyInfo<T>::GetDate(const Body& body)
{
	KY_FUNCTION("OneBodyInfo<T>::GetDate");

	KY_ASSERT(body.GetProfile() != KY_NULL,
		("Please refer to the 'Using the BodyInfoManager Service in C++' section of the user guide"));
	const KyUInt32 index = body.GetPosInProfile();
	KY_ASSERTK(index < body.GetProfile()->GetBodies().GetCount());
	if (index < this->m_data.GetCount())
	{
		const KyUInt32 dateId = this->m_datesId->Get(index);
		return this->m_bodyInfoManager.GetDate(dateId);
	}
	KY_ASSERTK(index < this->m_data.GetCount());
	KY_MESSAGE(KY_LOG_ERROR, ("Please refer to the 'Using the BodyInfoManager Service in C++' section of the user guide"));

	return 0.f;
}

template <typename T> void OneBodyInfo<T>::OnAddBody(const Body& body)
{
	if (this->m_immediateMode)
		return;
	if (body.GetProfile() == &this->m_profile)
	{
		const KyUInt32 newSize = this->m_data.GetCount() + 1;
		this->Resize(newSize);
	}
	Invalidate(body);
	this->m_infoIt = 0;
}

template <typename T> void OneBodyInfo<T>::OnRemoveBody(const Body& body)
{
	if (this->m_immediateMode)
		return;
	
	if (body.GetProfile() != &this->m_profile)
		return;

	// In profile, we have replaced body by the last body, so we make the same change here.
	const KyUInt32 curIndex = body.GetPosInProfile();
	const KyUInt32 lastIndex = this->m_data.GetCount() - 1;

	this->MoveData(lastIndex, curIndex);
	this->Resize(lastIndex);
	this->m_infoIt = 0;
}

template <typename T> KyBool OneBodyInfo<T>::ComputeInfoCell(KyUInt32 index, KyBool& finished)
{
	KY_FUNCTION("OneBodyInfo<T>::ComputeInfoCell");

	KY_ASSERTK(index < this->m_profile.GetBodies().GetCount());

	++this->m_computationCount;
	Body* ent = this->m_profile.GetBodies()[index];
	KY_ASSERTK(ent != KY_NULL);
	OneBodyInfoExplorationIteratorReset(this->m_explorationIterator, this->m_exploredProfiles, index);
	T value;
	const KyBool valid = ComputeInfo(*ent, value, finished);
	if (valid && finished)
	{
		this->m_data[index] = value;
		this->Refresh(index);
	}
	return valid;
}

template <typename T> KyBool OneBodyInfo<T>::Update(KyUInt32& count)
{
	KY_FUNCTION("OneBodyInfo<T>::Update");

	if (this->m_immediateMode)
		return KY_TRUE;
	// apply filters
	for (List<BodyInfoFilter*>::Iterator filterIt = this->m_filters.GetFirst(); filterIt.IsValid(); ++filterIt)
	{
		BodyInfoFilter* filter = *filterIt;
		KY_ASSERTK(filter != KY_NULL);

		if (filter->ForbidComputation(*this))
			return KY_TRUE;
	}

	// for each body, we update the information if it is necesary
	const KyUInt32 nowId = this->m_bodyInfoManager.GetCurrentDateId();
	const KyFloat32 now = this->m_bodyInfoManager.GetDate(nowId);
	if (this->m_infoIt >= this->m_profile.GetBodies().GetCount())
		this->m_infoIt = 0;
	const KyUInt32 invalid = this->m_bodyInfoManager.GetInvalidDateId();
	for (; this->m_infoIt != this->m_profile.GetBodies().GetCount(); ++this->m_infoIt)
	{
		// manage the number of calculations
		if (count == 0)
			return KY_FALSE;
		--count;

		Body* body = this->m_profile.GetBodies()[this->m_infoIt];
		KY_ASSERTK(body != KY_NULL);

		// apply filters
		KyBool forbidComputation = KY_FALSE;
		for (List<BodyInfoFilter*>::Iterator filterIt = this->m_filters.GetFirst(); filterIt.IsValid(); ++filterIt)
		{
			BodyInfoFilter* filter = *filterIt;
			KY_ASSERTK(filter != KY_NULL);
			if (filter->ForbidComputation(*this,*body))
			{
				forbidComputation = KY_TRUE;
				break;
			}
		}
		if (forbidComputation)
			continue;

		const KyUInt32 index = this->m_infoIt;

		KyUInt32 dateId = this->m_datesId->Get(index);
		if (dateId < invalid)
		{
			const KyFloat32 date = this->m_bodyInfoManager.GetDate(dateId);
			if (now - date < this->m_validTime)
				continue;
		}
		KyBool finished = KY_TRUE;
		ComputeInfoCell(index,finished);
		if (finished == KY_FALSE)
			return KY_FALSE;
	}
	this->m_infoIt = KyUInt32MAXVAL;
	return KY_TRUE;
}

template <typename T> KyBool OneBodyInfo<T>::UpdateDates(KyUInt32& count)
{
	if (this->m_immediateMode)
		return KY_TRUE;
	// for each body, we update the information if it is necesary
	const KyFloat32 now = this->m_bodyInfoManager.GetCurrentDate();
	const KyUInt32 invalid = this->m_bodyInfoManager.GetInvalidDateId();
	const KyFloat32 dateUpdatePeriod = this->m_bodyInfoManager.GetDateInfo().m_dateUpdatePeriod;
	for ( ; m_dateIt < this->m_profile.GetBodies().GetCount(); ++m_dateIt)
	{
		if (count == 0)
			// we did enough calculations for now
			return KY_FALSE;
		--count;

		const KyUInt32 index = this->m_dateIt;
		const KyFloat32 date = ConcreteBodyInfo<T>::GetDate(index);
		if (date == KyFloat32MAXVAL)
			continue;
		const KyFloat32 dt = now - date;
		if (dt >= this->m_unsafeTime || dt >= dateUpdatePeriod)
			this->m_datesId->Set(index,invalid);
	}
	this->m_dateIt = 0;
	return KY_TRUE;
}

template <typename T> void OneBodyInfo<T>::Invalidate(const Body& body)
{
	KY_FUNCTION("OneBodyInfo<T>::Invalidate");

	if (this->m_immediateMode)
		return;
	KY_ASSERT(body.GetProfile() != KY_NULL,
		("Please refer to the 'Using the BodyInfoManager Service in C++' section of the user guide"));

	// Update info between 'body' and XXX
	if (body.GetProfile() == &this->m_profile)
	{
		const KyUInt32 index = body.GetPosInProfile();
		KY_ASSERTK(index < this->m_datesId->GetCount());
		const KyUInt32 invalid = this->m_bodyInfoManager.GetInvalidDateId();
		this->m_datesId->Set(index, invalid);
		KY_ASSERTK(this->m_datesId->Get(index) == invalid);
	}
}

} // namespace Kaim

#endif // KY_ONEENTITYINFO_INL
