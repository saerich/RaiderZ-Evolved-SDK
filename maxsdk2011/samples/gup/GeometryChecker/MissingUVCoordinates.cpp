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
// DESCRIPTION: Missing UV Coordinates checker
// AUTHOR: Michael Zyracki created 2008
//***************************************************************************/
#include<3dsmaxport.h>
#include "checkerimplementations.h"
#include <omp.h>
MissingUVCoordinatesChecker MissingUVCoordinatesChecker::_theInstance(MISSING_UV_COORDINATES_INTERFACE, _T("MissingUVCoordinates"), 0, NULL, FP_CORE,
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

class MissingUVCoordinatesClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return TRUE; }
	void *			Create(BOOL loading = FALSE) {return &MissingUVCoordinatesChecker::_theInstance; }
	const TCHAR *	ClassName() { return GetString(IDS_MISSING_UV_COORDINATES); }
	SClass_ID		SuperClassID() { return GUP_CLASS_ID; }
	Class_ID		ClassID() { return MISSING_UV_COORDINATES_CLASS_ID; }
	const TCHAR* 	Category() { return _T(""); }

	const TCHAR*	InternalName() { return _T("MissingUVCoordinatesClass"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle

};

static MissingUVCoordinatesClassDesc classDesc;
ClassDesc2* GetMissingUVCoordinatesClassDesc() { return &classDesc; }

bool MissingUVCoordinatesChecker::IsSupported(INode* node)
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
IGeometryChecker::ReturnVal MissingUVCoordinatesChecker::TypeReturned()
{
	return IGeometryChecker::eVertices;
}
IGeometryChecker::ReturnVal MissingUVCoordinatesChecker::GeometryCheck(TimeValue t,INode *nodeToCheck, IGeometryChecker::OutputVal &val)
{
	val.mIndex.ZeroCount();
	if(IsSupported(nodeToCheck))
	{
		LARGE_INTEGER	ups,startTime;
		QueryPerformanceFrequency(&ups);
		QueryPerformanceCounter(&startTime); //used to see if we need to pop up a dialog 
		bool compute = true;
		bool checkTime = GetIGeometryCheckerManager()->GetAutoUpdate();//only check time for the dialog if auto update is active!
		UVWChannel uvmesh;
		ObjectState os  = nodeToCheck->EvalWorldState(t);
		Object *obj = os.obj;
		if(os.obj->IsSubClassOf(triObjectClassID))
		{
			TriObject *tri = dynamic_cast<TriObject *>(os.obj);
			if(tri)
			{
				BitArray arrayOfVertices;
				arrayOfVertices.SetSize(tri->mesh.numVerts);
				arrayOfVertices.ClearAll();
				IGeometryChecker::ReturnVal returnval=IGeometryChecker::eFail;
				int numChannels = tri->mesh.getNumMaps();
				int index;
				for(int i=0;i<numChannels;++i)
				{
					if(tri->mesh.mapSupport(i))
					{

						MeshMap *map  = &tri->mesh.Map(i);
						if(map->getNumVerts()>0 &&map->getNumFaces()>0)
						{
							returnval= TypeReturned();
							int numFaces = map->getNumFaces();
							TVFace * tvFaces = map->tf;
							UVVert *  uvVerts= map->tv;
						
							#pragma omp parallel for
							for(int faceIndex =0;faceIndex<numFaces;++faceIndex)
							{
								if(compute)
								{
									TVFace& tvFace = tvFaces[faceIndex];
									Point3 tv = uvVerts[tvFace.t[0]];
									if(_isnan(tv.x) || !_finite(tv.x)||_isnan(tv.y) || !_finite(tv.y)||_isnan(tv.z) || !_finite(tv.z))
									{
										index = tri->mesh.faces[faceIndex].v[0];
										if(index>=0&&index<tri->mesh.numVerts)
										{
											#pragma omp critical
											{
												arrayOfVertices.Set(index);
											}
										}
									}
									tv = uvVerts[tvFace.t[1]];
									if(_isnan(tv.x) || !_finite(tv.x)||_isnan(tv.y) || !_finite(tv.y)||_isnan(tv.z) || !_finite(tv.z))
									{
										index = tri->mesh.faces[faceIndex].v[1];
										if(index>=0&&index<tri->mesh.numVerts)
										{
											#pragma omp critical
											{
												arrayOfVertices.Set(index);
											}
										}
									}
									tv = uvVerts[tvFace.t[2]];
									if(_isnan(tv.x) || !_finite(tv.x)||_isnan(tv.y) || !_finite(tv.y)||_isnan(tv.z) || !_finite(tv.z))
									{
										index = tri->mesh.faces[faceIndex].v[2];
										if(index>=0&&index<tri->mesh.numVerts)
										{
											#pragma omp critical
											{
												arrayOfVertices.Set(index);
											}
										}
									}
									if(checkTime==true)
									{
										#pragma omp critical
										{
											DialogChecker::Check(checkTime,compute,numFaces,startTime,ups);
										}
									}
								}
							}
						}
					}
				}
				if(arrayOfVertices.IsEmpty()==false) //we have overlapping faces
				{
					int localsize= arrayOfVertices.GetSize();
					for(int i=0;i<localsize;++i)
					{
						if(arrayOfVertices[i])
							val.mIndex.Append(1,&i);
					}
				}
				return returnval;
			}
			else return IGeometryChecker::eFail;
		}
		else if(os.obj->IsSubClassOf(polyObjectClassID))
		{
			PolyObject *poly = dynamic_cast<PolyObject *>(os.obj);
			if(poly)
			{
				BitArray arrayOfVertices;
				arrayOfVertices.SetSize(poly->GetMesh().numv);
				arrayOfVertices.ClearAll();
				IGeometryChecker::ReturnVal returnval=IGeometryChecker::eFail;
				int numChannels= poly->GetMesh().MNum();
				int index;
				for(int i=0;i<numChannels;++i)
				{
					if(poly->GetMesh().M(i))
					{
						MNMesh *mesh = &(poly->GetMesh());
						MNMap *mnmap = mesh->M(i);
						if(mnmap&&mnmap->numv>0&&mnmap->numf>0)
						{
							returnval= TypeReturned();
							int numFaces = mnmap->numf;
							#pragma omp parallel for
							for(int faceIndex =0;faceIndex<numFaces;++faceIndex)
							{
								if(compute)
								{
									Point3 tv;
									int a,b,c;
									MNMapFace *face = mnmap->F(faceIndex);
									UVVert *  uvVerts=  mnmap->v;
									for(int j=0;j<(face->deg);++j)
									{
										if(j==(face->deg-2))
										{
											a  = j; b = j+1; c = 0;
										}
										else if(j==(face->deg-1))
										{
											a  = j; b = 0; c = 1;
										}
										else
										{
											a  = j; b = j+1; c = j+2;
										}
										tv = uvVerts[face->tv[a]];
										if(_isnan(tv.x) || !_finite(tv.x)||_isnan(tv.y) || !_finite(tv.y)||_isnan(tv.z) || !_finite(tv.z))
										{
											index = mesh->f->vtx[a];
											if(index>=0&&index<mesh->numv)
											{
												#pragma omp critical
												{
													arrayOfVertices.Set(index);
												}
											}
										}
										tv = uvVerts[face->tv[b]];
										if(_isnan(tv.x) || !_finite(tv.x)||_isnan(tv.y) || !_finite(tv.y)||_isnan(tv.z) || !_finite(tv.z))
										{
											index = mesh->f->vtx[b];
											if(index>=0&&index<mesh->numv)
											{
												#pragma omp critical
												{
													arrayOfVertices.Set(index);
												}
											}
										}
										tv = uvVerts[face->tv[c]];
										if(_isnan(tv.x) || !_finite(tv.x)||_isnan(tv.y) || !_finite(tv.y)||_isnan(tv.z) || !_finite(tv.z))
										{
											index = mesh->f->vtx[c];
											if(index>=0&&index<mesh->numv)
											{
												#pragma omp critical
												{
													arrayOfVertices.Set(index);
												}
											}
										}
										if(checkTime==true)
										{
											#pragma omp critical
											{
												DialogChecker::Check(checkTime,compute,numFaces,startTime,ups);
											}
										}
									}
								}
							}	
						}
					}
				}
				if(arrayOfVertices.IsEmpty()==false) //we have overlapping faces
				{
					int localsize= arrayOfVertices.GetSize();
					for(int i=0;i<localsize;++i)
					{
						if(arrayOfVertices[i])
							val.mIndex.Append(1,&i);
					}
				}
				return returnval;
			}
			else return IGeometryChecker::eFail;
		}
	}
	return IGeometryChecker::eFail;
}

MSTR MissingUVCoordinatesChecker::GetName()
{
	return MSTR(_T(GetString(IDS_MISSING_UV_COORDINATES)));
}


