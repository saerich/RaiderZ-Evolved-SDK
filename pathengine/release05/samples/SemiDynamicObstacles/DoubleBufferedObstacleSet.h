//**********************************************************************
//
// Copyright (c) 2007
// PathEngine
// Lyon, France
//
// All Rights Reserved
//
//**********************************************************************

// Provides obstacle set double buffering for situations where semi-dynamic obstacles will be added and removed,
// but not moved or otherwise modified (e.g with changes in cost to traverse)
//
// In this case a single iAgent can be created for each obstacle and shared by foreground and background obstacle sets.

#ifndef DOUBLE_BUFFERED_OBSTACLE_SET_INCLUDED
#define DOUBLE_BUFFERED_OBSTACLE_SET_INCLUDED

#include <Windows.h>
#include <vector>

class iObstacleSet;
class iCollisionContext;
class iMesh;
class iShape;
class iAgent;

class cDoubleBufferedObstacleSet
{
    iMesh* _mesh;
    iShape* _agentShape;
    iObstacleSet* _foreground;
    iObstacleSet* _background;
    iObstacleSet* _beingAddedOverlay;
    std::vector<iAgent*> _beingRemoved;
    iObstacleSet* _toAddNextUpdateOverlay;
    iObstacleSet* _toRemoveNextUpdate; // not used in queries - essentially just used as a bag of agents to be removed
    iCollisionContext* _foregroundContext;
    iObstacleSet* _allAgents; // not used in queries - essentially just used as a bag of all agents added
    bool _foregroundLocked;
    mutable CRITICAL_SECTION _bufferSwapMutex;

    bool _updateInProgress;
    mutable CRITICAL_SECTION _updateInProgressMutex;
    double _lastUpdateTime;
    double _lastBufferSwapTime;

public:

    void doUpdate(); // to be run in worker thread, should not be called by external code

    // any named obstacle on the mesh will be added to the set on construction
    cDoubleBufferedObstacleSet(iMesh* mesh, iShape* agentShape, const char *const* preprocessAttributes);
    ~cDoubleBufferedObstacleSet();

    // replaces existing named obstacles
    void storeToNamedObstacles() const; // automatically locks the foreground internally

    // must be a newly created agent
    // added agents will subsequently be managed internally by this object
    // and should not be modified or destroyed by external code
    void addObstacle(iAgent* toAdd);  // automatically locks the foreground internally

    // if this agent is a dynamic agent that has not yet been included in an update then it will be removed straight away
    // otherwise removal will take effect on completion of next update
    void removeAndDeleteObstacle(iAgent* toRemove);  // automatically locks the foreground internally

    void startUpdate();

    bool updateInProgress() const;
    double getLastUpdateTime() const;
    double getLastBufferSwapTime() const;

    void lockForeground();
    void unlockForeground();

    // following to only be called whilst foreground is locked
    const iCollisionContext* getForegroundCollisionContext() const;
    const iObstacleSet* getForegroundPreprocessedSet() const;
    tSigned32 numberOfObstaclesAdded() const;
    tSigned32 numberOfPreprocessedObstacles() const;
    tSigned32 numberOfDynamicObstacles() const;
    void renderPreprocessed() const;
    void renderPreprocessedExpansion() const;
    void renderDynamic() const;
    void renderToBeDeleted() const;
};

#endif
