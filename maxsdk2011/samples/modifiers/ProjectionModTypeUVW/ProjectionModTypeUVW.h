/**********************************************************************
 *<
	FILE:		    ProjectionModTypeUVW.h
	DESCRIPTION:	Projection Modifier Type UVW Header
	CREATED BY:		Michael Russo
	HISTORY:		Created 03-23-2004

 *>	Copyright (c) 2004 Discreet, All Rights Reserved.
 **********************************************************************/

#ifndef __PROJECTIONMODTYPEUVW__H
#define __PROJECTIONMODTYPEUVW__H

#include "DllEntry.h"

#include "IProjectionMod.h"
#include "ObjectWrapper.h"
#include "ProjectionHolderUVW.h"

#define PROJECTIONMODTYPEUVW_CLASS_ID	Class_ID(0x56bb7872, 0x2d4c74ef)
#define PBLOCK_REF 0

class ProjectionState;

//=============================================================================
//
//	Class ProjectionModTypeUVW
//
//=============================================================================
class ProjectionModTypeUVW : public IProjectionModType
{
	protected:
		static Interface*	mpIP;
		Interval			mValid;
		IParamBlock2		*mpPBlock;

		bool				mbEnabled;
		bool				mbEditing;
		TSTR				mName;
		int					miIndex;
		HWND				mhPanel;
		bool				mbSuspendPanelUpdate;
		bool				mbAlwaysUpdate;
		int					miSourceMapChannel;
		int					miTargetMapChannel;
		bool				mbInModifyObject;

		ISpinnerControl*	mSpinnerSourceMapChannel;
		ISpinnerControl*	mSpinnerTargetMapChannel;

	public:
		ProjectionModTypeUVW(BOOL create);
		~ProjectionModTypeUVW();

		IProjectionMod*		mpPMod;

		const TCHAR *	ClassName() { return GetString(IDS_PROJECTIONMODTYPEUVW_CLASS_NAME); }
		SClass_ID		SuperClassID() { return REF_TARGET_CLASS_ID; }
		Class_ID ClassID() { return PROJECTIONMODTYPEUVW_CLASS_ID;}

		//ParamBlock params
		enum { pb_params };
		enum {	pb_name, 
				pb_holderName, pb_holderAlwaysUpdate, pb_holderCreateNew,
				pb_sourceMapChannel,
				pb_targetSameAsSource, pb_targetMapChannel, 
				pb_projectMaterialIDs, pb_sameTopology, 
				pb_ignoreBackfacing,	//this is whether the backfaces are checked for intersection, when this is checked back faces are checked if all the other checks fail first
				pb_testSeams,			//when this is TRUE the system will try to detect faces that cross seams and fix them up
				pb_checkEdgeRatios,		//when this is TRUE this will campare the ratio of the geometry of the face versus the ratio of the uvw face, if the ratio difference is large it will try to fix up the face
				pb_weldUVs,				//this will weld up the resulting UVs to clean up the UV space, only UVW vertices that share the same geo vert will welded
				pb_weldUVsThreshold,	//this is the threshold use to weld the vertices.  It is not a UVW space value but a ratio of the edge length to the neighbors.
				pb_edgeRatiosThreshold	//this is the ratio used to determine when a UVW face and a XYZ face ar not similiar
		};

		int			NumRefs() { return 1; }
		RefTargetHandle GetReference(int i) { return mpPBlock; }
		void		SetReference(int i, RefTargetHandle rtarg) { mpPBlock=(IParamBlock2*)rtarg; }

		int			NumParamBlocks() { return 1; }
		IParamBlock2* GetParamBlock(int i) { return mpPBlock; }
		IParamBlock2* GetParamBlockByID(BlockID id) { return (mpPBlock->ID() == id) ? mpPBlock : NULL; }

		RefTargetHandle Clone( RemapDir &remap );

		//	IProjectionModType
		void		SetIProjectionMod( IProjectionMod *pPMod ) { mpPMod = pPMod; }

		bool		CanProject(Tab<INode*> &tabSourceNodes, int iSelIndex, int iNodeIndex);
		void		Project(Tab<INode*> &tabSourceNodes, int iSelIndex, int iNodeIndex);

		void		SetInitialName(int iIndex);
		const TCHAR*		GetName();
		void		SetName(const TCHAR *name);

		void		Enable(bool enable) { mbEnabled = enable; }
		bool		IsEditing() { return mbEditing; }
		void		EndEditing() { mbEditing = false;}
		void		EnableEditing(bool enable) { mbEditing = enable; }

		void		BeginEditParams(IObjParam  *ip, ULONG flags,Animatable *prev);
		void		EndEditParams(IObjParam *ip,ULONG flags,Animatable *next);       

