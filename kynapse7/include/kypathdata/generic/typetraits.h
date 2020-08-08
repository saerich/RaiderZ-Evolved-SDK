/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file types.h 
	\brief Aggregates the header files that define base types and macros.
	\ingroup kypathdata_generic */

#ifndef KY_TYPETRAITS_H
#define KY_TYPETRAITS_H

#include <kypathdata/generic/types.h>

namespace Kaim
{
	// static operators

	template <bool left, bool right>
	struct Or
	{
		static const bool value = true;
	};

	template<> 
	struct Or<false, false>
	{
		static const bool value = false;
	};

	// IsIntegral

	template <typename T>
	struct IsIntegral
	{
		static const bool value = false;
	};

	template <>
	struct IsIntegral<KyUInt8>
	{
		static const bool value = true;
	};

	template <>
	struct IsIntegral<KyUInt16>
	{
		static const bool value = true;
	};

	template <>
	struct IsIntegral<KyUInt32>
	{
		static const bool value = true;
	};

#ifdef KYINT64_DEFINED
	template <>
	struct IsIntegral<KyUInt64>
	{
		static const bool value = true;
	};
#endif

	template <>
	struct IsIntegral<KyInt8>
	{
		static const bool value = true;
	};

	template <>
	struct IsIntegral<KyInt16>
	{
		static const bool value = true;
	};

	template <>
	struct IsIntegral<KyInt32>
	{
		static const bool value = true;
	};

#ifdef KYINT64_DEFINED
	template <>
	struct IsIntegral<KyInt64>
	{
		static const bool value = true;
	};
#endif

	template <>
	struct IsIntegral<bool>
	{
		static const bool value = true;
	};

	// already defined through KyUInt8
	//template <>
	//struct IsIntegral<KyBool>
	//{
	//	static const bool value = true;
	//};
	

	//template <>
	//struct IsIntegral<KyFixed>
	//{
	//	static const bool value = true;
	//};

	// IsFloat

	template <typename T>
	struct IsFloat
	{
		static const bool value = false;
	};

	template <>
	struct IsFloat<KyFloat32>
	{
		static const bool value = true;
	};

	template <>
	struct IsFloat<KyFloat64>
	{
		static const bool value = true;
	};

	// IsVoid

	template <typename T>
	struct IsVoid
	{
		static const bool value = false;
	};

	template <>
	struct IsVoid<void>
	{
		static const bool value = true;
	};

	//IsArithmetic

	template <typename T>
	struct IsArithmetic
	{
		static const bool value = Or<IsIntegral<T>::value, IsFloat<T>::value>::value;
	};

	//IsArithmetic

	template <typename T>
	struct IsFundamental
	{
		static const bool value = Or<IsArithmetic<T>::value, IsVoid<T>::value>::value;
	};


	// Optimized constructor && destructor for fundamental types

	namespace Internal
	{
		template <bool isFundamental>
		struct DefaultConstructorImpl;

		template <>
		struct DefaultConstructorImpl</*isFundamental*/false>
		{
			template <typename T>
			void operator()(T* ptr) 
			{
				new (ptr) T;
			}
		};

		template <>
		struct DefaultConstructorImpl</*isFundamental*/true>
		{
			template <typename T>
			void operator()(T* /*ptr*/) 
			{
				// no call for fundamental types
			}
		};

		// Point of customization for special cases
		template <typename T>
		struct DefaultConstructor
		{
			void operator()(T* ptr) 
			{
				DefaultConstructorImpl<IsFundamental<T>::value>()(ptr);
			}
		};

		// Destructor

		template <bool isFundamental>
		struct DestructorImpl;

		template <>
		struct DestructorImpl</*isFundamental*/false>
		{
			template <typename T>
			void operator()(T* ptr) 
			{
				static_cast<void>(ptr); // to avoid warning with classes with trivial destructors
				ptr->~T();
			}
		};

		template <>
		struct DestructorImpl</*isFundamental*/true>
		{
			template <typename T>
			void operator()(T* /*ptr*/) 
			{
				// no call for fundamental types
			}
		};

		// Point of customization for special cases
		template <typename T>
		struct Destructor
		{
			void operator()(T* ptr) 
			{
				DestructorImpl<IsFundamental<T>::value>()(ptr);
			}
		};
	}

	template <typename T>
	inline void CallDefaultConstructor(T* ptr) 
	{
		Internal::DefaultConstructor<T>()(ptr);
	}

	template <typename T>
	inline void CallDestructor(T* ptr) 
	{
		Internal::Destructor<T>()(ptr);
	};
}

#endif // KY_TYPETRAITS_H

