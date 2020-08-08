//**********************************************************************
//
// Copyright (c) 2007
// PathEngine
// Lyon, France
//
// All Rights Reserved
//
//**********************************************************************

#include "Header.h"
#include "BuildGround.h"
#include "Globals.h"
#include "SolidObjects.h"
#include "SolidObject.h"
#include "MeshObject.h"
#include "VectorBuildingOutputStream.h"
#include "FileOutputStream.h"
#include "LoadBinary.h"
#include "SaveBinary.h"
#include "i_contentprocessing3d.h"
#include "i_testbed.h"
#include <sstream>

//... for debugging purposes only
//#include "FileOutputStream.h"

static bool
RangesTouchOrOverlap(const cHorizontalRange& r1, const cHorizontalRange& r2)
{
    if(r1.maxX < r2.minX)
    {
        return false;
    }
    if(r1.minX > r2.maxX)
    {
        return false;
    }
    if(r1.maxY < r2.minY)
    {
        return false;
    }
    if(r1.minY > r2.maxY)
    {
        return false;
    }
    return true;
}

//static iMesh*
//BuildTileResult(
//        std::vector<const iFaceVertexMesh*> meshInstances,
//        const iSolidObjects& solidObjects,
//        tSigned32 extrudeHeight, tSigned32 maxStepHeight, tSigned32 maxSlope,
//        const char *const* options
//        )
//{
//    iProgressCallBack* progressCallBack = gTestBed->getTestBedProgressCallBack();
//    cVectorBuildingOutputStream vos;
//    gCP3D->buildAndSaveGround(
//        VectorBuffer(meshInstances), SizeL(meshInstances),
//        &solidObjects,
//        extrudeHeight, maxStepHeight, maxSlope,
//        options,
//        progressCallBack,
//        "tok", false, &vos
//        );
//    if(vos._buffer.empty())
//    {
//        return 0;
//    }
//    return gPathEngine->loadMeshFromBuffer(
//            "tok",
//            VectorBuffer(vos._buffer), SizeL(vos._buffer),
//            0
//            );
//}

static iMesh*
BuildTileResult_Cached(
        tSigned32 tileIndex,
        std::vector<const iFaceVertexMesh*> meshInstances,
        const iSolidObjects& solidObjects,
        tSigned32 extrudeHeight, tSigned32 maxStepHeight, tSigned32 maxSlope,
        const char *const* options
        )
{
    iProgressCallBack* progressCallBack = gTestBed->getTestBedProgressCallBack();

    std::string snapshotName, meshName;
    {
        std::ostringstream oss;
        oss << "tileResult" << tileIndex << ".snapshot.tok";
        snapshotName = oss.str();
    }
    {
        std::ostringstream oss;
        oss << "tileResult" << tileIndex << ".tok";
        meshName = oss.str();
    }

    progressCallBack->updateProgress("Building snapshot for tile", 0.f);
    cVectorBuildingOutputStream snapshotVOS;
    gCP3D->saveSnapshot(
            VectorBuffer(meshInstances), SizeL(meshInstances),
            &solidObjects,
            extrudeHeight, maxStepHeight, maxSlope,
            options,
            "tok", &snapshotVOS
            );
    const std::vector<char>& currentSnapshot = snapshotVOS._buffer;

    std::vector<char> cachedSnapshot;
    if(FileExists(snapshotName.c_str()))
    {
        LoadBinary(snapshotName.c_str(), cachedSnapshot);
    }
    if(currentSnapshot == cachedSnapshot)
    {
      // 3D source data is exactly identical to cached snapshot
      // so we can just reload the existing ground result for this tile
        std::vector<char> cachedMesh;
        LoadBinary(meshName.c_str(), cachedMesh);
        if(cachedMesh.empty())
        {
            return 0;
        }
        progressCallBack->updateProgress("Loading cached result", 0.f);
        iMesh* result = gPathEngine->loadMeshFromBuffer(
                "tok",
                VectorBuffer(cachedMesh), SizeL(cachedMesh),
                0
                );
        progressCallBack->updateProgress("Loading cached result (completed)", 1.f);
        return result;
    }

    cVectorBuildingOutputStream meshVOS;
    gCP3D->buildAndSaveGround(
        VectorBuffer(meshInstances), SizeL(meshInstances),
        &solidObjects,
        extrudeHeight, maxStepHeight, maxSlope,
        options,
        progressCallBack,
        "tok", false, &meshVOS
        );

    SaveBinary(snapshotName.c_str(), currentSnapshot);
    SaveBinary(meshName.c_str(), meshVOS._buffer);

    if(meshVOS._buffer.empty())
    {
        return 0;
    }
    return gPathEngine->loadMeshFromBuffer(
            "tok",
            VectorBuffer(meshVOS._buffer), SizeL(meshVOS._buffer),
            0
            );
}

