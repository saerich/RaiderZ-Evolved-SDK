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
// DESCRIPTION: Overlapping Faces checker
// AUTHOR: Michael Zyracki created 2008
//***************************************************************************/
#include<3dsmaxport.h>
#include "checkerimplementations.h"
#include <omp.h>

OverlappingFacesChecker OverlappingFacesChecker::_theInstance(OVERLAPPING_FACES_INTERFACE, _T("OverlappingFaces"), 0, NULL, FP_CORE,
	GUPChecker::kGeometryCheck, _T("Check"), 0,TYPE_ENUM, GUPChecker::kOutputEnum,0,3,
		_T("time"), 0, TYPE_TIMEVALUE,
		_T("nodeToCheck"), 0, TYPE_INODE,
		_T("results"), 0, TYPE_INDEX_TAB_BR,
	GUPChecker::kHasPropertyDlg, _T("hasPropertyDlg"),0,TYPE_bool,0,0,
	GUPChecker::kShowPropertyDlg, _T("showPropertyDlg"),0,TYPE_VOID,0,0,
    properties,
		OverlappingFacesChecker::kGetTolerance,  OverlappingFacesChecker::kSetTolerance, _T("tolerance"), 0,TYPE_FLOAT,

	enums,
		GUPChecker::kOutputEnum, 4,	
		_T("Failed"), GUPChecker::kFailed,
		_T("Vertices"), GUPChecker::kVertices,
		_T("Edges"), GUPChecker::kEdges,
		_T("Faces"), GUPChecker::kFaces,
	end	
																			
  );

class OverlappingFacesClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return TRUE; }
	void *			Create(BOOL loading = FALSE) {return &OverlappingFacesChecker::_theInstance; }
	const TCHAR *	ClassName() { return GetString(IDS_OVERLAPPING_FACES); }
	SClass_ID		SuperClassID() { return GUP_CLASS_ID; }
	Class_ID		ClassID() { return OVERLAPPING_FACES_CLASS_ID; }
	const TCHAR* 	Category() { return _T(""); }

	const TCHAR*	InternalName() { return _T("OverlappingFacesClass"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle

};

static OverlappingFacesClassDesc classDesc;
ClassDesc2* GetOverlappingFacesClassDesc() { return &classDesc; }


static INT_PTR CALLBACK OverlappingFacesDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	OverlappingFacesChecker *ov = DLGetWindowLongPtr<OverlappingFacesChecker *>(hDlg);

	switch (msg) {
	case WM_INITDIALOG:
		PositionWindowNearCursor(hDlg);
		DLSetWindowLongPtr(hDlg, lParam);
		ov = (OverlappingFacesChecker *)lParam;
		setupSpin(hDlg, IDC_TOLERANCE_SPIN, IDC_TOLERANCE, ov->GetTolerance());
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
            goto done;
		case IDCANCEL:
			EndDialog(hDlg, FALSE);
			return FALSE;
		}
		break;

	case WM_CLOSE:
done:
		theHold.Begin();
		ov->SetTolerance(getSpinVal(hDlg, IDC_TOLERANCE_SPIN));
		theHold.Accept(GetString(IDS_PARAMETER_CHANGED));

		EndDialog(hDlg, TRUE);
		return FALSE;
	}
	return FALSE;
}


void OverlappingFacesChecker::ShowPropertyDlg()
{
	HWND hDlg = GetCOREInterface()->GetMAXHWnd();
	DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_OVERLAPPING_FACES),
				hDlg, OverlappingFacesDlgProc, (LPARAM)this);
}

bool OverlappingFacesChecker::IsSupported(INode* node)
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
IGeometryChecker::ReturnVal OverlappingFacesChecker::TypeReturned()
{
	return IGeometryChecker::eFaces;
}


__inline BOOL WithinTri(DPoint3 bry)
{
	static const float localEPSILON	=	0.0001f;

	if (bry.x<-1e-5f || bry.x>1.0001f || bry.y<-1e-5f || bry.y>1.0001f
		|| bry.z<-1e-5f || bry.z>1.0001f)
		return FALSE;
	if (fabs(bry.x + bry.y + bry.z - 1.0f) > localEPSILON) 
		return FALSE;
	else
		return TRUE;
}

bool OverlappingFacesChecker::Dist2PtToTri(const Point3 &pt, const Point3 &a, const Point3 &b,
				   const Point3 &c, const Point3 &norm,Point3 &position,Point3 &bary,float &dist, float *distTolSquared)

