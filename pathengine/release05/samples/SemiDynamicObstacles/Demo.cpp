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
#include "DoubleBufferedObstacleSet.h"
#include "DynamicAgentManager.h"
#include "EntryPoint.h"
#include "PartitionedTerrain.h"
#include "FileOutputStream.h"
#include "LoadBinary.h"
#include "Error.h"
#include <string>
#include <vector>
#include <sstream>
#include <math.h>
#include <time.h>

#define extern
#include "Globals.h"
#undef extern

//#define USER_CONTROLLED_AGENT

class cAverageTimeTracker
{
    std::vector<double> _times;
    tSigned32 _bufferPosition;
    bool _bufferFull;
public:
    cAverageTimeTracker(tSigned32 bufferSize) :
      _times(bufferSize),
      _bufferPosition(0),
      _bufferFull(false)
    {
        assertD(bufferSize > 0);
    }
    void addTiming(double value)
    {
        _times[_bufferPosition++] = value;
        if(_bufferPosition == SizeL(_times))
        {
            _bufferPosition = 0;
            _bufferFull = true;
        }
    }
    bool ready() const
    {
        return _bufferFull;
    }
    double get()
    {
        assertD(ready());
        double result = _times[0];
        tSigned32 i;
        for(i = 1; i != SizeL(_times); ++i)
        {
            result += _times[i];
        }
        return result /= static_cast<double>(SizeL(_times));
    }
};

static void
RotateShapeCoords(tSigned32 vertices, const double* vertexCoords, double rotateBy, std::vector<tSigned32>& result)
{
    double sinOf = sin(rotateBy);
    double cosOf = cos(rotateBy);
    result.resize(vertices * 2);
    tSigned32 i;
    for(i = 0; i != vertices; ++i)
    {
        double x = vertexCoords[i * 2];
        double y = vertexCoords[i * 2 + 1];
        double rotatedX = sinOf * y + cosOf * x;
        double rotatedY = cosOf * y - sinOf * x;
        result[i * 2] = static_cast<tSigned32>(rotatedX);
        result[i * 2 + 1] = static_cast<tSigned32>(rotatedY);
    }
}

static void
PreGenerateRotationShapes(
        tSigned32 vertices, const double* vertexCoords,
        std::vector<iShape*>& result
        )
{
    static const tSigned32 NUMBER_OF_ROTATIONS = 30;
    static const double PI = 3.14159265358979323;
    result.resize(NUMBER_OF_ROTATIONS);
    std::vector<tSigned32> rotatedCoords;
    tSigned32 i;
    for(i = 0; i != NUMBER_OF_ROTATIONS; ++i)
    {
        double theta = PI * 2 * i / NUMBER_OF_ROTATIONS;
        RotateShapeCoords(vertices, vertexCoords, theta, rotatedCoords);
        if(!gPathEngine->shapeIsValid(vertices, &rotatedCoords[0]))
        {
            Error("Fatal", "Shape coordinates are not valid after rotation. Avoid small edges and angles near 180.");
        }
        result[i] = gPathEngine->newShape(vertices, &rotatedCoords[0]);
    }
}

static iAgent*
RandomlyPlaceObstacle(iMesh* mesh, const cPosition& centre, tSigned32 range, const std::vector<iShape*>& shapeRotations)
{
    int i = rand() % SizeL(shapeRotations);
    cPosition p = mesh->generateRandomPositionLocally(centre, range);
    return mesh->placeAgent(shapeRotations[i], p);
}

static iAgent*
PlaceWithRandomRotation(
        iMesh* mesh, const std::vector<iShape*>& shapeRotations, const cPosition& p
        )
{
    int i = rand() % SizeL(shapeRotations);
    return mesh->placeAgent(shapeRotations[i], p);
}

static void
RandomlyPlaceObstacles(
        iMesh* mesh,
        const cPosition& centre,
        const std::vector<iShape*>& shapeRotations,
        tSigned32 numberToAdd,
        tSigned32 range,
        cDoubleBufferedObstacleSet& addTo
        )
{
    tSigned32 i;
    for(i = 0; i != numberToAdd; ++i)
    {
        iAgent* placed = RandomlyPlaceObstacle(mesh, centre, range, shapeRotations);
        addTo.addObstacle(placed);
    }
}

