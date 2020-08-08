/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_asyncmanagement */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_ASYNCMODULE_H
#define KY_ASYNCMODULE_H

#include <kyruntime/asyncmanagement/asyncmanager.h>

#include <kyruntime/core/metaclass.h>

#include <kypathdata/basesystem/config.h>
#include <kypathdata/generic/memory.h>


namespace Kaim
{

class AsyncModuleClass;


/*!	AsyncModule is the base class for all asynchronous processing modules: classes that wrap chunks of 
	CPU-intensive calculations that will be delegated to a secondary thread or subordinate processing units.
	\pp For details on the asynchronous processing framework, see :USERGUIDE:"Asynchronous Processing".
	\pp The AsyncModule class cannot be used as-is. You must use one of the derived classes supplied with \SDKName or write
	your own custom class that derives from AsyncModule.
	\section writingcustomasyncmodule Writing a custom AsyncModule class
	Writing a custom class that derives from AsyncModule involves the following considerations.
	\subsection asyncmoduleconst Constructor
	All classes that derive from AsyncModule must have a constructor that follows the #AsyncModuleConstructor type definition. 
	\subsection asyncmodulevm Virtual methods
	All classes that derive from AsyncModule must implement the BeginAsyncProcessing() and EndAsyncProcessing() methods.
	\pp You may re-implement other virtual methods, or use the default implementations if you prefer.
	\subsection asyncmodulemetaclass MetaClass macros
	In order for \SDKName to recognize your AsyncModule class, you must include the following macro definitions:
	-	#KY_DECLARE_ASYNC_MODULE: Place this macro in the declaration of your new class. This macro takes one argument: the 
		name of your new class.
	-	#KY_IMPLEMENT_ASYNC_MODULE: Place this macro in the implementation file of your new class. This macro takes the following
		arguments:
		-	The name of your new class.
		-	A reference to the MetaClass of the class from which your new class derives, if different from AsyncModule.
			For example, &CMyAsyncModule::Class. If your new class derives directly from AsyncModule,
			the value of this argument should be #KY_NULL.
	\ingroup kyruntime_asyncmanagement */
class AsyncModule: public VirtualMemObject
{
public:
	/*!	\label_constructor For internal use. */
	AsyncModule(AsyncManager& asyncMgr) : m_asyncMgr(&asyncMgr) {}

	/*!	\label_virtualdestructor For internal use. */
	virtual ~AsyncModule() {}

	/*!	\label_getclass */
	virtual const AsyncModuleClass&  GetClass() const = 0;


	/*!	\label_reinit */
	virtual void ReInit() {}

	/*!	Called automatically by the AsyncManager to alert the AsyncModule that it should begin processing
		its calculations, or resume a calculation paused in the previous frame. If you create your own class
		of AsyncModule, you must implement this method. */
	virtual void BeginAsyncProcessing() {}

	/*!	Called automatically by the AsyncManager to alert the AsyncModule that it should stop processing
		its calculations, or pause them until the next frame. If you create your own class of AsyncModule,
		you must implement this method. Your implementation must block the calling thread until all asynchronous
		processing has been stopped. */
	virtual void EndAsyncProcessing() {}

	/*!	\label_getworld */
	const World& GetWorld() const;

	/*!	\label_getworld */
	World& GetWorld();

	/*!	\label_getengine */
	Engine& GetEngine();

	/*!	\label_getengine */
	const Engine& GetEngine() const;

protected:
	AsyncManager* m_asyncMgr; /*!< A pointer to the AsyncManager using this object. Do not modify. */
};


/*!	This macro must be included in the declaration of any AsyncModule class, to set up the MetaClass 
	used by the \SDKName object factory.
	\ingroup kyruntime_asyncmanagement */
#define KY_DECLARE_ASYNC_MODULE(ky_class) \
	static Kaim::AsyncModuleClass Class; \
	static Kaim::AsyncModule* KyCreate(AsyncManager&); \
	const Kaim::AsyncModuleClass&  GetClass() const { return ky_class::Class; }

/*!	This macro must be included in the implementation of any AsyncModule class, to set up the MetaClass 
	used by the \SDKName object factory.
	\ingroup kyruntime_asyncmanagement */
#define KY_IMPLEMENT_ASYNC_MODULE(ky_class,ky_base_class) \
	Kaim::AsyncModuleClass ky_class::Class(#ky_class,ky_class::KyCreate,(Kaim::AsyncModuleClass*)ky_base_class); \
	Kaim::AsyncModule* ky_class::KyCreate(AsyncManager& asyncMgr) \
	{ \
		KY_LOG_MEMORY(ky_class::Class.GetName()); \
		Kaim::AsyncModule* ptr = (Kaim::AsyncModule*) KY_NEW_CLASS(ky_class)(asyncMgr); \
		KY_REGISTER_METACLASS_VPTR(ky_class::Class, (*(void**)ptr)); \
		return ptr; \
	}


/*!	Defines the signature for the constructor function that is called automatically each time \SDKName 
	needs to create an instance of a class that derives from AsyncModule. Each class that derives from 
	AsyncModule must contain a constructor function that matches this type definition, and that passes
	its arguments on to the constructor function of its parent class.
	\param AsyncManager			The AsyncManager that will manage this AsyncModule.
	\ingroup kyruntime_asyncmanagement */
typedef AsyncModule* (*AsyncModuleConstructor)(AsyncManager&);

/*!	AsyncModuleClass is the MetaClass for AsyncModule, created and used internally by
	the \SDKName object factory.
	\ingroup kyruntime_asyncmanagement */
class AsyncModuleClass: public MetaClass<AsyncModule, AsyncModuleConstructor>
{
public:
	/*!	Constructor for the AsyncModule MetaClass. For internal use. */
	AsyncModuleClass(const char* className,  
		AsyncModuleConstructor ctr, AsyncModuleClass* baseClass = (AsyncModuleClass*)NULL);
};

} // namespace Kaim

#endif // KY_ASYNCMODULE_H
