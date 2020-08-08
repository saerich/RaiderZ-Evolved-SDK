//**************************************************************************/
// Copyright (c) 1998-2006 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//**************************************************************************/
// DESCRIPTION: These are uv data
// AUTHOR: Peter Watje
// DATE: 2006/10/07 
//***************************************************************************/

#ifndef __TVDATA__H
#define __TVDATA__H


class UnwrapMod;

//PELT
class UVWHitData
{
public:
	int index;
	float dist;
};

class UVW_TVFaceOldClass
{
public:
int t[4];
int FaceIndex;
int MatID;
int flags;
Point3 pt[4];
};
//this is the Max9 format we need to keep it to load older files
class UVW_TVVertClass_Max9
{
public:
	 UVW_TVVertClass_Max9() 
	{
		p = Point3(0.0f,0.0f,0.0f);
		flags = 0;
		influence = 0.0f;
	}

	Point3 p;
	float influence;	
	BYTE flags;
};

class UVW_TVVectorClass
{
public:
//index into the texture list
	int handles[8];
	int interiors[4];
//index into the geometric list
	int vhandles[8];
	int vinteriors[4];

	UVW_TVVectorClass()
		{
		for (int i = 0; i < 4; i++)
			{
			interiors[i] = -1;
			vinteriors[i] = -1;
			}
		for (int i = 0; i < 8; i++)
			{
			handles[i] = -1;
			vhandles[i] = -1;
			}

		}


};


class UVW_TVFaceClass
{
public:

//index into the texture vertlist
	int *t;
	int FaceIndex;
	int MatID;
	int flags;
//count of number of vertex currently can only 3 or 4 but with poly objects this will change
	int count;
//index into the geometric vertlist
	int *v;
	UVW_TVVectorClass *vecs;

	UVW_TVFaceClass()
		{
		count = 3;
		flags = 0;
		vecs = NULL;
		t = NULL;
		v = NULL;
		}
	~UVW_TVFaceClass()
		{
		if (vecs) delete vecs;
		vecs = NULL;
		if (t) delete [] t;
		t = NULL;
		if (v) delete [] v;
		t = NULL;

		}
	UVW_TVFaceClass* Clone();
	void DeleteVec();
	int FindGeomEdge(int a, int b);
	int FindUVEdge(int a, int b);

	ULONG SaveFace(ISave *isave);
	ULONG SaveFace(FILE *file);


	ULONG LoadFace(ILoad *iload);
	ULONG LoadFace(FILE *file);

	void GetConnectedUVVerts(int a, int &v1, int &v2);
	void GetConnectedGeoVerts(int a, int &v1, int &v2);
	int GetGeoVertIDFromUVVertID(int uvID);

};



//need a table of TVert pointers
class UVW_TVVertClass
{
public:
	UVW_TVVertClass() 
	{
		p = Point3(0.0f,0.0f,0.0f);
		flags = 0;
		influence = 0.0f;
		mControllerID = -1;
	}

	Point3 GetP() {return p;}
	void SetP(Point3 pos ) { p = pos; }

	float GetInfluence() {return influence;}
	void  SetInfluence(float v) { influence = v; }
	
	BYTE GetFlag() {return flags; }
	void SetFlag(BYTE flg) {flags = flg; }
	void OrFlag(BYTE flg) {flags |= flg; }
	void AnfFlag(BYTE flg) {flags &= flg; }

	BOOL IsDead() { return flags & FLAG_DEAD;}
	void SetDead() { flags |= FLAG_DEAD;}
	void SetAlive() { flags &= ~FLAG_DEAD;}


	BOOL IsHidden() { return flags & FLAG_HIDDEN;}
	void Hide() { flags |= FLAG_HIDDEN;}
	void Show() { flags &= ~FLAG_HIDDEN;}


	BOOL IsFrozen() { return flags & FLAG_FROZEN;}
	void Freeze() { flags |= FLAG_FROZEN;}
	void Unfreeze() { flags &= ~FLAG_FROZEN;}

	int GetControlID() { return mControllerID; }
	void SetControlID(int id) { mControllerID = id; }

	BOOL IsWeightModified() { return flags & FLAG_WEIGHTMODIFIED;}
	void SetWeightModified() { flags |= FLAG_WEIGHTMODIFIED;}
	void SetWeightUnModified() { flags &= ~FLAG_WEIGHTMODIFIED;}

