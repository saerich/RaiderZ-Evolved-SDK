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
// DESCRIPTION: Isolated Vertex Checker
// AUTHOR: Michael Zyracki created 2008
//***************************************************************************/
#include<3dsmaxport.h>
#include "checkerimplementations.h"
#include <omp.h>

IsolatedVertexChecker IsolatedVertexChecker::_theInstance(ISOLATED_VERTEX_INTERFACE, _T("IsolatedVertices"), 0, NULL, FP_CORE,
	GUPChecker::kGeometryCheck, _T("Check"),  0,TYPE_ENUM, GUPChecker::kOutputEnum,0,3,
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

class IsolatedVertexClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return TRUE; }
	void *			Create(BOOL loading = FALSE) {return &IsolatedVertexChecker::_theInstance; }
	const TCHAR *	ClassName() { return GetString(IDS_ISOLATED_VERTEX); }
	SClass_ID		SuperClassID() { return GUP_CLASS_ID; }
	Class_ID		ClassID() { return ISOLATED_VERTEX_CLASS_ID; }
	const TCHAR* 	Category() { return _T(""); }

	const TCHAR*	InternalName() { return _T("IsolatedVertexClass"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle

};

static IsolatedVertexClassDesc classDesc;
ClassDesc2* GetIsolatedVertexClassDesc() { return &classDesc; }

bool IsolatedVertexChecker::IsSupported(INode* node)
{
	if(node)
	{
		ObjectState os  = node->EvalWorldState(GetCOREInterface()->GetTime());
		Object *obj = os.obj;
		if(obj&&obj->IsSubClassOf(triObjectClassID)||   //no patches obj->IsSubClassOf(patchObjectClassID)||
			obj->IsSubClassOf(polyObjectClassID))
			return true;
	}
	return false;
}
IGeometryChecker::ReturnVal IsolatedVertexChecker::TypeReturned()
{
	return IGeometryChecker::eVertices;
}
IGeometryChecker::ReturnVal IsolatedVertexChecker::GeometryCheck(TimeValue t,INode *nodeToCheck, IGeometryChecker::OutputVal &val)
{
	val.mIndex.ZeroCount();
	if(IsSupported(nodeToCheck))
	{
		LARGE_INTEGER	ups,startTime;
		QueryPerformanceFrequency(&ups);
		QueryPerformanceCounter(&startTime); //used to see if we need to pop up a dialog 
		bool compute = true;
		bool checkTime = GetIGeometryCheckerManager()->GetAutoUpdate();//only check time for the dialog if auto update is active!
		ObjectState os  = nodeToCheck->EvalWorldState(t);
		Object *obj = os.obj;
		BitArray arrayOfVerts; //we fill this up with the verts, if not empty then we have isolated vets
		if(os.obj->IsSubClassOf(triObjectClassID))
		{
			TriObject *tri = dynamic_cast<TriObject *>(os.obj);
			if(tri)
			{
				Mesh &mesh = tri->GetMesh();
				arrayOfVerts.SetSize(mesh.numVerts);
				arrayOfVerts.SetAll ();
				for (int i=0; i<mesh.numFaces; i++) {
					
					for (int j=0; j<3; j++) arrayOfVerts.Clear(mesh.faces[i].v[j]);
					if(checkTime==true)
					{
						DialogChecker::Check(checkTime,compute,mesh.numFaces,startTime,ups);
						if(compute==false)
							break;
						
					}
				}
			}
			else return IGeometryChecker::eFail;
		}
		else if(os.obj->IsSubClassOf(polyObjectClassID))
		{
			PolyObject *poly = dynamic_cast<PolyObject *>(os.obj);
			if(poly)
			{
				MNMesh &mnMesh = poly->GetMesh();
				arrayOfVerts.SetSize(mnMesh.numv);
				arrayOfVerts.SetAll();
				for(int i=0;i<mnMesh.numf;++i)
				{
					for(int j=0;j<mnMesh.f[i].deg;++j) 
					{
						if(mnMesh.f[i].GetFlag(MN_DEAD)==0)
							arrayOfVerts.Clear(mnMesh.f[i].vtx[j]);
					}
					if(checkTime==true)
					{
						DialogChecker::Check(checkTime,compute,mnMesh.numf,startTime,ups);
						if(compute==false)
							break;
					}
				}
			}
			else return IGeometryChecker::eFail;
		}
		if(arrayOfVerts.IsEmpty()==false) //we have an isolated vierts
		{
			int localsize= arrayOfVerts.GetSize();
			for(int i=0;i<localsize;++i)
			{
				if(arrayOfVerts[i])
					val.mIndex.Append(1,&i);
			}
		}
		return TypeReturned();
	}
	return IGeometryChecker::eFail;
}

MSTR IsolatedVertexChecker::GetName()
{
	return MSTR(_T(GetString(IDS_ISOLATED_VERTEX)));
}