		IOResult	Load(ILoad *pILoad);
		IOResult	Save(ISave *pISave);

		void		ModifyObject(TimeValue t, ModContext &mc, ObjectState * os, INode *inode, IProjectionModData *pPModData);

		void		DeleteThis() { delete this; }

		//	ProjectModTypeUVW
		void		MainPanelInitDialog( HWND hWnd );
		void		MainPanelDestroy( HWND hWnd );
		void		MainPanelUpdateUI();
		void		MainPanelUpdateMapChannels();

		int			GetSourceMapChannel() { return miSourceMapChannel; };
		void		SetSourceMapChannel(int iMapChannel) { miSourceMapChannel = iMapChannel; };
		int			GetTargetMapChannel() { return miTargetMapChannel; };
		void		SetTargetMapChannel(int iMapChannel) { miTargetMapChannel = iMapChannel; };
		bool		GetProjectMaterialsIDs();
		bool		GetAlwaysUpdate();

		void		ProjectToTarget(INode *pNodeSrc, ObjectState * os, INode *pNodeTarget, BitArray *pSOSel, IProjectionModData *pPModData );
		ProjectionHolderUVW *GetProjectionHolderUVW( Object* obj );
		bool		GetSOSelData( INode *pSourceNode, int iSelIndex, BitArray &sel );
		GenFace		GetChannelFace( ObjectWrapper *object, int iFace, int iMapChannel );
		Point3*		GetChannelVert( ObjectWrapper *object, int iVert, int iMapChannel );

		bool		InitProjectionState( ProjectionState &projState, INode *pNodeSrc, ObjectState * os, INode *pNodeTarget, ProjectionHolderUVW *pMod, BitArray *pSOSel, IProjectionModData *pPModData );
		void		FillInHoldDataOptions( ProjectionHolderUVWData *pData );
		void		FillInHoldDataSameTopology( ProjectionState &projState );
		void		FillInHoldDataProjection( ProjectionState &projState );
		void		FillInHoldDataClusterProjection( ProjectionState &projState );
		int			FindMostHitFace(Tab<int> &tabFaceHits);
		void		CreateClusterData( ProjectionState &projState );
		bool		CreateClusterEdges( ProjectionState &projState );
		int			ExpandClusterFaces( ProjectionState &projState, Tab<int> &clusterFaceList, int iNewFaceIndex, BitArray &faceVisited );
		bool		CreateClusterTraverseEdges( ProjectionState &projState, int iFaceIndex, int iClusterNum, BitArray &faceVisited, Box3 &bBox);
		void		FindClosestFaceAndEdgeByCluster( ProjectionState &projState, Point3 &p3Tar, int iMainCluster, int &iClosestEdgeIndex, int &iClosestFaceIndex );
		void		ProjectToEdgeToFindUV( ProjectionState &projState, Point3 &p3Tar, int iEdgeIndex, int iFaceIndex, Point3 &UV );

		// This method fixes up the UVW geometry/topology after the initial projection.  Doing stuff like welding and fixing
		// seam issues.  UVW faces are initially exploded 1 uvw vert per face corner.
		// ProjectionState &projState - just the project state data
		// int iTargetFace - this is the index of the target face to check
		// Tab<int> &tabFaceHits - this are indices of the hit faces on the source one for each corner
		// int iCenterFaceHit - this is the centriod hit face index on the source
		// Point3 &ptCenter - geo center of the target?
		// Point3 &ptCenterUVW - uvw center of the target face?
		// int &vertexBufferOffset - this is the index into our explode uvw face list, we increment this in the method to iterate to the next face
		void		FixMapFaceData( ProjectionState &projState, int iTargetFace, Tab<int> &tabFaceHits, int iCenterFaceHit, Point3 &ptCenter, Point3 &ptCenterUVW, int &vertexBufferOffset );
		int			GetUVQuadrant( ProjectionState &projState, int iCluster, Point3 &ptUVW );
		

		//this tries to guess the best face to approx our uvws from.  It is a guess on which face covers the most of the
		//target face. It returns the face that should be used.  It will return -1 if it does not find a suitable face
		//	ProjectionState &projState - the projection state data
		//  int iTargetFace - the target face we want to check
		//  Tab<int> &tabFaceHits - the face that the corners of the target face hit
		//  int centerFaceHit - is the center face hit
		int			FindMasterFace(ProjectionState &projState, int iTargetFace, Tab<int> &tabFaceHits, int centerFaceHit);

		//This tests to see whether a specific geo target face crosses a uvw seam on the source geo mesh	
		// ProjectionState &projState - just the projection state data
		// in targetFace - the target face that is to be checked
		// int *sourceFace - these are the faces that target face vertices intersected
		// int sourceFaceCount - this is the number of faces in the source face array
		BOOL		TestFaceCrossesSeams( ProjectionState &projState, int targetFace, Tab<int> &sourceFace);

