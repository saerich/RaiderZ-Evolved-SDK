/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*! \file kypathdata/blob/endianness.h
	\ingroup kypathdata_blob */

// primary contact: GUAL - secondary contact: NOBODY
#ifndef KyPathData_Endianness_H
#define KyPathData_Endianness_H

#include "kypathdata/generic/types.h"

namespace Kaim
{


/*! Collects endianness types and utilities used in the blob serialization framework.
	\ingroup kypathdata_blob */
class Endianness // class with static functions only
{
public:
	/*! Enumerates the possible types of endianness. */ 
	enum Type
	{
		KY_LITTLE_ENDIAN = 0, /*!< Little-endian format (e.g. Windows, Linux). */
		KY_BIG_ENDIAN = 1, /*!< Big-endian format (e.g. PlayStation 3, Xbox 360). */
		Type_FORCE32 = 0xFFFFFFFF
	};

	/*! Enumerates the possible types of endianness relative to the current platform. */ 
	enum Target
	{
		SYSTEM_ENDIANNESS = 0, /*!< The same type of endianness as the current platform. */
		INVERSE_SYSTEM_ENDIANNESS = 1, /*!< The opposite type of endianness from the current platform. */
		Target_FORCE32 = 0xFFFFFFFF
	};

	/*! Retrieves the endianness of the current platform. */ 
	static Type GetSystemEndianness()
	{
#if defined(KY_XBOX360)
		return KY_BIG_ENDIAN;
#elif defined(KY_PS3)
		return KY_BIG_ENDIAN;
#elif defined(KY_WII)
		return KY_BIG_ENDIAN;
#else
		return KY_LITTLE_ENDIAN;
#endif
	}

	/*! Retrieves the endianness opposite to that of the current platform. */ 
	static Type GetInverseSystemEndianness()
	{
#if defined(KY_XBOX360)
		return KY_LITTLE_ENDIAN;
#elif defined(KY_PS3)
		return KY_LITTLE_ENDIAN;
#elif defined(KY_WII)
		return KY_LITTLE_ENDIAN;
#else
		return KY_BIG_ENDIAN;
#endif
	}

	/*! Retrieves the endianness opposite to the specified type. */ 
	static Type GetInverseEndianness(Type endianness)
	{
		return (endianness == KY_BIG_ENDIAN) ? KY_LITTLE_ENDIAN : KY_BIG_ENDIAN;
	}

	/*! Retrieves the endianness from a 4-byte memory location. */
	static Type GetEndiannessFromMem32(const KyUInt32* mem)
	{
		return (*mem == 0) ? KY_LITTLE_ENDIAN : KY_BIG_ENDIAN;
	}

	/*! Sets a 4-byte memory location to the specified endianness. */
	static void SetEndiannessToMem32(Type endianness, KyUInt32* mem)
	{
		*mem = 0;
		if (endianness == KY_BIG_ENDIAN)
			*((char*)mem) = 1; // set first byte to 1 whatever the endianness is
	}

	/*! Swaps the endianness in a 4-byte memory location. */
	static void SwapEndiannessInMem32(KyUInt32* mem)
	{
		Type endianness = GetEndiannessFromMem32(mem);
		Type inverseEndianness = GetInverseEndianness(endianness);
		SetEndiannessToMem32(inverseEndianness, mem);
	}

	/*! Swaps the endianness of the data in a 16-bit buffer. */
	static inline void Swap16(void* x)
	{
		(*(KyUInt16*)x) =
			( (*(KyUInt16*)x) >> 8 ) |
			( (*(KyUInt16*)x) << 8 );
	}

	/*! Swaps the endianness of the data in a 32-bit buffer. */
	static inline void Swap32(void* x)
	{
		(*(KyUInt32*)x) =
			( ( (*(KyUInt32*)x) >> 24 )              ) |
			( ( (*(KyUInt32*)x) <<  8 ) & 0x00FF0000 ) |
			( ( (*(KyUInt32*)x) >>  8 ) & 0x0000FF00 ) |
			( ( (*(KyUInt32*)x) << 24 )              );
	}
};

/*! \label_swapendianness \ingroup kypathdata_blob */
inline void SwapEndianness(Endianness::Target, KyInt32&  x ) { Kaim::Endianness::Swap32(&x); }

/*! \label_swapendianness \ingroup kypathdata_blob */
inline void SwapEndianness(Endianness::Target, KyUInt32& x ) { Kaim::Endianness::Swap32(&x); }

/*! \label_swapendianness \ingroup kypathdata_blob */
inline void SwapEndianness(Endianness::Target, KyInt16&  x ) { Kaim::Endianness::Swap16(&x); }

/*! \label_swapendianness \ingroup kypathdata_blob */
inline void SwapEndianness(Endianness::Target, KyUInt16& x ) { Kaim::Endianness::Swap16(&x); }

/*! \label_swapendianness \ingroup kypathdata_blob */
inline void SwapEndianness(Endianness::Target, KyInt8&     ) {}

/*! \label_swapendianness \ingroup kypathdata_blob */
inline void SwapEndianness(Endianness::Target, KyUInt8&    ) {}

/*! \label_swapendianness \ingroup kypathdata_blob */
inline void SwapEndianness(Endianness::Target, char&       ) {}

/*! \label_swapendianness \ingroup kypathdata_blob */
inline void SwapEndianness(Endianness::Target, KyFloat32& x) { Kaim::Endianness::Swap32(&x); }


}


#endif
