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
#include "DoubleBufferedObstacleset.h"
#include "Error.h"
#include "Globals.h"
#include "i_pathengine.h"
#include "i_testbed.h"
#include <algorithm>
#include <sstream>
#include <string.h>
#include <time.h>

// thread proc for the worker threads
DWORD WINAPI
DoUpdate_ThreadProc(LPVOID globalsVP)
{
    cDoubleBufferedObstacleSet* set = static_cast<cDoubleBufferedObstacleSet*>(globalsVP);
    set->doUpdate();
    return 0;
}

void
cDoubleBufferedObstacleSet::doUpdate()
{
    assertD(updateInProgress());

    {
        clock_t start, finish;
        start = clock();
        _background->updateCollisionPreprocessFor(_agentShape);
        _background->updatePathfindingPreprocessFor(_agentShape);
        finish = clock();
        _lastUpdateTime = static_cast<double>(finish - start) / CLOCKS_PER_SEC;
    }

    clock_t start, finish;
    start = clock();

    EnterCriticalSection(&_bufferSwapMutex);

    tSigned32 i;
    for(i = 0; i != SizeL(_beingRemoved); ++i)
    {
        iAgent* agent = _beingRemoved[i];
        _foreground->removeAgent(agent);
        assertD(!_background->includes(agent));
        delete agent; // automatically removed from _allAgents
    }
    _beingRemoved.resize(0);

    _foregroundContext->removeObstacleSet(_foreground);
    _foregroundContext->removeObstacleSet(_beingAddedOverlay);

    while(!_beingAddedOverlay->empty())
    {
        iAgent* agent = _beingAddedOverlay->getAgent(0);
        _foreground->addAgent(agent);
        _beingAddedOverlay->removeAgent(agent);
    }

    std::swap(_foreground, _background);

    _foregroundContext->addObstacleSet(_foreground);
    _foregroundContext->updateCollisionPreprocessFor(_agentShape);

    LeaveCriticalSection(&_bufferSwapMutex);
    finish = clock();
    _lastBufferSwapTime = static_cast<double>(finish - start) / CLOCKS_PER_SEC;

    EnterCriticalSection(&_updateInProgressMutex);
    _updateInProgress = false;
    LeaveCriticalSection(&_updateInProgressMutex);
}

