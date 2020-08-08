/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

/*! \file
	\ingroup kypdg */

// primary contact: GUAL - secondary contact: NOBODY
#ifndef KyPdg_IParallelForInterface_H
#define KyPdg_IParallelForInterface_H


#include <kypathdata/generic/memory.h>


namespace Kaim
{

/*! Provides an abstract base interface for an object that treats an element of data that can be passed to an object that derives from
	IParallelForInterface. 
	\pp You should not need to implement this interface yourself; specialized implementations for different kinds of data elements 
	may be provided with \SDKName. 
	\pp If you write a custom implementation of IParallelForInterface, your IParallelForInterface class just needs to call the Do() method of the
	IParallelElementFunctor passed to its IParallelForInterface::ParallelFor() method for each element passed to IParallelForInterface::ParallelFor().
	\ingroup kypdg */
class IParallelElementFunctor : public VirtualMemObject
{
public:
	/*! This method is carries out whatever processing needs to be done for the specified data element. */
	virtual void Do(void* element) = 0;
	/*! \label_virtualdestructor For internal use. */
	virtual ~IParallelElementFunctor() {}
};

/*! Provides an abstract base interface for an object that can treat computational jobs in parallel. 
	\pp This class cannot be used as-is; you must use an instance of a class that derives from IParallelForInterface and that implements
	its virtual methods. \SDKName provides a default implementation, KyGlue::TbbParallelForInterface, which uses the Threaded Building Blocks
	(TBB) libraries from Intel to manage the parallel execution of its computational jobs. 
	\pp You can also create your own implementation of IParallelForInterface if you need to support a different parallel computing system. For
	an example implementation, see the code for the KyGlue::TbbParallelForInterface class, available in the \c integration directory.
	\ingroup kypdg */
class IParallelForInterface : public VirtualMemObject
{
public:
	/* For internal use only. */
	class InitObject
	{
	public:
		InitObject(IParallelForInterface* parallelFor) : m_parallelFor(parallelFor) { if (m_parallelFor) m_parallelFor->Init(); }
		~InitObject() { if (m_parallelFor) m_parallelFor->Terminate(); }
		IParallelForInterface* m_parallelFor;
	};

public:
	/*! Identifies a pointer to the ThreadLocalStorage data returned in a call to GetTlsData(). */
	typedef void* TlsDataPtr;

	/*! You must call this method at least once for each thread that you create. It returns a pointer to an object
		that contains internal ThreadLocalStorage data allocated by \SDKName. You must store these pointers in your
		implementation of IParallelForInterface, and release them all through calls to TerminateTlsData(). */
	static TlsDataPtr GetTlsData();

	/*! You must call this method for each pointer that you have retrieved from GetTlsData(). */
	void TerminateTlsData(TlsDataPtr tlsDataPtr);

	/*! \label_virtualdestructor */
	virtual ~IParallelForInterface() {}

	/*! Called once by the Generator at the beginning of the data generation process. This method is expected to perform
		whatever steps are needed to initialize the parallelization mechanism. */
	virtual KyResult Init() = 0;

	/*! Called once by the Generator at the end of the data generation process. This method is expected to perform
		whatever steps are needed to shut down the parallelization mechanism. It should also call TerminateTlsData() to
		release each pointer retrieved through calls to GetTlsData(). */
	virtual KyResult Terminate() = 0;

	/*! This is the main method of the IParallelForInterface. It is called by \SDKName to manage the parallel execution of an array of
		computational jobs. An implementation of this method should call the IParallelElementFunctor::Do() method for each element in 
		the \c elements array.
		\param elements				An array of void pointers, each of which points to a computational job that needs to be carried out.
		\param elementsCount		The number of elements in the \c elements array.
		\param functor				A pointer to the object that should be responsible for actually treating each element in the \c elements
									array. 
		\return A #KyResult that indicates the success or failure of the parallel computations. */
	virtual KyResult ParallelFor(void** elements, KyUInt32 elementsCount, Kaim::IParallelElementFunctor* functor) = 0;
};


}


#endif
