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
// DESCRIPTION: Overlapping Vertices Checker
// AUTHOR: Michael Zyracki created 2008
//***************************************************************************/
#include<3dsmaxport.h>
#include "checkerimplementations.h"
/*
   x
  /|\
 / | \
x--o--x
 \   /
  \ /
   x

Vertex 'o' above is a tvert 

pseudo code for the test.
For each triangle/poly
    For each edge
	     see if there are any other edges that are 'parallel' to that edge, whose length is less than that of that edge.
*/

TVertsChecker TVertsChecker::_theInstance(TVERTS_INTERFACE, _T("TVerts"), 0, NULL, FP_CORE,
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

class TVertsClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return TRUE; }
	void *			Create(BOOL loading = FALSE) {return &TVertsChecker::_theInstance; }
	const TCHAR *	ClassName() { return GetString(IDS_TVERTS); }
	SClass_ID		SuperClassID() { return GUP_CLASS_ID; }
	Class_ID		ClassID() { return TVERTS_CLASS_ID; }
	const TCHAR* 	Category() { return _T(""); }

	const TCHAR*	InternalName() { return _T("TVertsClass"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle

};

static TVertsClassDesc classDesc;
ClassDesc2* GetTVertsClassDesc() { return &classDesc; }

bool TVertsChecker::IsSupported(INode* node)
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
IGeometryChecker::ReturnVal TVertsChecker::TypeReturned()
{
	return IGeometryChecker::eVertices;
}

IGeometryChecker::ReturnVal TVertsChecker::GeometryCheck(TimeValue t,INode *nodeToCheck, IGeometryChecker::OutputVal &val)
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
		EdgeFaceList edgeList;
		if(os.obj->IsSubClassOf(triObjectClassID))
		{
			TriObject *tri = dynamic_cast<TriObject *>(os.obj);
			if(tri)
			{
				Mesh &mesh = tri->GetMesh();
				edgeList.CreateEdgeFaceList(mesh,true);
				if(checkTime)
				{
					DialogChecker::Check(checkTime,compute,tri->mesh.numFaces,startTime,ups);
					if(compute==false)
						return IGeometryChecker::eFail;
				}
				for(int i=0;i<mesh.numVerts;++i)//numverts == edgeList.mVertexEdges.Count()
				{
					if(edgeList.mVertexEdges[i]->mEdges.Count()==3) //tvert, 3 edges, no open edges
					{
						//now make sure no edges are open...
						if(edgeList.mVertexEdges[i]->mEdges[0]->mFaces.Count()>1&&
						   edgeList.mVertexEdges[i]->mEdges[1]->mFaces.Count()>1&&
						   edgeList.mVertexEdges[i]->mEdges[2]->mFaces.Count()>1)
							val.mIndex.Append(1,&i);
					}
					if(checkTime)
					{
						DialogChecker::Check(checkTime,compute,tri->mesh.numFaces,startTime,ups);
						if(compute==false)
							return IGeometryChecker::eFail;	
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
				edgeList.CreateEdgeFaceList(mnMesh,true);
				if(checkTime)
				{
					DialogChecker::Check(checkTime,compute,mnMesh.numf,startTime,ups);
					if(compute==false)
						return IGeometryChecker::eFail;
				}
				// NO, causes defect Defect 1161279 - T-Vertices breaks polygon when flipping 
				// so removed for now but leave in case it's needed we make sure that vedge is not null now instead.
				//mnMesh.FillInMesh(); //make sure the mesh has the vedg..
				if(mnMesh.vedg)
				{
					for(int i=0;i<mnMesh.numv;++i)
					{
						if(mnMesh.vedg[i].Count()==3) //tvert, 3 edges
						{
							if(mnMesh.e[mnMesh.vedg[i][0]].f1>-1 && mnMesh.e[mnMesh.vedg[i][0]].f2>-1 &&
								mnMesh.e[mnMesh.vedg[i][1]].f1>-1 && mnMesh.e[mnMesh.vedg[i][1]].f2>-1 &&
								mnMesh.e[mnMesh.vedg[i][2]].f1>-1 && mnMesh.e[mnMesh.vedg[i][2]].f2>-1)
							val.mIndex.Append(1,&i);
						}
						if(checkTime)
						{
							DialogChecker::Check(checkTime,compute,mnMesh.numf,startTime,ups);
							if(compute==false)
								return IGeometryChecker::eFail;
						}
					}
				}
			}
			else return IGeometryChecker::eFail;
		}

		return TypeReturned();
	}
	return IGeometryChecker::eFail;
}

MSTR TVertsChecker::GetName()
{
	return MSTR(_T(GetString(IDS_TVERTS)));
}
