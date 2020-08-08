/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*	\file allocatoradapter.h
	Contains the definition of a system used internally to allocate memory for STL components
	through the IMemoryBridge interface.
	\ingroup kypathdata_basesystem */

#ifndef KYALLOCATORADAPTER_H
#define KYALLOCATORADAPTER_H

#include <stddef.h> /* size_t, ptrdiff_t */

#include <kypathdata/basesystem/basesystem.h>

namespace Kaim
{
	namespace details
	{
		template<class T> 
		inline T* Allocate(size_t count, T*)
		{
			return static_cast<T*>(Kaim::GetBaseSystem().Allocate((KyUInt32)count * sizeof(T)));
		}

		template<class T> 
		void Deallocate(T* ptr)
		{	
			// deallocate object at ptr
			Kaim::GetBaseSystem().DeAllocate(ptr);
		}

		template<class T> 
		void Deallocate(T* ptr, size_t)
		{	
			Deallocate(ptr);
		}

		template<class T> 
		inline T* LogAllocate(size_t count, T*)
		{
		#ifdef KY_MEMORY_LOG_ENABLED
			return static_cast<T*>(Kaim::GetBaseSystem().Allocate((KyUInt32)count * sizeof(T), Kaim::Call("allocator_adapter<>::allocate", __FILE__, __LINE__)));
		#else // KY_MEMORY_LOG_ENABLED
			return Allocate(count, ((T*)0));
		#endif // KY_MEMORY_LOG_ENABLED
		}

		template<class T> 
		void LogDeallocate(T* ptr, size_t)
		{	
			// deallocate object at ptr
		#ifdef KY_MEMORY_LOG_ENABLED
			Kaim::GetBaseSystem().DeAllocate(ptr, Kaim::Call("allocator_adapter<>::deallocate", __FILE__, __LINE__));
		#else // KY_MEMORY_LOG_ENABLED
			Deallocate(ptr);
		#endif// KY_MEMORY_LOG_ENABLED
		}
	}

	template<class T>
	struct allocator_adapter
	{
	public:
		typedef T value_type;
		typedef T*			pointer;
		typedef T&			reference;
		typedef const T*	const_pointer;
		typedef const T&	const_reference;

		typedef size_t				size_type;
		typedef ptrdiff_t			difference_type;

		template<class Other>
		struct rebind
		{	// convert an allocator_adapter<T> to an allocator_adapter<Other>
			typedef allocator_adapter<Other> other;
		};

		pointer address(reference value) const
		{	// return address of mutable value
			return (&value);
		}

		const_pointer address(const_reference value) const
		{	// return address of nonmutable value
			return (&value);
		}

		allocator_adapter()
		{	// construct default allocator_adapter (do nothing)
		}

		allocator_adapter(const allocator_adapter<T>&)
		{	// construct by copying (do nothing)
		}

		template<class Other>
		allocator_adapter(const allocator_adapter<Other>&)
		{	// construct from a related allocator_adapter (do nothing)
		}

		template<class Other>
		allocator_adapter<T>& operator=(const allocator_adapter<Other>&)
		{	// assign from a related allocator_adapter (do nothing)
			return (*this);
		}

		void deallocate(pointer ptr, size_type count)
		{	// deallocate object at ptr
			details::LogDeallocate(ptr, count);
		}

		pointer allocate(size_type count)
		{	// allocate array of count elements
			return details::LogAllocate(count, ((pointer)0));
		}

		pointer allocate(size_type count, const void  *)
		{	// allocate array of count elements, ignore hint
			return (allocate(count));
		}

		void construct(pointer ptr, const T& value)
		{	// construct object at ptr with value value
			::new (ptr) T(value);
		}

		void destroy(pointer ptr)
		{	// destroy object at ptr
			static_cast<void>(ptr); // to avoid warning
			ptr->~T();
		}

		size_type max_size() const 
		{	// estimate maximum array size
			size_type count = (size_type)(-1) / sizeof (T);
			return (0 < count ? count : 1);
		}
	};

	template<class T,	class Other> 
	inline bool operator==(const allocator_adapter<T>&, const allocator_adapter<Other>&) 
	{	
		return true;
	}

	template<class T,	class Other> 
	inline bool operator!=(const allocator_adapter<T>&, const allocator_adapter<Other>&) 
	{	
		return false;
	}

	template<class T>
	struct allocator_adapter_nolog : allocator_adapter<T>
	{
	public:
		typedef T value_type;
		typedef T*			pointer;
		typedef T&			reference;
		typedef const T*	const_pointer;
		typedef const T&	const_reference;

		typedef size_t				size_type;
		typedef ptrdiff_t			difference_type;

		template<class Other>
		struct rebind
		{	// convert an allocator_adapter_nolog<T> to an allocator_adapter_nolog<Other>
			typedef allocator_adapter_nolog<Other> other;
		};

		allocator_adapter_nolog()
		{	// construct default allocator_adapter_nolog (do nothing)
		}

		allocator_adapter_nolog(const allocator_adapter_nolog<T>&)
		{	// construct by copying (do nothing)
		}

		template<class Other>
		allocator_adapter_nolog(const allocator_adapter_nolog<Other>&)
		{	// construct from a related allocator_adapter (do nothing)
		}

		template<class Other>
		allocator_adapter_nolog<T>& operator=(const allocator_adapter_nolog<Other>&)
		{	// assign from a related allocator_adapter (do nothing)
			return (*this);
		}

		void deallocate(pointer ptr, size_type count)
		{	// deallocate object at ptr
			details::Deallocate(ptr, count);
		}

		pointer allocate(size_type count)
		{	// allocate array of count elements
			return details::Allocate(count, ((pointer)0));
		}
	};

	template<class T,	class Other> 
	inline bool operator==(const allocator_adapter_nolog<T>&, const allocator_adapter_nolog<Other>&) 
	{	
		return true;
	}

	template<class T,	class Other> 
	inline bool operator!=(const allocator_adapter_nolog<T>&, const allocator_adapter_nolog<Other>&) 
	{	
		return false;
	}

} // namespace Kaim


#endif // KYALLOCATORADAPTER_H