static void
GenerateWallShapeCoords(
        tSigned32 startX, tSigned32 startY,
        tSigned32 endX, tSigned32 endY,
        tSigned32 radius,
        std::vector<tSigned32>& result
        )
{
    if(startX > endX || (startX == endX && startY > endY))
    {
        std::swap(startX, endX);
        std::swap(startY, endY);
    }
    if(startX == endX || startY == endY)
    {
        result.push_back(startX - radius);
        result.push_back(startY - radius);
        result.push_back(startX - radius);
        result.push_back(endY + radius);
        result.push_back(endX + radius);
        result.push_back(endY + radius);
        result.push_back(endX + radius);
        result.push_back(startY - radius);
        return;
    }
    if(startY > endY)
    {
        result.push_back(startX - radius);
        result.push_back(startY - radius);
        result.push_back(startX - radius);
        result.push_back(startY + radius);
        result.push_back(startX + radius);
        result.push_back(startY + radius);
        result.push_back(endX + radius);
        result.push_back(endY + radius);
        result.push_back(endX + radius);
        result.push_back(endY - radius);
        result.push_back(endX - radius);
        result.push_back(endY - radius);
        return;
    }
    result.push_back(startX + radius);
    result.push_back(startY - radius);
    result.push_back(startX - radius);
    result.push_back(startY - radius);
    result.push_back(startX - radius);
    result.push_back(startY + radius);
    result.push_back(endX - radius);
    result.push_back(endY + radius);
    result.push_back(endX + radius);
    result.push_back(endY + radius);
    result.push_back(endX + radius);
    result.push_back(endY - radius);
}

static iAgent*
PlaceWallAgent(
        const cPosition& start,
        const cPosition& end,
        tSigned32 radius, 
        iMesh* mesh
        )
{
    assertD(radius >= 1);
    std::vector<tSigned32> coords;
    GenerateWallShapeCoords(start.x, start.y, end.x, end.y, radius, coords);
    return mesh->placeLargeStaticObstacle(SizeL(coords) / 2, &coords[0], start);
}

static iMesh*
BuildBaseGround()
{
  // generate terrain stand-in geometry for the range -10000,-10000 -> 10000,10000
  // world coordinates are in centimetres, so this corresponds to a 200 metres by 200 metres region centred on the origin
    cPartitionedTerrain terrain(-10000, -10000, 2000, 10);
    //cPartitionedTerrain terrain(-30000, -30000, 2000, 30);

    std::vector<const iFaceVertexMesh*> groundParts;
    groundParts.push_back(&terrain);

    const char* options[5];
    options[0] = "partitionTranslationTo3D";
    options[1] = "true";
    options[2] = "useIdentityMapping";
    options[3] = "true";
    options[4] = 0;
    return gPathEngine->buildMeshFromContent(&groundParts.front(), groundParts.size(), options);
}