	BOOL IsRigPoint() { return flags & FLAG_RIGPOINT;}
	void SetRigPoint(BOOL val)
	{ 
		if (val)
			flags |= FLAG_RIGPOINT;
		else
			flags &= ~FLAG_RIGPOINT;

	}


private:
	Point3 p;
	float influence;
	int  mControllerID;
	BYTE flags;
};

//need a table of edgs
class UVW_TVEdgeDataClass
{
public:
//indices into the vert list
	int a,avec;
	int b,bvec;
	int flags;
	int lookupIndex;
	
	int ga,gb;
	
	Tab<int> faceList;

	UVW_TVEdgeDataClass()
		{
			ga = -1;
			gb = -1;
		}
	~UVW_TVEdgeDataClass()
		{
		}
};

class UVW_TVEdgeClass
	{
public:
	Tab<UVW_TVEdgeDataClass*> data;
	BYTE flags;
	};

//Pelt
class EData
{
public:
	int edgeIndex;
	int vertexIndex;
};

class AdjacentItem
{
public:
	Tab<int> index;
};

class VConnections
{
public:
	VConnections *closestNode;          //parent for th dystraka algorithm
	float accumDist;
	BOOL solved;
	int vid;
	Tab<EData> connectingVerts;
	//these is used for our priority list for the unsolved nodes
	VConnections *linkedListChild; //child for the linked list
	VConnections *linkedListParent; //parent for the linked list
};

/***************************************************************************************************
This class just contains a list of all controllers for the Unwrap, this may span several local
mod data since we store the controlles in the modifier
****************************************************************************************************/
class UVW_VertexControllerClass
{
public:
	Tab<Control*> cont;	//just a table of point3 controllers, the local mod data holds indices into this list
};

/***************************************************************************************************
This class contains all our UVWs and geometric information we need to generate the UVWs data.  It is 
mish/mash of mesh/poly/patch data since we support all 3 types.
****************************************************************************************************/
class UVW_ChannelClass
{
public:
	//Destructor / Destructor
	UVW_ChannelClass();
	~UVW_ChannelClass();

	int channel;

	Tab<UVW_TVVertClass> v;
	Tab<UVW_TVFaceClass*> f;

	Tab<UVW_TVEdgeClass*> e;
	Tab<UVW_TVEdgeDataClass*> ePtrList;
	
	//PELT
	//geo edge data
	Tab<UVW_TVEdgeClass*> ge;				
	Tab<UVW_TVEdgeDataClass*> gePtrList;


		
	Tab<Point3> geomPoints;

	//these 2 methods are just for backwards compatibility to max9 from max9.5
	ULONG SaveFacesMax9(ISave *isave);
	ULONG LoadFacesMax9(ILoad *iload);


	ULONG LoadFaces(ILoad *iload);
	ULONG LoadFaces(FILE *file);

	ULONG SaveFaces(ISave *isave);
	ULONG SaveFaces(FILE *file);


	void SetCountFaces(int newct);

	void CloneFaces(Tab<UVW_TVFaceClass*> &t);
	void AssignFaces(Tab<UVW_TVFaceClass*> &t);
	void FreeFaces();
	void Dump();

	void MarkDeadVertices();

	BOOL edgesValid;
	BOOL mGeoEdgesValid;
	void FreeEdges();
	void BuildEdges();

	void FreeGeomEdges();
	void BuildGeomEdges();
	

	void AppendEdge(Tab<UVW_TVEdgeClass*> &e,int index1,int vec1, int index2,int vec2, int face, BOOL hidden, int gva=-1, int gvb=-1);
	void EdgeListFromPoints(Tab<int> &selEdges, int a, int b, Point3 vec);
	int GetNextEdge(int currentEdgeIndex, int cornerVert, int currentFace);
	void SplitUVEdges(BitArray esel);
	//give 2 geo points find the geo edge for them
	//returns -1 if no valid edge found
	int FindGeoEdge(int a, int b);

	//give 2 uv points find the uv edge for them
	//returns -1 if no valid edge found
	int FindUVEdge(int a, int b);

