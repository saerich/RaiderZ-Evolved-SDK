/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file memory.h
	\brief Declaration of memory management macros and classes.
	\ingroup kypathdata_generic */

#ifndef KY_MEMORY_H
#define KY_MEMORY_H

//--------------------------------------------------------------------------------------------------
// Some people may want to define new (e.g.: #define new new(__LINE__,__FILE__))
#undef KY_NEW_DEFINITION
#ifdef new
	#define KY_NEW_DEFINITION
	#pragma message("[KYNO] You will have to undef your 'new' macro before calling KY_NEW macros")
	#pragma message("[KYNO] To do so, use pragma push_macro(\"new\"), undef new, pragma pop_macro(\"new\")")
	#pragma push_macro("new")
	#undef new
#endif

// Some people may want to define delete (e.g.: #define delete delete_FILE=__FILE__; delete_LINE=__LINE__; delete)
#undef KY_DELETE_DEFINITION
#ifdef delete
	#define KY_DELETE_DEFINITION
	#pragma message("[KYNO] You will have to undef your 'delete' macro before calling KY_DELETE macros")
	#pragma message("[KYNO] To do so, use pragma push_macro(\"delete\"), undef delete, pragma pop_macro(\"delete\")")
	#pragma push_macro("delete")
	#undef delete
#endif
//--------------------------------------------------------------------------------------------------

#include <new>
#include <stddef.h>
#include "kypathdata/generic/types.h"
#include "kypathdata/generic/helpers.h"
#ifdef KY_MEMORY_LOG_ENABLED
	#include "kypathdata/generic/callstack.h"
#endif

namespace Kaim
{
	class MemoryAllocator
	{
	public:
#ifdef KY_MEMORY_LOG_ENABLED
		static void* Allocate(size_t s, const Kaim::Call& call);
		static void DeAllocate(void* p, const Kaim::Call& call);
		static void* AllocateAligned(KyUInt32 alignement, KyUInt32 size, const Kaim::Call& call);
		static void DeAllocateAligned(void* p, const Kaim::Call& call);
#else
		static void* Allocate(size_t s);
		static void DeAllocate(void* p);
		static void* AllocateAligned(KyUInt32 alignement, KyUInt32 size);
		static void DeAllocateAligned(void* p);
#endif // KY_MEMORY_LOG_ENABLED
	};

