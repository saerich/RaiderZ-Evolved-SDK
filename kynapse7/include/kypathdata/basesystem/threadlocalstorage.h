/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/


#ifndef KY_THREAD_LOCAL_STORAGE_H
#define KY_THREAD_LOCAL_STORAGE_H

#include "kypathdata/generic/basetypes.h"
#include "kypathdata/generic/memory.h"


namespace Kaim
{

//-----------------------------------------------------------------------------------
//                                 KY_WINDOWS_ANY
//-----------------------------------------------------------------------------------
#if defined (KY_WINDOWS_ANY)

class WinTls
{
public:
	static unsigned long KyTlsAlloc();
	static void* KyTlsGetValue(unsigned long dwTlsIndex);
	static int KyTlsSetValue(unsigned long dwTlsIndex, void* lpTlsValue);
	static int KyTlsFree(unsigned long dwTlsIndex);
};


template<typename ptrT> class ThreadLocalStorageWrapper
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	ThreadLocalStorageWrapper()   { m_tlsIdx = (KyUInt32)WinTls::KyTlsAlloc(); }
	~ThreadLocalStorageWrapper()  { WinTls::KyTlsFree(m_tlsIdx); }
	inline ptrT Get()             { return (ptrT)WinTls::KyTlsGetValue(m_tlsIdx); }
	inline void Set(ptrT ptr)     { WinTls::KyTlsSetValue(m_tlsIdx, (void*)ptr);  }
private:
	KyUInt32 m_tlsIdx;
};

//-----------------------------------------------------------------------------------
//                                     KY_LINUX_ANY
//-----------------------------------------------------------------------------------
#elif defined (KY_LINUX_ANY)

template<typename ptrT> class ThreadLocalStorageWrapper
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	ThreadLocalStorageWrapper()  {}
	~ThreadLocalStorageWrapper() {}
	inline ptrT Get()            { return TlsInstance(); }
	inline void Set(ptrT ptr)    { TlsInstance() = ptr; }
private:
	inline ptrT& TlsInstance() { static __thread ptrT tls_ptr; return tls_ptr; }
};

//-----------------------------------------------------------------------------------
//                           !KY_WINDOWS_ANY && !KY_LINUX_ANY
//-----------------------------------------------------------------------------------
#else

// no ThreadLocalStorage on other platforms that Windows and Linux
template<typename ptrT> class ThreadLocalStorageWrapper
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	ThreadLocalStorageWrapper()  {}
	~ThreadLocalStorageWrapper() {}
	inline ptrT Get()            { return Instance(); }
	inline void Set(ptrT ptr)    { Instance() = ptr; }
private:
	inline ptrT& Instance() { static ptrT s_ptr; return s_ptr; }
};


#endif


} // namespace Kaim


#endif
