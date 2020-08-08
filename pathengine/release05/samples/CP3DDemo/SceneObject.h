//**********************************************************************
//
// Copyright (c) 2006-2007
// PathEngine
// Lyon, France
//
// All Rights Reserved
//
//**********************************************************************

#ifndef CONTENT_PROCESSING_3D_DEMO__SCENE_OBJECT_INCLUDED
#define CONTENT_PROCESSING_3D_DEMO__SCENE_OBJECT_INCLUDED

#include "i_pathengine.h"
#include <string>

class cSimpleDOM;
class iFaceVertexMesh;
class cRenderPreprocess;

class bSceneObject
{
protected:

    std::string _name;
    std::string _type;
    float _rotation;
    float _scale;
    tSigned32 _translationX, _translationY, _translationZ;

    void invalidateCachedData();
    virtual const iFaceVertexMesh* generateMesh() = 0;
    
protected:

    void generateCachedDataIfNotValid();

    const iFaceVertexMesh* _cachedMesh;
    cRenderPreprocess* _cachedFacesRenderPreprocess;
    cRenderPreprocess* _cachedEdgesRenderPreprocess;
    cHorizontalRange _cachedRange;

public:

    bSceneObject(const cSimpleDOM& e);
    ~bSceneObject();

    void addAttributes(cSimpleDOM& e);

    void addRotation(float r);
    void addScale(float toAdd, float minClamp, float maxClamp);
    void addTranslation(tSigned32 x, tSigned32 y, tSigned32 z, tSigned32 range);

    const iFaceVertexMesh* getCachedMesh();
    const cHorizontalRange& cachedRange() const;
    void renderFaces();
    void renderEdges();
};

#endif
