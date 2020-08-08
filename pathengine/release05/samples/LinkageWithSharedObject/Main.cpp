
#include "i_pathengine.h"
#include "PathEngine_DirectLinkage.h"
#include <dlfcn.h>
#include <stdio.h>
#include <signal.h>
#include <vector>

class cErrorHandler : public iErrorHandler
{
public:
    eAction handle(const char* type, const char* description, const char *const* attributes)
    {
        printf("Error handler called:\n");
        printf(type);
        printf("\n");
        printf(description);
        printf("\n");
        raise(SIGTRAP);
        return CONTINUE;
    }
};

cErrorHandler gErrorHandler;

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

int
main (int argc, char* argv[])
{
    const char* errorString;

    void* handle = dlopen("./PathEngine.so", RTLD_NOW);
    if(!handle)
    {
        printf("Problem loading shared object.\n");
        errorString = dlerror();
        if(errorString)
        {
            printf(errorString);
            printf("\n");
        }
        printf("Please copy the PathEngine shared object file to load from the build directory into the working directory before running.\n");
        return 1;
    }

    iPathEngine* (*PathEngine_InitialiseAndObtainRootInterface_Pointer)(iErrorHandler*);
    //void (*PathEngine_ShutDown_Pointer)(); // explicit shutdown is possible, but not required

    dlerror();
    *(void **)(&PathEngine_InitialiseAndObtainRootInterface_Pointer) = dlsym(handle, "PathEngine_InitialiseAndObtainRootInterface");
    errorString = dlerror();
    if(errorString)
    {
        printf(errorString);
        printf("\n");
        dlclose(handle);
        return 1;
    }
    //*(void **)(&PathEngine_ShutDown_Pointer) = dlsym(handle, "PathEngine_ShutDown");
    //errorString = dlerror();
    //if(errorString)
    //{
    //    printf(errorString);
    //    printf("\n");
    //    dlclose(handle);
    //    return 1;
    //}

    iPathEngine* pathEngine = (*PathEngine_InitialiseAndObtainRootInterface_Pointer)(&gErrorHandler);

    if(pathEngine->getInterfaceMajorVersion() != PATHENGINE_INTERFACE_MAJOR_VERSION
        ||
        pathEngine->getInterfaceMinorVersion() < PATHENGINE_INTERFACE_MINOR_VERSION)
    {
        gErrorHandler.handle("Fatal", "LoadPathEngine: pathengine version is incompatible with headers used for compilation.", 0);
        return false;
    }


// Do stuff with the SDK here
    if(RunSimpleTest(pathEngine))
    {
        printf("Simple test completed successfully.\n");
    }
    else
    {
        printf("Simple test failed!\n");
    }


    //(*PathEngine_ShutDown_Pointer);

    int error = dlclose(handle);
    if(error)
    {
        printf("error from dclose()\n");
        return 1;
    }
    return 0;
}