		//This returns whether 2 line segments intersect
		//Point3 *targetVerts - the array containing the first segment, must contain 2 point3
		//Point3 *sourceEdge - the array containing the second segment, must contain 2 point3
		BOOL		IntersectEdge(Point3 *targetVerts, Point3 *sourceEdge);
		
		//just a quick line segment to point test, returns the distance from the line segment to point
		// Point3 p1 - the point to check
		// Point3 l1,l2 - the end points of the line segment
		// float &u - the percent along the line segment that is closest to the point
		float		LineToPoint(Point3 p1, Point3 l1, Point3 l2, float &u);

		//this does a comparison against edge length ratio to see if the target face UVW and GEO faces are similiar
		// ProjectionState &projState - just the projection state data
		// int iTargetFace - the target face that is to be checked
		// float ratioThreshold - this is the distance ratio which determines when a face becomes dissimilar
		// float &ratio - this is the largest change in ratio so you can compare the change to other faces
		BOOL CheckEdgeRatio(ProjectionState &projState, int iTargetFace, float ratioThresold, float &ratio);

		//this does a comparison against angle ratio to see if the target face UVW and GEO faces are similiar
		// ProjectionState &projState - just the projection state data
		// int iTargetFace - the target face that is to be checked
		// float ratioThreshold - this is the distance ratio which determines when a face becomes dissimilar
		// float &ratio - this is the largest change in ratio so you can compare the change to other faces
		BOOL CheckAngleRatio(ProjectionState &projState, int iTargetFace, float ratioThresold, float &ratio);


		//this will try to approximate a uvw projection based on bary coords, typically called when a seam is crossed or the face looks
		//like an error face
		// ProjectionState &projState - just the projection state data
		// int iTargetFace - the target face that is to be checked
		// int masterFace - this is the face to try to build from
		// Tab<Point3> &results - this is the results
		void ReprojectFace(ProjectionState &projState, int iTargetFace, int masterFace, Tab<int> &tabFaceHits, Tab<Point3> &uvResults);



	private:
		//temporary variable used for TestFaceCrossesSeams to reduce memory reallocation DO NOT USE elsewhere
		BitArray mProcessedFaces;
};

//=============================================================================
//
//	Class ProjectionState
//
//=============================================================================
class ProjectionState 
{
public:

	ProjectionState(): objectSource(NULL), objectTarget(NULL), 
		pPInter(NULL), pData(NULL), iProjType(pt_sameTopology), pSOSel(NULL) {};

	enum { pt_sameTopology, pt_projection, pt_projectionCluster };

	int							iProjType;
	ObjectWrapper*				objectSource;
	ObjectWrapper*				objectTarget;
	IProjectionIntersector*		pPInter;
	Matrix3						mat;
	ProjectionHolderUVWData*	pData;
	Tab<int>					tabCluster;
	BitArray					bitClusterEdges;
	Tab<Point2>					tabClusterCenter;
	BitArray*					pSOSel;

	BOOL	mIgnoreBackFaces;
	BOOL	mTestSeams;
	BOOL	mWeld;
	float	mWeldThreshold;
	BOOL	mCheckEdgeRatios;
	float	mEdgeRatioThresold;

	void Cleanup() {
		if( objectTarget ) {
			delete objectTarget;
			objectTarget = NULL;
		}
		if( pPInter ) {
			pPInter->DeleteThis();
			pPInter = NULL;
		}
	}
};


//this is just a linked list data struct so we can keep tarck off all the neighbors
//nothing exciting here 
class Neighbor;

class Neighbor
{
public:
	int mID;
	Neighbor *mNext;
};

class NeighborList
{
public:
	NeighborList()
	{
		mStart = NULL;
	}
	~NeighborList()
	{
		Neighbor *current = mStart;
		while (current != NULL)
		{
			Neighbor *temp = current;
			current = current->mNext;
			delete temp;
		}
	};
	Neighbor *mStart;
	void Add(int id)
	{
		Neighbor *current = mStart;
		Neighbor *last = mStart;
		BOOL found = FALSE;
		while (current != NULL)
		{
			if (current->mID == id)
			{
				found = TRUE;
				current = NULL;
			}
			else
			{
				last = current;
				current = current->mNext;
			}
		}
		if (!found)
		{
			Neighbor *newEntry = new Neighbor();
			newEntry->mID = id;
			newEntry->mNext = NULL;
			if (last == NULL)
				mStart = newEntry;
			else last->mNext = newEntry;
		}
	};
};

#endif // __PROJECTIONMODTYPEUVW__H
