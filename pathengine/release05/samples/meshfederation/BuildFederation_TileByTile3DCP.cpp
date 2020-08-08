
#include "Header.h"
#include "BuildFederation_TileBytile3DCP.h"
#include "FileOutputStream.h"
#include "SimpleDOM.h"
#include "LoadBinary.h"
#include "i_pathengine.h"
#include "i_testbed.h"
#include <string>
#include <fstream>
#include <sstream>

static void
BuildTile(
        iTestBed* testBed,
        iMeshFederation* federation,
        long tileIndex,
        const iSourceTiling& tiling,
        const std::vector<const iMesh*>& sourceTiles
        )
{
    tSigned32 numberOfTileMeshes = federation->getNumberOfGroundTilesOverlapped(tileIndex, &tiling);
    std::vector<tSigned32> tileMeshIndices(numberOfTileMeshes);
    federation->getGroundTilesOverlapped(tileIndex, &tiling, &tileMeshIndices[0], numberOfTileMeshes);
    std::vector<const iMesh*> tileMeshes(numberOfTileMeshes);
    tSigned32 i;
    for(i = 0; i != numberOfTileMeshes; ++i)
    {
        tileMeshes[i] = sourceTiles[tileMeshIndices[i]];
    }
    iMesh* tileMesh = federation->buildTileMeshFromLocalGroundTiles(
            tileIndex, &tiling,
            &tileMeshes[0], numberOfTileMeshes,
            0
            );
    std::ostringstream fileName;
    fileName << "federationTile" << tileIndex << ".tok";
    cFileOutputStream fos(fileName.str().c_str());
    tileMesh->saveGround("tok", true, &fos);
    delete tileMesh;
    testBed->printTextLine(5, fileName.str().c_str());
    testBed->printTextLine(5, "finished generating federation tile:");
    testBed->printTextLine(5, "Generating Federation Tile Meshes");
    testBed->update();
}

void
LoadAllSourceTiles(
        iPathEngine* pathEngine, 
        tSigned32 numberToLoad,
        std::vector<const iMesh*>& result
        )
{
    tSigned32 i;
    for(i = 0; i != numberToLoad; ++i)
    {
        std::string meshName;
        {
            std::ostringstream oss;
            oss << "tileResult" << i << ".tok";
            meshName = oss.str();
        }
        std::vector<char> meshBuffer;
        LoadBinary(meshName.c_str(), meshBuffer);
        if(meshBuffer.empty())
        {
            result.push_back(0);
        }
        else
        {
            iMesh* sourceTile = pathEngine->loadMeshFromBuffer(
                    "tok",
                    VectorBuffer(meshBuffer), SizeL(meshBuffer),
                    0
                    );
            result.push_back(sourceTile);
        }
    }
}

void
BuildFederation_TileByTile3DCP(
        iPathEngine* pathEngine, iTestBed* testBed,
        long tileSize, long overlap
        )
{
    iSourceTiling* tiling;
    if(!FileExists("tiling.tok"))
    {
        Error("Fatal", "Tiling file not found. Please run the 3D content processing first to generated the necessary tiling and tiled ground result pieces.");
    }
    {
        std::vector<char> tilingBuffer;
        LoadBinary("tiling.tok", tilingBuffer);
        tiling = pathEngine->loadSourceTiling("tok", VectorBuffer(tilingBuffer), SizeL(tilingBuffer));
    }

    std::vector<const iMesh*> sourceTiles;
    LoadAllSourceTiles(pathEngine, tiling->size(), sourceTiles);

    cHorizontalRange worldRange;
    tiling->getWorldRange(worldRange);

    iMeshFederation* federation = pathEngine->buildMeshFederation_TilingOnly(worldRange, tileSize, overlap);

    long i;
    for(i = 0; i != federation->size(); ++i)
    {
        BuildTile(testBed, federation, i, *tiling, sourceTiles);
    }
    delete tiling;

    std::ostringstream fileName;
    fileName << "federation.tok";
    cFileOutputStream fos(fileName.str().c_str());
    federation->save("tok", &fos);
    delete federation;
}

