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
#include "MeshObject.h"
#include "Globals.h"
#include "Error.h"
#include "i_pathengine.h"
#include "i_testbed.h"
#include <vector>

const iFaceVertexMesh*
cMeshObject::generateMesh()
{
    delete _cachedInstance;
    _cachedInstance = _baseChunk->instantiate_FloatRotateAndScale(
            _rotation, _scale,
            _translationX, _translationY,
            static_cast<float>(_translationZ),
            -1
            );
    return _cachedInstance->getGroundPointer();
}

cMeshObject::cMeshObject(const cSimpleDOM& e) :
 bSceneObject(e)
{
    std::string fileName = "..\\resource\\content3D\\";
    fileName.append(_type);
    fileName.append(".tok");
    tUnsigned32 bufferSize;
    char* buffer = gTestBed->loadBinary(fileName.c_str(), bufferSize);
    if(buffer == 0)
    {
        Error("Fatal", "Failed to load buffer for a mesh object.");
    }
    _baseChunk = gPathEngine->loadContentChunk(buffer, bufferSize);
    gTestBed->freeBuffer(buffer);
    _cachedInstance = 0;
}

cMeshObject::~cMeshObject()
{
    delete _cachedInstance;
    delete _baseChunk;
}
