/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_core */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_METACLASS_H
#define KY_METACLASS_H

#include <kyruntime/core/global.h>

#include <kypathdata/generic/macros.h>
#include <kypathdata/generic/memory.h>
#include <kypathdata/basesystem/logger.h>

#include <string.h>


#if defined(_MSC_VER)
#pragma warning (disable:4786) // Name truncation to 255 chars in debug mode.
#endif

//	The maximum number of instances of each MetaClass that can be registered. For internal use.
#define KYNO_MAX_METACLASS_COUNT 64

namespace Kaim
{
	/*	This class is used internally by the \SDKName object factory. It provides some high-level
		methods for \MetaClasses, inherited by MetaClass. You should not need to use this class directly. 
		\ingroup kyruntime_core */
	class MetaClassBase
	{
	public:
		struct Repository
		{
			static const KyInt32 max_size =  KYNO_MAX_METACLASS_COUNT * 60;

			typedef MetaClassBase** iterator;

			Repository() : m_size(0) {}

			void PushBack(MetaClassBase* pMetaClass) { m_array[m_size++] = pMetaClass; }
			KyInt32 Size() const { return m_size; }
			iterator Begin() { return m_array; }
			iterator End() { return m_array + m_size; }

		protected:
			MetaClassBase* m_array[max_size];
			KyInt32	m_size;
		};

	public:
		static Repository& GetRepositoryInstance()
		{
			static Repository _instance;
			return _instance;
		}

	public:
		const char* GetName() const { return m_name; }
		const void* Vptr() const { return m_vptr; }

	public:
		MetaClassBase() : m_vptr(0) {}

	public:	
		void RegisterVirtualTable(void* vptr)
		{
			if (m_vptr == 0)
			{
				m_vptr = vptr;
				GetRepositoryInstance().PushBack(this);
			}
		}


	protected:
		const char* m_name;
		void* m_vptr;
	};

	#define KY_REGISTER_METACLASS_VPTR(ky_class, ky_vptr) ky_class.RegisterVirtualTable(ky_vptr)

/*!	MetaClass is a template for an object that allows the \SDKName object factory to create
	a new instance of a class given only the class name, and to test the class inheritance of objects at runtime. 
	\pp You should not have to deal with MetaClass objects directly. Each time you create a class for a new
	\SDKName component, such as a Bot, WorldService or PathObject, you must include specific macros in the declaration
	and implementation of your new class. These macros take care of setting up the MetaClass for your
	component. The MetaClass is then used automatically by the \SDKName object factory when it needs to create an
	instance of your class.
	\pp You can use the MetaClass to determine whether or not an object is an instance of a desired class.
	Access the MetaClass for the object through its GetClass() method or its Class member, and use the IsA() method of the MetaClass to 
	test whether or not the MetaClass matches the MetaClass of the class you want to test. For example:
	\code
	Kaim::ICanGo* cangoModifier = m_PathFinder->GetCanGoModifier();
	if (cangoModifier->GetClass().IsA(Kaim::CanGo_NavMesh::Class))
	{
	   Kaim::CanGo_NavMesh* canGoNavMesh = 
										static_cast<Kaim::CanGo_NavMesh*>(cangoModifier);
	   ...
	} \endcode
	\ingroup kyruntime_core */
template <typename TheClass, typename Ctor>
class MetaClass : public MetaClassBase
{
public:
	typedef TheClass Cls;
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	/*! For internal use only. */
	static KyBool Register(MetaClass& cls);
	/*! For internal use only. */
	static void UnRegisterAll();
	/*! For internal use only. */
	static MetaClass* GetClassByName(const char* name);
	/*! For internal use only. */
	KyInt32 m_id;

public:
	/*!	\label_constructor For internal use. */
	MetaClass(
		const char* className, 
		Ctor ctr, 
		MetaClass* baseClass = (MetaClass*) NULL);
		
	/*!	\label_virtualdestructor For internal use. */
	virtual ~MetaClass();

	/*!	Indicates whether or not two MetaClass objects are identical. */
	KyBool operator==(const MetaClass& cls) const { return &cls == this; }
	
	/*!	Indicates whether or not two MetaClass objects are different. */
	KyBool operator!=(const MetaClass& cls) const { return !(*this == cls); }

	/*!	Indicates whether or not this MetaClass is an instance of a specified MetaClass or inherits the specified MetaClass.
		\param cls				The MetaClass to test against.
		\return #KY_TRUE if this MetaClass inherits \c cls either directly or indirectly, or #KY_FALSE if this MetaClass
				does not inherit \c cls at all. */
	inline KyBool IsA(const MetaClass& cls) const;
	
	/*!	Returns the base class of this MetaClass, or #KY_NULL if none is specified. */
	MetaClass* GetBaseClass() const { return m_baseClass; }
	
	/*!	Returns a pointer to a function that instantiates the class. */
	Ctor GetCtor() const { return m_constructor; }

	/*!	Returns the number of instances of this MetaClass that have been created. */
	static KyInt32 GetClassCount() { return GetHolder().m_classeCount; }


protected:
	Ctor m_constructor;
	MetaClass* m_baseClass;
	
public:
	/*	For internal use. */
	class Holder
	{
		KY_DEFINE_NEW_DELETE_OPERATORS
	public:
		Holder() { static KyBool init = KY_TRUE; if (init) m_classeCount = 0; init = KY_FALSE; }
		~Holder() { Clear(); }
		void Clear() { m_classeCount = 0; }
		MetaClass* m_classes[KYNO_MAX_METACLASS_COUNT ];
		KyInt32 m_classeCount;
	};

	/*	For internal use. */
	static Holder& GetHolder()
	{
		static Holder s_holder;
		return s_holder;
	}
};

#define KY_META_CLASS_DUMP_KNOWN_CLASSES(C) \
			KY_MESSAGE(KY_LOG_ERROR, ("  Known classes are:")); \
			KY_MESSAGE(KY_LOG_ERROR, ("   --- begin ---")); \
			for (KyInt32 i = 0; i < C::GetHolder().m_classeCount; ++i) \
				KY_MESSAGE(KY_LOG_ERROR, ("   [%s]", C::GetHolder().m_classes[i]->GetName())); \
			KY_MESSAGE(KY_LOG_ERROR, ("   --- end ---")); \
			KY_MESSAGE(KY_LOG_ERROR, ("This may be due to dead code stripping. Try looking at the FAQ in doc :")); \
			KY_MESSAGE(KY_LOG_ERROR, ("\"What causes the message \"ERROR: Invalid definition: invalid entity class (CMyEntity)\"?\""));

#include <kyruntime/core/metaclass.inl>

} // namespace Kaim

#endif // KY_METACLASS_H
