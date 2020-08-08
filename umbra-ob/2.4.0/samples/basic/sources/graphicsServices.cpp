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

// Platformlib includes.
#include "platform.hpp"

// Application includes.
#include "graphicsServices.hpp"

// System includes.
#include <assert.h>

using namespace BasicSample;

/*-------------------------------------------------------------------*//*!
 *
 * \brief   Allocates a platform specific query object.
 *
 * \return  Returns a void pointer to a platform specific occlusion
 *          query object.
 *
 *          This function is used internally by Umbra Occlusion Booster.
 *
 *//*-------------------------------------------------------------------*/

void* SampleGraphicsServices::allocateQueryObject(void)
{
    return Platform::allocateQuery();
}

/*-------------------------------------------------------------------*//*!
 *                                                                       
 * \brief   Releases a query objects allocated with
 *          SampleGraphicsServices::allocateQueryObject.
 *
 * \param   query   The query object which will be released.
 *
 *          This function is used internally by Umbra Occlusion Booster.
 *
 *//*-------------------------------------------------------------------*/

void SampleGraphicsServices::releaseQueryObject(void* query)
{
	assert(query);
	Platform::releaseQuery( (Platform::Query*)query );
}
