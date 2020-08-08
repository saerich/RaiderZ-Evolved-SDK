/*-------------------------------------------------------------------*//*!
 *
 * Umbra Occlusion Booster
 * -----------------------------------------
 *
 * (C) 2009-2010 Umbra Software Ltd. 
 * All Rights Reserved.
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

#include "plObject.hpp"

using namespace Platform;

int Object::s_objectCount = 0;

/*-------------------------------------------------------------------*//*!
 * \brief   Object constructor.
 *
 * \param   mesh    Mesh class.
 *//*-------------------------------------------------------------------*/

Object::Object(Mesh* mesh) :
m_mesh(mesh)
{
    PLATFORM_ASSERT(m_mesh);
    s_objectCount++;
}

/*-------------------------------------------------------------------*//*!
 * \brief   Object destructor.
 *//*-------------------------------------------------------------------*/

Object::~Object()
{
    s_objectCount--;
}

/*-------------------------------------------------------------------*//*!
 * \brief   Getter for the Mesh object.
 *
 * \return  Pointer to a Mesh object.
 *//*-------------------------------------------------------------------*/

const Mesh* Object::getMesh() const
{
    return m_mesh;
}

/*-------------------------------------------------------------------*//*!
 * \brief   Set object color.
 *
 * \param   col     Object color as a Vector4.
 *//*-------------------------------------------------------------------*/

void Object::setColor(const Vector4& col)
{
    m_color = col;
}

/*-------------------------------------------------------------------*//*!
 * \brief   Get object color.
 *
 * \return  Object color as a Vector4 reference.
 *//*-------------------------------------------------------------------*/

const Vector4& Object::getColor(void) const
{
    return m_color;
}

/*-------------------------------------------------------------------*//*!
 * \brief   Set object transformation matrix.
 *
 * \param   mtx     Object transformation matrix as a Matrix4x4.
 *//*-------------------------------------------------------------------*/

void Object::setObjectToCellMatrix(const Matrix4x4 &mtx)
{
    m_objectToCell = mtx;
}

/*-------------------------------------------------------------------*//*!
 * \brief   Get object transfomration matrix.
 *
 * \return  Object transformation matrix as a Matrix4x4 reference.
 *//*-------------------------------------------------------------------*/

const Matrix4x4& Object::getObjectToCellMatrix(void) const
{
    return m_objectToCell;
}

/*-------------------------------------------------------------------*//*!
 * \brief   Get total amount of objects created.
 *
 * \return  Total amount of Objects created.
 *//*-------------------------------------------------------------------*/

int Object::getObjectCount()
{
    return s_objectCount;
}
