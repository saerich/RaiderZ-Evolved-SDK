//**********************************************************************
//
// Copyright (c) 2006
// PathEngine
// Lyon, France
//
// All Rights Reserved
//
//**********************************************************************

#ifndef CONTENT_PROCESSING_3D_DEMO__MESH_OBJECT_INCLUDED
#define CONTENT_PROCESSING_3D_DEMO__MESH_OBJECT_INCLUDED

#include "SceneObject.h"

class iContentChunk;
class iContentChunkInstance;
class cSimpleDOM;

class cMeshObject : public bSceneObject
{
    iContentChunk* _baseChunk;
    iContentChunkInstance* _cachedInstance;

    virtual const iFaceVertexMesh* generateMesh();

public:

    cMeshObject(const cSimpleDOM& e);
    ~cMeshObject();
};

#endif
