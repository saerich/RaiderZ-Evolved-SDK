#ifndef __COMMANDER_HPP
#define __COMMANDER_HPP

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
 * \brief     Platform independent commander sample
 * 
 *//*-------------------------------------------------------------------*/

// Platformlib includes.
#include "platform.hpp"
#include "plMath.hpp"

// Application includes.
#include "graphicsServices.hpp"

// Umbra includes.
#include "umbraCommander.hpp"

// System includes.
#include <vector>
#include <map>
#include <assert.h>

namespace MultithreadedSample
{

/*-------------------------------------------------------------------*//*!
 *          
 * \brief	Sample's commander, a callback class. All Occlusion Booster
 *			integrations require a commander.
 *
 *          SampleCommander contains the bulk of the necessary
 *          integration code for Umbra Occlusion Booster. Umbra 
 *          Occlusion Booster issues callbacks to SampleCommander, which
 *			are handled in the command function. The callbacks can be 
 *			thought as Occlusion Booster's instructions for the
 *          rendering engine.
 *
 *			Commander of the multithreaded sample is identical to the
 *			commander of basic sample.
 *
 *			For example, the commander renders objects and performs 
 *			occlusion queries on Occlusion Booster's request.
 *
 *			Commanders are inherited from Umbra::Commander.
 *
 * \see		Umbra::Commander::Command
 *
 *//*-------------------------------------------------------------------*/

class SampleCommander : public Umbra::Commander
{
public: 
                        SampleCommander			(SampleGraphicsServices*); 
    virtual				~SampleCommander		    (void); 

    virtual void		command					(Command c);

    //------------------------------------------------------------------------
    // Global flags of the class
    //------------------------------------------------------------------------

    static bool                     m_wireframe;

private:

    struct DebugLine
    {
        Platform::Vector4 c;
        Platform::Vector3 a;
        Platform::Vector3 b;
    };

    std::vector<Platform::Object*>	m_depthObjects;		/*!< List of objects for depth pass */		
    std::vector<Platform::Object*>	m_visibleObjects;	/*!< List of objects for color pass */		

    int                             m_lineCount;
};

} // namespace MultithreadedSample

//------------------------------------------------------------------------
#endif // __SAMPLECOMMANDER_HPP

