/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*! \file
	\ingroup kypathdata_generic */

#ifndef KYGUID_H
#define KYGUID_H

#include <kypathdata/generic/basetypes.h>
#include <kypathdata/generic/memory.h>
#include <kypathdata/blob/endianness.h>

#include <string.h> // memcmp
#include <kypathdata/basesystem/logger.h>


#if defined(KY_WINDOWS_ANY) // || defined(KY_LINUX_ANY)

#define GUID_GENERATION_SUPPORTED

#endif


namespace Kaim
{

/*! The KyGuid class represents a globally unique ID. It is used by the PathData generation framework to uniquely
	identify each different sector treated by the Generator.
	\ingroup kypathdata_generic */
class KyGuid
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	KyGuid();
	explicit KyGuid(const char guidString[/*37*/]);
	KyGuid(const KyGuid& rhs) { *this = rhs; }
	explicit KyGuid(KyUInt8 values[16]);
	explicit KyGuid(KyUInt16 values[8]);
	explicit KyGuid(KyUInt32 values[4]);
	explicit KyGuid(KyUInt64 values[2]);

	bool operator < (const KyGuid& rhs) const { return memcmp(m_uuid, rhs.m_uuid, sizeof (m_uuid)) < 0; }
	bool operator <= (const KyGuid& rhs) const { return memcmp(m_uuid, rhs.m_uuid, sizeof (m_uuid)) <= 0; }
	bool operator > (const KyGuid& rhs) const { return !operator<=(rhs); }
	bool operator >= (const KyGuid& rhs) const { return !operator<(rhs); }
	bool operator == (const KyGuid& rhs) const { return memcmp(m_uuid, rhs.m_uuid, sizeof (m_uuid)) == 0; }
	bool operator != (const KyGuid& rhs) const { return !operator==(rhs); }

	KyGuid& operator = (const KyGuid& rhs) { memcpy(m_uuid, rhs.m_uuid, sizeof (m_uuid)); return *this; }

	/*! Indicates whether or not the specified KyGuid is valid. */
	bool IsValid() const { return *this != s_invalidGuid; }

	void ToString(char guidString[/*37*/]) const;
	void InitFromString(const char guidString[/*37*/]);
	static const KyGuid s_defaultGuid; /*!< Represents the default KyGuid value. */
	static const KyGuid s_invalidGuid; /*!< Represents an invalid KyGuid value. */

	/*! Regenerates a new value for this object that is guaranteed to be unique. Implemented only on Windows platforms. */
	void Regenerate() { Generate(*this); }
public:
	/*! Retrieves a new KyGuid that is guaranteed to be unique. Implemented only on Windows platforms. */
	static KyGuid Generate()
	{
#ifdef GUID_GENERATION_SUPPORTED
		KyGuid res; Generate(res); return res;
#else
		KY_FUNCTION("KyGuid::Generate");
		KY_RELEASE_MESSAGE(KY_LOG_WARNING, ("Guid is generation is not supported on Linux. Please provide a Guid."));
		return s_defaultGuid;
#endif
	}
protected:
	static void Generate(KyGuid& kyGuid);

protected:
	KyUInt8 m_uuid[16];
};

bool IsValidGuidString(const char guidStr[/*37*/]);
inline void SwapEndianness(Endianness::Target /*e*/, KyGuid& /*self*/)
{
	//no swap needed on guid
}

} // namespace Kaim

#endif
