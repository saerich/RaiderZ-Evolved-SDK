/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

// primary contact: ? - secondary contact: NOBODY
#ifndef KYSTRINGSTREAM_H
#define KYSTRINGSTREAM_H

#include <kypathdata/containers/kystring.h>

#include <kypathdata/generic/kyguid.h>
#include <kypathdata/generic/cstdio.h>

namespace Kaim
{
class Vec3f;

class StringStream
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	typedef KyString::SizeType SizeType;

public:
	StringStream();
	explicit StringStream(KyString& buffer);

public:
	/* Insert data with format. */
	StringStream& operator<<(KyInt32 value);

	/* Insert data with format. */
	StringStream& operator<<(KyUInt32 value);

	/* Insert data with format. */
	StringStream& operator<<(KyFloat32 value);

	/* Insert data with format. */
	StringStream& operator<<(KyFloat64 value);

	/* Insert data with format. */
	StringStream& operator<<(bool value);

	/* Insert data with format. */
	StringStream& operator<<(const char* value);

	/* Insert data with format. */
	StringStream& operator<<(const KyString& value);

	/* Insert data with format. */
	StringStream& operator<<(char value);

	/* Insert formatter. */
	StringStream& operator<<(StringStream& (*pf)(StringStream&));

	const KyString& Str() const;

public:
	void Clear();

	void Terminate();

public:
	KyResult Append(const char* value, SizeType length);

	KyResult Append(char value);

private:
	KyResult Reserve(SizeType size);

private:
	template <typename T>
	KyResult Append(T value, const char* format);

	template <typename T>
	static SizeType MaxDigits(T /*value*/);

private:
	StringStream(const StringStream&);
	StringStream& operator=(const StringStream&);

private:
	KyString m_localBuffer;
	KyString& m_buffer;
};

template <typename T>
KyResult StringStream::Append(T value, const char* format)
{
	const SizeType max_size = MaxDigits(value);

	KY_FORWARD_ERROR_NO_LOG(Reserve(m_buffer.Size() + max_size));

	const KyInt32 count = KY_SNPRINTF(m_buffer.End(), max_size, format, value);
	if (count >= 0)
	{
		KY_FORWARD_ERROR_NO_LOG(m_buffer.Resize(m_buffer.Size() + count));
	}

	return Kaim::Result::Success;
}

template <typename T>
StringStream::SizeType StringStream::MaxDigits(T /*value*/)
{ 
	return sizeof(T) << 2; // sizeof * 4 : e.g. 16 digits max for a KyInt32 or a KyFloat32
}

/* Insert end of line. */
StringStream& Endl(StringStream& stream);

/* Insert end of string. */
StringStream& Ends(StringStream& stream);

/* Insert formatted Vec3f. */
StringStream& operator<<(StringStream& stream, const Vec3f& value);

/* Insert formatted quid. */
StringStream& operator<<(StringStream& stream, const KyGuid& value);
}


#endif // KYSTRINGSTREAM_H