{
	Point3 edge1,edge2,edge3;
	Point3 pt1,pt2,pt3;
	Point3 res;

	//find the dot's from the 3 edges....
	Point3 ptTemp;
	edge1 = b -a;
	ptTemp = pt -a;
	Point3 ptOnPlane;

	//find point on plane
	float d = DotProd(a,norm);
	float t = DotProd(pt ,norm) -d;
	ptOnPlane = pt + norm*-t;
	dist =  LengthSquared(pt-ptOnPlane);
	if(distTolSquared&& dist> *distTolSquared)
		return false; // too far away from the plane

	bary =  BaryCoords(a,b,c,ptOnPlane);
	if(WithinTri(bary))
	{
		position = ptOnPlane;
		return true;
	}
	return false;
}

DWORD WINAPI fn(LPVOID arg)
{
	return(0);
}


//Escape Checker
void EscapeChecker::Setup () {
	GetAsyncKeyState (VK_ESCAPE);	// Clear any previous presses.
}

bool EscapeChecker::Check ()
{
	mMessageFilter.RunNonBlockingMessageLoop();
	return mMessageFilter.Aborted();
}

EscapeChecker EscapeChecker::theEscapeChecker;


	


class CacheMeshFaceNormals
{

public:
	CacheMeshFaceNormals(MNMesh &mesh): mMeshNormals(NULL),mNumFaces(0){CreateNormals(mesh);};
	~CacheMeshFaceNormals(){FreeCache();}

	//return the Normal for the given tri in the specified face
	Point3 Normal(int faceNum, int triNum);
	Tab<int> & GetTriangles(int faceNum);
private:
	void CreateNormals(MNMesh &mesh);
	void FreeCache();

	struct MeshNormals
	{
		int mNumTris;
		Point3 *mNormals;
		Tab<int> mTriangles;
	};

	MeshNormals *mMeshNormals;
	int mNumFaces;
};

void CacheMeshFaceNormals::CreateNormals(MNMesh &mesh)
{
	FreeCache();
	if(mesh.numf>0)
	{
		mNumFaces = mesh.numf;
		mMeshNormals = new MeshNormals[mNumFaces];
		Tab<int> tri;
		#pragma omp parallel for private(tri)
		for(int f = 0;f<mNumFaces;++f)
		{
			mMeshNormals[f].mNormals = NULL; //make sure that's NULL'ed out, else we may delete garbage when we free the cache.
			if(mesh.f[f].GetFlag(MN_DEAD)==0) 
			{
				MNFace *face = &mesh.f[f];
				mMeshNormals[f].mNumTris = face->TriNum(); //deg -2
				mMeshNormals[f].mNormals = new Point3[face->TriNum()];
				int deg = face->deg;
				face->GetTriangles(tri);
				mMeshNormals[f].mTriangles = tri;
				int		*vv		= face->vtx;
				int triCount =0;
				for (int tt=0; tt<tri.Count(); tt+=3)
				{
					int *triv = tri.Addr(tt);
					Point3 &v0 = mesh.v[vv[triv[0]]].p;
					Point3 &v1 = mesh.v[vv[triv[1]]].p;
					Point3 &v2 = mesh.v[vv[triv[2]]].p;
					Point3 edge1 = v1-v0;
					Point3 edge2 = v2-v1;
					Point3 cross = CrossProd(edge2,edge1);
					Point3 normal = cross.Normalize();
					mMeshNormals[f].mNormals[triCount].x = normal.x;
					mMeshNormals[f].mNormals[triCount].y = normal.y;
					mMeshNormals[f].mNormals[triCount].z = normal.z;
					++triCount;
				}
			}
		}
	}
}

void CacheMeshFaceNormals::FreeCache()
{
	if(mMeshNormals)
	{
		for(int i=0;i<mNumFaces;++i)
		{
			if(mMeshNormals[i].mNormals)
				delete [] mMeshNormals[i].mNormals;
		}
		delete [] mMeshNormals;
		mMeshNormals = NULL;
	}
	mNumFaces = 0;
}
Point3 CacheMeshFaceNormals::Normal(int faceNum,int triNum)
{
	DbgAssert(faceNum>-1&&faceNum<mNumFaces);
	return mMeshNormals[faceNum].mNormals[triNum];
}
Tab<int> & CacheMeshFaceNormals::GetTriangles(int faceNum)
{
	return mMeshNormals[faceNum].mTriangles;
}

