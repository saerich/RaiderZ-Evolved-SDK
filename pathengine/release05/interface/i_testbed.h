//**********************************************************************
//
// Copyright (c) 2006
// Thomas Young, PathEngine
// Lyon, France
//
// All Rights Reserved
//
//**********************************************************************

#ifndef TESTBED_INTERFACE_HAS_BEEN_INCLUDED
#define TESTBED_INTERFACE_HAS_BEEN_INCLUDED

#if defined(__GNUC__) && defined(__x86_64__)
typedef int tSigned32;
typedef unsigned int tUnsigned32;
#else
typedef long tSigned32;
typedef unsigned long tUnsigned32;
#endif

const tSigned32 TESTBED_INTERFACE_MAJOR_VERSION = 3;
const tSigned32 TESTBED_INTERFACE_MINOR_VERSION = 4;

#ifndef PATHENGINE_POD_CLASSES_HAVE_BEEN_DEFINED
#define PATHENGINE_POD_CLASSES_HAVE_BEEN_DEFINED
class cPosition
{
public:
    cPosition() : cell(-1) {}
    tSigned32 x, y, cell;
    bool operator==(const cPosition& rhs) const
    {
        if(cell == -1)
            return cell == rhs.cell;
        return cell == rhs.cell && x == rhs.x && y == rhs.y;
    }
    bool operator!=(const cPosition& rhs) const
    {
        return !((*this) == rhs);
    }
};
class cHorizontalRange
{
public:
    tSigned32 minX, minY, maxX, maxY;
    bool operator==(const cHorizontalRange& rhs) const
    {
        return minX == rhs.minX && minY == rhs.minY && maxX == rhs.maxX && maxY == rhs.maxY;
    }
    bool operator!=(const cHorizontalRange& rhs) const
    {
        return !((*this) == rhs);
    }
};
#endif

class iShape;
class iMesh;
class iAgent;
class iPath;
class iPathEngine;
class iCollisionContext;
class iObstacleSet;
class iFaceVertexMesh;
class iProgressCallBack;
class cRenderPreprocess;

class iTestBed
{
    void operator delete(void* void_pointer) {}
public:

    virtual const char *const* getVersionAttributes() const = 0;
    virtual tSigned32 getInterfaceMajorVersion() const = 0;
    virtual tSigned32 getInterfaceMinorVersion() const = 0;

    virtual void reportError(const char* error_type, const char* error_string) = 0;

    virtual char* loadBinary(const char* filename, tUnsigned32& buffer_size) = 0;
    virtual void freeBuffer(char* buffer) = 0;

    virtual void setMesh(const iMesh* mesh) = 0;
    virtual void releaseMesh() = 0;
    virtual iMesh* getMesh() = 0;

    virtual const char* receiveKeyMessage() = 0;
    virtual bool getKeyState(const char* keystring) = 0;
    virtual bool getLeftMouseState() = 0;
    virtual bool getRightMouseState() = 0;
    virtual void getMouseScreenPosition(tSigned32& screenx, tSigned32& screeny) = 0;
    virtual void getMouseScreenDelta(tSigned32& screendx, tSigned32& screendy) = 0;

    virtual cPosition positionAtMouse() = 0;

    virtual void setColour(const char* colour) = 0;
    virtual void setColourRGB(float r, float g, float b) = 0;
    virtual void setAdditiveBlending(float alpha) = 0;
    virtual void setOpaque() = 0;

    virtual void zoomExtents() = 0;
    virtual void lookAt(const cPosition& target, float angle, float elevation, float distance) = 0;
    virtual void lookAtWithPrecision(const cPosition& target, float precisionX, float precisionY, float angle, float elevation, float distance) = 0;
    virtual bool meshObstructsCamera() = 0;

    virtual void drawMesh() = 0;
    virtual void drawMeshEdges() = 0;
    virtual void drawBurntInObstacles() = 0;
    virtual void drawMeshExpansion(iShape* shape) = 0;
    virtual void drawSmallConvexExpansions(iShape* shape) = 0;
    virtual void drawAgent(const iAgent* agent, tSigned32 height) = 0;
    virtual void drawAgentWithPrecision(const iAgent* agent, tSigned32 height, float precisionX, float precisionY) = 0;
    virtual void drawAgentHeading(const iAgent* agent, tSigned32 size, tSigned32 height, float heading) = 0;
    virtual void drawAgentHeadingWithPrecision(const iAgent* agent, tSigned32 size, tSigned32 height, float heading, float precisionX, float precisionY) = 0;
    virtual void drawAgentExpansion(const iAgent* agent, iShape* expandedBy) = 0;
    virtual void drawAgentsInContext(const iCollisionContext* context, tSigned32 height) = 0;
    virtual void drawShape(iShape* shape, const cPosition& position, tSigned32 height) = 0;
    virtual void drawPath(const iPath* path) = 0;
    virtual void drawPosition(const cPosition& position, tSigned32 size) = 0;
    virtual void drawLine(const cPosition& start, const cPosition& end) = 0;

    virtual void drawSquareBounds(const cPosition& start, tSigned32 distance) = 0;

    virtual void printText8by16(tSigned32 x, tSigned32 y, const char* text) = 0;
    virtual void printTextLine(tSigned32 x, const char* text) = 0;

    virtual void update() = 0;

    virtual tSigned32 createSound(const char* fileName, tSigned32 numberOfBuffers) = 0;
    virtual void playSound(tSigned32 index) = 0;

    virtual tSigned32 loadSplashImage(const char* fileName) = 0;
    virtual void displaySplashImage(tSigned32 x, tSigned32 y, tSigned32 imageIndex) = 0;
    virtual void displaySplashImage_Centred(tSigned32 imageIndex) = 0;

    virtual void drawRangeBounds(const cHorizontalRange& bounds) = 0;

    virtual cRenderPreprocess* preprocessFaceVertexMesh(const iFaceVertexMesh& mesh) = 0;
    virtual cRenderPreprocess* preprocessFaceVertexMeshEdges(const iFaceVertexMesh& mesh) = 0;
    virtual void drawRenderPreprocess(cRenderPreprocess* renderPreprocess) = 0;
    virtual void deleteRenderPreprocess(cRenderPreprocess* renderPreprocess) = 0;
    virtual iProgressCallBack* getTestBedProgressCallBack() = 0;
    virtual void zoomExtents_FaceVertex(
            const iFaceVertexMesh *const * meshObjectPointers,
            tSigned32 numberOfMeshObjects
            ) = 0;

    virtual void drawAgentsInObstacleSet(const iObstacleSet* set, tSigned32 height) = 0;

    virtual cRenderPreprocess* preprocessShapeWithHeight(iShape* shape, tSigned32 height) = 0;
    virtual void drawPreprocessedGeometryAtPosition(cRenderPreprocess* renderPreprocess, const cPosition& position) = 0;
};

#endif // Include guard
