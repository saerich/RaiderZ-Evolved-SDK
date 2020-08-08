/**********************************************************************
 *<
	FILE: editsops.cpp

	DESCRIPTION:  Edit Shape OSM operations

	CREATED BY: Tom Hudson & Rolf Berteig

	HISTORY: created 25 April, 1995

 *>	Copyright (c) 1995, All Rights Reserved.
 **********************************************************************/
#include "dllmain.h"
#include "editpat.h"

extern CoreExport Class_ID patchClassID; 

// in mods.cpp
extern HINSTANCE hInstance;

/*-------------------------------------------------------------------*/

static void XORDottedLine( HWND hwnd, IPoint2 p0, IPoint2 p1, bool bErase, bool bPresent)
	{
	IPoint2 pts[2];
    pts[0].x = p0.x;
    pts[0].y = p0.y;
    pts[1].x = p1.x;
    pts[1].y = p1.y;
    XORDottedPolyline(hwnd, 2, pts, 0, bErase, !bPresent);
	}

/*-------------------------------------------------------------------*/

EPTempData::~EPTempData()
	{
	if (patch) delete patch;
	}

EPTempData::EPTempData(EditPatchMod *m,EditPatchData *pd)
	{
	patch = NULL;
	patchValid.SetEmpty();
	patchData = pd;
	mod = m;
	}

void EPTempData::Invalidate(PartID part,BOOL patchValid)
	{
	if ( !patchValid ) {
		delete patch;
		patch = NULL;
		}
	if ( part & PART_TOPO ) {
		}
	if ( part & PART_GEOM ) {
		}
	if ( part & PART_SELECT ) {
		}
	}

PatchMesh *EPTempData::GetPatch(TimeValue t)
	{
	if ( patchValid.InInterval(t) && patch ) {
		return patch;
	} else {
		patchData->SetFlag(EPD_UPDATING_CACHE,TRUE);
		mod->NotifyDependents(Interval(t,t), 
			PART_GEOM|SELECT_CHANNEL|PART_SUBSEL_TYPE|PART_DISPLAY|PART_TOPO,
		    REFMSG_MOD_EVAL);
		patchData->SetFlag(EPD_UPDATING_CACHE,FALSE);
		return patch;
		}
	}

BOOL EPTempData::PatchCached(TimeValue t)
	{
	return (patchValid.InInterval(t) && patch);
	}

void EPTempData::UpdateCache(PatchObject *patchOb)
	{
	if ( patch ) delete patch;
	patch = new PatchMesh(patchOb->patch);

	patchValid = FOREVER;
	
	// These are the channels we care about.
	patchValid &= patchOb->ChannelValidity(0,GEOM_CHAN_NUM);
	patchValid &= patchOb->ChannelValidity(0,TOPO_CHAN_NUM);
	patchValid &= patchOb->ChannelValidity(0,SELECT_CHAN_NUM);
	patchValid &= patchOb->ChannelValidity(0,SUBSEL_TYPE_CHAN_NUM);
	patchValid &= patchOb->ChannelValidity(0,DISP_ATTRIB_CHAN_NUM);	
	patchValid &= patchOb->ChannelValidity(0,TEXMAP_CHAN_NUM);	
	patchValid &= patchOb->ChannelValidity(0,VERT_COLOR_CHAN_NUM);	
	}

