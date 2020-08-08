#ifndef PXD_CONTEXT_H
#define PXD_CONTEXT_H

#include "px_config.h"
#include "px_task.h"

#ifdef __cplusplus

extern "C"
{
#endif

/*!
\file
Context handling
*/

/************************************************************************/
/* Context handling, types                                              */
/************************************************************************/

typedef PxdHandle PxdContext;
typedef PxdHandle PxdAtom;

/*!
Context type. Used when creating a new context.
*/
typedef enum PxdContextType_
{
	/*!
	This context should run on CPU only
	*/
	PXD_CPU,
	
#ifdef PXD_SUPPORT_PPU	
	/*!
	This context should utilize the PPU where possible
	*/
	PXD_PPU
#endif
}PxdContextType;

typedef enum PxdBroadPhaseImplementation_
{
	PXD_BP_SINGLE_SAP,
	PXD_BP_MULTI_SAP,
}PxdBroadPhaseImplementation;

/*!
Get the context type
\param context
The context to get the type of.
\return
Context type. 
*/
PXD_EXPORT_FUNCTION PxdContextType PxdContextGetType(PxdContext context);

/*!
Create a new context

\param type
Context type. This cannot be changed afterwards.
\param agPPUConnection
Pointer to an AgPPUConnection. Backdoor for sharing one PPU connection between
different types of scenes (fluid, cloth and RB)
\param bp
Selects broad-phase implementation
\return
Context identifier. Zero on failure.
*/
PXD_EXPORT_FUNCTION PxdContext PxdContextCreate(PxdContextType type, void* agPPUConnection, PxdBroadPhaseImplementation bp, bool useImprovedSpringSolver);

/*!
Destroy context

\param context
Context identifier
*/
PXD_EXPORT_FUNCTION void PxdContextDestroy(PxdContext context);

/*!
Associate a sub task callback with a context.

\param contexts
Context identifier
\param userSubTaskCallback
User callback function for subtask queuing and execution
\param userParam
User parameter that is passed along on every call to the userSubTaskCallback
function
*/
PXD_EXPORT_FUNCTION void PxdContextSetSubTaskCallback(PxdContext context, PxdUserSubTaskCallback userSubTaskCallback, void* userParam);

#ifdef PXD_SUPPORT_DEBUG_RENDERER

	enum PxDebugColor
	{
		PX_ARGB_BLACK	= 0xff000000,
		PX_ARGB_RED		= 0xffff0000,
		PX_ARGB_GREEN	= 0xff00ff00,
		PX_ARGB_BLUE	= 0xff0000ff,
		PX_ARGB_YELLOW	= 0xffffff00,
		PX_ARGB_MAGENTA	= 0xffff00ff,
		PX_ARGB_CYAN	= 0xff00ffff,
		PX_ARGB_WHITE	= 0xffffffff,
	};

	struct PxDebugPoint
	{
		PxdVector	p;
		PxU32		color;
	};

	struct PxDebugLine
	{
		PxdVector	p0;
		PxdVector	p1;
		PxU32		color;
	};

	struct PxDebugTriangle
	{
		PxdVector	p0;
		PxdVector	p1;
		PxdVector	p2;
		PxU32		color;
	};

PXD_EXPORT_FUNCTION bool PxdContextClearDebugData(PxdContext context);

PXD_EXPORT_FUNCTION bool PxdContextGetDebugData(PxdContext context,
	PxU32& nbPoints, const PxDebugPoint*& points,
	PxU32& nbLines, const PxDebugLine*& lines,
	PxU32& nbTriangles, const PxDebugTriangle*& triangles
	);
#endif


#ifdef __cplusplus
}
#endif


#endif