	/* AllocTag is used to avoid calling new on Kynapse classes. Use KY_NEW_CLASS or KY_NEW_CLASS_ARRAY instead. */
	struct AllocTag {};
	inline AllocTag GetAllocTag() { return AllocTag(); }
}

	#ifdef KY_MEMORY_LOG_ENABLED
		//--------------------------------------------------------------------------------------------
		// macros for built-in types - no constructor or destructor called
		/*! Invokes the IMemoryBridge to allocate memory on the heap for a new instance of a type. No constructor is called. \ingroup kypathdata_generic */
		#define KY_MALLOC(T, n) ((T*)Kaim::MemoryAllocator::Allocate((KyUInt32)((n) * sizeof(T)), Kaim::Call("KY_MALLOC("#T", "#n")", __FILE__, __LINE__)))
		/*! Invokes the IMemoryBridge to free memory on the heap for the specified object. No destructor is called. \ingroup kypathdata_generic */
		#define KY_FREE(p) do { Kaim::MemoryAllocator::DeAllocate(p, Kaim::Call()); } while (p != p)
		/*! Invokes the IMemoryBridge to allocate aligned memory on the heap for a new instance of a type. No constructor is called. \ingroup kypathdata_generic */
		#define KY_MALLOC_ALIGNED(T, n, alignement) ((T*)Kaim::MemoryAllocator::AllocateAligned(alignement,(KyUInt32)((n) * sizeof(T)), Kaim::Call("KY_MALLOC_ALIGNED("#T", "#n", "#alignement")", __FILE__, __LINE__)))
		/*! Invokes the IMemoryBridge to allocate a buffer of aligned memory on the heap. \ingroup kypathdata_generic */
		#define KY_MALLOC_SIZED_ALIGNED(size, alignement) (Kaim::MemoryAllocator::AllocateAligned(alignement,size, Kaim::Call("KY_MALLOC_SIZED_ALIGNED("#size", "#alignement")", __FILE__, __LINE__)))
		/*! Invokes the IMemoryBridge to free aligned memory on the heap for the specified object. No destructor is called. \ingroup kypathdata_generic */
		#define KY_FREE_ALIGNED(p) do { Kaim::MemoryAllocator::DeAllocateAligned(p, Kaim::Call()); } while (p != p)

		//--------------------------------------------------------------------------------------------
		// macros for classes that use KY_NEW_DELETE_OPERATORS
		// IsAManagedObjectPtr() is here to avoid calling these macros on a built-in type or a non kynapse class
		/*! Invokes the IMemoryBridge to allocate memory on the heap for a new instance of a \SDKName class. \ingroup kypathdata_generic */
		#define KY_NEW_CLASS(T) new(Kaim::Call("KY_NEW_CLASS("#T")", __FILE__, __LINE__)) T
		/*! Invokes the IMemoryBridge to allocate memory on the heap for an array of new instances of a \SDKName class. \ingroup kypathdata_generic */
		#define KY_NEW_CLASS_ARRAY(T,n) new(Kaim::Call("KY_NEW_CLASS_ARRAY("#T", "#n")", __FILE__, __LINE__)) T[n]
		/*! Invokes the IMemoryBridge to de-allocate memory on the heap for the specified \SDKName object. \ingroup kypathdata_generic */
		#define KY_DELETE_CLASS(p) do { Kaim::CheckManagedPtr(p); delete (p); } while (p != p)
		/*! Invokes the IMemoryBridge to de-allocate memory on the heap for the specified array of \SDKName objects. \ingroup kypathdata_generic */
		#define KY_DELETE_CLASS_ARRAY(p) do { Kaim::CheckManagedPtr(p); delete[] (p); } while (p != p)

		//--------------------------------------------------------------------------------------------
		// macros for non-Kynapse classes that need construction/destruction
		// Requires exact class for destruction
		/*! Invokes the IMemoryBridge to allocate memory on the heap for a new instance of a class external to the \SDKName SDK. \ingroup kypathdata_generic */
		#define KY_NEW_EXTERN_CLASS(T) new(Kaim::MemoryAllocator::Allocate(sizeof(T),Kaim::Call("KY_NEW_EXTERN_CLASS("#T")", __FILE__, __LINE__))) T
		/*! Invokes the IMemoryBridge to de-allocate memory on the heap for a specified object that is of a class external to the
			\SDKName SDK. Requires the exact class of the object to destroy. \ingroup kypathdata_generic */
		#define KY_DELETE_EXTERN_CLASS(T,p) if (p) { p->~T(); Kaim::MemoryAllocator::DeAllocate(p, Kaim::Call()); }

	#elif defined(__SPU__)

		#define KY_MALLOC(T, n) ((T*)malloc(sizeof(T)*n))
		#define KY_FREE(p) do { free(p); } while (p != p)
		#define KY_NEW_CLASS(T) new T
		#define KY_NEW_CLASS_ARRAY(T,n) new T[n]
		#define KY_DELETE_CLASS(T) delete T
		#define KY_DELETE_CLASS_ARRAY(T) delete[] T

	#else //KY_MEMORY_LOG_ENABLED
		//--------------------------------------------------------------------------------------------
		// macros for built-in types - no constructor or destructor called
		#define KY_MALLOC(T, n) ((T*)Kaim::MemoryAllocator::Allocate((KyUInt32)((n) * sizeof(T))))
		#define KY_FREE(p) do { Kaim::MemoryAllocator::DeAllocate(p); } while (p != p)
		#define KY_MALLOC_ALIGNED(T, n, alignement) ((T*)Kaim::MemoryAllocator::AllocateAligned(alignement,(KyUInt32)((n) * sizeof(T))))
		#define KY_MALLOC_SIZED_ALIGNED(size, alignement) (Kaim::MemoryAllocator::AllocateAligned(alignement,size))
		#define KY_FREE_ALIGNED(p) do { Kaim::MemoryAllocator::DeAllocateAligned(p); } while (p != p)

		//--------------------------------------------------------------------------------------------
		// macros for classes that use KY_NEW_DELETE_OPERATORS
		// IsAManagedObjectPtr() is here to avoid calling these macros on a built-in type or a non kynapse class
		#define KY_NEW_CLASS(T) new(Kaim::GetAllocTag()) T
		#define KY_NEW_CLASS_ARRAY(T, n) new(Kaim::GetAllocTag()) T[n]
		#define KY_DELETE_CLASS(p) do { Kaim::CheckManagedPtr(p); delete (p); } while (p != p)
		#define KY_DELETE_CLASS_ARRAY(p) do { Kaim::CheckManagedPtr(p); delete[] (p); } while (p != p)

		//--------------------------------------------------------------------------------------------
		// macros for non-Kynapse classes that need construction/destruction
		// Requires exact class for destruction
		#define KY_NEW_EXTERN_CLASS(T) new(Kaim::MemoryAllocator::Allocate(sizeof(T))) T
		#define KY_DELETE_EXTERN_CLASS(T,p) if (p) { p->~T(); Kaim::MemoryAllocator::DeAllocate(p); }

	#endif //KY_MEMORY_LOG_ENABLED


/*! This macro defines new and delete operators that invoke the IMemoryBridge interface provided by the
	Bridge in order to allocate and free memory. For internal use in the declarations of classes that
	do not derive from VirtualMemObject.
	\ingroup kypathdata_generic */
	
