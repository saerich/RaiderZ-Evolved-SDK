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
#include "Scene.h"
#include "MeshObject.h"
#include "SolidObject.h"
#include "Error.h"
#include "Globals.h"
#include "SimpleDOM.h"
#include "BuildGround.h"
#include "i_testbed.h"

void
cScene::zoomExtentsSelected()
{
    const iFaceVertexMesh* mesh = _asSceneObjects[_currentlySelected]->getCachedMesh();
    gTestBed->zoomExtents_FaceVertex(&mesh, 1);
}

cScene::cScene(const cSimpleDOM& placement)
{
    _currentlySelected = -1;
    tSigned32 i;
    const cSimpleDOM& meshObjects = placement.refFirstChildWithName("meshObjects");
    for(i = 0; i != SizeL(meshObjects._children); ++i)
    {
        _meshObjects.push_back(new cMeshObject(meshObjects._children[i]));
        _asSceneObjects.push_back(_meshObjects.back());
    }
    const cSimpleDOM& solidObjects = placement.refFirstChildWithName("solidObjects");
    for(i = 0; i != SizeL(solidObjects._children); ++i)
    {
        _solidObjects.push_back(new cSolidObject(solidObjects._children[i]));
        _asSceneObjects.push_back(_solidObjects.back());
    }
}
cScene::~cScene()
{
    tSigned32 i;
    for(i = 0; i != SizeL(_meshObjects); ++i)
    {
        delete _meshObjects[i];
    }
    for(i = 0; i != SizeL(_solidObjects); ++i)
    {
        delete _solidObjects[i];
    }
}

bool
cScene::empty() const
{
    return _meshObjects.empty() && _solidObjects.empty();
}

void
cScene::initSelection()
{
    assertR(!empty());
    _currentlySelected = 0;
}

void
cScene::updateWithEditing()
{
    assertD(_currentlySelected != -1);
    iTestBed& testBed = *gTestBed;
    {
        bSceneObject& currentSceneObject = *_asSceneObjects[_currentlySelected];
        tSigned32 dx, dy;
        testBed.getMouseScreenDelta(dx, dy);
        if(testBed.getKeyState("_LSHIFT") && testBed.getKeyState("_LMOUSE") && testBed.getKeyState("_MMOUSE"))
        {
            currentSceneObject.addRotation(static_cast<float>(dx) * 0.01f);
        }
        else
        if(testBed.getKeyState("_LSHIFT") && testBed.getKeyState("_LMOUSE"))
        {
            currentSceneObject.addTranslation(dx * 40, dy * 40, 0, 100000);
        }
        else
        if(testBed.getKeyState("_LSHIFT") && testBed.getKeyState("_MMOUSE"))
        {
            currentSceneObject.addTranslation(0, 0, dy * 40, 100000);
        }
        else
        if(testBed.getKeyState("_RSHIFT") && testBed.getKeyState("_LMOUSE"))
        {
            currentSceneObject.addScale(static_cast<float>(dy) * 0.01f, 0.1f, 10.f);
        }
    }
}

void
cScene::handleKeyPress(const char* keyPressed)
{
    switch(keyPressed[1])
    {
    case '_':
        {
            if(strcmp("PAGEUP", keyPressed + 2) == 0)
            {
                ++_currentlySelected;
                if(_currentlySelected == SizeL(_meshObjects) + SizeL(_solidObjects))
                {
                    _currentlySelected = 0;
                }
            }
            else if(strcmp("PAGEDOWN", keyPressed + 2) == 0)
            {
                --_currentlySelected;
                if(_currentlySelected < 0)
                {
                    _currentlySelected = SizeL(_meshObjects) + SizeL(_solidObjects) - 1;
                }
            }
            else if(strcmp("HOME", keyPressed + 2) == 0)
            {
                zoomExtentsSelected();
            }
            else if(strcmp("END", keyPressed + 2) == 0)
            {
                zoomExtentsAll();
            }
            break;
        }
    }
}

void
cScene::render(bool highlightSelected)
{
    iTestBed& testBed = *gTestBed;
    tSigned32 i;
    testBed.setColourRGB(0.0f,0.0f,0.85f);
    for(i = 0; i != SizeL(_asSceneObjects); ++i)
    {
        if(!highlightSelected || i != _currentlySelected)
        {
            _asSceneObjects[i]->renderFaces();
        }
    }
    if(highlightSelected)
    {
        testBed.setColourRGB(0.25f,0.25f,1.f);
        _asSceneObjects[_currentlySelected]->renderFaces();
    }

    testBed.setColour("blue");
    for(i = 0; i != SizeL(_asSceneObjects); ++i)
    {
        if(!highlightSelected || i != _currentlySelected)
        {
            _asSceneObjects[i]->renderEdges();
        }
    }
    if(highlightSelected)
    {
        testBed.setColour("white");
        _asSceneObjects[_currentlySelected]->renderEdges();
    }
}

void
cScene::save(cSimpleDOM& doc) const
{
    tSigned32 i;
    doc._name = "placement3D";
    doc._children.push_back(cSimpleDOM());
    cSimpleDOM& meshObjects = doc._children.back();
    meshObjects._name = "meshObjects";
    for(i = 0; i != SizeL(_meshObjects); ++i)
    {
        meshObjects._children.push_back(cSimpleDOM());
        cSimpleDOM& meshObject = meshObjects._children.back();
        meshObject._name = "meshObject";
        _meshObjects[i]->addAttributes(meshObject);
    }
    doc._children.push_back(cSimpleDOM());
    cSimpleDOM& solidObjects = doc._children.back();
    solidObjects._name = "solidObjects";
    for(i = 0; i != SizeL(_solidObjects); ++i)
    {
        solidObjects._children.push_back(cSimpleDOM());
        cSimpleDOM& solidObject = solidObjects._children.back();
        solidObject._name = "solidObject";
        _solidObjects[i]->addAttributes(solidObject);
    }
}

void
cScene::zoomExtentsAll()
{
    std::vector<const iFaceVertexMesh*> meshes;
    tSigned32 i;
    for(i = 0; i != SizeL(_meshObjects); ++i)
    {
        meshes.push_back(_meshObjects[i]->getCachedMesh());
    }
    for(i = 0; i != SizeL(_solidObjects); ++i)
    {
        meshes.push_back(_solidObjects[i]->getCachedMesh());
    }
    gTestBed->zoomExtents_FaceVertex(&meshes.front(), SizeL(meshes));
}

iMesh*
cScene::buildGround(tSigned32 extrudeHeight, tSigned32 maxStepHeight, tSigned32 maxSlope)
{
    //return BuildGround(_meshObjects, _solidObjects, extrudeHeight, maxStepHeight, maxSlope);
    cHorizontalRange worldRange;
    worldRange.minX = -16000;
    worldRange.minY = -50000;
    worldRange.maxX = 60000;
    worldRange.maxY = 40000;
    const tSigned32 tileSize = 20000;
    iSourceTiling* tiling = gPathEngine->buildRegularSourceTiling(worldRange, tileSize);
    iMesh* result = BuildGround_UseTiledProcessing(
            _meshObjects, _solidObjects,
            *tiling, 
            extrudeHeight, maxStepHeight, maxSlope
            );
    delete tiling;
    return result;
}


