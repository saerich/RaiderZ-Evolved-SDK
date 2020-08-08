//**********************************************************************
//
// Copyright (c) 2007
// PathEngine
// Lyon, France
//
// All Rights Reserved
//
//**********************************************************************

#include <vector>

class iMesh;
class cMeshObject;
class cSolidObject;
class iSourceTiling;

iMesh*
BuildGround(
        const std::vector<cMeshObject*>& meshObjects,
        const std::vector<cSolidObject*>& solidObjects,
        tSigned32 extrudeHeight, tSigned32 maxStepHeight, tSigned32 maxSlope
        );

iMesh*
BuildGround_UseTiledProcessing(
        const std::vector<cMeshObject*>& meshObjects,
        const std::vector<cSolidObject*>& solidObjects,
        const iSourceTiling& tiling,
        tSigned32 extrudeHeight, tSigned32 maxStepHeight, tSigned32 maxSlope
        );
