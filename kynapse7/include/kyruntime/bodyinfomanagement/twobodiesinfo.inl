/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download,
* or which otherwise accompanies this software in either electronic or hard copy form.
*/




/*!	\file
	\ingroup kyruntime_bodyinfomanagement */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_TWOENTITYINFO_INL
#define KY_TWOENTITYINFO_INL

#include <kyruntime/core/timemanager.h>

#include <kypathdata/generic/cstdio.h>


namespace Kaim
{
	/////////////////////////////////////////////////////////////////////////////////
	// TwoBodiesInfo
	/////////////////////////////////////////////////////////////////////////////////

	template <typename T> void TwoBodiesInfo<T>::AddInputProfile(BodyProfile* profile)
	{
		KY_FUNCTION("TwoBodiesInfo<T>::AddInputProfile");

#ifdef KYDEBUG
		for (KyUInt32 i = 0; i < m_inputProfiles.GetCount(); ++i)
		{
			if (m_inputProfiles[i]==profile)
			{
				KY_MESSAGE(KY_LOG_WARNING, ("This profile [%s] is already taken into account for this BodyInfo [%s]",
					profile->GetName(), this->GetName()));
				return;
			}
		}
#endif

		m_inputProfiles.PushBack(profile);
	}


	template <typename T> KyUInt32 TwoBodiesInfo<T>::GetCellCount() const
	{
		KyUInt32 n = this->m_profile.GetBodies().GetCount();
		KyUInt32 p = 0;
		for (KyUInt32 i = 0; i < this->m_inputProfiles.GetCount(); ++i)
			p += this->m_inputProfiles[i]->GetBodies().GetCount();
		return n * p;
	}

	template <typename T> TwoBodiesInfo<T>::TwoBodiesInfo(const char* name, BodyInfoManager& bodyInfoManager, BodyProfile& profile) :
		ConcreteBodyInfo<T>(name, bodyInfoManager, profile),
		m_inputBodyMax(0),
		m_dateIt(0)
	{
		m_infoIt.m_info = this;
	}

	template <typename T> KyUInt32 TwoBodiesInfo<T>::GetIndex(const Body& body1, const Body& body2) const
	{
		KY_FUNCTION("TwoBodiesInfo<T>::GetIndex");

		if (body1.GetProfile() != &(this->GetProfile()) || body2.GetProfile() == KY_NULL)
		{
			KY_MESSAGE(KY_LOG_ERROR, ("Please refer to the 'Using the BodyInfoManager Service in C++' section of the user guide"));
			return KyUInt32MAXVAL;
		}

		const BodyProfile* profile2 = body2.GetProfile();
		KyUInt32 colOffset = 0;
		for (KyUInt32 i = 0; i < this->m_inputProfiles.GetCount(); ++i)
		{
			const BodyProfile* p = this->m_inputProfiles[i];

			if (p != profile2)
			{
				colOffset += p->GetBodies().GetCount();
				continue;
			}

			const KyUInt32 line = body1.GetPosInProfile();
			if (line >= this->m_profile.GetBodies().GetCount())
			{
				KY_MESSAGE(KY_LOG_WARNING, ("Cannot manage the body '%s' in BodyInfo '%s'. Did you remove this body from the world ?", body1.GetName(), this->GetName()));
				return KyUInt32MAXVAL;
			}

			const KyUInt32 ent2PosInProfile = body2.GetPosInProfile();
			if (ent2PosInProfile >= profile2->GetBodies().GetCount())
			{
				KY_MESSAGE(KY_LOG_WARNING, ("Cannot manage the body '%s' in Profile '%s'. Did you remove this body from the world ?", body2.GetName(), profile2->GetName()));
				return KyUInt32MAXVAL;
			}

			const KyUInt32 col = colOffset + ent2PosInProfile;
			const KyUInt32 index = line * this->m_inputBodyMax + col;

			if (index >= this->m_datesId->GetCount())
			{
				KY_MESSAGE(KY_LOG_ERROR, ("Invalid parameter, please look for 'Using the BodyInfoManager Service in C++' in the user guide"));
				return KyUInt32MAXVAL;
			}
			return index;
		}

		KY_MESSAGE(KY_LOG_WARNING, ("Invalid parameter, please look for 'Using the BodyInfoManager Service in C++' in the user guide"));
		return KyUInt32MAXVAL;
	}

