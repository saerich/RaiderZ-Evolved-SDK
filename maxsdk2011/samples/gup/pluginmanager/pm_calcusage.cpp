// Copyright 2008 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.  
//
//

#include "plugman.h"


#define SCENE_REF_MTL_EDIT 		0
#define SCENE_REF_MTL_LIB  		1
#define SCENE_REF_SOUNDOBJ 		2
#define SCENE_REF_ROOTNODE 		3 
#define SCENE_REF_REND	   		4
#define SCENE_REF_SELSETS		5
#define SCENE_REF_TVNODE		6
#define SCENE_REF_GRIDREF		7
#define SCENE_REF_RENDEFFECTS	8
#define SCENE_REF_GLOBSHADTYPE	9
#define SCENE_REF_LAYERREF		10
#define NUM_SCENE_REFS			11



void NodeEnum(INode* node, RefEnumProc *proc)
{
	node->EnumRefHierarchy(*proc);
	for (int c = 0; c < node->NumberOfChildren(); c++) {
		NodeEnum(node->GetChildNode(c),proc);
	}
}


void EnumEverything(RefEnumProc &rproc) {
	Interface *ip = GetCOREInterface();
	ReferenceTarget *theScene = ip->GetScenePointer();

    INode *Rootnode = ip->GetRootNode();
	NodeEnum(Rootnode,&rproc);

	RefTargetHandle theRef = theScene->GetReference(SCENE_REF_MTL_LIB);
	if (theRef)
		theRef->EnumRefHierarchy(rproc);
	theRef = theScene->GetReference(SCENE_REF_MTL_EDIT);
	if (theRef)
		theRef->EnumRefHierarchy(rproc);
	theRef = theScene->GetReference(SCENE_REF_REND);
	if (theRef)
		theRef->EnumRefHierarchy(rproc);
	theRef = theScene->GetReference(SCENE_REF_SOUNDOBJ);
	if (theRef)
		theRef->EnumRefHierarchy(rproc);
	theRef = theScene->GetReference(SCENE_REF_RENDEFFECTS);
	if (theRef)
		theRef->EnumRefHierarchy(rproc);
	}

class CountClassUses: public RefEnumProc {
	public:
		int proc(ReferenceMaker *m);
	};

int CountClassUses::proc(ReferenceMaker *m) { 
	ClassEntry *ce = GetCOREInterface()->GetDllDir().ClassDir().FindClassEntry(m->SuperClassID(),m->ClassID());
	if (ce) 
		ce->IncUseCount();
	return REF_ENUM_CONTINUE;
	}

static void ZeroClassUseCounts(Interface *ip) {
	ClassDirectory &cd = ip->GetDllDir().ClassDir();
	for (int i=0; i<cd.Count(); i++) {
		SubClassList& scl = cd[i];
		for (int j=0; j<scl.Count(ACC_ALL); j++) 
			scl[j].SetUseCount(0);
		}
	}


void ComputeClassUse(Interface *ip) {
	ZeroClassUseCounts(ip);

	CountClassUses ccu;
	ccu.BeginEnumeration();
	EnumEverything(ccu);
	ccu.EndEnumeration();
	}