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
#include "Scene.h"
#include "FileOutputStream.h"
#include "LoadWhiteSpaceDelimited.h"
#include "SaveWhiteSpaceDelimited.h"
#include "SimpleDOM.h"
#include "MessageBox.h"
#include "i_contentprocessing3d.h"
#include "i_testbed.h"
#include <fstream>
#include <sstream>

#define extern
#include "Globals.h"
#undef extern

void
Demo(iPathEngine* pathEngine, iTestBed* testBed, iContentProcessing3D* cp3D)
{
    const tSigned32 maxStepHeight = 500; // allow maximum 50 centimetres step height
    const tSigned32 maxSlope = 65; // allow maximum 65% gradient

    gPathEngine = pathEngine;
    gTestBed = testBed;
    gCP3D = cp3D;

    cSimpleDOM placement;
    {
        std::ifstream is("..\\resource\\placement3D\\demoPlacement.txt");
        if(!is.good())
        {
            Error("Fatal", "Could not open placement script from '..\\resource\\placement3D\\demoPlacement.txt'.");
        }
        LoadWhiteSpaceDelimited(is, placement);
    }
    assertR(placement._name == "placement3D");
    cScene scene(placement);

    if(scene.empty())
    {
        Error("Fatal", "Scene is empty.");
        return;
    }
    scene.initSelection();
    scene.zoomExtentsAll();

    iShape *agentShape;
    {
        tSigned32 shapeCoords[] =
        {
            80,200,
            200,80,
            200,-80,
            80,-200,
            -80,-200,
            -200,-80,
            -200,80,
            -80,200,
        };
        agentShape = pathEngine->newShape(sizeof(shapeCoords) / sizeof(*shapeCoords) / 2, shapeCoords);
    }

    std::vector<tSigned32> agentHeights;
    agentHeights.push_back(1700); // 1 metre 70 centimetres
    agentHeights.push_back(2800); // 2 metres 80 centimetres
    agentHeights.push_back(500); // 50 centimetres
    tSigned32 currentAgentHeight = 0;

    while(1)
    {
        bool proceedToNextState;
        proceedToNextState = false;
        do
        {
            scene.updateWithEditing();
            scene.render(true);
            testBed->setColour("green");
            {
                std::ostringstream oss;
                oss << "Current agent height is: " << agentHeights[currentAgentHeight] << " (millimeters)";
                testBed->printTextLine(10, oss.str().c_str());
            }

            const char* keyPressed;
            while(keyPressed = testBed->receiveKeyMessage())
            {
                if(keyPressed[0]!='d')
                {
                // only interested in key down messages
                    continue;
                }
                switch(keyPressed[1])
                {
                case '_':
                    if(strcmp("SPACE", keyPressed + 2) == 0)
                    {
                        proceedToNextState = true;
                        break;
                    }
                    break;
                case 'H':
                    currentAgentHeight++;
                    if(currentAgentHeight == SizeL(agentHeights))
                    {
                        currentAgentHeight = 0;
                    }
                    break;
                case 'S':
                    {
                        cSimpleDOM toSave;
                        scene.save(toSave);
                        std::ofstream os("..\\resource\\placement3D\\demoPlacement.txt");
                        if(!os.good())
                        {
                            Error("Fatal", "Could not open placement script to write at '..\\resource\\placement3D\\placement3D.txt'.");
                        }
                        SaveWhiteSpaceDelimited(toSave, os);
                        ShowMessageBox("3D content processing demo", "Scene contents saved successfully.");
                    }

                    break;
                }
                scene.handleKeyPress(keyPressed);
            }

            testBed->update();
        }
        while(!proceedToNextState);

        {
            cSimpleDOM toSave;
            scene.save(toSave);
            std::ofstream os("..\\resource\\placement3D\\lastProcessed.txt");
            if(!os.good())
            {
                Error("Fatal", "Could not open placement script to write at '..\\resource\\placement3D\\lastProcessed.txt'.");
            }
            SaveWhiteSpaceDelimited(toSave, os);
        }

        iMesh* mesh = scene.buildGround(agentHeights[currentAgentHeight], maxStepHeight, maxSlope);

        if(mesh == 0)
        {
            Error("NonFatal", "No ground faces in result of 3D content processing. Returning to edit mode.");
            continue;
        }

        iProgressCallBack* progressCallBack = gTestBed->getTestBedProgressCallBack();
        progressCallBack->updateProgress("Generating collision preprocess", 0.f);
        mesh->generateCollisionPreprocessFor(agentShape, 0);
        progressCallBack->updateProgress("Generating pathfind preprocess", 0.f);
        mesh->generatePathfindPreprocessFor(agentShape, 0);
        progressCallBack->updateProgress("Generating pathfind preprocess (completed)", 1.f);
        testBed->setMesh(mesh);

        iAgent* agent = 0;
        iPath* agentPath = 0;
        cPosition failedPlacementMarker;
        cPosition failedTargetMarker;
        cPosition failedPathfindMarker;
        failedPlacementMarker.cell = -1;
        bool paused = true;
        bool renderScene = false;

        proceedToNextState = false;
        do
        {
            if(renderScene)
            {
                testBed->setAdditiveBlending(0.2f);
                scene.render(false);
                testBed->setOpaque();
            }
            testBed->setColour("green");
            testBed->drawMesh();
            testBed->setColour("darkGreen");
            testBed->drawMeshEdges();
            testBed->setColour("orange");
            testBed->drawAgent(agent, agentHeights[currentAgentHeight]);
            testBed->drawPath(agentPath);
            testBed->setColour("red");
            if(failedPlacementMarker.cell != -1)
            {
                testBed->drawPosition(failedPlacementMarker, 1000);
                testBed->printTextLine(10, "Failed to resolve an unobstructed position for agent placement");
            }
            else if(failedTargetMarker.cell != -1)
            {
                testBed->drawPosition(failedTargetMarker, 1000);
                testBed->printTextLine(10, "Failed to resolve an unobstructed target position");
            }
            else if(failedPathfindMarker.cell != -1)
            {
                testBed->drawPosition(failedPathfindMarker, 1000);
                testBed->printTextLine(10, "No path to the selected target");
            }
            if(paused)
            {
                testBed->setColour("white");
                testBed->printTextLine(10, "Paused");
            }
            else if(agent)
            {
                agent->advanceAlongPath(agentPath, 200.f, 0);
            }

            const char* keyPressed;
            while(keyPressed = testBed->receiveKeyMessage())
            {
                if(keyPressed[0]!='d')
                {
                // only interested in key down messages
                    continue;
                }
                switch(keyPressed[1])
                {
                case '_':
                    if(strcmp("SPACE", keyPressed + 2) == 0)
                    {
                        proceedToNextState = true;
                        break;
                    }
                    break;
                case 'R':
                    renderScene = !renderScene;
                    break;
                case 'P':
                    paused = !paused;
                    break;
                case 'A':
                    failedPlacementMarker.cell = -1;
                    failedTargetMarker.cell = -1;
                    failedPathfindMarker.cell = -1;
                    if(agentPath)
                    {
                        delete agentPath;
                        agentPath = 0;
                    }
                // place or move the player controlled agent
                    {
                        cPosition p = testBed->positionAtMouse();
                        if(p.cell == -1)
                        {
                            break;
                        }
                        cPosition unobstructedP = mesh->findClosestUnobstructedPosition(agentShape, 0, p, 1000); // resolve unobstructed position within range of 1 metre
                        if(unobstructedP.cell == -1)
                        {
                            failedPlacementMarker = p;
                            break;
                        }
                    // place the agent if not yet placed, otherwise move to the requested position
                        if(!agent)
                        {
                            agent = mesh->placeAgent(agentShape, unobstructedP);
                        }
                        else
                        {
                            agent->moveTo(unobstructedP);
                        }
                    }
                    break;
                case 'T':
                    if(agent)
                    {
                    // set a target for pathfinding
                        failedPlacementMarker.cell = -1;
                        failedTargetMarker.cell = -1;
                        failedPathfindMarker.cell = -1;
                        if(agentPath)
                        {
                            delete agentPath;
                            agentPath = 0;
                        }

                        cPosition p = testBed->positionAtMouse();
                        if(p.cell == -1)
                        {
                            break;
                        }
                        cPosition unobstructedP = mesh->findClosestUnobstructedPosition(agentShape, 0, p, 1000); // resolve unobstructed position within range of 1 metre
                        if(unobstructedP.cell == -1)
                        {
                            failedTargetMarker = p;
                            break;
                        }
                        agentPath = agent->findShortestPathTo(0, unobstructedP);
                        if(agentPath == 0)
                        {
                            failedPathfindMarker = unobstructedP;
                        }
                    }
                    break;
                }
            }

            testBed->update();
        }
        while(!proceedToNextState);

        testBed->releaseMesh();
        delete mesh;
    }
}