	template <typename T> KyBool TwoBodiesInfo<T>::GetBodiesFromIndex(KyUInt32 index, Body*& body1, Body*& body2)
	{
		if (index > this->m_data.GetCount())
			return KY_FALSE;
		const KyUInt32 id1 = index / m_inputBodyMax;
		if (id1 >= this->m_profile.GetBodies().GetCount())
			return KY_FALSE;
		body1 = this->m_profile.GetBodies()[id1];
		if (body1 == KY_NULL)
			return KY_FALSE;
		const KyUInt32 mod = index % m_inputBodyMax;
		KyUInt32 n = mod;
		for (KyUInt32 i = 0; i < m_inputProfiles.GetCount(); ++i)
		{
			BodyProfile* p = m_inputProfiles[i];
			if (n < p->GetBodies().GetCount())
			{
				body2 = p->GetBodies()[n];
				if (body2 == KY_NULL)
					return KY_FALSE;
				return KY_TRUE;
			}
			n -= p->GetBodies().GetCount();
		}
		return KY_FALSE;
	}

	template <typename T> void TwoBodiesInfo<T>::SetValue(const Body& body1, const Body& body2, const T& value)
	{
		KY_FUNCTION("TwoBodiesInfo<T>::SetValue");
		KY_ASSERT(body1.GetProfile() == &this->m_profile,
			("Please refer to the 'Using the BodyInfoManager Service in C++' section of the user guide"));

		const KyUInt32 index = GetIndex(body1, body2);
		if (index < this->m_data.GetCount())
		{
			this->m_data[index] = value;
			this->Refresh(index);
		}
	}

	template <typename T> KyBool TwoBodiesInfo<T>::IsManaged(const Body& body1, const Body& body2) const
	{
		KY_FUNCTION("TwoBodiesInfo<T>::IsManaged")

		// Check if the body1 profile is the same than the EntiyInfo and
		// if the body2 profile is not null
		if (body1.GetProfile() != &(this->GetProfile())
			|| body2.GetProfile() == KY_NULL)
		{
			return KY_FALSE;
		}

		// Check if the body1 is managed by the profile (an body removed from the world give wrong position)
		const KyUInt32 entity1Pos = body1.GetPosInProfile();
		if (entity1Pos >= this->m_profile.GetBodies().GetCount())
		{
			KY_MESSAGE(KY_LOG_WARNING, ("Cannot manage the body '%s' in BodyInfo '%s'. Did you remove this body from the world ?", body1.GetName(), this->GetName()));
			return KY_FALSE;
		}

		const BodyProfile* profile2 = body2.GetProfile();

		// Check if the body2 is managed by the profile2 (an body removed from the world give wrong position)
		const KyUInt32 entity2Pos = body2.GetPosInProfile();
		if (entity2Pos >= profile2->GetBodies().GetCount())
		{
			KY_MESSAGE(KY_LOG_WARNING, ("Cannot manage the body '%s' in Profile '%s'. Did you remove this body from the world ?", body2.GetName(), profile2->GetName()));
			return KY_FALSE;
		}

		// Check if we can find the body2 profile
		for (KyUInt32 i = 0; i < m_inputProfiles.GetCount(); ++i)
		{
			const BodyProfile* p = m_inputProfiles[i];

			if (p == profile2)
				return KY_TRUE;
		}
		return KY_FALSE;
	}

	template <typename T> BodyInfo::ValidityStatus TwoBodiesInfo<T>::GetValue(
		const Body& body1, const Body& body2, T& value,
		BodyInfo::ValidityStatus requestedStatus,
		BodyInfo::AccessMode accessMode)
	{
		KY_FUNCTION("TwoBodiesInfo<T>::GetValue");

		if (requestedStatus == BodyInfo::INVALID)
			return BodyInfo::INVALID;

		// immediate mode
		if (this->m_immediateMode || !IsManaged(body1,body2))
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

					KyBool finished = KY_TRUE;
					const KyBool valid = ComputeInfo(body1, body2, value, finished);
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

					KyBool finished = KY_TRUE;
					const KyBool valid = ComputeInfo(body1, body2, value, finished);
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
		const KyUInt32 index = GetIndex(body1, body2);
		if (index == KyUInt32MAXVAL)
		{
			KY_MESSAGE(KY_LOG_WARNING, ("The requested information is not available"));
			return BodyInfo::INVALID;
		}
		const BodyInfo::ValidityStatus status = ConcreteBodyInfo<T>::GetValue(index, value, requestedStatus, accessMode);
		return status;
	}

