/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


#ifndef KYARCHIVE_H
#define KYARCHIVE_H

#include "kypathdata/generic/types.h"
#include "kypathdata/generic/cstdio.h"
#include "kypathdata/containers/staticstack.h"
#include "kypathdata/basesystem/io.h"
#include <kypathdata/containers/kystring.h>

namespace Kaim
{

/* Holds a name-value pair. */
template <class T>
struct NamedValue
{
	const char* name;
	T* value;

	NamedValue(const char* name_, T& value_) : name(name_), value(&value_)
	{
	}

	NamedValue(const NamedValue& other) : name(other.name), value(other.value)
	{
	}
};

/* Helper function to make a named-value pair. */
template <class T>
NamedValue<T> MakeNamedValue(const char* name, T& value)
{
	return NamedValue<T>(name, value);
}

/* Serializes data in a file in XML format. */
class Archive
{
public:
	/* Constructs an archive from a file. */
	explicit Archive(KyFilePtr& file_) : file(file_), indent_level(0)
	{
	}

	/* Writes a name-value pair. */
	template <class T>
	Archive& operator << (const NamedValue<T>& namedValue)
	{
		Indent();
		StartTag(namedValue.name);
		*this << *namedValue.value;
		EndTag(namedValue.name);
		EndOfLine();
		return *this;
	}

	/* Writes a string value. */
	Archive& operator << (const char* value)
	{
		if (value)
		{
			const char* start = "<![CDATA[";
			const char* end = "]]>";

			Kaim::Fwrite(start, 1, (KyUInt32)StrLen(start), file);
			Kaim::Fwrite(value, 1, (KyUInt32)StrLen(value), file);
			Kaim::Fwrite(end, 1, (KyUInt32)StrLen(end), file);
		}
		else
		{
			const char* empty = "<!--empty-->";
			Kaim::Fwrite(empty, 1, (KyUInt32)StrLen(empty), file);
		}
		return *this;
	}
#ifdef KYINT64_DEFINED
	/* Writes a KyInt64 value. */
	Archive& operator << (KyInt64 value)
	{
		char str[64];

#if defined(KY_WINDOWS_ANY)
		KY_SPRINTF(str, "%I64d", value);
#elif defined(KY_LINUX64)
		KY_SPRINTF(str, "%ld", value);
#else
		KY_SPRINTF(str, "%lld", value);
#endif // defined(KY_WINDOWS_ANY)
		Kaim::Fwrite(str, 1, (KyUInt32)StrLen(str), file);
		return *this;
	}

	/* Writes a KyUInt64 value. */
	Archive& operator << (KyUInt64 value)
	{
		char str[64];
#if defined(KY_WINDOWS_ANY)
		KY_SPRINTF(str, "%I64u", value);
#elif defined(KY_LINUX64)
		KY_SPRINTF(str, "%lu", value);
#else
		KY_SPRINTF(str, "%llu", value);
#endif // defined(KY_WINDOWS_ANY)
		Kaim::Fwrite(str, 1, (KyUInt32)StrLen(str), file);
		return *this;
	}

#endif // KYINT64_DEFINED

	/* Writes a KyInt32 value. */
	Archive& operator << (KyInt32 value)
	{
		char str[64];
		KY_SPRINTF(str, "%d", value);
		Kaim::Fwrite(str, 1, (KyUInt32)StrLen(str), file);
		return *this;
	}

	/* Writes a KyUInt32 value. */
	Archive& operator << (KyUInt32 value)
	{
		char str[64];
		KY_SPRINTF(str, "%u", value);
		Kaim::Fwrite(str, 1, (KyUInt32)StrLen(str), file);
		return *this;
	}

	/* Writes a KyFloat32 value. */
	Archive& operator << (KyFloat32 value)
	{
		char str[64];
		KY_SPRINTF(str, "%f", value);
		Kaim::Fwrite(str, 1, (KyUInt32)StrLen(str), file);
		return *this;
	}

	/* Writes a ptr_type value. */
	Archive& operator << (ptr_type value)
	{
		char str[64];
		KY_SPRINTF(str, "%p", value);
		Kaim::Fwrite(str, 1, (KyUInt32)StrLen(str), file);
		return *this;
	}

	/* Writes a StaticStack. */
	template <class T, KyUInt32 Capacity>
	Archive& operator << (const StaticStack<T, Capacity>& value)
	{
		typename StaticStack<T, Capacity>::size_type count = value.Size();
		*this << MakeNamedValue("count", count);

		for (typename StaticStack<T, Capacity>::const_iterator_type it = value.Begin(); it != value.End(); it++)
		{
			*this << MakeNamedValue("item", *it);
		}
		return *this;
	}

	/* Writes a compound value. */
	template <class T>
	Archive& operator << (const T& value)
	{
		EndOfLine();
		++indent_level;
		value.Write(*this);
		--indent_level;
		Indent();
		return *this;
	}

protected:
	/* Writes the start tag */
	void StartTag(const char* name) const
	{
		const char* start = "<";
		const char* end = ">";

		Kaim::Fwrite(start, 1, (KyUInt32)StrLen(start), file);
		Kaim::Fwrite(name, 1, (KyUInt32)StrLen(name), file);
		Kaim::Fwrite(end, 1, (KyUInt32)StrLen(end), file);
	}

	/* Writes the end tag */
	void EndTag(const char* name) const
	{
		const char* start = "</";
		const char* end = ">";

		Kaim::Fwrite(start, 1, (KyUInt32)StrLen(start), file);
		Kaim::Fwrite(name, 1, (KyUInt32)StrLen(name), file);
		Kaim::Fwrite(end, 1, (KyUInt32)StrLen(end), file);
	}

	void Indent() const
	{
		const char* tab = "  ";
		for (KyUInt32 i = 0; i < indent_level; ++i)
		{
			Kaim::Fwrite(tab, 1, (KyUInt32)StrLen(tab), file);
		}
	}

	void EndOfLine() const
	{
		const char* endl = "\n";
		Kaim::Fwrite(endl, 1, (KyUInt32)StrLen(endl), file);
	}

protected:
	KyFilePtr file;
	KyUInt32 indent_level;
};

} // namespace Kaim

#define KAIM_ARCHIVE_NAMEDVALUE(name) Kaim::MakeNamedValue(#name, name)

#endif // KYARCHIVE_H
