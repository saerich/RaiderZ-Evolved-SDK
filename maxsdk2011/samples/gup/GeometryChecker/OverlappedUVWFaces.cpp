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
// DESCRIPTION:Overlapped UVW Faces Checker and supporting classes
// AUTHOR: Michael Zyracki created 2008
//***************************************************************************/
#include<3dsmaxport.h>
#include "checkerimplementations.h"
#include <omp.h>

//uvw channel supporting class
void UVWChannel::Face::DeleteVerts()
{
	if(mTVVerts) {delete []mTVVerts;} 
	mTVVerts = NULL; 
	mNumVerts = 0;
}
void UVWChannel::Face::AllocateVerts(int degree)
{
	DeleteVerts();
	mNumVerts = degree;
	if(degree>0) 
		mTVVerts = new int[degree];
} 



void UVWChannel::SetWithMesh(Mesh *mesh,int channel)
{
	MeshMap *map  = &mesh->Map(channel);
	if(map->getNumVerts()>0 &&map->getNumFaces()>0)
	{
		int i;
		mNumVerts = map->getNumVerts();
		mVerts = new UVVert[mNumVerts];
		for(i=0;i<mNumVerts;++i)
		{
			mVerts[i] = map->tv[i];
		}

		mNumFaces = map->getNumFaces();
		mFaces = new UVWChannel::Face[mNumFaces];
		TVFace * faces = map->tf;
		for(i=0;i<mNumFaces;++i)
		{
			mFaces[i].AllocateVerts(3);
			for(int j=0;j<mFaces[i].mNumVerts;++j)
			{
				mFaces[i].mTVVerts[j] = faces[i].getTVert(j);
			}
		}
	}
}
void UVWChannel::SetWithMesh(MNMesh *mesh,int channel)
{
	MNMap *mnmap = mesh->M(channel);
	if(mnmap&&mnmap->numv>0&&mnmap->numf>0)
	{
		int i;
		mNumVerts = mnmap->numv;
		mVerts = new UVVert[mNumVerts];
		for(i=0;i<mNumVerts;++i)
		{
			mVerts[i] = mnmap->v[i];
		}		
		
		mNumFaces = mnmap->numf;
		mFaces = new UVWChannel::Face[mNumFaces];
		for(i=0;i<mNumFaces;++i)
		{
			MNMapFace *face = mnmap->F(i);
			mFaces[i].AllocateVerts(face->deg);
			for(int j=0;j<mFaces[i].mNumVerts;++j)
			{
				mFaces[i].mTVVerts[j] = face->tv[j];
			}
		}
	}
}

void UVWChannel::DeleteData()
{
	if(mVerts)
		delete []mVerts;
	mVerts = NULL;
	mNumVerts = 0;
	if(mFaces)
		delete []mFaces;
	mFaces=  NULL;
	mNumFaces =0;
}

int UVWChannel::GetNumTVVerts()
{
	return mNumVerts;
}
Point3 UVWChannel::GetTVVert(int tvvert)
{
	if(tvvert>-1&&tvvert<mNumVerts)
		return mVerts[tvvert];
	DbgAssert(1);
	return Point3(0,0,0);
}
int UVWChannel::GetNumOfFaces()
{
	return mNumFaces;
}
int UVWChannel::GetFaceDegree(int face)
{
	if(face>-1&&face<mNumFaces)
	{
		return mFaces[face].mNumVerts;
	}
	DbgAssert(1);
	return 3;
}
int UVWChannel::GetFaceTVVert(int face, int index) //return tvert index
{
	if(face>-1&&face<mNumFaces)
	{
		return mFaces[face].mTVVerts[index];
	}
	DbgAssert(1);
	return 0;
}
//this is just a single element/pixel of our map that we render into
class OverlapCell
{
public:

	int mFaceID;		//the face that rendered into this pixel
};