	template <typename T> KyFloat32 TwoBodiesInfo<T>::GetDate(const Body& body1, const Body& body2)
	{
		KY_FUNCTION("TwoBodiesInfo<T>::GetDate");
		const KyUInt32 index = GetIndex(body1,body2);
		KY_ASSERT(index < this->m_datesId->GetCount(), ("Please refer to the 'Using the BodyInfoManager Service in C++' section of the user guide"));
		const KyUInt32 dateId = this->m_datesId->Get(index);
		const KyFloat32 date = this->m_bodyInfoManager.GetDate(dateId);
		return date;
	}

	template <typename T> KyBool TwoBodiesInfo<T>::ComputeInfoCell(KyUInt32 index, KyBool& finished)
	{
		++this->m_computationCount;
		Body* body1 = KY_NULL;
		Body* body2 = KY_NULL;
		if (GetBodiesFromIndex(index, body1, body2) == KY_FALSE)
			return KY_FALSE;
		return ComputeInfoCell(*body1, *body2, index, finished);
	}

	template <typename T> void TwoBodiesInfo<T>::OnAddBody(const Body& body)
	{
		if (this->m_immediateMode)
			return;
		KY_FUNCTION("TwoBodiesInfo<T>::OnAddBody");

		const KyUInt32 oldInputEntityMax = m_inputBodyMax;
		const KyUInt32 oldEntityCount = oldInputEntityMax ? this->m_data.GetCount() / oldInputEntityMax : 0;
		KyUInt32 inputEntityMax = 0;
		KyUInt32 newPos = 0;

		for (KyUInt32 i = 0; i < this->m_inputProfiles.GetCount(); ++i)
		{
			const BodyProfile& profile = *this->m_inputProfiles[i];

			inputEntityMax += profile.GetBodies().GetCount();
			if (&profile == body.GetProfile())
				newPos = inputEntityMax - 1;
		}
		m_inputBodyMax = inputEntityMax;
		const KyUInt32 entityCount = body.GetProfile() == &this->m_profile ? oldEntityCount + 1 : oldEntityCount;
		const KyUInt32 n = entityCount * inputEntityMax;

		this->Resize(n);

		if (m_inputBodyMax != oldInputEntityMax)
		{
			// Update info between XXX and 'body'
			KyUInt32 newIndex = oldEntityCount * m_inputBodyMax;
			for (KyUInt32 oldIndex = oldEntityCount * oldInputEntityMax; oldIndex != 0; )
			{
				--oldIndex;
				--newIndex;
				if (oldIndex % oldInputEntityMax == newPos - 1)
					--newIndex;
				this->MoveData(oldIndex, newIndex);
			}
			KY_ASSERTK(newIndex == 0);
		}
		Invalidate(body);
		this->m_infoIt.Reset();
	}

	template <typename T> void TwoBodiesInfo<T>::OnRemoveBody(const Body& body)
	{
		if (this->m_immediateMode)
			return;

		if (!m_inputBodyMax || !this->m_data.GetCount())
			return;

		KY_FUNCTION("TwoBodiesInfo<T>::OnRemoveBody");

		const KyUInt32 oldInputBodyMax = m_inputBodyMax;
		const KyUInt32 oldBodyCount = this->m_data.GetCount() / oldInputBodyMax;
		KyUInt32 newBodyCount = oldInputBodyMax;
		if (body.GetProfile() == &this->m_profile)
		{
			--newBodyCount;
			// Update info between 'body' and XXX
			// In profile, we have replaced body by the last body, so we make the same change here.
			const KyUInt32 curIndex = body.GetPosInProfile();
			const KyUInt32 lastIndex = oldBodyCount - 1;

			// swap line.
			for (KyUInt32 i = 0; i < oldInputBodyMax; ++i)
			{
				this->MoveData(lastIndex + i, curIndex + i);
			}
		}
		// Update info between XXX and 'body'
		// In profile, we have replaced body by the last body, so we make the same change here.
		KyUInt32 newPos = KyUInt32MAXVAL;
		KyUInt32 oldPos = KyUInt32MAXVAL;
		KyUInt32 inputBodyMax = 0;
		for (KyUInt32 i = 0; i < this->m_inputProfiles.GetCount(); ++i)
		{
			const BodyProfile& profile = *this->m_inputProfiles[i];

			if (&profile == body.GetProfile())
			{
				oldPos = inputBodyMax + body.GetPosInProfile();
				newPos = inputBodyMax + profile.GetBodies().GetCount();
			}
			inputBodyMax += profile.GetBodies().GetCount();
		}
		m_inputBodyMax = inputBodyMax;
		if (m_inputBodyMax != oldInputBodyMax)
		{
			if (newPos != oldPos)
			{
				// In profile, we have replaced body by the last body, so we make the same change here.
				// swap column
				for (KyUInt32 i = 0; i < newBodyCount; ++i)
				{
					this->MoveData(i * oldInputBodyMax + oldPos, i * oldInputBodyMax + newPos);
				}
			}
			// delete column
			KyUInt32 index = 0;
			for (KyUInt32 i = 0; i < newBodyCount; ++i)
			{
				for (KyUInt32 j = 0; j < oldInputBodyMax; ++j)
				{
					if (j != newPos)
					{
						this->MoveData(i * oldInputBodyMax + j, index++);
					}
				}
			}
			KY_ASSERTK(index == newBodyCount * m_inputBodyMax);
		}
		this->Resize(newBodyCount * m_inputBodyMax);
		this->m_infoIt.Reset();
	}

