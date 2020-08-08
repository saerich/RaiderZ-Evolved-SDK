//**********************************************************************
//
// Copyright (c) 2006
// PathEngine
// Lyon, France
//
// All Rights Reserved
//
//**********************************************************************

#ifndef CONTENT_PROCESSING_3D_DEMO__SOLID_OBJECT_INCLUDED
#define CONTENT_PROCESSING_3D_DEMO__SOLID_OBJECT_INCLUDED

#include "SceneObject.h"
#include "SolidObjects.h"
#include <vector>

class cSolidObjects;

class cSolidObject : public bSceneObject
{
    std::vector<std::vector<tSigned32> > _operands;
    double _cosOfRotation, _sinOfRotation;
    cSolidObjects _cachedTransformedOperands;
    iFaceVertexMesh* _cachedHulls;

    void transformOperand(tSigned32 i);

    virtual const iFaceVertexMesh* generateMesh();

public:

    cSolidObject(const cSimpleDOM& e);
    ~cSolidObject();

    void addTransformedOperands(cSolidObjects& addTo);
};

#endif
