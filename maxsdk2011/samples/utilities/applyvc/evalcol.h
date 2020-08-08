/**********************************************************************
 *<
	FILE:			EvalColor.cpp
	DESCRIPTION:	Vertex Color Renderer
	CREATED BY:		Christer Janson
	HISTORY:		Created Monday, December 12, 1996

 *>	Copyright (c) 1997 Kinetix, All Rights Reserved.
 **********************************************************************/
//
// Description:
// These functions calculates the diffuse or ambient color at each vertex
// or face of an INode.
//
// Exports:
// BOOL calcMixedVertexColors(INode*, TimeValue, int, ColorTab&, EvalColProgressCallback* callb = NULL);
//      This function calculates the interpolated diffuse or ambient 
//      color at each vertex of an INode.
//      Usage: Pass in a node pointer and the TimeValue to generate
//      a list of Colors corresponding to each vertex in the mesh
//      Use the int flag to specify if you want to have diffuse or 
//      ambient colors, or if you want to use the lights in the scene.
//      Note: 
//        You are responsible for deleting the Color objects in the table.
//      Additional note:
//        Since materials are assigned by face, this function renders each
//        face connected to the specific vertex (at the point of the vertex)
//        and then mixes the colors.
//
//***************************************************************************

#ifndef __EVALCOL__H
#define __EVALCOL__H

#ifndef USE_ORIGINAL_VERTEX_PAINT
#include "istdplug.h"
#endif	// !use_original_vertex_paint

//#define LIGHT_AMBIENT		0x00
//#define LIGHT_DIFFUSE		0x01
//#define LIGHT_SCENELIGHT	0x02

#ifndef USE_ORIGINAL_VERTEX_PAINT
// SR NOTE64: was previously
//    using IAssignVertexColors_R7::LightingModel;
// which is not kosher with VS2005 (outside a derived class def).

typedef IAssignVertexColors_R7::Options2   EvalVCOptions;

#else	// use_original_vertex_paint
// The possible lighting models to use for the vertex colors
typedef enum {
    kLightingOnly = 0,      // Store lighting only
    kShadedLighting = 1,    // Store shaded color with lighting
    kShadedOnly = 2         // Store shaded color without lighting
} LightingModel;

// The options used when calculating the vertex colors
struct EvalVCOptions {
    bool mixVertColors; // calcMixedVertexColors() OR calcFaceColors() should be called depending on this option
    bool castShadows;
    bool useMaps;
    bool useRadiosity;
    bool radiosityOnly;
    LightingModel lightingModel;
};

// Stores a color for each vertex of a triangle face.
class FaceColor {
public:
	Color colors[3];
};
#endif	// use_original_vertex_paint

#ifndef USE_ORIGINAL_VERTEX_PAINT
typedef IVertexPaint::FaceColor		FaceColor;
typedef IVertexPaint::FaceColorTab	FaceColorTab;
typedef IVertexPaint::VertColorTab	ColorTab;
#else	// use_original_vertex_paint
typedef Tab<Color*> ColorTab;
typedef Tab<FaceColor*> FaceColorTab;
#endif	// use_original_vertex_paint

// Progress callback for the evaluation functions below
class EvalColProgressCallback {
public:
	virtual BOOL progress(float prog) = 0;
};

// Returns the active radiosity engine, if compatible with this utility. NULL otherwise.
RadiosityEffect* GetCompatibleRadiosity();

BOOL calcMixedVertexColors(INode* node, TimeValue t, const EvalVCOptions& evalOptions, ColorTab& vxColTab, EvalColProgressCallback* callb = NULL);
BOOL calcFaceColors(INode* node, TimeValue t, const EvalVCOptions& evalOptions, FaceColorTab& faceColTab, EvalColProgressCallback* callb = NULL);


#endif //__EVALCOL__H