	float LineToPoint(Point3 p1, Point3 l1, Point3 l2);
	int EdgeIntersect(Point3 p, float threshold, int i1, int i2);

	float edgeScale;

	void BuildAdjacentUVEdgesToVerts(Tab<AdjacentItem*> &verts);
	void BuildAdjacentGeomEdgesToVerts(Tab<AdjacentItem*> &verts);
	
	void BuildAdjacentUVFacesToVerts(Tab<AdjacentItem*> &verts);

	Point3 UVFaceNormal(int index);
	Point3 GeomFaceNormal(int index);

	Matrix3 MatrixFromGeoFace(int index);
	Matrix3 MatrixFromUVFace(int index);

	void LoadOlderVersions(ILoad *iload);
	void ResolveOldData(UVW_ChannelClass &oldData);

	//this is a list of all our system locked vertices, we cannot change these vertices
	//these are typically when you have a selection flowing up the stack, we cannot change
	//any of the vertices that are not selected.  So we set this flag for those verts
	BitArray mSystemLockedFlag;

};


class VertexLookUpDataClass
{
public:
int index;
int newindex;
Point3 p;

};
class VertexLookUpListClass
{
public:
BitArray sel;
Tab<VertexLookUpDataClass> d;
void addPoint(int a_index, Point3 a);
};

class VEdges
{
public:
	Tab<int> edgeIndex;
};


class GTVData
{
public:
	int gIndex;
	int tIndex;
};

class GeomToTVEdges
{
public:
	Tab<GTVData> edgeInfo;
};

class EdgeVertInfo
{
public:
	int faceIndex;

	int uvIndexP;
	int geoIndexP;

	int uvIndexA;
	int geoIndexA;


	int uvIndexN;
	int geoIndexN;

	float angle;

	Point3 idealPos;
	Point3 idealPosNormalized;
	Point3 tempPos;
	Point3 tempPos2;
	
	
};

class ClusterInfo
{
public:
	int uvCenterIndex;
	int geoCenterIndex;
	float angle;	
	float counterAngle;	
	Point3 delta;
	Tab<EdgeVertInfo> edgeVerts;
};

class MyPoint3Tab
{
public:
	Tab<Point3> p;
};


class BXPInterpData
{
public:
	BXPInterpData()
	{
		x = 0.0f;
		y = 0.0f;
		
		pos = Point3(0.0f,0.0f,0.0f);
		color = Point3(0.0f,0.0f,0.0f);
		normal = Point3(0.0f,0.0f,0.0f);

	}
	float x,y;
	Point3 pos;
	Point3 color;
	Point3 normal;
	int IntX()
	{
		int ix0 = x;
		if (x - floor(x) >= 0.5f) ix0 += 1;
		return ix0;
	}
	int IntY()
	{
		int ix0 = y;
		if (y - floor(y) >= 0.5f) ix0 += 1;
		return ix0;
	}


	BXPInterpData operator-(const BXPInterpData& v) 
	const 
	{
		BXPInterpData temp;
		temp.x = x-v.x;
		temp.y = x-v.y;
		temp.pos = pos-v.pos;
		temp.color = color-v.color;
		temp.normal = normal-v.normal;
		return temp;
	}

	BXPInterpData operator+(const BXPInterpData& v) 
	const 
	{
		BXPInterpData temp;
		temp.x = x+v.x;
		temp.y = x+v.y;
		temp.pos = pos+v.pos;
		temp.color = color+v.color;
		temp.normal = normal+v.normal;
		return temp;
	}

	BXPInterpData operator/(const BXPInterpData& v) 
	const 
	{
		BXPInterpData temp;
		temp.x = x/v.x;
		temp.y = x/v.y;
		temp.pos = pos/v.pos;
		temp.color = color/v.color;
		temp.normal = normal/v.normal;
		return temp;
	}

	BXPInterpData operator/(const float &v) 
	const 
	{
		BXPInterpData temp;
		if (v != 0.0f)
		{
			temp.x = x/v;
			temp.y = x/v;
			temp.pos = pos/v;
			temp.color = color/v;
			temp.normal = normal/v;
		}
		else
		{
			temp.x = x;
			temp.y = x;
			temp.pos = pos;
			temp.color = color;
			temp.normal = normal;
		}
		return temp;
	}

};


#endif // __UWNRAP__H
