//**********************************************************************
//
// Copyright (c) 2006
// PathEngine
// Lyon, France
//
// All Rights Reserved
//
//**********************************************************************

#include "Header.h"
#include "SolidObject.h"
#include "SolidObjects.h"
#include "LoadWhiteSpaceDelimited.h"
#include "Globals.h"
#include "Error.h"
#include "SimpleDOM.h"
#include "i_pathengine.h"
#include "i_contentprocessing3d.h"
#include <math.h>
#include <fstream>

static void
TransformPoint(
        double cosOfRotation, double sinOfRotation,
        float scale,
        tSigned32 translationX, tSigned32 translationY, tSigned32 translationZ,
        const tSigned32* from,
        tSigned32* to
        )
{
    to[0] = static_cast<tSigned32>((from[0] * cosOfRotation + from[1] * sinOfRotation) * scale) + translationX;
    to[1] = static_cast<tSigned32>((from[1] * cosOfRotation - from[0] * sinOfRotation) * scale) + translationY;
    to[2] = static_cast<tSigned32>(from[2] * scale) + translationZ;
}

void
cSolidObject::transformOperand(tSigned32 i)
{
    tSigned32 numberOfPoints = SizeL(_operands[i]) / 3;
    tSigned32 j;
    for(j = 0; j != numberOfPoints; ++j)
    {
        TransformPoint(
                _cosOfRotation, _sinOfRotation,
                _scale,
                _translationX, _translationY, _translationZ, 
                &_operands[i][j * 3],
                &_cachedTransformedOperands._data[i][j * 3]
                );
    }
}

const iFaceVertexMesh* 
cSolidObject::generateMesh()
{
    _cosOfRotation = cos(_rotation);
    _sinOfRotation = sin(_rotation);
    tSigned32 i;
    for(i = 0; i != _operands.size(); ++i)
    {
        transformOperand(i);
    }
    gCP3D->deleteHullsForVisualisation(_cachedHulls);
    _cachedHulls = gCP3D->generateHullsForVisualisation(_cachedTransformedOperands);
    return _cachedHulls;
}

cSolidObject::cSolidObject(const cSimpleDOM& e) :
 bSceneObject(e)
{
    std::string fileName = "..\\resource\\solidObjects\\";
    fileName.append(_type);
    fileName.append(".txt");
    std::ifstream is(fileName.c_str());
    cSimpleDOM doc;
    LoadWhiteSpaceDelimited(is, doc);
    assertR(doc._name == "solidObjects");    
    _operands.resize(doc._children.size());
    tSigned32 i;
    for(i = 0; i != SizeL(_operands); ++i)
    {
        const cSimpleDOM& e = doc._children[i];
        assertR(e._name == "solidObject");    
        _operands[i].resize(e._children.size() * 3);
        tSigned32 j;
        for(j = 0; j != SizeL(e._children); ++j)
        {
            const cSimpleDOM& p = e._children[j];
            assertR(p._name == "point");    
            _operands[i][j * 3 + 0] = p.attributeAsLong("x");
            _operands[i][j * 3 + 1] = p.attributeAsLong("y");
            _operands[i][j * 3 + 2] = p.attributeAsLong("z");
        }
    }
    _cachedTransformedOperands._data.resize(_operands.size());
    for(i = 0; i != SizeL(_operands); ++i)
    {
        assertR(SizeL(_operands[i]) >= 3);
        tSigned32 numberOfPoints = SizeL(_operands[i]) / 3;
        assertR(SizeL(_operands[i]) == numberOfPoints * 3);
        _cachedTransformedOperands._data[i].resize(_operands[i].size());
    }
    _cachedHulls = 0;
}

cSolidObject::~cSolidObject()
{
    gCP3D->deleteHullsForVisualisation(_cachedHulls);
}

void
cSolidObject::addTransformedOperands(cSolidObjects& solidObjects)
{
    generateCachedDataIfNotValid();
    tSigned32 i;
    for(i = 0; i != SizeL(_operands); ++i)
    {
        solidObjects._data.push_back(_cachedTransformedOperands._data[i]);
    }
}
