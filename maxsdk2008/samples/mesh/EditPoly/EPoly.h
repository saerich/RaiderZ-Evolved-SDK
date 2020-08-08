/**********************************************************************
 *<
	FILE: EPoly.h

	DESCRIPTION:

	CREATED BY: Steve Anderson

	HISTORY:

 *>	Copyright (c) 1999, All Rights Reserved.
 **********************************************************************/

#ifndef __EPOLY__H
#define __EPOLY__H

#include "Max.h"
#include "IParamm2.h"
#include "evrouter.h"
#include "resource.h"

//#define __DEBUG_PRINT_EDIT_POLY

TCHAR *GetString(int id);
#define BIGFLOAT 9999999.f

extern ClassDesc2 *GetEditPolyDesc();

extern HINSTANCE hInstance;
extern int enabled;

// Polymesh selection toolbar icons - used in select and edit tools.
class PolySelImageHandler {
public:
	HIMAGELIST images, hPlusMinus;

	PolySelImageHandler () : images(NULL), hPlusMinus(NULL) { }
	~PolySelImageHandler () { if (images) ImageList_Destroy (images); if (hPlusMinus) ImageList_Destroy (hPlusMinus); }
	HIMAGELIST LoadImages ();
	HIMAGELIST LoadPlusMinus ();
};

extern PolySelImageHandler *GetPolySelImageHandler();
extern bool CheckNodeSelection (Interface *ip, INode *inode);

class RefmsgKillCounter {
private:
	friend class KillRefmsg;
	LONG	counter;

public:
	RefmsgKillCounter() : counter(-1) {}

	bool DistributeRefmsg() { return counter < 0; }
};

class KillRefmsg {
private:
	LONG&	counter;

public:
	KillRefmsg(RefmsgKillCounter& c) : counter(c.counter) { ++counter; }
	~KillRefmsg() { --counter; }
};

// IDs for the command modes in Editable Poly and Edit Poly.
#define CID_CREATEVERT				CID_USER+0x1000
#define CID_CREATEEDGE  			CID_USER+0x1001
#define CID_CREATEFACE  			CID_USER+0x1002
#define CID_DIVIDEEDGE				CID_USER+0x1018
#define CID_DIVIDEFACE				CID_USER+0x1019
#define CID_EXTRUDE					CID_USER+0x1020
#define CID_EXTRUDE_VERTEX_OR_EDGE	CID_USER+0x104A	
#define CID_POLYCHAMFER				CID_USER+0x1028
#define CID_BEVEL					CID_USER+0x102C
#define CID_INSET					CID_USER+0x102D	
#define CID_OUTLINE_FACE			CID_USER+0x102E
#define CID_CUT						CID_USER+0x1038	
#define CID_QUICKSLICE				CID_USER+0x1050	
#define CID_WELD					CID_USER+0x1040
#define CID_SLIDE_EDGES				CID_USER+0x1054	
#define CID_HINGE_FROM_EDGE			CID_USER+0x1058	
#define CID_PICK_HINGE				CID_USER+0x105A
#define CID_EDITTRI					CID_USER+0x1048
#define CID_TURNEDGE				CID_USER+0x104E
#define CID_BRIDGE_BORDER			CID_USER + 0x1060
#define CID_BRIDGE_POLYGON			CID_USER + 0x1061
#define CID_PICK_BRIDGE_1			CID_USER+0x1062
#define CID_PICK_BRIDGE_2			CID_USER+0x1063
#define CID_BRIDGE_EDGE				CID_USER+0x1064

// For toggling shaded faces with soft selection
// Methods defined in PolyEdOps; used in both Edit Poly and Editable Poly.
class ToggleShadedRestore : public RestoreObj {
	INode *mpNode;
	bool mOldShowVertCol, mOldShadedVertCol, mNewShowVertCol;
	int mOldVertexColorType;

public:
	ToggleShadedRestore (INode *pNode, bool newShow);
	void Restore(int isUndo);
	void Redo();
	int Size() { return sizeof (void *) + 3*sizeof(bool) + sizeof(int); }
	TSTR Description() { return TSTR(_T("ToggleShadedRestore")); }
};

typedef struct GenSoftSelData_tag {
	BOOL useSoftSel; //whether soft selection is active
	BOOL useEdgeDist;
	int edgeIts;
	BOOL ignoreBack;
	float falloff, pinch, bubble;
	GenSoftSelData_tag() //a struct with a constructor :)
	{	useSoftSel=FALSE, useEdgeDist=FALSE, edgeIts=0,
		ignoreBack=FALSE, falloff=20.0f, pinch=bubble=0.0f; }
} GenSoftSelData;

TCHAR *LookupOperationName (int opID);
TCHAR *LookupEditPolySelLevel (int opID);
TCHAR *LookupMNMeshSelLevel (int opID);
TCHAR *LookupParameterName (int paramID);
int LookupParameterType (int paramID);

#endif
