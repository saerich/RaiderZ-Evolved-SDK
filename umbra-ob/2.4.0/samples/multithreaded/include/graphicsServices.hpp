#ifndef __GRAPHICSSERVICES_HPP
#define __GRAPHICSSERVICES_HPP

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
 * \brief     Graphics services class.
 * 
 *//*-------------------------------------------------------------------*/

// Umbra includes.
#include "umbraLibrary.hpp"

namespace MultithreadedSample
{

/*-------------------------------------------------------------------*//*!
 *
 * \brief   GraphicsServices is a class that's required to be
 *          implemented in any integration code. It's functions are
 *          used internally by Umbra Occlusion Booster.
 *
 *			SampleGraphicsServices is derived from 
 *          Umbra::LibraryDefs::GraphicsServices.
 *
 *			SampleGraphicsServices has functions for occlusion query
 *          allocation and deallocation. Umbra Occlusion Booster
 *          uses these functions internally for allocating and 
 *          deallocating occlusion query objects.
 *
 *			SampleGraphicsServices of the multithreaded sample is
 *			identical to GraphicsServices of basic sample. 
 *
 *//*-------------------------------------------------------------------*/

class SampleGraphicsServices : public Umbra::LibraryDefs::GraphicsServices
{
protected:

    virtual void*   allocateQueryObject		(void);
    virtual void    releaseQueryObject		(void* query);

};

} // namespace MultithreadedSample

#endif // __GRAPHICSSERVICES_HPP
