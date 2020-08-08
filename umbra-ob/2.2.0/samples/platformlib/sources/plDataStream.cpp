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
 * \brief     Utility functions.
 * 
 *//*-------------------------------------------------------------------*/

#include "plDataStream.hpp"
#include "platform.hpp"

#include <string.h>
#include <assert.h>

static const int MATRIX4x4_SIZE = 16; // \todo
static const int MATRIX4x3_SIZE = 12; // \todo
static const int MAX_SIZE       = 16;

using namespace Platform;

/*-------------------------------------------------------------------*//*!
 * \brief   DataStream default constructor.
 *//*-------------------------------------------------------------------*/

DataStream::DataStream(void)
{
    init(2048*16*4*2);
}

/*-------------------------------------------------------------------*//*!
 * \brief   DataStream constructor.
 *//*-------------------------------------------------------------------*/

DataStream::DataStream(size_t sz)
{
    init(sz);
}

#ifndef SPU

/*-------------------------------------------------------------------*//*!
 * \brief   DataStream destructor.
 *//*-------------------------------------------------------------------*/

DataStream::~DataStream(void)
{
    Platform::deleteSemaphore(m_writeSemaphore);
    Platform::deleteSemaphore(m_readSemaphore);

    std::free(m_data);
    m_data = NULL;
}

/*-------------------------------------------------------------------*//*!
 * \brief   Initialize data stream
 *
 * \param
 *
 * \note    Used only on PS3.
 *//*-------------------------------------------------------------------*/

void DataStream::init(size_t sz)
{
    m_freeDataCountPtr = &m_freeDataCount;
    m_dataAvailablePtr = &m_dataAvailable;
    m_data = (unsigned int*)std::malloc(sz*sizeof(unsigned int));
    m_dataSize = (int)sz;
    m_freeDataCount = 0;
    m_dataAvailable = 0;
    m_writeSemaphore = Platform::createSemaphore(0);
    m_readSemaphore = Platform::createSemaphore(0);

    m_reader = NULL;
    m_writer = NULL;

    m_writeState.m_writeDataAvailable = m_dataSize - MAX_SIZE;
    m_writeState.m_writeDataCount = 0;
    m_writeState.m_writeIndex = 0;

    m_readState.m_readDataAvailable = 0;
    m_readState.m_readDataCount = 0;
    m_readState.m_readIndex = 0;
}

#endif // SPU

/*-------------------------------------------------------------------*//*!
 * \brief   Get data stream size.
 *
 * \return  Data stream size.
 *//*-------------------------------------------------------------------*/

int DataStream::getDataSize()
{
    return m_dataSize;
}

/*-------------------------------------------------------------------*//*!
 * \brief
 *//*-------------------------------------------------------------------*/

void DataStream::attachReader(DataStreamReader* r)
{ 
    assert(m_reader == NULL); 
    m_reader = r; 
    m_reader->m_state = m_readState;
}

/*-------------------------------------------------------------------*//*!
 * \brief
 *//*-------------------------------------------------------------------*/

void DataStream::attachWriter(DataStreamWriter* w) 
{ 
    assert(m_writer == NULL); 
    m_writer = w; 
    m_writer->m_state = m_writeState;
}

/*-------------------------------------------------------------------*//*!
 * \brief
 *//*-------------------------------------------------------------------*/

void DataStream::detachReader(void)
{ 
    assert(m_reader); 
    m_readState = m_reader->m_state;
    m_reader = NULL; 
}

/*-------------------------------------------------------------------*//*!
 * \brief
 *//*-------------------------------------------------------------------*/

void DataStream::detachWriter(void) 
{ 
    assert(m_writer); 
    m_writeState = m_writer->m_state;
    m_writer = NULL; 
}

/*-------------------------------------------------------------------*//*!
 * \brief   DataStreamReader constructor.
 *//*-------------------------------------------------------------------*/

DataStreamReader::DataStreamReader()
{
}

/*-------------------------------------------------------------------*//*!
 * \brief   DataStreamReader destructor.
 *//*-------------------------------------------------------------------*/

DataStreamReader::~DataStreamReader()
{
    deinit();
}

/*-------------------------------------------------------------------*//*!
 * \brief   DataStremRender initializer.
 *//*-------------------------------------------------------------------*/

void DataStreamReader::init(DataStream* stream)
{
    m_stream = stream;
    stream->attachReader(this);
}

/*-------------------------------------------------------------------*//*!
 * \brief   DataStreamReader deinitializer.
 *//*-------------------------------------------------------------------*/

