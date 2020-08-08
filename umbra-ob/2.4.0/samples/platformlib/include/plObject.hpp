#ifndef __PLOBJECT_HPP
#define __PLOBJECT_HPP

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
 * \brief     Platform object class.
 * 
 *//*-------------------------------------------------------------------*/

#include "plMath.hpp"

namespace Platform
{
	
class Mesh;

/*-------------------------------------------------------------------*//*!
 *
 * \brief   Platform object class.
 *
 *			Object is an instance of a mesh, and thus keeps a pointer
 *			to the mesh, it's color and transformation matrix.
 *
 *//*-------------------------------------------------------------------*/

class Object
{
public:

						Object			        (Mesh* mesh);
						~Object			        (void);

	// Setters
    void				setColor                (const Vector4& col);
    void				setObjectToCellMatrix   (const Matrix4x4& mtx);

	// Getters
    const Vector4&		getColor                (void) const;
    const Mesh*	        getMesh                 (void) const;
	const Matrix4x4&	getObjectToCellMatrix   (void) const;

	// Total object count
	static int          getObjectCount          (void);

private:

	Matrix4x4           m_objectToCell;
	Vector4             m_color;
    Mesh*               m_mesh;

	static int			s_objectCount;
};

} // namespace Platform

//------------------------------------------------------------------------
#endif // __PLOBJECT_HPP
