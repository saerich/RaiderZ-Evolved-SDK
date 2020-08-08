#ifndef __PLMESH_HPP
#define __PLMESH_HPP

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
 * \brief   Mesh class.
 * 
 *//*-------------------------------------------------------------------*/

namespace Platform
{

/*-------------------------------------------------------------------*//*!
 *
 * \brief   Platform mesh class.
 *
 *			The mesh implemented separately for each platform.
 *
 *//*-------------------------------------------------------------------*/

class Mesh
{
public:

						Mesh				(void);
	virtual				~Mesh				(void);

	// Renders the mesh
	virtual void		draw				(void) const = 0;

};
} // namespace Platform

//------------------------------------------------------------------------
#endif // __PLMESH_HPP
