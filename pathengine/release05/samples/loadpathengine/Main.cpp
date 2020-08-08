
#include "loadpathengine.h"
#include "i_pathengine.h"
#include "test_linkage.h"
#include "MessageBox.h"
#include <vector>
#include <windows.h>

class cErrorHandler : public iErrorHandler
{
    cErrorHandler(){}
public:
    eAction handle(const char* error_type, const char* error_string, const char *const* attributes)
    {
        ShowMessageBox(error_type,error_string);
        return CONTINUE;
    }
    static cErrorHandler& refInstance()
    {
        static cErrorHandler the_instance;
        return the_instance;
    }
};

// hard coded simple 2 tri square ground mesh
// (in a real application, an iFaceVertexMesh derived class will usually wrap application specific geometry instead)
class cSimpleSquareGround : public iFaceVertexMesh
{
public:

// iFaceVertexMesh interface

    long faces() const
    {
        return 2;
    }
    long vertices() const
    {
        return 4;
    }
    long vertexIndex(long face, long vertexInFace) const
    {
        if(face == 0)
        {
            switch(vertexInFace)
            {
            default:
            case 0:
                return 0;
            case 1:
                return 1;
            case 2:
                return 2;
            }
        }
        switch(vertexInFace)
        {
        default:
        case 0:
            return 2;
        case 1:
            return 1;
        case 2:
            return 3;
        }
    }
    long vertexX(long vertex) const
    {
        if(vertex > 2)
        {
            return 100;
        }
        return -100;
    }
    long vertexY(long vertex) const
    {
        if((vertex & 1) != 0)
        {
            return 100;
        }
        return -100;
    }
    float vertexZ(long vertex) const
    {
        return 0.f;
    }
    long faceAttribute(long face, long attributeIndex) const
    {
        return -1;
    }
};

static bool
RunSimpleTest(iPathEngine* pathEngine)
{
    cSimpleSquareGround sourceGround;
    std::vector<const iFaceVertexMesh*> groundParts;
    groundParts.push_back(&sourceGround);
    iMesh* mesh = pathEngine->buildMeshFromContent(&groundParts.front(), static_cast<tSigned32>(groundParts.size()), 0);
    if(!mesh)
    {
        return false;
    }
    delete mesh;
    return true;
}

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
    iPathEngine* pathEngine = LoadPathEngine("pathEngine", &cErrorHandler::refInstance());
    if(!pathEngine)
        return 0;

// check if interface for dll loaded is compatible with the headers used for compilation
    if(pathEngine->getInterfaceMajorVersion()!=PATHENGINE_INTERFACE_MAJOR_VERSION
        ||
        pathEngine->getInterfaceMinorVersion()<PATHENGINE_INTERFACE_MINOR_VERSION)
    {
        cErrorHandler::refInstance().handle("Fatal","LoadPathEngine: pathEngine version is incompatible with headers used for compilation.",0);
        return 0;
    }

// Do stuff with the SDK here..
    if(RunSimpleTest(pathEngine))
    {
        ShowMessageBox("'loadpathengine' example project", "Simple test completed successfully.");
    }
    else
    {
        ShowMessageBox("'loadpathengine' example project", "Simple test failed!");
    }

    return 0;
}