//similar to what's in uvwunwrap
//this is a map that we use to render our faces into to see which ones overlap
class OverlapMap
{
public:
	//intitializes our buffer and clears it out
	void Init()
	{
		mBufferSize = 2000;
		mBuffer = new OverlapCell[mBufferSize*mBufferSize];
		//clear the buffer
		for (int i = 0; i < mBufferSize*mBufferSize; i++)
		{
			mBuffer[i].mFaceID = -1;
		}
	}
	~OverlapMap()
	{
		delete [] mBuffer;
	}

	//this renders a face into our buffer and selects the faces that intersect if there are any
	//	faceIndex - the index of the face we want to render/check
	//  pa,pb,pc - this is the normalized UVW coords of the face that we want to check
	void Map(UVWChannel &uvmesh,int faceIndex, Point3 pa, Point3 pb, Point3 pc,BitArray &selection);

private:
	//this see if this pixel intersects and faces in the buffer
	//if so it select the face in the buffer and the incoming face
	//	mapIndex is the index into the buffer
	//	ld is the local data of the face we are checking
	//	faceIndex is the index of the face we are cheking
	void Hit(UVWChannel &uvmesh,int mapIndex, int faceIndex,BitArray &election);

	OverlapCell *mBuffer;	//our map buffer
	int mBufferSize;		//the width/height of the buffer

};



void OverlapMap::Hit(UVWChannel &uvMesh,int mapIndex, int faceIndex,BitArray &selection)
{
	BOOL hit = TRUE;

	if (mBuffer[mapIndex].mFaceID == -1)
		hit = FALSE;

	//if nothing in this cell just add it
	if (!hit)
	{
		mBuffer[mapIndex].mFaceID = faceIndex;
	}
	else
	{
		//have somethign in the cell need to check
		//get the ld and face id in the cell
		int baseFaceIndex = mBuffer[mapIndex].mFaceID;
		//hit on the same mesh id cannot be the same
		if ((baseFaceIndex != faceIndex))
		{
			selection.Set(faceIndex);
			selection.Set(baseFaceIndex);
		}
		//hit on different mesh dont care about the face ids
	}
}

