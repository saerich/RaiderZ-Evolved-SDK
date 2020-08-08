/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\brief Contains some helper classes and functions.
	\ingroup kypathdata_generic */

#include <kypathdata/generic/basetypes.h>
#include <kypathdata/generic/macros.h>

#ifndef KYHELPERS
#define KYHELPERS

#ifndef __GCCXML__

#if defined(KY_XBOX360)
#include <ppcintrinsics.h>
#endif

#if defined(KY_PS3)
#if defined(__PPU__)
#include <ppu_intrinsics.h>
#else
#include <math.h>
#endif
#endif

#endif // __GCCXML__

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//	min/max
///////////////////////////////////////////////////////////////////////////////////////////////////////////


namespace Kaim
{

#if defined(KY_XBOX360) && !defined(__GCCXML__)
KY_FORCE_INLINE KyFloat32 Fsel(KyFloat32 cmp, KyFloat32 v1, KyFloat32 v2)
{
	return (KyFloat32)__fsel(cmp, v1, v2);
}
#elif defined(KY_PS3) && defined(__PPU__) && !defined(__GCCXML__)
KY_FORCE_INLINE KyFloat32 Fsel(KyFloat32 cmp, KyFloat32 v1, KyFloat32 v2)
{
	return __fsels(cmp, v1, v2);
}
#elif defined(KY_WII) && !defined(__GCCXML__)
inline float Fsel(KyFloat32 cmp, KyFloat32 v1, KyFloat32 v2)
{
	return (KyFloat32)__fsel(cmp, v1, v2);
}
#else
/*! If the first argument is greater than 0, returns \c v1. Otherwise, returns \c v2.
	\ingroup kypathdata_generic */
KY_FORCE_INLINE KyFloat32 Fsel(KyFloat32 cmp, KyFloat32 v1, KyFloat32 v2)
{
	return cmp >= 0.0f ? v1 : v2;
}
#endif

/*!	Returns the lesser of the two specified values.
	\tparam T		The type of values provided.
	\ingroup kypathdata_generic */
template<typename T>
KY_FORCE_INLINE T Min(const T& a, const T& b)
{
	return (a < b) ? a : b;
}

/*!	Returns the greater of the two specified values.
	\tparam T		The type of values provided.
	\ingroup kypathdata_generic */
template<typename T>
KY_FORCE_INLINE T Max(const T& a, const T& b)
{
	return (a > b) ? a : b;
}

template <typename T>
KY_FORCE_INLINE T Min(const T& a, const T& b, const T& c)
{
	return Min(Min(a, b), c);
}

template <typename T>
KY_FORCE_INLINE T Max(const T& a, const T& b, const T& c)
{
	return Max(Max(a, b), c);
}

#if defined(KY_XBOX360) || defined(KY_PS3) || defined(KY_WII)

template<>
KY_FORCE_INLINE KyFloat32 Min(const KyFloat32& a, const KyFloat32& b)
{
	return Fsel(a - b, b, a);
}

template<>
KY_FORCE_INLINE KyFloat32 Max(const KyFloat32& a, const KyFloat32& b)
{
	return Fsel(a - b, a, b);
}

#endif

template<typename T>
struct remove_unsigned
{
	typedef T type;
};

template<>
struct remove_unsigned<KyUInt8>
{
	typedef KyInt8 type;
};
template<>
struct remove_unsigned<KyUInt16>
{
	typedef KyInt16 type;
};
template<>
struct remove_unsigned<KyUInt32>
{
	typedef KyInt32 type;
};

#ifdef KY_64BIT_PLATFORM
template<>
struct remove_unsigned<KyUInt64>
{
	typedef KyInt64 type;
};
#endif

/*! Branchless version of (c ? a : b), but a and b are computed. 
	\tparam T		The type of all three values.
	\ingroup kypathdata_generic */
template <typename T>
inline T Cond(bool c, T a, T b)
{
	typedef typename remove_unsigned<T>::type signedType;

	return static_cast<T>(b + (-signedType(c) & (a - b)));
}

/*!	Returns the size of a fixed-size array.
	\ingroup kypathdata_generic */
template<typename T, KyUInt32 N>
KyUInt32 ArraySize(T (&) [N]) { return N; }

/*!	Indicates whether or not the specified value is false.
	\ingroup kypathdata_generic */
KY_FORCE_INLINE bool IsFalse(bool t) { return !t; }

/* Used to avoid warning (warning(4189) : local variable is initialized but not referenced. )
		in release when using some variable only in debug macro (KY_MESSAGE, KY_ASSERTK). */
template <typename T>
void AvoidWarningForUnusedVariable(const T&) {}


} // namespace Kaim

#endif //KYHELPERS