#ifdef KY_MEMORY_LOG_ENABLED
	#define KY_DEFINE_NEW_DELETE_OPERATORS \
		public: \
			static KyBool IsAManagedObjectPtr() { return KY_TRUE; }  \
			void* operator new(size_t s, const Kaim::Call& call) { return Kaim::MemoryAllocator::Allocate(s, call); } \
			void* operator new[](size_t s, const Kaim::Call& call) {  return Kaim::MemoryAllocator::Allocate(s, call); } \
			void* operator new(size_t , void* placement_new) { return placement_new; } \
			void* operator new[](size_t , void* placement_new) {  return placement_new; } \
			void operator delete(void* p, size_t ) { Kaim::MemoryAllocator::DeAllocate(p, Kaim::Call()); } \
			void operator delete[](void* p, size_t ) { Kaim::MemoryAllocator::DeAllocate(p, Kaim::Call()); } \
			/* not used : keep to remove compiler warning*/ void  operator delete(void* /* p */, void* /* placement_new */) {} \
			/* not used : keep to remove compiler warning*/ void  operator delete[](void* /* p */, void* /* placement_new */) {} \
			/* not used : keep to remove compiler warning*/ void  operator delete(void* p, const Kaim::Call& call) { Kaim::MemoryAllocator::DeAllocate(p, call); }  \
			/* not used : keep to remove compiler warning*/ void  operator delete[](void* p, const Kaim::Call& call) { Kaim::MemoryAllocator::DeAllocate(p, call); }  \
		private: \
			/*do not mix standard new and logging delete*/ void* operator new(size_t s);     \
			/*do not mix standard new and logging delete*/ void* operator new[](size_t s);   \
		private:

#elif defined(__SPU__)
	#define KY_DEFINE_NEW_DELETE_OPERATORS

#else // !KY_MEMORY_LOG_ENABLED...
	#define KY_DEFINE_NEW_DELETE_OPERATORS \
		public: \
			static KyBool IsAManagedObjectPtr() { return KY_TRUE; }  \
			void* operator new(size_t s, Kaim::AllocTag)    { return Kaim::MemoryAllocator::Allocate(s); } \
			void* operator new[](size_t s, Kaim::AllocTag)  { return Kaim::MemoryAllocator::Allocate(s); } \
			void* operator new(size_t , void* placement_new) { return placement_new; } \
			void* operator new[](size_t , void* placement_new) {  return placement_new; } \
			void operator delete(void* p, size_t )           { Kaim::MemoryAllocator::DeAllocate(p); } \
			void operator delete[](void* p, size_t )         { Kaim::MemoryAllocator::DeAllocate(p); } \
			/* not used : keep to remove compiler warning*/ void  operator delete(void* /* p */, void* /* placement_new */) {}  \
			/* not used : keep to remove compiler warning*/ void  operator delete[](void* /* p */, void* /* placement_new */) {}  \
			/* not used : keep to remove compiler warning*/ void operator delete(void* p, Kaim::AllocTag)   { Kaim::MemoryAllocator::DeAllocate(p); } \
			/* not used : keep to remove compiler warning*/ void operator delete[](void* p, Kaim::AllocTag) { Kaim::MemoryAllocator::DeAllocate(p); } \
		private: \
			/*do not mix standard new and logging delete*/ void* operator new(size_t s);     \
			/*do not mix standard new and logging delete*/ void* operator new[](size_t s);   \
		private:

#endif // KY_MEMORY_LOG_ENABLED

namespace Kaim
{


/*! VirtualMemObject is the base class for all Kynapse classes that have a virtual destructor.
	It redefines the new and delete operators to invoke the IMemoryBridge interface
	in order to allocate and de-allocate memory for instances of the class.
	\ingroup kypathdata_generic	*/
class VirtualMemObject
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	/*!	\label_baseconstructor */
	VirtualMemObject() {}

	/*!	\label_virtualdestructor */
	virtual ~VirtualMemObject() {}
};


template <class T>
void CheckManagedPtr(T*)
{
	AvoidWarningForUnusedVariable(T::IsAManagedObjectPtr);
}

}

#ifdef KY_PS3
	#ifdef KY_MEMORY_LOG_ENABLED
		inline void* KyMemAlign(KyUInt32 alignement, KyUInt32 size) { return  Kaim::MemoryAllocator::AllocateAligned(alignement,size, Kaim::Call("KyMemAlign", __FILE__, __LINE__)); }
		inline void KyFreeAlign(void* p) {return Kaim::MemoryAllocator::DeAllocateAligned(p, Kaim::Call());}
	#else // KY_MEMORY_LOG_ENABLED
		inline void* KyMemAlign(KyUInt32 alignement, KyUInt32 size) { return  Kaim::MemoryAllocator::AllocateAligned(alignement,size); }
		inline void KyFreeAlign(void* p) { return Kaim::MemoryAllocator::DeAllocateAligned(p); }
	#endif // KY_MEMORY_LOG_ENABLED
#endif //KY_PS3


//--------------------------------------------------------------------
// Restore user defined new/delete definitions
#ifdef KY_NEW_DEFINITION
	#pragma pop_macro("new")
#endif
#ifdef KY_DELETE_DEFINITION
	#pragma pop_macro("delete")
#endif


#endif