void OverlapMap::Map(UVWChannel &uvMesh, int faceIndex, Point3 pa, Point3 pb, Point3 pc,BitArray &selection)
{
	pa *= mBufferSize;
	pb *= mBufferSize;
	pc *= mBufferSize;

	long x1 = (long) pa.x;
	long y1 = (long) pa.y;
	long x2 = (long) pb.x;
	long y2 = (long) pb.y;
	long x3 = (long) pc.x;
	long y3 = (long) pc.y;

	//sort top to bottom
	int sx[3], sy[3];
	sx[0] = x1;
	sy[0] = y1;

	if (y2 < sy[0])
	{
		sx[0] = x2;
		sy[0] = y2;

		sx[1] = x1;
		sy[1] = y1;
	}
	else 
	{
		sx[1] = x2;
		sy[1] = y2;
	}

	if (y3 < sy[0])
	{
		sx[2] = sx[1];
		sy[2] = sy[1];

		sx[1] = sx[0];
		sy[1] = sy[0];


		sx[0] = x3;
		sy[0] = y3;
	}
	else if (y3 < sy[1])
	{
		sx[2] = sx[1];
		sy[2] = sy[1];

		sx[1] = x3;
		sy[1] = y3;
	}
	else
	{
		sx[2] = x3;
		sy[2] = y3;
	}

	int hit = 0;

	int width = mBufferSize;
	int height = mBufferSize;
	//if flat top
	if (sy[0] == sy[1])
	{

		float xInc0 = 0.0f;
		float xInc1 = 0.0f;
		float yDist = DL_abs(sy[0] - sy[2]);
		float xDist0 = sx[2] - sx[0];
		float xDist1 = sx[2] - sx[1];
		xInc0 = xDist0/yDist;		
		xInc1 = xDist1/yDist;

		float cx0 = sx[0];
		float cx1 = sx[1];
		for (int i = sy[0]; i < sy[2]; i++)
		{

			int ix0 = cx0;
			int ix1 = cx1;

			if (cx0 - floor(cx0) >= 0.5f) ix0 += 1;
			if (cx1 - floor(cx1) >= 0.5f) ix1 += 1;


			//line 
			if (ix0 > ix1)
			{
				int temp = ix0;
				ix0 = ix1;
				ix1 = temp;
			}
			for (int j = ix0; j <= ix1; j++)
			{
				int index = j + i * width;
				if ((j >= 0) && (j <width) && (i >= 0) && (i < height))
				{

					if ((j > (ix0+1)) && (j < (ix1-1)))
					{
						Hit(uvMesh,index,  faceIndex,selection)	;					
					}	
					else
					{
						mBuffer[index].mFaceID = faceIndex;
					}
				}

			}
			cx0 += xInc0;
			cx1 += xInc1;
		}

	}
	//it flat bottom
	else if (sy[1] == sy[2])
	{

		float xInc0 = 0.0f;
		float xInc1 = 0.0f;
		float yDist = DL_abs(sy[0] - sy[2]);
		float xDist0 = sx[1]- sx[0];
		float xDist1 = sx[2] - sx[0];
		xInc0 = xDist0/yDist;		
		xInc1 = xDist1/yDist;

		float cx0 = sx[0];
		float cx1 = sx[0];
		for (int i = sy[0]; i < sy[2]; i++)
		{

			int ix0 = cx0;
			int ix1 = cx1;

			if (cx0 - floor(cx0) >= 0.5f) ix0 += 1;
			if (cx1 - floor(cx1) >= 0.5f) ix1 += 1;

			//line 
			if (ix0 > ix1)
			{
				int temp = ix0;
				ix0 = ix1;
				ix1 = temp;
			}
			for (int j = ix0; j <= ix1; j++)
			{

				int index = j + i * width;
				if ((j >= 0) && (j <width) && (i >= 0) && (i < height))
				{
					if ((j > (ix0+1)) && (j < (ix1-1)))
					{
						Hit(uvMesh,index, faceIndex,selection);						
					}
					else
					{
						mBuffer[index].mFaceID = faceIndex;
					}
					//						processedPixels.Set(index,TRUE);
				}
			}
			cx0 += xInc0;
			cx1 += xInc1;
		}		

	}
	else
	{


		float xInc1 = 0.0f;
		float xInc2 = 0.0f;
		float xInc3 = 0.0f;
		float yDist0to1 = DL_abs(sy[1] - sy[0]);
		float yDist0to2 = DL_abs(sy[2] - sy[0]);
		float yDist1to2 = DL_abs(sy[2] - sy[1]);

		float xDist1 = sx[1]- sx[0];
		float xDist2 = sx[2] - sx[0];
		float xDist3 = sx[2] - sx[1];
		xInc1 = xDist1/yDist0to1;		
		xInc2 = xDist2/yDist0to2;
		xInc3 = xDist3/yDist1to2;

		float cx0 = sx[0];
		float cx1 = sx[0];
		//go from s[0] to s[1]
		for (int i = sy[0]; i < sy[1]; i++)
		{

			int ix0 = cx0;
			int ix1 = cx1;

			if (cx0 - floor(cx0) >= 0.5f) ix0 += 1;
			if (cx1 - floor(cx1) >= 0.5f) ix1 += 1;

			//line 
			if (ix0 > ix1)
			{
				int temp = ix0;
				ix0 = ix1;
				ix1 = temp;
			}
			for (int j = ix0; j <= ix1; j++)
			{

				int index = j + i * width;
				if ((j >= 0) && (j <width) && (i >= 0) && (i < height))
				{
					if ((j > (ix0+1)) && (j < (ix1-1)))
					{
						Hit(uvMesh,index, faceIndex,selection);						
					}
					else
					{
						mBuffer[index].mFaceID = faceIndex;
					}
				}

			}
			cx0 += xInc1;
			cx1 += xInc2;
		}	
		//go from s[1] to s[2]
		for (int i = sy[1]; i < sy[2]; i++)
		{

			int ix0 = cx0;
			int ix1 = cx1;

			if (cx0 - floor(cx0) >= 0.5f) ix0 += 1;
			if (cx1 - floor(cx1) >= 0.5f) ix1 += 1;

			//line 
			if (ix0 > ix1)
			{
				int temp = ix0;
				ix0 = ix1;
				ix1 = temp;
			}
			for (int j = ix0; j <= ix1; j++)
			{

				int index = j + i * width;
				if ((j >= 0) && (j <width) && (i >= 0) && (i < height))
				{
					if ( (j > (ix0+1)) && (j < (ix1-1)))
					{
						Hit(uvMesh,index, faceIndex,selection);						
					}
					else
					{
						mBuffer[index].mFaceID = faceIndex;
					}
				}

			}
			cx0 += xInc3;
			cx1 += xInc2;
		}	

	}
}
OverlappedUVWFacesChecker OverlappedUVWFacesChecker::_theInstance(OVERLAPPED_UVW_FACES_INTERFACE, _T("OverlappedUVWFaces"), 0, NULL, FP_CORE,
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


class OverlappedUVWFacesClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return TRUE; }
	void *			Create(BOOL loading = FALSE) {return &OverlappedUVWFacesChecker::_theInstance; }
	const TCHAR *	ClassName() { return GetString(IDS_OVERLAPPED_UVW_FACES); }
	SClass_ID		SuperClassID() { return GUP_CLASS_ID; }
	Class_ID		ClassID() { return OVERLAPPED_UVW_FACES_CLASS_ID; }
	const TCHAR* 	Category() { return _T(""); }

	const TCHAR*	InternalName() { return _T("OverlappedUVWFacesClass"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle

};

static OverlappedUVWFacesClassDesc classDesc;
ClassDesc2* GetOverlappedUVWFacesClassDesc() { return &classDesc; }

bool OverlappedUVWFacesChecker::IsSupported(INode* node)
{
	if(node)
	{
		ObjectState os  = node->EvalWorldState(GetCOREInterface()->GetTime());
		Object *obj = os.obj;
		if(obj&&obj&&obj->IsSubClassOf(triObjectClassID)||
			obj->IsSubClassOf(polyObjectClassID))
			return true;
	}
	return false;
}
IGeometryChecker::ReturnVal OverlappedUVWFacesChecker::TypeReturned()
{
	return IGeometryChecker::eFaces;
}


IGeometryChecker::ReturnVal OverlappedUVWFacesChecker::GeometryCheckWithUVMesh(UVWChannel &uvmesh,TimeValue t,INode *nodeToCheck, BitArray &arrayOfFaces)
{
	LARGE_INTEGER	ups,startTime;
	QueryPerformanceFrequency(&ups);
	QueryPerformanceCounter(&startTime); //used to see if we need to pop up a dialog 
	bool checkTime = GetIGeometryCheckerManager()->GetAutoUpdate();//only check time for the dialog if auto update is active!
	//get our bounding box
	Box3 bounds;
	bounds.Init();

	int numVerts = uvmesh.GetNumTVVerts();
	int i;
	for (i = 0; i < numVerts; ++i)
	{
		Point3 tv = uvmesh.GetTVVert(i);
		if(_isnan(tv.x) || !_finite(tv.x)||_isnan(tv.y) || !_finite(tv.y)||_isnan(tv.z) || !_finite(tv.z))
			return IGeometryChecker::eFail; //if we have a bad tvert bail out...
		bounds += uvmesh.GetTVVert(i);
	}
	//put a small fudge to keep faces off the edge
	bounds.EnlargeBy(0.05f);
	//build our transform

	float xScale = bounds.pmax.x - bounds.pmin.x;
	float yScale = bounds.pmax.y - bounds.pmin.y;
	Point3 offset = bounds.pmin;

	//create our buffer
	OverlapMap overlapMap;
	overlapMap.Init();
	
	Interface *ip = GetCOREInterface();
	int numProcsToUse = omp_get_num_procs()-1;
	if(numProcsToUse<0)
		numProcsToUse = 1;
	TCHAR buf[256];
	TCHAR tmpBuf[64];
	_tcscpy(tmpBuf,GetString(IDS_CHECK_TIME_USED));
	_tcscpy(buf,GetString(IDS_RUNNING_GEOMETRY_CHECKER));
	ip->PushPrompt(buf);
	int total = 0;
	bool compute = true;
	EscapeChecker::theEscapeChecker.Setup();
	for (int i = 0; i < uvmesh.GetNumOfFaces(); ++i)
	{
		if(compute==true)
		{
			//loop through the faces
			int deg = uvmesh.GetFaceDegree(i);
			int index[4];
			index[0] = uvmesh.GetFaceTVVert(i,0);
			Point3 pa = uvmesh.GetTVVert(index[0]);
			pa.x -= offset.x;
			pa.x /= xScale;

			pa.y -= offset.y;
			pa.y /= yScale;

			for (int j = 0; j < deg -2; j++)
			{
				index[1] = uvmesh.GetFaceTVVert(i,j+1);
				index[2] = uvmesh.GetFaceTVVert(i,j+2);
				Point3 pb = uvmesh.GetTVVert(index[1]);
				Point3 pc = uvmesh.GetTVVert(index[2]);

				pb.x -= offset.x;
				pb.x /= xScale;
				pb.y -= offset.y;
				pb.y /= yScale;

				pc.x -= offset.x;
				pc.x /= xScale;
				pc.y -= offset.y;
				pc.y /= yScale;

				//add face to buffer
				//select anything that overlaps
				overlapMap.Map(uvmesh,i,pa,pb,pc,arrayOfFaces);
			}
		}
		total += 1;
		
		if(compute==true)
		{
			if(EscapeChecker::theEscapeChecker.Check())
			{
				compute = false;
				_tcscpy(buf,GetString(IDS_EXITING_GEOMETRY_CHECKER));
				ip->ReplacePrompt(buf);
				break; //can now break, no omp
			}
		
			if((total%100)==0)
			{
				{
					float percent = (float)total/(float)uvmesh.GetNumOfFaces()*100.0f;
					_stprintf(buf,tmpBuf,percent);
					ip->ReplacePrompt(buf);
				}     
			}
			if(checkTime==true)
			{
				DialogChecker::Check(checkTime,compute,uvmesh.GetNumOfFaces(),startTime,ups);
				if(compute==false)
					break;
			}
		}
		
	}
	ip->PopPrompt();
	

	return TypeReturned();

}

IGeometryChecker::ReturnVal OverlappedUVWFacesChecker::GeometryCheck(TimeValue t,INode *nodeToCheck, IGeometryChecker::OutputVal &val)
{
	val.mIndex.ZeroCount();
	if(IsSupported(nodeToCheck))
	{
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
						uvmesh.DeleteData();
						uvmesh.SetWithMesh(&tri->mesh,i);
						if(uvmesh.GetNumOfFaces()>1)
						{
							//okay now run it with that uvmesh
							returnval = GeometryCheckWithUVMesh(uvmesh,t, nodeToCheck,arrayOfFaces);
							if(returnval ==IGeometryChecker::eFail)
								return returnval;
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
						uvmesh.DeleteData();
						uvmesh.SetWithMesh(&poly->GetMesh(),i);
						if(uvmesh.GetNumOfFaces()>1)
						{
							//okay now run it with that uvmesh
							returnval = GeometryCheckWithUVMesh(uvmesh,t, nodeToCheck,arrayOfFaces);
							if(returnval ==IGeometryChecker::eFail)
								return returnval;
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

MSTR OverlappedUVWFacesChecker::GetName()
{
	return MSTR(_T(GetString(IDS_OVERLAPPED_UVW_FACES)));
}

