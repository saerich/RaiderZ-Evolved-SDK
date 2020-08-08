/*==============================================================================
// Copyright (c) 1998-2008 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//**************************************************************************/
// DESCRIPTION: Flipped Faces aka Faces Orientation Checker
// AUTHOR: Michael Zyracki created 2008
//***************************************************************************/
#include<3dsmaxport.h>
#include "checkerimplementations.h"

FlippedFacesChecker FlippedFacesChecker::_theInstance(FLIPPED_FACES_INTERFACE, _T("FacesOrientation"), 0, NULL, FP_CORE,
	GUPChecker::kGeometryCheck, _T("Check"), 0,TYPE_ENUM, GUPChecker::kOutputEnum,0,3,
		_T("time"), 0, TYPE_TIMEVALUE,
		_T("nodeToCheck"), 0, TYPE_INODE,
		_T("results"), 0, TYPE_INDEX_TAB_BR,
	GUPChecker::kHasPropertyDlg, _T("hasPropertyDlg"),0,TYPE_bool,0,0,
	GUPChecker::kShowPropertyDlg, _T("showPropertyDlg"),0,TYPE_VOID,0,0,
 	enums,
		GUPChecker::kOutputEnum, 4,	
		_T("Failed"), GUPChecker::kFailed,
		_T("Vertices"), GUPChecker::kVertices,
		_T("Edges"), GUPChecker::kEdges,
		_T("Faces"), GUPChecker::kFaces,
	end	
																			
  );

class FlippedFacesClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return TRUE; }
	void *			Create(BOOL loading = FALSE) {return &FlippedFacesChecker::_theInstance; }
	const TCHAR *	ClassName() { return GetString(IDS_FLIPPED_FACES); }
	SClass_ID		SuperClassID() { return GUP_CLASS_ID; }
	Class_ID		ClassID() { return FLIPPED_FACES_CLASS_ID; }
	const TCHAR* 	Category() { return _T(""); }

	const TCHAR*	InternalName() { return _T("FlippedFacesClass"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle

};

static FlippedFacesClassDesc classDesc;
ClassDesc2* GetFlippedFacesClassDesc() { return &classDesc; }

bool FlippedFacesChecker::IsSupported(INode* node)
{
	if(node)
	{
		ObjectState os  = node->EvalWorldState(GetCOREInterface()->GetTime());
		Object *obj = os.obj;
		if(obj&&obj->IsSubClassOf(triObjectClassID)||
			obj->IsSubClassOf(polyObjectClassID))
			return true;
	}
	return false;
}
IGeometryChecker::ReturnVal FlippedFacesChecker::TypeReturned()
{
	return IGeometryChecker::eFaces;
}
IGeometryChecker::ReturnVal FlippedFacesChecker::GeometryCheck(TimeValue t,INode *nodeToCheck, IGeometryChecker::OutputVal &val)
{
	val.mIndex.ZeroCount();
	if(IsSupported(nodeToCheck))
	{
		ObjectState os  = nodeToCheck->EvalWorldState(t);
		Object *obj = os.obj;
	
		if(os.obj->IsSubClassOf(triObjectClassID))
		{
			TriObject *tri = dynamic_cast<TriObject *>(os.obj);
			if(tri&&tri->mesh.numFaces>0)
			{
				tri->mesh.checkNormals(TRUE);
				for(int i=0;i<tri->mesh.numFaces;++i)
				{
					val.mIndex.Append(1,&i);
				}
				return TypeReturned();
			}
			else return IGeometryChecker::eFail;
		}
		else if(os.obj->IsSubClassOf(polyObjectClassID))
		{
			PolyObject *poly = dynamic_cast<PolyObject *>(os.obj);
			if(poly&&poly->GetMesh().numf>0)
			{
				IGeometryChecker::ReturnVal returnval=IGeometryChecker::eFail;
				MNMesh &mnMesh = poly->GetMesh();
				for(int i=0;i<mnMesh.numf;++i)
				{
					val.mIndex.Append(1,&i);
				}
				return TypeReturned();
			}
			else return IGeometryChecker::eFail;
		}
		
		return TypeReturned();
	}
	return IGeometryChecker::eFail;
}

void FlippedFacesChecker::DisplayOverride(TimeValue t, INode* inode, HWND hwnd,Tab<int> &results)
{
	DisplayIn3DViewport d3dViewport;
	Color c;
	COLORREF cr = ColorMan()->GetColor(GEOMETRY_DISPLAY_COLOR);

	c = Color(cr);
	DisplayIn3DViewport::DisplayParams params;
	params.mC = &c;
	params.mSeeThrough = false;
	params.mLighted = false;
	params.mFlipFaces= true;
	params.mDrawEdges = true;
	d3dViewport.DisplayResults(params,t,inode,hwnd,TypeReturned(),results);
}


MSTR FlippedFacesChecker::GetName()
{
	return MSTR(_T(GetString(IDS_FLIPPED_FACES)));
}