cDoubleBufferedObstacleSet::cDoubleBufferedObstacleSet(iMesh* mesh, iShape* agentShape, const char *const* preprocessAttributes)
{
    _lastUpdateTime = 0.0;
    _lastBufferSwapTime = 0.0;
    _mesh = mesh;
    _agentShape = agentShape;
    std::vector<const char*> combinedAttributes;
    if(preprocessAttributes)
    {
        while(preprocessAttributes[0])
        {
            assertD(preprocessAttributes[1]);
            const char* attribute(preprocessAttributes[0]);
            const char* value(preprocessAttributes[1]);
            preprocessAttributes += 2;
            if(strcmp("markForPreprocessing", attribute) == 0)
            {
                Error("NonFatal", "'markForPreprocessing' attribute should not be supplied for cDoubleBufferedObstacleSet construction, over-ridden.");
                continue;
            }
            if(strcmp("requireExplicitAgentRemoval", attribute) == 0)
            {
                Error("NonFatal", "'warnOnImplicitStateChanges' attribute should not be supplied for cDoubleBufferedObstacleSet construction, over-ridden.");
                continue;
            }
            combinedAttributes.push_back(attribute);
            combinedAttributes.push_back(value);
        }
    }
    combinedAttributes.push_back(0);
    _allAgents = _mesh->newObstacleSet_WithAttributes(&combinedAttributes[0]);
    _toAddNextUpdateOverlay = _mesh->newObstacleSet_WithAttributes(&combinedAttributes[0]);
    _toRemoveNextUpdate = _mesh->newObstacleSet_WithAttributes(&combinedAttributes[0]);
    combinedAttributes.pop_back();
    combinedAttributes.push_back("warnOnImplicitStateChanges");
    combinedAttributes.push_back("true");
    combinedAttributes.push_back(0);
    _beingAddedOverlay = _mesh->newObstacleSet_WithAttributes(&combinedAttributes[0]);
    combinedAttributes.pop_back();
    combinedAttributes.push_back("markForPreprocessing");
    combinedAttributes.push_back("true");
    combinedAttributes.push_back(0);
    _foreground = _mesh->newObstacleSet_WithAttributes(&combinedAttributes[0]);
    _background = _mesh->newObstacleSet_WithAttributes(&combinedAttributes[0]);
    _foregroundContext = _mesh->newContext();
    _foregroundContext->addObstacleSet(_foreground);
    _foregroundContext->addObstacleSet(_toAddNextUpdateOverlay); // this obstacle set is permanently included in the context, for simplicity
    _updateInProgress = false;
    tSigned32 i;
    for(i = 0; i != _mesh->getNumberOfNamedObstacles(); ++i)
    {
        iAgent* agent;
        const char* id_ignored;
        _mesh->retrieveNamedObstacleByIndex(i, agent, id_ignored);
        _allAgents->addAgent(agent);
        _foreground->addAgent(agent);
        _background->addAgent(agent);
    }
    _foreground->updatePathfindingPreprocessFor(_agentShape);
    _foregroundContext->updateCollisionPreprocessFor(_agentShape);
    _foregroundLocked = false;
    InitializeCriticalSection(&_updateInProgressMutex);
    InitializeCriticalSection(&_bufferSwapMutex);
}
cDoubleBufferedObstacleSet::~cDoubleBufferedObstacleSet()
{
    assertD(!updateInProgress());
    assertD(!_foregroundLocked);
    DeleteCriticalSection(&_updateInProgressMutex);
    DeleteCriticalSection(&_bufferSwapMutex);
    delete _foreground;
    delete _background;
    delete _beingAddedOverlay;
    delete _toAddNextUpdateOverlay;
    delete _toRemoveNextUpdate;
    delete _foregroundContext;
    while(_allAgents->getNumberOfAgents())
    {
        delete _allAgents->getAgent(0);
    }
}

void
cDoubleBufferedObstacleSet::storeToNamedObstacles() const
{
    EnterCriticalSection(&_bufferSwapMutex);
    _mesh->clearAllNamedObstacles();
    tSigned32 i;
    for(i = 0; i != _allAgents->size(); ++i)
    {
        std::ostringstream oss; 
        oss << i;
        _mesh->storeNamedObstacle(oss.str().c_str(), _allAgents->getAgent(i));
    }
    LeaveCriticalSection(&_bufferSwapMutex);
}

void
cDoubleBufferedObstacleSet::addObstacle(iAgent* toAdd)
{
    EnterCriticalSection(&_bufferSwapMutex);
    assertD(!_allAgents->includes(toAdd));
    _allAgents->addAgent(toAdd);
    _toAddNextUpdateOverlay->addAgent(toAdd);
    LeaveCriticalSection(&_bufferSwapMutex);
}
void
cDoubleBufferedObstacleSet::removeAndDeleteObstacle(iAgent* toRemove)
{
    EnterCriticalSection(&_bufferSwapMutex);
    assertD(_allAgents->includes(toRemove));
    if(_toAddNextUpdateOverlay->includes(toRemove))
    {
        delete toRemove; // will be removed from the relevant sets automatically
    }
    else if(!_toRemoveNextUpdate->includes(toRemove)) // check not already flagged for removal
    {
      _toRemoveNextUpdate->addAgent(toRemove);
    }
    LeaveCriticalSection(&_bufferSwapMutex);
}