//this test is basically for each face, we find a vertex that overlaps...
IGeometryChecker::ReturnVal OverlappingFacesChecker::GeometryCheck(TimeValue t,INode *nodeToCheck, IGeometryChecker::OutputVal &val)
{
	val.mIndex.ZeroCount();
	if(IsSupported(nodeToCheck))
	{
		LARGE_INTEGER	ups,startTime;
		QueryPerformanceFrequency(&ups);
		QueryPerformanceCounter(&startTime); //used to see if we need to pop up a dialog 
		bool checkTime = GetIGeometryCheckerManager()->GetAutoUpdate();//only check time for the dialog if auto update is active!
		ObjectState os  = nodeToCheck->EvalWorldState(t);
		Object *obj = os.obj;
		
		BitArray arrayOfFaces;
		Interface *ip = GetCOREInterface();
		TCHAR buf[256];
		TCHAR tmpBuf[64];
		_tcscpy(tmpBuf,GetString(IDS_CHECK_TIME_USED));
		_tcscpy(buf,GetString(IDS_RUNNING_GEOMETRY_CHECKER));
		if(os.obj->IsSubClassOf(triObjectClassID))
		{
			TriObject *tri = dynamic_cast<TriObject *>(os.obj);
			if(tri)
			{
				ip->PushPrompt(buf);
				Mesh mesh = tri->GetMesh();
				mesh.checkNormals(TRUE); //we need the normals to be built, if they aren't already.
				arrayOfFaces.SetSize(mesh.numFaces);
				int deg = 3;
				float tolSquared= GetTolerance()*GetTolerance();
				int total = 0;
				EscapeChecker::theEscapeChecker.Setup();
				for (int i=0; i<mesh.numFaces; i++)
				{
					bool partOfFace;
					float distance;
					Point3 ptOnPlane,bary;
					Point3 &normal1 = mesh.getFaceNormal(i);
					#pragma omp parallel for  
					for(int j=0;j<mesh.numFaces;++j)
					{
						Point3 &normal2 = mesh.getFaceNormal(j);
						float dot = normal1%normal2;
						if((dot>.9999f&&dot<1.0001f)||(dot<-.9999f&&dot>-1.0001f)) //coplanar
						{
							for(int z =0;z<deg;++z)
							{
								int vert = mesh.faces[j].v[z]; //vert on the face
								//make sure that vert isn't part of the face, if so skip it
								partOfFace= false;
								for(int k=0;k<deg;++k)
								{
									if(mesh.faces[i].v[k]==vert)
									{
										partOfFace =true;
										break;
									}
								}
								if(partOfFace==false)
								{
									Point3 &v0 = mesh.verts[mesh.faces[i].v[0]];
									Point3 &v1 = mesh.verts[mesh.faces[i].v[1]];
									Point3 &v2 = mesh.verts[mesh.faces[i].v[2]];
									Point3 &point = mesh.verts[vert];
									//see if we are inside the triangle and then check distance
									if(Dist2PtToTri(point,v0,v1,v2,normal1,ptOnPlane,bary,distance,&tolSquared)==true)
									{
										#pragma omp critical
										{
											arrayOfFaces.Set(j);
											arrayOfFaces.Set(i);
										}
									}
								}
							}
						}
					}
				
					total += 1;
					{
						
						LARGE_INTEGER endTime;
						QueryPerformanceCounter(&endTime);
						float sec = (float)((double)(endTime.QuadPart-startTime.QuadPart)/(double)ups.QuadPart);
						if(sec>ESC_TIME)
						{
							if(EscapeChecker::theEscapeChecker.Check())
							{
								_tcscpy(buf,GetString(IDS_EXITING_GEOMETRY_CHECKER));
								ip->ReplacePrompt(buf);
								break;
							}
						}
						if((total%100)==0)
						{
							{
								float percent = (float)total/(float)mesh.numFaces*100.0f;;
								_stprintf(buf,tmpBuf,percent);
								ip->ReplacePrompt(buf);
							}     
						}
						if(checkTime==true)
						{
							bool compute = true;
							DialogChecker::Check(checkTime,compute,mesh.numFaces,startTime,ups);
							if(compute == false)
								break;
						}
					}
				}
				ip->PopPrompt();
			}
			else return IGeometryChecker::eFail;

		}
		else if(os.obj->IsSubClassOf(polyObjectClassID))
		{
			PolyObject *poly = dynamic_cast<PolyObject *>(os.obj);
			if(poly)
			{
				ip->PushPrompt(buf);
				MNMesh mnMesh = poly->GetMesh();
				arrayOfFaces.SetSize(mnMesh.numf);
				float tolSquared= GetTolerance()*GetTolerance();
				int total = 0;
				EscapeChecker::theEscapeChecker.Setup();
				CacheMeshFaceNormals cachedNormals(mnMesh);
				//Tab<int>tri;
				//Tab<int> newTri;
				for (int i=0; i<mnMesh.numf; i++)
				{
					if(mnMesh.f[i].GetFlag(MN_DEAD)==0) 
					{
						bool partOfFace;
						float distance;
						Point3 ptOnPlane,bary;
						MNFace *face = &mnMesh.f[i];
						int deg = face->deg;
						Tab<int> &tri = cachedNormals.GetTriangles(i);
						int		*vv		= face->vtx;
						int triNum1 = 0;  //keep track which triangle we are looking at for the polly
						for (int tt=0; tt<tri.Count(); tt+=3)
						{
							int *triv = tri.Addr(tt);
							Point3 &v0 = mnMesh.v[vv[triv[0]]].p;
							Point3 &v1 = mnMesh.v[vv[triv[1]]].p;
							Point3 &v2 = mnMesh.v[vv[triv[2]]].p;
							Point3 normal1 = cachedNormals.Normal(i,triNum1);
							++triNum1;
							#pragma omp parallel for 
							for(int j=0;j<mnMesh.numf;++j)
							{
								if(i!=j&&mnMesh.f[j].GetFlag(MN_DEAD)==0)
								{
									int *vv2 = mnMesh.f[j].vtx;
									Tab<int> &newTri = cachedNormals.GetTriangles(j);
									int triNum2 = 0;  //keep track which triangle we are looking at for the polly
									for (int tt2=0; tt2<newTri.Count(); tt2+=3)
									{
										int *triv2 = newTri.Addr(tt2);
										Point3 &v0a = mnMesh.v[vv2[triv2[0]]].p;
										Point3 &v1a = mnMesh.v[vv2[triv2[1]]].p;
										Point3 &v2a = mnMesh.v[vv2[triv2[2]]].p;
										Point3 normal2 = cachedNormals.Normal(j,triNum2);
										++triNum2;
										float dot = normal1%normal2;
										if((dot>.9999f&&dot<1.0001f)||(dot<-.9999f&&dot>-1.0001f)) //coplanar
										{
											//okay it's coplanar we check the 3 verts onto the plane of the triangle...
											for(int z =0;z<3;++z)
											{
												int vert = vv2[triv2[z]];
												//check if part of face i
												partOfFace= false;
												for(int k=0;k<3;++k) //make sure it's not the vert on the orignal
												{
													if(vv[triv[k]]==vert)
													{
														partOfFace =true;
														break;
													}
												}
												if(partOfFace==false)
												{
													//get the point from the current triangle
													Point3 point = mnMesh.v[vert].p;
													//see if we are inside the triangle and then check distance
													if(Dist2PtToTri(point,v0,v1,v2,normal1,ptOnPlane,bary,distance,&tolSquared)==true)
													{
														#pragma omp critical
														{
															arrayOfFaces.Set(j);
															arrayOfFaces.Set(i);
														}
														z = 3;
														tt2 = newTri.Count();
														break; //only need to check one triangle
													}
												}
											}
										}
									}
								}
							}
						}
					}
                    total += 1;
					
					LARGE_INTEGER endTime;
					QueryPerformanceCounter(&endTime);
					float sec = (float)((double)(endTime.QuadPart-startTime.QuadPart)/(double)ups.QuadPart);
					if(sec>ESC_TIME)
					{
						if(EscapeChecker::theEscapeChecker.Check())
						{
							_tcscpy(buf,GetString(IDS_EXITING_GEOMETRY_CHECKER));
							ip->ReplacePrompt(buf);
							break;
						}
					}
					if((total%100)==0)
					{
						{
							float percent = (float)total/(float)mnMesh.numf*100.0f;;
							_stprintf(buf,tmpBuf,percent);
							ip->ReplacePrompt(buf);
						}     
					}
					if(checkTime==true)
					{
						bool compute = true;
						DialogChecker::Check(checkTime,compute,mnMesh.numf,startTime,ups);
						if(compute ==false)
							break;
					}
				}
				ip->PopPrompt();
			}
			else return IGeometryChecker::eFail;
		}

		if(arrayOfFaces.IsEmpty()==false) //we have overlapping faces
		{
			for(int i=0;i<arrayOfFaces.GetSize();++i) 
			{
				if(arrayOfFaces[i])
					val.mIndex.Append(1,&i);
			}
		}
		return TypeReturned();
	}
	return IGeometryChecker::eFail;
}

MSTR OverlappingFacesChecker::GetName()
{
	return MSTR(_T(GetString(IDS_OVERLAPPING_FACES)));
}