iMesh*
BuildGround(
        const std::vector<cMeshObject*>& meshObjects,
        const std::vector<cSolidObject*>& solidObjects,
        tSigned32 extrudeHeight, tSigned32 maxStepHeight, tSigned32 maxSlope
        )
{
    std::vector<const iFaceVertexMesh*> meshInstances;
    cSolidObjects allSolidObjects;
    tSigned32 i;
    for(i = 0; i != SizeL(meshObjects); ++i)
    {
        meshInstances.push_back(meshObjects[i]->getCachedMesh());
    }
    for(i = 0; i != SizeL(solidObjects); ++i)
    {
        solidObjects[i]->addTransformedOperands(allSolidObjects);
    }

    iProgressCallBack* progressCallBack = gTestBed->getTestBedProgressCallBack();

    iMesh* result;
    //{
    //    cFileOutputStream fos("..\\internalResource\\content3D\\contentProcessing3DDemo.tok");
    //    Save3DContentSnapshot(
    //        VectorBuffer(meshInstances), SizeL(meshInstances),
    //        &solidObjects,
    //        extrudeHeight,
    //        maxStepHeight,
    //        maxSlope,
    //        0,
    //        "tok", &fos
    //        );
    //}

    cVectorBuildingOutputStream vos;
    gCP3D->buildAndSaveGround(
            VectorBuffer(meshInstances), SizeL(meshInstances),
            &allSolidObjects,
            extrudeHeight,
            maxStepHeight,
            maxSlope,
            0, // no options
            progressCallBack,
            "tok", false, &vos
            );
    result = gPathEngine->loadMeshFromBuffer(
            "tok",
            VectorBuffer(vos._buffer), SizeL(vos._buffer),
            0
            );

    //cFileOutputStream fos("contentProcessingResult.tok");
    //result->saveGround("tok", false, &fos);

    return result;
}

iMesh*
BuildGround_UseTiledProcessing(
        const std::vector<cMeshObject*>& meshObjects,
        const std::vector<cSolidObject*>& solidObjects,
        const iSourceTiling& tiling,
        tSigned32 extrudeHeight, tSigned32 maxStepHeight, tSigned32 maxSlope
        )
{
    {
      // save the source tiling object together with the tiled ground result
      // this is not actually required for this demo
      // but is necessary for mesh federation example to build from the tiled ground result
        cFileOutputStream fos("tiling.tok");
        tiling.save("tok", &fos);
    }
    const char* options[3];
    options[0] = "clipToTile";
    options[2] = 0;
    std::vector<const iMesh*> tileMeshes;
    tSigned32 tileIndex;
    for(tileIndex = 0; tileIndex != tiling.size(); ++tileIndex)
    {
        cHorizontalRange tileFilterRange;
        tiling.getTileFilterRange(tileIndex, tileFilterRange);
        std::vector<const iFaceVertexMesh*> meshInstances;
        cSolidObjects allSolidObjects;
        tSigned32 i;
        for(i = 0; i != SizeL(meshObjects); ++i)
        {
            if(RangesTouchOrOverlap(meshObjects[i]->cachedRange(), tileFilterRange))
            {
                meshInstances.push_back(meshObjects[i]->getCachedMesh());
            }
        }
        for(i = 0; i != SizeL(solidObjects); ++i)
        {
            if(RangesTouchOrOverlap(solidObjects[i]->cachedRange(), tileFilterRange))
            {
                solidObjects[i]->addTransformedOperands(allSolidObjects);
            }
        }
        options[1] = tiling.getTileRangeAsString(tileIndex);
        //iMesh* tileResult = BuildTileResult(
        //        meshInstances, allSolidObjects, 
        //        extrudeHeight, maxStepHeight, maxSlope,
        //        options
        //        );
        iMesh* tileResult = BuildTileResult_Cached(
                tileIndex,
                meshInstances, allSolidObjects, 
                extrudeHeight, maxStepHeight, maxSlope,
                options
                );
        tileMeshes.push_back(tileResult);
    }

    iProgressCallBack* progressCallBack = gTestBed->getTestBedProgressCallBack();
    progressCallBack->updateProgress("Connecting tiled ground result pieces", 0.f);
    iMesh* result = gPathEngine->buildMeshFromGroundTiles(
            &tiling,
            &tileMeshes[0], static_cast<tSigned32>(tileMeshes.size()),
            0
            );
    progressCallBack->updateProgress("Connecting tiled ground result pieces (completed)", 1.f);
    tSigned32 i;
    for(i = 0; i != static_cast<tSigned32>(tileMeshes.size()); ++i)
    {
        delete tileMeshes[i];
    }

    //cFileOutputStream fos("contentProcessingResult.tok");
    //result->saveGround("tok", false, &fos);

    return result;
}