	template <typename T> KyBool TwoBodiesInfo<T>::ComputeInfoCell(Body& body1, Body& body2, KyUInt32 index, KyBool& finished)
	{
		++this->m_computationCount;

		T value;
		const KyBool valid = ComputeInfo(body1, body2, value, finished);

		if (valid && finished)
		{
			this->m_data[index] = value;
			this->Refresh(index);
		}
		return valid;
	}

	template <typename T> TwoBodiesInfoIterator<T>::TwoBodiesInfoIterator() :
		m_curEnt1It(0), m_curProfileId(0), m_colOffset(0), m_lineOffset(0), m_curEnt2It(0),
		m_finished(KY_TRUE), m_index(KyUInt32MAXVAL), m_info(KY_NULL)
	{
	}

	template <typename T> void TwoBodiesInfoIterator<T>::Reset()
	{
		KY_FUNCTION("TwoBodiesInfoIterator<T>::Reset");
		m_curProfileId = 0;
		m_colOffset = 0;
		m_curEnt1It = 0;
		if (m_curEnt1It >= m_info->GetProfile().GetBodies().GetCount())
		{
			m_index = 0;
			m_curEnt2It = KyUInt32MAXVAL;
			m_finished = KY_TRUE;
		}
		else
		{
			const KyUInt32 line = m_curEnt1It;
			KY_ASSERTK(line < m_info->GetProfile().GetBodies().GetCount());
			m_lineOffset = line * m_info->GetInputBodyMax();

			const Array<BodyProfile*>& relatedProfiles = m_info->GetInputProfiles();
			for ( ; m_curProfileId < relatedProfiles.GetCount(); ++m_curProfileId)
			{
				const BodyProfile* profile = relatedProfiles[m_curProfileId];
				if (profile->GetBodies().GetCount() == 0)
				{
					continue;
				}
				m_curEnt2It = 0;
				m_index = m_lineOffset + m_colOffset + m_curEnt2It;
				break;
			}
			if (m_curEnt2It < m_info->GetInputProfiles()[m_curProfileId]->GetBodies().GetCount())
				m_finished = KY_FALSE;
			else
				m_finished = KY_TRUE;
		}
	}

	template <typename T> bool TwoBodiesInfoIterator<T>::operator == (
		const TwoBodiesInfoIterator<T>& it)
	{
		return (m_finished == it.m_finished && m_curProfileId == it.m_curProfileId
			&& m_info == it.m_info && m_curEnt1It == it.m_curEnt1It && m_curEnt2It == it.m_curEnt2It);
	}

	template <typename T> bool TwoBodiesInfoIterator<T>::operator != (
		const TwoBodiesInfoIterator<T>& it)
	{
		return !(*this == it);
	}

