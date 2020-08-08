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
// DESCRIPTION: Flipped UVW Faces checker
// AUTHOR: Michael Zyracki created 2008
//***************************************************************************/
#include<3dsmaxport.h>
#include "checkerimplementations.h"
#include <omp.h>
FlippedUVWFacesChecker FlippedUVWFacesChecker::_theInstance(FLIPPED_UVW_FACES_INTERFACE, _T("FlippedUVWFaces"), 0, NULL, FP_CORE,
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
class FlippedUVWFacesClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return TRUE; }
	void *			Create(BOOL loading = FALSE) {return &FlippedUVWFacesChecker::_theInstance; }
	const TCHAR *	ClassName() { return GetString(IDS_FLIPPED_UVW_FACES); }
	SClass_ID		SuperClassID() { return GUP_CLASS_ID; }
	Class_ID		ClassID() { return FLIPPED_UVW_FACES_CLASS_ID; }
	const TCHAR* 	Category() { return _T(""); }

	const TCHAR*	InternalName() { return _T("FlippedUVWFacesClass"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle

};

static FlippedUVWFacesClassDesc classDesc;
ClassDesc2* GetFlippedUVWFacesClassDesc() { return &classDesc; }


bool FlippedUVWFacesChecker::IsSupported(INode* node)
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
IGeometryChecker::ReturnVal FlippedUVWFacesChecker::TypeReturned()
{
	return IGeometryChecker::eFaces;
}
IGeometryChecker::ReturnVal FlippedUVWFacesChecker::GeometryCheck(TimeValue t,INode *nodeToCheck, IGeometryChecker::OutputVal &val)
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
				BitArray arrayOfFaces;
				arrayOfFaces.SetSize(tri->mesh.numFaces);
				arrayOfFaces.ClearAll();
				IGeometryChecker::ReturnVal returnval=IGeometryChecker::eFail;
				int numChannels = tri->mesh.getNumMaps();
				for(int i=0;i<numChannels;++i)
				{
					if(tri->mesh.mapSupport(i))
					{
						returnval = TypeReturned();
						MeshMap *map  = &tri->mesh.Map(i);
						if(map->getNumVerts()>0 &&map->getNumFaces()>0)
						{
							int numFaces = map->getNumFaces();
							TVFace * tvFaces = map->tf;
							UVVert *  uvVerts= map->tv;
							#pragma omp parallel for
							for(int faceIndex =0;faceIndex<numFaces;++faceIndex)
							{
								if(compute)
								{
									Point3 tv[3];
									TVFace& tvFace = tvFaces[faceIndex];
									tv[0] = uvVerts[tvFace.t[0]];
									tv[1] = uvVerts[tvFace.t[1]];
									tv[2] = uvVerts[tvFace.t[2]];
									Point3 mapNormal = FNormalize( (tv[1] - tv[0]) ^ (tv[2] - tv[1]) );
									if( mapNormal.z<0 )
									{
										#pragma omp critical
										{
											arrayOfFaces.Set(faceIndex);
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
				if(arrayOfFaces.IsEmpty()==false) //we have overlapping faces
				{
					int localsize= arrayOfFaces.GetSize();
					for(int i=0;i<localsize;++i)
					{
						if(arrayOfFaces[i])
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
				BitArray arrayOfFaces;
				arrayOfFaces.SetSize(poly->GetMesh().numf);
				arrayOfFaces.ClearAll();
				IGeometryChecker::ReturnVal returnval=IGeometryChecker::eFail;
				int numChannels= poly->GetMesh().MNum();//do this!
				for(int i=0;i<numChannels;++i)
				{
					if(poly->GetMesh().M(i))
					{
						MNMesh *mesh = &(poly->GetMesh());
						MNMap *mnmap = mesh->M(i);
						if(mnmap&&mnmap->numv>0&&mnmap->numf>0)
						{
							returnval = TypeReturned();
							int numFaces = mnmap->numf;
							#pragma omp parallel for
							for(int faceIndex =0;faceIndex<numFaces;++faceIndex)
							{
								if(compute)
								{
									MNMapFace *face = mnmap->F(faceIndex);
									UVVert *  uvVerts=  mnmap->v;
									Point3 tv[3];
									int a,b,c;							
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
										tv[0] = uvVerts[face->tv[a]];
										tv[1] = uvVerts[face->tv[b]];
										tv[2] = uvVerts[face->tv[c]];
										Point3 mapNormal = FNormalize( (tv[1] - tv[0]) ^ (tv[2] - tv[1]) );
										if( mapNormal.z<0 )
										{
											arrayOfFaces.Set(faceIndex);
											break;  //break out since for this one set in the poly we have a flipped face.
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
				if(arrayOfFaces.IsEmpty()==false) //we have overlapping faces
				{
					int localsize= arrayOfFaces.GetSize();
					for(int i=0;i<localsize;++i)
					{
						if(arrayOfFaces[i])
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

MSTR FlippedUVWFacesChecker::GetName()
{
	return MSTR(_T(GetString(IDS_FLIPPED_UVW_FACES)));
}