void
cDoubleBufferedObstacleSet::startUpdate()
{
    assertD(!updateInProgress());
    _updateInProgress = true;
    tSigned32 i;
    assertD(_beingAddedOverlay->empty());
    for(i = 0; i != _toAddNextUpdateOverlay->size(); ++i)
    {
        iAgent* agent = _toAddNextUpdateOverlay->getAgent(i);
        _beingAddedOverlay->addAgent(agent);
        _background->addAgent(agent);
    }
    _toAddNextUpdateOverlay->clear();
    for(i = 0; i != _toRemoveNextUpdate->size(); ++i)
    {
        iAgent* agent = _toRemoveNextUpdate->getAgent(i);
        _background->removeAgent(agent);
        _beingRemoved.push_back(agent);
    }
    _toRemoveNextUpdate->clear();
    _foregroundContext->addObstacleSet(_beingAddedOverlay);
    HANDLE threadHandle = CreateThread(0, 0, DoUpdate_ThreadProc, static_cast<LPVOID>(this), 0, 0);
}

bool
cDoubleBufferedObstacleSet::updateInProgress() const
{
    EnterCriticalSection(&_updateInProgressMutex);
    bool result = _updateInProgress;
    LeaveCriticalSection(&_updateInProgressMutex);
    return result;
}

double
cDoubleBufferedObstacleSet::getLastUpdateTime() const
{
    assertD(!updateInProgress()); // could also return this during update, with mutex protection
    return _lastUpdateTime;
}
double
cDoubleBufferedObstacleSet::getLastBufferSwapTime() const
{
    assertD(!updateInProgress()); // could also return this during update, with mutex protection
    return _lastBufferSwapTime;
}

void
cDoubleBufferedObstacleSet::lockForeground()
{
    assertD(!_foregroundLocked);
    _foregroundLocked = true;
    EnterCriticalSection(&_bufferSwapMutex);
}
void
cDoubleBufferedObstacleSet::unlockForeground()
{
    assertD(_foregroundLocked);
    _foregroundLocked = false;
    LeaveCriticalSection(&_bufferSwapMutex);
}

const iCollisionContext*
cDoubleBufferedObstacleSet::getForegroundCollisionContext() const
{
    assertD(_foregroundLocked);
    return _foregroundContext;
}

const iObstacleSet*
cDoubleBufferedObstacleSet::getForegroundPreprocessedSet() const
{
    assertD(_foregroundLocked);
    return _foreground;
}

tSigned32
cDoubleBufferedObstacleSet::numberOfObstaclesAdded() const
{
    assertD(_foregroundLocked);
    return _allAgents->size();
}

tSigned32
cDoubleBufferedObstacleSet::numberOfPreprocessedObstacles() const
{
    assertD(_foregroundLocked);
    return _foreground->size();
}
tSigned32
cDoubleBufferedObstacleSet::numberOfDynamicObstacles() const
{
    assertD(_foregroundLocked);
    return _beingAddedOverlay->size() + _toAddNextUpdateOverlay->size();
}

void
cDoubleBufferedObstacleSet::renderPreprocessed() const
{
    assertD(_foregroundLocked);
    gTestBed->drawAgentsInObstacleSet(_foreground, 40);
}
void
cDoubleBufferedObstacleSet::renderPreprocessedExpansion() const
{
    assertD(_foregroundLocked);
    tSigned32 i;
    for(i = 0; i != _foreground->getNumberOfAgents(); ++i)
    {
        gTestBed->drawAgentExpansion(_foreground->getAgent(i), _agentShape);
    }
}
void
cDoubleBufferedObstacleSet::renderDynamic() const
{
    assertD(_foregroundLocked);
    gTestBed->drawAgentsInObstacleSet(_beingAddedOverlay, 40);
    gTestBed->drawAgentsInObstacleSet(_toAddNextUpdateOverlay, 40);
}
void
cDoubleBufferedObstacleSet::renderToBeDeleted() const
{
    assertD(_foregroundLocked);
    gTestBed->drawAgentsInObstacleSet(_toRemoveNextUpdate, 40);
    tSigned32 i;
    for(i = 0; i != SizeL(_beingRemoved); ++i)
    {
        gTestBed->drawAgent(_beingRemoved[i], 40);
    }
}