	template <typename T> void TwoBodiesInfoIterator<T>::IncrementLineIndex()
	{
		KY_FUNCTION("TwoBodiesInfoIterator<T>::IncrementLineIndex()");

		// 1- finished ?
		if (m_finished)
		{
			return;
		}

		// 2- move to the next element
		const Array<BodyProfile*>& relatedProfiles = m_info->GetInputProfiles();
		for (;;)
		{
			// 2.1- move to the next body from the current related profile
			++m_curEnt2It;
			if (m_curEnt2It < relatedProfiles[m_curProfileId]->GetBodies().GetCount())
			{
				KY_ASSERTK(relatedProfiles[m_curProfileId] != KY_NULL);
				m_index = m_lineOffset + m_colOffset + m_curEnt2It;
				return;
			}

			// 2.2- move to the next related profile
			for (++m_curProfileId; m_curProfileId < relatedProfiles.GetCount(); ++m_curProfileId)
			{
				BodyProfile* prevRelProf = relatedProfiles[m_curProfileId-1];
				m_colOffset += prevRelProf->GetBodies().GetCount();

				BodyProfile* curRelProf = relatedProfiles[m_curProfileId];
				KY_ASSERTK(curRelProf != KY_NULL);

				m_curEnt2It = 0;
				if (m_curEnt2It < curRelProf->GetBodies().GetCount())
				{
					m_index = m_lineOffset + m_colOffset + m_curEnt2It;
					return;
				}
			}

			// 2.3- move to the next body from m_info->profile
			++m_curEnt1It;
			if (m_curEnt1It >= m_info->GetProfile().GetBodies().GetCount())
			{
				m_finished = KY_TRUE;
				return;
			}
			const KyUInt32 line = m_curEnt1It;
			m_lineOffset = line * m_info->GetInputBodyMax();
			for (m_colOffset = 0, m_curProfileId = 0; m_curProfileId < relatedProfiles.GetCount(); ++m_curProfileId)
			{
				BodyProfile* curRelProf = relatedProfiles[m_curProfileId];
				KY_ASSERTK(curRelProf != KY_NULL);

				m_curEnt2It = 0;
				if (m_curEnt2It < curRelProf->GetBodies().GetCount())
				{
					m_index = m_lineOffset + m_colOffset + m_curEnt2It;
					return;
				}
				m_colOffset += curRelProf->GetBodies().GetCount();
			}
			m_finished = KY_TRUE;
		}
	}

	template <typename T> void TwoBodiesInfoIterator<T>::IncrementColumnIndex()
	{
		KY_FUNCTION("TwoBodiesInfoIterator<T>::IncrementColumnIndex()");

		if (m_finished)
		{
			return;
		}

		// move to the next body from m_info->profile
		++m_curEnt1It;
		if (m_curEnt1It >= m_info->GetProfile().GetBodies().GetCount())
		{
			m_finished = KY_TRUE;
			return;
		}
		const KyUInt32 line = m_curEnt1It;
		KY_ASSERTK(line < m_info->GetProfile().GetBodies().GetCount());
		m_lineOffset = line * m_info->GetInputBodyMax();

		// find the first related body
		const Array<BodyProfile*>& inputProfiles = m_info->GetInputProfiles();
		for (m_colOffset = 0, m_curProfileId = 0; m_curProfileId < inputProfiles.GetCount(); ++m_curProfileId)
		{
			BodyProfile* curRelProf = inputProfiles[m_curProfileId];
			KY_ASSERTK(curRelProf != KY_NULL);

			if (curRelProf->GetBodies().GetCount() == 0)
			{
				continue;
			}
			m_curEnt2It = 0;
			m_index = m_lineOffset + m_colOffset + m_curEnt2It;
			return;
		}
		m_finished = KY_TRUE;
	}

	template <typename T> KyBool TwoBodiesInfoIterator<T>::GetBodies(Body*& body1, Body*& body2) const
	{
		if (m_curEnt1It < m_info->GetProfile().GetBodies().GetCount() && m_curEnt2It < m_info->GetInputProfiles()[m_curProfileId]->GetBodies().GetCount())
		{
			body1 = m_info->GetProfile().GetBodies()[m_curEnt1It];
			body2 = m_info->GetInputProfiles()[m_curProfileId]->GetBodies()[m_curEnt2It];
			return KY_TRUE;
		}
		return KY_FALSE;
	}