void
demo(iPathEngine* pathEngine, iTestBed* testBed)
{
    gPathEngine = pathEngine;
    gTestBed = testBed;

    iShape* agentShape;
    {
        tSigned32 array[]=
        {
            -20,-20,
            -20,20,
            20,20,
            20,-20,
        };
        agentShape = gPathEngine->newShape(sizeof(array) / sizeof(*array) / 2, array);
    }

    iShape* erasorShape;
    {
        tSigned32 array[]=
        {
            -4000,-4000,
            -4000,4000,
            4000,4000,
            4000,-4000,
        };
        erasorShape = gPathEngine->newShape(sizeof(array) / sizeof(*array) / 2, array);
    }

    std::vector<iShape*> rockShapes;
    {
        double array[]=
        {
            -180., 60.,
            -60., 100.,
            210., 20.,
            186., -36.,
            120., -48.,
            -180., -60.,
        };
        PreGenerateRotationShapes(sizeof(array) / sizeof(*array) / 2, array, rockShapes);
    }
    std::vector<iShape*> hutShapes;
    {
        double array[]=
        {
            -250., -200.,
            -250., 200.,
            250., 200.,
            250., -200.
        };
        PreGenerateRotationShapes(sizeof(array) / sizeof(*array) / 2, array, hutShapes);
    }

  // load an existing mesh, if previously saved out
  // or if the file is not present then build a new mesh
    iMesh* mesh;
    if(FileExists("..\\resource\\meshes\\semiDynamicObstacles.tok"))
    {
        char* buffer;
        unsigned long size;
        buffer = gTestBed->loadBinary("..\\resource\\meshes\\semiDynamicObstacles.tok", size);
        mesh = gPathEngine->loadMeshFromBuffer("tok", buffer, size, 0);
        gTestBed->freeBuffer(buffer);
    }
    else
    {
        mesh = BuildBaseGround();
    }

    gTestBed->setColour("white");
    gTestBed->printTextLine(10, "Generating base mesh preprocess...");
    gTestBed->update();

    mesh->generateCollisionPreprocessFor(agentShape, 0);
    mesh->generateCollisionPreprocessFor(erasorShape, 0);

    {
      // base pathfinding preprocess must currently be present on the mesh to use preprocessed obstacle sets
        const char* attributes[3];
        attributes[0] = "split_with_circumference_below";
        attributes[1] = "5000";
        attributes[2] = 0;
        mesh->generatePathfindPreprocessFor(agentShape, attributes);
    }


    gTestBed->setColour("white");
    gTestBed->printTextLine(10, "Initialising double buffered obstacle set...");
    gTestBed->update();

    const char* preprocessAttributes[5];
    preprocessAttributes[0] = "split_with_circumference_below";
    preprocessAttributes[1] = "5000";
    preprocessAttributes[2] = "enableConnectedRegionQueries";
    preprocessAttributes[3] = "true";
    preprocessAttributes[4] = 0;
    cDoubleBufferedObstacleSet set(mesh, agentShape, preprocessAttributes);

    gTestBed->setColour("white");
    gTestBed->printTextLine(10, "Initialising double buffered obstacle set... (completed)");
    gTestBed->update();

    gTestBed->setMesh(mesh); // set this mesh as the current mesh for rendering
    gTestBed->zoomExtents(); // place camera to see all of newly loaded mesh

    gTestBed->setColour("white");
    gTestBed->printTextLine(10, "Initialising dynamic agents...");
    gTestBed->update();

    set.lockForeground();
#ifdef USER_CONTROLLED_AGENT
    iAgent* userControlledAgent;
    iPath* userControlledAgentPath;
    {
        cPosition p;
        do
        {
            p = mesh->generateRandomPosition();
        }
        while(mesh->testPointCollision(agentShape, set.getForegroundCollisionContext(), p));
        userControlledAgent = mesh->placeAgent(agentShape, p);
        userControlledAgentPath = 0;
    }
#else
    cDynamicAgentManager dynamicAgents(mesh, agentShape, set.getForegroundCollisionContext(), 1000);
#endif
    set.unlockForeground();

    gTestBed->setColour("white");
    gTestBed->printTextLine(10, "Initialising dynamic agents... (completed)");
    gTestBed->update();

    bool erasorOn = false;
    bool displayTimings = false;

    cAverageTimeTracker moveAlongTimeTracker(40);
    cAverageTimeTracker rePathTimeTracker(40);
    cAverageTimeTracker drawAgentsTimeTracker(40);
    cAverageTimeTracker assignRegionsTimeTracker(40);
    double lastUserPathTime = 0.0;

    cPosition wallStart, wallEnd;

    while(1)
    {
    // generate or regenerate set of placed obstacle
        bool regenerate = false;
        do
        {
#ifdef USER_CONTROLLED_AGENT
            if(userControlledAgentPath)
            {
                set.lockForeground();
                iCollisionInfo* ci = userControlledAgent->advanceAlongPath(userControlledAgentPath, 40.f, set.getForegroundCollisionContext());
                if(ci)
                {
                    delete ci;
                    delete userControlledAgentPath;
                    userControlledAgentPath = 0;
                }
                else
                {
                    if(userControlledAgentPath->size() == 1)
                    {
                        delete userControlledAgentPath;
                        userControlledAgentPath = 0;
                    }
                }
                set.unlockForeground();
            }
#else
            {
                set.lockForeground();
                assertR(!set.getForegroundPreprocessedSet()->pathfindPreprocessNeedsUpdate(agentShape));
                clock_t start, finish;
                start = clock();
                dynamicAgents.moveAlongPaths(set.getForegroundCollisionContext());
                finish = clock();
                moveAlongTimeTracker.addTiming(static_cast<double>(finish - start) / CLOCKS_PER_SEC);
                start = clock();
                dynamicAgents.rePath(set.getForegroundPreprocessedSet(), set.getForegroundCollisionContext(), 20);
                finish = clock();
                rePathTimeTracker.addTiming(static_cast<double>(finish - start) / CLOCKS_PER_SEC);
                set.unlockForeground();
            }
            {
                clock_t start, finish;
                start = clock();
                set.lockForeground();
                dynamicAgents.assignRegions(set.getForegroundPreprocessedSet());
                set.unlockForeground();
                finish = clock();
                assignRegionsTimeTracker.addTiming(static_cast<double>(finish - start) / CLOCKS_PER_SEC);
            }
#endif

        // draw mesh
            gTestBed->setColourRGB(0.0f,0.0f,0.85f);
            gTestBed->drawMesh();
            gTestBed->setColour("blue");
            gTestBed->drawMeshEdges();
            gTestBed->setColour("white");
            gTestBed->drawBurntInObstacles();
            gTestBed->setColour("orange");
            gTestBed->drawMeshExpansion(agentShape);

        // draw obstacles in double buffered set
            {
                set.lockForeground();
                gTestBed->setColour("white");
                set.renderPreprocessed();
                gTestBed->setColour("orange");
                set.renderDynamic();
                gTestBed->setColour("red");
                set.renderToBeDeleted();
                gTestBed->setColour("orange");
                set.renderPreprocessedExpansion();
                set.unlockForeground();
            }

            if(erasorOn)
            {
                cPosition p = testBed->positionAtMouse();
                if(p.cell != -1)
                {
                    gTestBed->setColour("red");
                    gTestBed->setAdditiveBlending(0.3f);
                    gTestBed->drawShape(erasorShape, p, 20);
                    gTestBed->setOpaque();
                }
            }

            if(wallStart.cell != -1)
            {
                gTestBed->setColour("green");
                iAgent* a;
                if(gTestBed->getKeyState("_LSHIFT"))
                {
                    a = PlaceWallAgent(wallStart, wallEnd, 150, mesh);
                }
                else
                {
                    a = PlaceWallAgent(wallStart, wallEnd, 100, mesh);
                }
                gTestBed->setAdditiveBlending(0.3f);
                gTestBed->drawAgent(a, 20);
                gTestBed->setOpaque();
                delete a;
            }

#ifdef USER_CONTROLLED_AGENT
            gTestBed->setColour("green");
            gTestBed->drawAgent(userControlledAgent, 60);
            gTestBed->drawPath(userControlledAgentPath);
#else
            {
                clock_t start, finish;
                start = clock();
                dynamicAgents.renderAgents_ColouredByRegion();
                finish = clock();
                drawAgentsTimeTracker.addTiming(static_cast<double>(finish - start) / CLOCKS_PER_SEC);
            }
#endif

            gTestBed->setColour("orange");
            {
                std::ostringstream oss;
                set.lockForeground();
                oss << set.numberOfPreprocessedObstacles() << " preprocessed obstacles, ";
                oss << set.numberOfDynamicObstacles() << " dynamic obstacles, ";
                oss << set.getForegroundPreprocessedSet()->getNumberOfConnectedRegions(agentShape) << " connected regions";
#ifndef USER_CONTROLLED_AGENT
                oss << ", " << dynamicAgents.size()  << " moving agents";
#endif
                set.unlockForeground();
                gTestBed->printTextLine(0, oss.str().c_str());
            }
            if(set.updateInProgress())
            {
                gTestBed->printTextLine(0, "Preprocess update in progress..");
            }
            if(displayTimings)
            {
                gTestBed->setColour("purple");
                if(!set.updateInProgress())
                {
                    std::ostringstream oss;
                    oss << "last preprocess update time: " << set.getLastUpdateTime();
                    gTestBed->printTextLine(0, oss.str().c_str());
                    //{
                    //    std::ostringstream oss;
                    //    oss << "last buffer swap time: " << set.getLastBufferSwapTime();
                    //    gTestBed->printTextLine(10, oss.str().c_str());
                    //}
                }
                if(moveAlongTimeTracker.ready())
                {
                    std::ostringstream oss;
                    oss << "advance along paths (average per frame): " << moveAlongTimeTracker.get() << "s";
                    gTestBed->printTextLine(0, oss.str().c_str());
                }
                if(rePathTimeTracker.ready())
                {
                    std::ostringstream oss;
                    oss << "new path generation (average per frame): " << rePathTimeTracker.get() << "s";
                    gTestBed->printTextLine(0, oss.str().c_str());
                }
                if(drawAgentsTimeTracker.ready())
                {
                    std::ostringstream oss;
                    oss << "draw agents (average per frame): " << drawAgentsTimeTracker.get() << "s";
                    gTestBed->printTextLine(0, oss.str().c_str());
                }
                if(assignRegionsTimeTracker.ready())
                {
                    std::ostringstream oss;
                    oss << "assign regions (average per frame): " << assignRegionsTimeTracker.get() << "s";
                    gTestBed->printTextLine(0, oss.str().c_str());
                }
#ifdef USER_CONTROLLED_AGENT
                {
                    std::ostringstream oss;
                    oss << "last user selected path time: " << lastUserPathTime << "s";
                    gTestBed->printTextLine(0, oss.str().c_str());
                }
#endif
            }
            gTestBed->setColour("lightgrey");
            gTestBed->printTextLine(0, "Press 'T' to toggle timing info");

        // tell the gTestBed to render this frame
            gTestBed->update();

        // receive and process messages for all keys pressed since last frame
            cPosition p = testBed->positionAtMouse();
            const char* keyPressed;
            while(keyPressed = gTestBed->receiveKeyMessage())
            {
                if(keyPressed[0] != 'd') // is it a key down message?
                    continue;

                switch(keyPressed[1])
                {
                case '_':
                    {
                        if(!strcmp("ESCAPE", keyPressed + 2))
                        {
                            return;
                        }
                        if(!strcmp("SPACE", keyPressed + 2) && !set.updateInProgress())
                        {
                            regenerate = true;
                        }
                        break;
                    }
                case 'H':
                    if(p.cell != -1)
                    {
                        if(gTestBed->getKeyState("_LSHIFT"))
                        {
                            RandomlyPlaceObstacles(mesh, p, hutShapes, 30, 5000, set);
                        }
                        else
                        {
                            set.addObstacle(PlaceWithRandomRotation(mesh, hutShapes, p));
                        }
                    }
                    break;
                case 'R':
                    if(p.cell != -1)
                    {
                        if(gTestBed->getKeyState("_LSHIFT"))
                        {
                            RandomlyPlaceObstacles(mesh, p, rockShapes, 25, 3000, set);
                        }
                        else
                        {
                            set.addObstacle(PlaceWithRandomRotation(mesh, rockShapes, p));
                        }
                    }
                    break;
                case 'W':
                    if(wallStart.cell != -1)
                    {
                        if(gTestBed->getKeyState("_LSHIFT"))
                        {
                            set.addObstacle(PlaceWallAgent(wallStart, wallEnd, 150, mesh));
                        }
                        else
                        {
                            set.addObstacle(PlaceWallAgent(wallStart, wallEnd, 100, mesh));
                        }
                        wallStart = wallEnd;
                    }
                    break;
                case 'D':
                    if(p.cell != -1)
                    {
                        set.lockForeground();
                        std::vector<iAgent*> resultsBuffer(set.numberOfObstaclesAdded() + 1); // add one to ensure buffer can be dereferenced without causing STL assert
                        tSigned32 numberOverlapped;
                        if(erasorOn)
                        {
                            mesh->getAllAgentsOverlapped(
                                    erasorShape, 
                                    set.getForegroundCollisionContext(),
                                    p,
                                    &resultsBuffer[0],
                                    numberOverlapped
                                    );
                        }
                        else
                        {
                            mesh->getAllAgentsOverlapped(
                                    agentShape,
                                    set.getForegroundCollisionContext(),
                                    p,
                                    &resultsBuffer[0],
                                    numberOverlapped
                                    );
                        }
                        set.unlockForeground();
                        tSigned32 i;
                        for(i = 0; i != numberOverlapped; ++i)
                        {
                            set.removeAndDeleteObstacle(resultsBuffer[i]);
                        }
                    }
                    break;
                case 'E':
                    erasorOn = !erasorOn;
                    break;
                case 'T':
                    displayTimings = !displayTimings;
                    break;
                case 'C':
                    if(p.cell != -1)
                    {
                        if(gTestBed->getKeyState("_LSHIFT") || wallStart.cell == -1)
                        {
                          // start new wall
                            wallStart = p;
                        }
                        wallEnd = p;
                    }
                    break;
                case 'S':
                    {
                        set.storeToNamedObstacles();
                        cFileOutputStream fos("..\\resource\\meshes\\semiDynamicObstacles.tok");
                        mesh->saveGround("tok", true, &fos);
                    }
                    break;
#ifdef USER_CONTROLLED_AGENT
                case 'P':
                    delete userControlledAgentPath;
                    userControlledAgentPath = 0;
                    if(p.cell != -1)
                    {
                        set.lockForeground();
                        cPosition target = mesh->findClosestUnobstructedPosition(
                                agentShape, 
                                set.getForegroundCollisionContext(),
                                p, 100
                                );
                        if(target.cell != -1)
                        {
                            const tSigned32 numberOfIterations = 100;
                            tSigned32 i;
                            clock_t start, finish;
                            start = clock();
                            for(i = 0; i != numberOfIterations; ++i)
                            {
                                delete userControlledAgentPath;
                                userControlledAgentPath = userControlledAgent->findShortestPathTo(
                                        set.getForegroundCollisionContext(),
                                        target
                                        );
                            }
                            finish = clock();
                            lastUserPathTime = static_cast<double>(finish - start) / CLOCKS_PER_SEC / numberOfIterations;
                        }
                        set.unlockForeground();
                    }
                    break;
#endif
                }
            }
        }
        while(regenerate == false);
        set.startUpdate();        
    }
}
