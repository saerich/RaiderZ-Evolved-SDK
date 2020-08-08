
#include "entrypoint.h"
#include "FileOutputStream.h"
#include "LoadWhiteSpaceDelimited.h"
#include "LoadContentChunkPlacement.h"
#include "SimpleDOM.h"
#include "PartitionedTerrain.h"
#include <vector>
#include <string>
#include <sstream>
#include <fstream>


void demo(iPathEngine* pathEngine, iTestBed* testBed)
{
    cSimpleDOM placementScript;
    {
        std::ifstream is("..\\resource\\contentChunkPlacement\\contentchunks_example.txt");
        if(!is.good())
        {
            Error("Fatal", "Could not open content chunk placement script('resource\\contentChunkPlacement\\contentchunks_example.txt').");
            return;
        }
        LoadWhiteSpaceDelimited(is, placementScript);
    }

    std::vector<iContentChunk*> loadedChunks;
    std::vector<iContentChunkInstance*> placedInstances;
    LoadContentChunkPlacement(pathEngine, testBed, placementScript, loadedChunks, placedInstances, false);

  // generate terrain stand-in geometry for the range -100000,-100000 -> 100000,100000
  // world coordinates are in millimetres, so this corresponds to a 200 metres by 200 metres region centred on the origin
    cPartitionedTerrain terrain(-100000, -100000, 20000, 10);

    std::vector<const iFaceVertexMesh*> groundParts;

    groundParts.push_back(&terrain);
    for(size_t i = 0; i < placedInstances.size(); ++i)
    {
        if(placedInstances[i]->getContentChunk()->hasGroundComponent())
        {
            groundParts.push_back(placedInstances[i]->getGroundPointer());
        }
    }

    // the following lines can be uncommented to write a file containing a snapshot of the data being passed into content processing
    // this can be imported into 3DS Max, and is useful for troubleshooting any problems with the content processing
//    {
//        cFileOutputStream fos("contentSnapshot.tok");
//        pathEngine->saveContentData(&groundParts.front(), groundParts.size(), "tok", &fos);
//    }

    iMesh* mesh = pathEngine->buildMeshFromContent(&groundParts.front(), groundParts.size(), 0);

    if(mesh)
    {
        for(size_t i = 0; i < placedInstances.size(); ++i)
        {
            std::ostringstream prefix;
            prefix << i << ':';
            placedInstances[i]->addAnchorsAndShapes(mesh, prefix.str().c_str(), 1);
        }
    }
    for(size_t i = 0; i < loadedChunks.size(); ++i)
    {
        delete loadedChunks[i];
    }
    loadedChunks.clear();

    if(mesh == 0)
    {
        return;
    }

    testBed->setMesh(mesh);
    testBed->zoomExtents();

    while(!testBed->getKeyState("_SPACE"))
    {
        testBed->setColourRGB(0.0f,0.0f,0.85f);
        testBed->drawMesh();
        testBed->setColour("blue");
        testBed->drawMeshEdges();
        testBed->setColour("white");
        testBed->drawBurntInObstacles();
        testBed->printTextLine(10, "press space to quit..");
        testBed->printTextLine(10, "result of buildMeshFromContent()");
        testBed->update();
    }
}
