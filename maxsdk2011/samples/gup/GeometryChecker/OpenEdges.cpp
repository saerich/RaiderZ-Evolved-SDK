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
// DESCRIPTION: Open Edges Checker
// AUTHOR: Michael Zyracki created 2008
//***************************************************************************/
#include<3dsmaxport.h>
#include "checkerimplementations.h"
#include <omp.h>

//Edge Face List Supporting Class

EdgeFaceList::EdgeFaceList()
{
	mNumVertexEdges = 0;
	mVertexEdges = NULL;
}

EdgeFaceList::~EdgeFaceList()
{
	DeleteData();
}

void EdgeFaceList::DeleteData()
{
	if(mVertexEdges)
	{
		for(int i=0;i<mNumVertexEdges;++i)
			if(mVertexEdges[i])
				delete mVertexEdges[i];
	}
	mNumVertexEdges = 0;
}

void EdgeFaceList::CreateEdges()
{
	mVertexEdges = new VertexEdges*[mNumVertexEdges];
	for(int i = 0;i<mNumVertexEdges;++i)
	{
		mVertexEdges[i] = new VertexEdges();
	}
}
void EdgeFaceList::CreateEdgeFaceList(Mesh &mesh,bool createComplete)
{
	DeleteData();
	if(mesh.numFaces<=0||mesh.numVerts<=0)
		return;
	mNumVertexEdges = (mesh.numVerts);
	CreateEdges();
	//go through each face, and for each vertex find the edge (we actually doulbe but that's okay
	int a,b;
	int numVerts = 3;
	Edge *newEdge;
	for(int i=0;i<mesh.numFaces;++i)
	{
		for(int j=0;j<numVerts;++j) //create one edge per each face vertex
		{
			if(j!=(numVerts-1))
			{
				a = j; b = j+1;
			}
			else
			{
				a = j; b =0; 
			}
			//create one edges for every edge, and we treat the a index to be less than b!
			int verta = mesh.faces[i].v[a];
			int vertb = mesh.faces[i].v[b];
			if(verta<vertb)
			{
				int temp = vertb;
				vertb  = verta;
				verta = temp;
			}
			int loopSize = (createComplete==true) ? 2 :1;
			for(int loop=0;loop<loopSize;++loop)//may need to do it twice if createComplete ==true
			{
				//see if that verta and vert b combo exists...
				if(loop==1)
				{
					//swap verta and vertb....
					int temp = vertb;
					vertb = verta;
					verta = temp;
				}

				//see if that verta and vert b combo exists...
				bool needToCreate = true;
				for(int z =0;z< mVertexEdges[verta]->mEdges.Count();++z)
				{
					if(mVertexEdges[verta]->mEdges[z]&&mVertexEdges[verta]->mEdges[z]->mVert1==verta&&
						mVertexEdges[verta]->mEdges[z]->mVert2==vertb)
					{
						mVertexEdges[verta]->mEdges[z]->mFaces.Append(1,&i);
						needToCreate=false;
						break;
					}
				}
				if(needToCreate==true)
				{
					newEdge = new Edge;
					newEdge->mVert1 = verta;
					newEdge->mVert2 = vertb;
					newEdge->mFaces.Append(1,&i);
					newEdge->mEdgeNum = CheckerMeshEdge::GetEdgeNum(i,a);//edge num is the tri(i) and the local edge #(a)
					mVertexEdges[verta]->mEdges.Append(1,&newEdge);
				}
			}
		}
	}
}

void EdgeFaceList::CreateEdgeFaceList(MNMesh &mesh,bool createComplete)
{
	DeleteData();
	if(mesh.nume<=0)
		return;
	mNumVertexEdges = (mesh.numv);
	CreateEdges();
	//go through each edge, and for each edge find the vertex and the faces it belongs too.
	Edge *newEdge;
	for(int i=0;i<mesh.nume;++i)
	{
		MNEdge *mnEdge = &mesh.e[i];
		if(mnEdge&&mnEdge->GetFlag(MN_DEAD)==0)
		{
			int verta = mnEdge->v1;
			int vertb = mnEdge->v2;
			if(verta<vertb)
			{
				int temp = vertb;
				vertb  = verta;
				verta = temp;
			}
			int loopSize = (createComplete==true) ? 2 :1;
			for(int loop=0;loop<loopSize;++loop)//may need to do it twice if createComplete ==true
			{
				//see if that verta and vert b combo exists...
				if(loop==1)
				{
					//swap verta and vertb....
					int temp = vertb;
					vertb = verta;
					verta = temp;
				}
				bool needToCreate = true;
				for(int z =0;z< mVertexEdges[verta]->mEdges.Count();++z)
				{
					if(mVertexEdges[verta]->mEdges[z]&&mVertexEdges[verta]->mEdges[z]->mVert1==verta&&
						mVertexEdges[verta]->mEdges[z]->mVert2==vertb)
					{
						//now make sure that the face doesn't already exist, then append!
						//need to do it for f1 and f2.
						bool needToAddFace1 =true;
						bool needToAddFace2 =true;
						int y;
						for(y = 0;y<mVertexEdges[verta]->mEdges[z]->mFaces.Count();++y)
						{
							if(mnEdge->f1==mVertexEdges[verta]->mEdges[z]->mFaces[y])
							{
								needToAddFace1 = false;
							}
							if(mnEdge->f2==mVertexEdges[verta]->mEdges[z]->mFaces[y])
							{
								needToAddFace2 = false;
							}
						}
						if(needToAddFace1==true)
							mVertexEdges[verta]->mEdges[z]->mFaces.Append(1,&mnEdge->f1);
						if(needToAddFace2==true)
							mVertexEdges[verta]->mEdges[z]->mFaces.Append(1,&mnEdge->f2);
						needToCreate=false;
						break;
					}
				}
				if(needToCreate==true)
				{
					newEdge = new Edge;
					newEdge->mVert1 = verta;
					newEdge->mVert2 = vertb;
					if(mnEdge->f1!=-1)
						newEdge->mFaces.Append(1,&mnEdge->f1);
					if(mnEdge->f2!=-1)
						newEdge->mFaces.Append(1,&mnEdge->f2);
					newEdge->mEdgeNum = i;
					mVertexEdges[verta]->mEdges.Append(1,&newEdge);
				}
			}
		}
	}
}

