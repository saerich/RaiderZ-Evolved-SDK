//**********************************************************************
//
// Copyright (c) 2006
// PathEngine
// Lyon, France
//
// All Rights Reserved
//
//**********************************************************************

#ifndef CONTENT_PROCESSING_3D_DEMO__SCENE_INCLUDED
#define CONTENT_PROCESSING_3D_DEMO__SCENE_INCLUDED

#include <vector>

class bSceneObject;
class cMeshObject;
class cSolidObject;
class bSceneObject;
class cSimpleDOM;
class iMesh;

class cScene
{
    std::vector<cMeshObject*> _meshObjects;
    std::vector<cSolidObject*> _solidObjects;
    std::vector<bSceneObject*> _asSceneObjects;
    tSigned32 _currentlySelected;

    void zoomExtentsSelected();

public:

    cScene(const cSimpleDOM& placement);
    ~cScene();

    bool empty() const;
    void initSelection();

    void updateWithEditing();
    void handleKeyPress(const char* keyPressed);
    void render(bool highlightSelected);

    void save(cSimpleDOM& saveTo) const;

    void zoomExtentsAll();

    iMesh* buildGround(tSigned32 extrudeHeight, tSigned32 maxStepHeight, tSigned32 maxSlope);
};

#endif
