//**********************************************************************
//
// Copyright (c) 2006-2007
// PathEngine
// Lyon, France
//
// All Rights Reserved
//
//**********************************************************************

#include "Header.h"
#include "Globals.h"
#include "Error.h"
#include "SceneObject.h"
#include "SimpleDOM.h"
#include "i_testbed.h"
#include <sstream>

void
bSceneObject::invalidateCachedData()
{
    gTestBed->deleteRenderPreprocess(_cachedFacesRenderPreprocess);
    gTestBed->deleteRenderPreprocess(_cachedEdgesRenderPreprocess);
    _cachedFacesRenderPreprocess = 0;
    _cachedEdgesRenderPreprocess = 0;
}

void
bSceneObject::generateCachedDataIfNotValid()
{
    if(_cachedFacesRenderPreprocess)
    {
        return;
    }
    _cachedMesh = generateMesh();
    _cachedFacesRenderPreprocess = gTestBed->preprocessFaceVertexMesh(*_cachedMesh);
    _cachedEdgesRenderPreprocess = gTestBed->preprocessFaceVertexMeshEdges(*_cachedMesh);
    assertD(_cachedMesh->vertices());
    _cachedRange.minX = _cachedRange.maxX = _cachedMesh->vertexX(0);
    _cachedRange.minY = _cachedRange.maxY = _cachedMesh->vertexY(0);
    tSigned32 i;
    for(i = 1; i != _cachedMesh->vertices(); ++i)
    {
        tSigned32 x = _cachedMesh->vertexX(i);
        if(x < _cachedRange.minX)
        {
            _cachedRange.minX = x;
        }
        if(x > _cachedRange.maxX)
        {
            _cachedRange.maxX = x;
        }
        tSigned32 y = _cachedMesh->vertexY(i);
        if(y < _cachedRange.minY)
        {
            _cachedRange.minY = y;
        }
        if(y > _cachedRange.maxY)
        {
            _cachedRange.maxY = y;
        }
    }
}

bSceneObject::bSceneObject(const cSimpleDOM& e)
{
    _name = e.getAttribute("name");
    _type = e.getAttribute("type");
    _rotation = e.attributeAsFloat("rotation");
    _scale = e.attributeAsFloat("scale");
    _translationX = e.attributeAsLong("x");
    _translationY = e.attributeAsLong("y");
    _translationZ = e.attributeAsLong("z");
    _cachedFacesRenderPreprocess = 0;
    _cachedEdgesRenderPreprocess = 0;
}

bSceneObject::~bSceneObject()
{
    gTestBed->deleteRenderPreprocess(_cachedFacesRenderPreprocess);
    gTestBed->deleteRenderPreprocess(_cachedEdgesRenderPreprocess);
}

void
bSceneObject::addAttributes(cSimpleDOM& e)
{
    e._attributes["name"] = _name;
    e._attributes["type"] = _type;
    {
        std::ostringstream oss;
        oss << _rotation;
        e._attributes["rotation"] = oss.str();
    }
    {
        std::ostringstream oss;
        oss << _scale;
        e._attributes["scale"] = oss.str();
    }
    {
        std::ostringstream oss;
        oss << _translationX;
        e._attributes["x"] = oss.str();
    }
    {
        std::ostringstream oss;
        oss << _translationY;
        e._attributes["y"] = oss.str();
    }
    {
        std::ostringstream oss;
        oss << _translationZ;
        e._attributes["z"] = oss.str();
    }
}

void
bSceneObject::addRotation(float r)
{
    _rotation += r;
    invalidateCachedData();
}

void
bSceneObject::addScale(float toAdd, float minClamp, float maxClamp)
{
    _scale += toAdd;
    if(_scale < minClamp)
    {
        _scale = minClamp;
    }
    if(_scale > maxClamp)
    {
        _scale = maxClamp;
    }
    invalidateCachedData();
}

void
bSceneObject::addTranslation(tSigned32 x, tSigned32 y, tSigned32 z, tSigned32 range)
{
    _translationX += x;
    if(_translationX < -range)
    {
        _translationX = range;
    }
    if(_translationX > range)
    {
        _translationX = range;
    }
    _translationY += y;
    if(_translationY < -range)
    {
        _translationY = range;
    }
    if(_translationY > range)
    {
        _translationY = range;
    }
    _translationZ += z;
    if(_translationZ < -range)
    {
        _translationZ = range;
    }
    if(_translationZ > range)
    {
        _translationZ = range;
    }
    invalidateCachedData();
}

const iFaceVertexMesh*
bSceneObject::getCachedMesh()
{
    generateCachedDataIfNotValid();
    return _cachedMesh;
}
const cHorizontalRange&
bSceneObject::cachedRange() const
{
    return _cachedRange;
}
void
bSceneObject::renderFaces()
{
    generateCachedDataIfNotValid();
    gTestBed->drawRenderPreprocess(_cachedFacesRenderPreprocess);
}
void
bSceneObject::renderEdges()
{
    generateCachedDataIfNotValid();
    gTestBed->drawRenderPreprocess(_cachedEdgesRenderPreprocess);
}
