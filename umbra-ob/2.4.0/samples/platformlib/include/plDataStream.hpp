#ifndef __PLDATASTREAM_HPP
#define __PLDATASTREAM_HPP

/*-------------------------------------------------------------------*//*!
 *
 * Umbra Occlusion Booster
 * -----------------------------------------
 *
 * (C) 2009-2010 Umbra Software Ltd. 
 * All Rights Reserved.
 *
 *
 * This file consists of unpublished, proprietary source code of
 * Umbra Software, and is considered Confidential Information for
 * purposes of non-disclosure agreement. Disclosure outside the terms
 * outlined in signed agreement may result in irrepairable harm to
 * Umbra Software and legal action against the party in breach.
 *
 * \file
 * \brief     A simple Application using Umbra.
 * 
 *//*-------------------------------------------------------------------*/

#include "plMath.hpp"

namespace Platform
{

class DataStreamReader;
class DataStreamWriter;

/*-------------------------------------------------------------------*//*!
 * \brief   DataStream class.
 *
 *			The purpose of the DataStream class is enable communication
 *			between threads. The DataStream itselfs exposes very little
 *			information. Instead, DataStreamReader and DataStreamWriter
 *			can be used to add and extract information to/from the 
 *			stream.
 *
 *			The multithreaded sample application uses a datastream to
 *			communicate with a rendering thread.
 *
 *//*-------------------------------------------------------------------*/

class DataStream
{
public:

                        DataStream  (void);
                        DataStream  (size_t sz);
                        ~DataStream (void);

    int                 getDataSize (void);

private:

    friend class        DataStreamReader;
    friend class        DataStreamWriter;

	// Writing state
    struct WriteState
    {
        int m_writeIndex;
        int m_writeDataCount;
        int m_writeDataAvailable;
    };

	// Reading state
    struct ReadState
    {
        int m_readIndex;
        int m_readDataCount;
        int m_readDataAvailable;
    };

	// Internal reader/writer management
    void                init        (size_t sz);
    void                attachReader(DataStreamReader* r);
    void                attachWriter(DataStreamWriter* w);
    void                detachReader(void);
    void                detachWriter(void);

    unsigned int*       m_data;
    int                 m_dataSize; 
    //void*               m_mutex;
    Semaphore*          m_writeSemaphore;
    Semaphore*          m_readSemaphore;

    DataStreamReader*   m_reader;
    DataStreamWriter*   m_writer;

    WriteState          m_writeState;
    ReadState           m_readState;

    // These members are accessed from both threads.
    volatile int        m_freeDataCount;
    volatile int        m_dataAvailable;
    volatile int*       m_freeDataCountPtr;
    volatile int*       m_dataAvailablePtr;
};

/*-------------------------------------------------------------------*//*!
 * \brief   Cached object class.
 *
 * \note    Separate implementation for PS3.
 *//*-------------------------------------------------------------------*/

#ifndef SPU

template <class T> 
class CachedObject
{
public:

            CachedObject    (void)          { m_object = 0; }
            CachedObject    (const T* obj)  { m_object = obj; }

    void    operator=       (T* t)          { m_object = t; }
    T*      operator->      (void)          { return m_object; }

    void    set             (T* obj)        { m_object = obj; }

private:
    char    m_localCopy[sizeof(T)];
    T*      m_object;
};

#else

#include <cell/dma.h>

template <class T>
class CachedObject
{
public:

            CachedObject    (void)          { m_object = 0; }
            CachedObject    (const T* obj)  { set(obj); }

    void    operator=       (T* t)          { set(t); }
    T*      operator->      (void)          { return (T*)&m_localCopy; }

    void    set             (T* obj)        {
                                                m_object = obj;
                                                cellDmaGet(&m_localCopy, (uint64_t)m_object, 64, 0, 0, 0);
                                                cellDmaWaitTagStatusAll(1);
                                            }

private:
    char    m_localCopy[64]  __attribute__((aligned(128)));
    T*      m_object  __attribute__((aligned(128)));
};

#endif

/*-------------------------------------------------------------------*//*!
 * \brief   DataStreamReader class.
 *
 *			Extracts data from a DataStream.
 *
 *//*-------------------------------------------------------------------*/

class DataStreamReader
{
public:

            DataStreamReader    (void);
            ~DataStreamReader   (void);

    void    init                (DataStream* stream);
    void    deinit              (void);

    bool    isDataAvailable     (void);

    // Generic reader function.
    template<typename T>
    bool get(T& value) 
    {
        int size = sizeof(value)/sizeof(unsigned int);
        reserve(size);

		T UNALIGNED *p = (T*)(m_stream->m_data+m_state.m_readIndex);
        value = *p;

        advance(size);

        return true;
    }
    
private:

    friend class DataStream;

    bool    reserve             (int);
    void    advance             (int);
    void    flush               (void);

    // Read data.
    DataStream::ReadState   m_state;

    // Shared.
    CachedObject<DataStream> m_stream;
};

/*-------------------------------------------------------------------*//*!
 * \brief   DataStreamWriter class.
 *
 *			Puts data into a DataStream.
 *
 *//*-------------------------------------------------------------------*/

class DataStreamWriter
{
public:

            DataStreamWriter    (void);
            ~DataStreamWriter   (void);

    void    init                (DataStream* stream);
    void    deinit              (void);

    void    flush               (void);

    // Generic stream writer function
    template<typename T>
    void put(const T& value) {
        int size = sizeof(value)>>2;
        reserve(size);

		T UNALIGNED *p = (T*)(m_stream->m_data+m_state.m_writeIndex);
		*p = value;

        advance(size);
        PLATFORM_ASSERT(m_state.m_writeDataAvailable >= 0);
    }

private:

    friend class DataStream;

    void    reserve             (int s);
    void    advance             (int);

    // Write data.
    DataStream::WriteState m_state;

    // Shared.
    CachedObject<DataStream> m_stream;
};

} // namespace Platform

#endif // __PLDATASTREAM_HPP