OpenEdgesChecker OpenEdgesChecker::_theInstance(OPEN_EDGES_INTERFACE, _T("OpenEdges"), 0, NULL, FP_CORE,
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

class OpenEdgesClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return TRUE; }
	void *			Create(BOOL loading = FALSE) {return &OpenEdgesChecker::_theInstance; }
	const TCHAR *	ClassName() { return GetString(IDS_OPEN_EDGES); }
	SClass_ID		SuperClassID() { return GUP_CLASS_ID; }
	Class_ID		ClassID() { return OPEN_EDGES_CLASS_ID; }
	const TCHAR* 	Category() { return _T(""); }

	const TCHAR*	InternalName() { return _T("OpenEdgesClass"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle

};

static OpenEdgesClassDesc classDesc;
ClassDesc2* GetOpenEdgesClassDesc() { return &classDesc; }

bool OpenEdgesChecker::IsSupported(INode* node)
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
IGeometryChecker::ReturnVal OpenEdgesChecker::TypeReturned()
{
	return IGeometryChecker::eEdges;
}
IGeometryChecker::ReturnVal OpenEdgesChecker::GeometryCheck(TimeValue t,INode *nodeToCheck, IGeometryChecker::OutputVal &val)
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
		BitArray arrayOfEdges; //we fill this up with the verts, if not empty then we have isolated vets
		EdgeFaceList edgeList;
		if(os.obj->IsSubClassOf(triObjectClassID))
		{
			TriObject *tri = dynamic_cast<TriObject *>(os.obj);
			if(tri)
			{
				Mesh &mesh = tri->GetMesh();
				edgeList.CreateEdgeFaceList(mesh);
				if(checkTime)
				{
					DialogChecker::Check(checkTime,compute,mesh.numFaces,startTime,ups);
					if(compute==false)
						return IGeometryChecker::eFail;
				}
				arrayOfEdges.SetSize(CheckerMeshEdge::GetTotalNumEdges(mesh.numFaces));
				arrayOfEdges.ClearAll();
			}
			else return IGeometryChecker::eFail;
		}
		else if(os.obj->IsSubClassOf(polyObjectClassID))
		{
			PolyObject *poly = dynamic_cast<PolyObject *>(os.obj);
			if(poly)
			{
				MNMesh &mnMesh = poly->GetMesh();
				edgeList.CreateEdgeFaceList(mnMesh);
				if(checkTime)
				{
					DialogChecker::Check(checkTime,compute,mnMesh.numf,startTime,ups);
					if(compute==false)
						return IGeometryChecker::eFail;
				}
				arrayOfEdges.SetSize(mnMesh.nume);
				arrayOfEdges.ClearAll();
			}
			else return IGeometryChecker::eFail;
		}

		#pragma omp parallel for
		for(int i=0;i<edgeList.mNumVertexEdges;++i)
		{
			for(int j=0;j<edgeList.mVertexEdges[i]->mEdges.Count();++j)
			{
				EdgeFaceList::Edge *localEdge = edgeList.mVertexEdges[i]->mEdges[j];
				if(localEdge)
				{
					if(localEdge->mFaces.Count()==1)
					{
						#pragma omp critical
						{
							arrayOfEdges.Set(localEdge->mEdgeNum);
						}
					}
				}
			}
		}
		if(arrayOfEdges.IsEmpty()==false) //we have an open edge
		{
			int localsize= arrayOfEdges.GetSize();
			for(int i=0;i<localsize;++i)
			{
				if(arrayOfEdges[i])
					val.mIndex.Append(1,&i);
			}
		}
		return TypeReturned();
	}
	return IGeometryChecker::eFail;
}

MSTR OpenEdgesChecker::GetName()
{
	return MSTR(_T(GetString(IDS_OPEN_EDGES)));
}