void DataStreamReader::deinit()
{
    if (m_stream->m_reader)
        m_stream->detachReader();
} 

/*-------------------------------------------------------------------*//*!
 * \brief
 *//*-------------------------------------------------------------------*/

bool DataStreamReader::reserve(int s)
{
    assert(s > 0 && s <= MAX_SIZE);

    while (m_state.m_readDataAvailable < s)
    {
        Platform::decSemaphore(m_stream->m_readSemaphore, true);
        m_state.m_readDataAvailable += Platform::atomicStore(m_stream->m_dataAvailablePtr, 0);
        assert(m_state.m_readDataAvailable >= s);
    }

    m_state.m_readDataCount += s;
    m_state.m_readDataAvailable -= s;

    assert(m_state.m_readDataAvailable >= 0);

    return true;
}

/*-------------------------------------------------------------------*//*!
 * \brief
 *//*-------------------------------------------------------------------*/

void DataStreamReader::advance(int s)
{
    m_state.m_readIndex += s;

    if (m_state.m_readIndex >= (m_stream->m_dataSize - MAX_SIZE))
        m_state.m_readIndex = 0;

    assert(m_state.m_readIndex < m_stream->m_dataSize);    

    if (m_state.m_readDataCount > 256)
        flush();
}

/*-------------------------------------------------------------------*//*!
 * \brief
 *//*-------------------------------------------------------------------*/

void DataStreamReader::flush(void)
{
    int previous = Platform::atomicAdd(m_stream->m_freeDataCountPtr, m_state.m_readDataCount);
    if (previous == 0 && m_state.m_readDataCount)
        Platform::incSemaphore(m_stream->m_writeSemaphore);
    m_state.m_readDataCount = 0;
}
 
/*-------------------------------------------------------------------*//*!
 * \brief
 *//*-------------------------------------------------------------------*/

bool DataStreamReader::isDataAvailable(void)
{
    return m_state.m_readDataAvailable || Platform::atomicGet(m_stream->m_dataAvailablePtr);
}

/*-------------------------------------------------------------------*//*!
 * \brief   DataStreamWriter constructor.
 *//*-------------------------------------------------------------------*/

DataStreamWriter::DataStreamWriter()
{
}


/*-------------------------------------------------------------------*//*!
 * \brief   DataStreamWriter destructor.
 *//*-------------------------------------------------------------------*/

DataStreamWriter::~DataStreamWriter()
{
    deinit();
}

/*-------------------------------------------------------------------*//*!
 * \brief   DataStreamWriter initalizer.
 *//*-------------------------------------------------------------------*/

void DataStreamWriter::init(DataStream* stream)
{
    m_stream = stream;
    stream->attachWriter(this);
}

/*-------------------------------------------------------------------*//*!
 * \brief   DataStreamWriter deinitializer.
 *//*-------------------------------------------------------------------*/

void DataStreamWriter::deinit()
{
    if (m_stream->m_writer)
        m_stream->detachWriter();
}

void DataStreamWriter::reserve(int s)
{
    while (m_state.m_writeDataAvailable < s)
    {
        Platform::decSemaphore(m_stream->m_writeSemaphore, true);
        m_state.m_writeDataAvailable += Platform::atomicStore(m_stream->m_freeDataCountPtr, 0);
    }

    m_state.m_writeDataCount += s;
    m_state.m_writeDataAvailable -=s;

    assert(m_state.m_writeDataAvailable >= 0);
}

/*-------------------------------------------------------------------*//*!
 * \brief
 *
 * \param
 *//*-------------------------------------------------------------------*/

void DataStreamWriter::advance(int s)
{
    m_state.m_writeIndex += s;

    if (m_state.m_writeIndex >= (m_stream->m_dataSize - MAX_SIZE))
        m_state.m_writeIndex = 0;

    assert(m_state.m_writeIndex < m_stream->m_dataSize);    

    if (m_state.m_writeDataCount > 256) // DEBUG DEBUG \todo
        flush();
}

/*-------------------------------------------------------------------*//*!
 * \brief
 *
 * \param
 *//*-------------------------------------------------------------------*/

void DataStreamWriter::flush(void)
{
    int previous = Platform::atomicAdd(m_stream->m_dataAvailablePtr, m_state.m_writeDataCount);
    if (previous == 0 && m_state.m_writeDataCount)
        Platform::incSemaphore(m_stream->m_readSemaphore);
    m_state.m_writeDataCount = 0;
}



 