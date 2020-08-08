
#include "i_pathengine.h"

class cPartitionedTerrain : public iFaceVertexMesh
{
    long _originX, _originY;
    long _squareSize, _squaresAcross;

public:

    cPartitionedTerrain(
            long originX, long originY,
            long squareSize, long squares
            ) :
     _originX(originX),
     _originY(originY),
     _squareSize(squareSize),
     _squaresAcross(squares)
    {
    }

// iFaceVertexMesh interface

    long faces() const
    {
        return _squaresAcross * _squaresAcross * 2;
    }
    long vertices() const
    {
        return (_squaresAcross + 1) * (_squaresAcross + 1);
    }
    long vertexIndex(long face, long vertexInFace) const
    {
        long baseVertex = face / 2 + face / 2 / _squaresAcross;
        if(face & 1)
        {
            switch(vertexInFace)
            {
            default:
//                invalid
            case 0:
                return baseVertex;
            case 1:
                return baseVertex + 1;
            case 2:
                return baseVertex + (_squaresAcross + 1) + 1;
            }
        }
        switch(vertexInFace)
        {
        default:
//            invalid
        case 0:
            return baseVertex;
        case 1:
            return baseVertex + (_squaresAcross + 1) + 1;
        case 2:
            return baseVertex + (_squaresAcross + 1);
        }
    }
    long vertexX(long vertex) const
    {
        return _originX + vertex / (_squaresAcross + 1) * _squareSize;
    }
    long vertexY(long vertex) const
    {
        return _originY + (vertex % (_squaresAcross + 1)) * _squareSize;
    }
    float vertexZ(long vertex) const
    {
        return 0.0f;
    }
    long faceAttribute(long face, long attributeIndex) const
    {
        if(attributeIndex == PE_FaceAttribute_SectionID)
        {
            return 0; // mark all faces as sectionID == 0 (first terrain layer)
        }
        return -1;
    }
};