	template <typename T> KyBool TwoBodiesInfo<T>::Update(KyUInt32& count)
	{
		KY_FUNCTION("TwoBodiesInfo<T>::Update");

		if (this->m_immediateMode)
			return KY_TRUE;

		// apply filters
		if (this->IsComputationForbidden())
			return KY_TRUE;

		// for each pair of entities, we update the information if it is necesary
		const KyUInt32 nowId = this->m_bodyInfoManager.GetCurrentDateId();
		const KyFloat32 now = this->m_bodyInfoManager.GetDate(nowId);
		if (!m_infoIt.IsValid())
			m_infoIt.Reset();

		const KyUInt32 invalid = this->m_bodyInfoManager.GetInvalidDateId();
		Body* firstEntity = KY_NULL;
		KyBool forbidFirstEntity = KY_FALSE;
		for (; m_infoIt.IsValid(); m_infoIt.Increment(forbidFirstEntity))
		{
			forbidFirstEntity = KY_FALSE;
			KY_ASSERTK(m_infoIt.GetIndex() < this->m_data.GetCount());

			// manage the number of calculations
			if (count == 0)
				return KY_FALSE;
			--count;

			Body* body1 = KY_NULL;
			Body* body2 = KY_NULL;
#ifdef KYDEBUG
			const KyBool entitiesFound = this->m_infoIt.GetBodies(body1, body2);
			KY_ASSERTK(entitiesFound == KY_TRUE);
#else
			this->m_infoIt.GetBodies(body1, body2);
#endif

			// apply filters
			if (firstEntity != body1)
			{
				firstEntity = body1;
				forbidFirstEntity = this->IsComputationForbidden(*body1);
				if (forbidFirstEntity)
					continue;
			}

			if (this->IsComputationForbidden(*body1, *body2))
				continue;

			// check whether the info must be recomputed
			const KyUInt32 index = m_infoIt.GetIndex();
			KyUInt32 dateId = this->m_datesId->Get(index);

			if (dateId != invalid)
			{
				const KyFloat32 date = this->m_bodyInfoManager.GetDate(dateId);
				if (now - date < this->m_validTime)
					continue;
			}

			// compute the info
			KyBool finished = KY_TRUE;
			ComputeInfoCell(*body1, *body2, index, finished);
			if (finished == KY_FALSE)
				return KY_FALSE;
		}
		return KY_TRUE;
	}

	template <typename T> KyBool TwoBodiesInfo<T>::UpdateDates(KyUInt32& infoCount)
	{
		if (this->m_immediateMode)
			return KY_TRUE;
		const KyFloat32 now = this->m_bodyInfoManager.GetWorld().GetAccumulatedTimeInUpdate();
		const KyUInt32 invalid = this->m_bodyInfoManager.GetInvalidDateId();

		for (KyUInt32 i = m_dateIt; i < this->m_datesId->GetCount(); ++i)
		{
			if (infoCount == 0)
			{
				m_dateIt = i;
				return KY_FALSE;
			}
			--infoCount;
			const KyFloat32 date = this->m_bodyInfoManager.GetDate(this->m_datesId->Get(i));
			const KyFloat32 dt = now - date;
			if (dt >= this->m_unsafeTime || dt >= this->m_bodyInfoManager.GetDateInfo().m_dateUpdatePeriod)
			{
				this->m_datesId->Set(i, invalid);
			}
		}
		m_dateIt = 0;
		return KY_TRUE;
	}

	template <typename T> void TwoBodiesInfo<T>::Invalidate(const Body& body)
	{
		KY_FUNCTION("TwoBodiesInfo<T>::Invalidate");

		if (this->m_immediateMode)
			return;
		// Update info between 'body' and XXX
		if (body.GetProfile() == &this->m_profile)
		{
			const KyUInt32 line = body.GetPosInProfile();
			KY_ASSERTK(line < body.GetProfile()->GetBodies().GetCount());

			const KyUInt32 index0 = line * m_inputBodyMax;
			const KyUInt32 invalid = this->m_bodyInfoManager.GetInvalidDateId();
			for (KyUInt32 i = 0; i < m_inputBodyMax; ++i)
			{
				const KyUInt32 index = index0 + i;
				KY_ASSERTK(index < this->m_datesId->GetCount());
				this->m_datesId->Set(index, invalid);
				KY_ASSERTK(this->m_datesId->Get(index) == invalid);
			}
		}

		// Update info between XXX and 'body'
		KyUInt32 colOffset = 0;
		const KyUInt32 invalid = this->m_bodyInfoManager.GetInvalidDateId();
		for (KyUInt32 i = 0; i < m_inputProfiles.GetCount(); ++i)
		{
			BodyProfile* p = m_inputProfiles[i];

			if (p != body.GetProfile())
			{
				colOffset += p->GetBodies().GetCount();
				continue;
			}
			KY_ASSERTK(p == body.GetProfile());
			const KyUInt32 col = colOffset + body.GetPosInProfile();
			const Array<Body*>& entities = this->GetProfile().GetBodies();
			for (KyUInt32 entIt = 0; entIt < entities.GetCount(); ++entIt)
			{
				const KyUInt32 line = entIt;
				const KyUInt32 index = col + line * m_inputBodyMax;
				KY_ASSERTK(index < this->m_datesId->GetCount());
				this->m_datesId->Set(index, invalid);
				KY_ASSERTK(this->m_datesId->Get(index) == invalid);
			}
			break;
		}
	}

} // namespace Kaim

#endif // KY_TWOENTITYINFO_INL
