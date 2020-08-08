
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
// DESCRIPTION: This contains our local data classes
// AUTHOR: Peter Watje
// DATE: 2006/10/07 
//***************************************************************************/

#ifndef __MESHTOPODATA__H
#define __MESHTOPODATA__H

class UnwrapMod;
class ClusterClass;

class MeshTopoData : public LocalModData {
public:


	
	MeshTopoData(ObjectState *os, int mapChannel);
	MeshTopoData();
	~MeshTopoData();
	LocalModData *Clone();

	//just sets all our pointers to null, it does not free anything
	void Init();

	//these fucntion let you get at the cached geo object
	//returns our cached mesh
	Mesh *GetMesh() {return mesh;}
	//returns our cached mnmesh
	MNMesh *GetMNMesh() {return mnMesh;}
	//returns our cached patch
	PatchMesh *GetPatch() {return patch;}

	//this free our geo data cache
	void FreeCache();

	//this sets the cache based on the object passed in
	void SetGeoCache(ObjectState *os);

	//this just makes sure to check the cache against the incoming mesh
	//it checks for topological and type changes and return whether or not it has detected a change
	BOOL HasCacheChanged(ObjectState *os);

	//this reset the caches and rebuilds 
	void Reset(ObjectState *os, int mapChannel);

	//this updates any geo data from the object in the stack into our cache
	void UpdateGeoData(TimeValue t,ObjectState *os);


	//this pushes the data on any of the contrroller back into the local data
	//we need to do this since all the animation is stored on the modifier and not 
	//local data
	void UpdateControllers(TimeValue t, UnwrapMod *mod);

	//this copy the face selection in our cache into the stack object for display purposes
	void CopyFaceSelection(ObjectState *os);

	//this applies the mapping in the cache to the pipe object
	void ApplyMapping(ObjectState *os, int mapChannel,TimeValue t);

	//this returns the current face selection for this local data
	BitArray &GetFaceSel() { return mFSel; }
	//this copies the a selection into our unwrap face selection
	void SetFaceSel(BitArray &set);

	//this gets/sets whether the incoming pipe object is set to face selection
	BOOL HasIncomingFaceSelection();
	BitArray GetIncomingFaceSelection();




	//these are just load save function for the various components
	ULONG SaveTVVertSelection(ISave *isave);
	ULONG SaveGeoVertSelection(ISave *isave);
	ULONG SaveTVEdgeSelection(ISave *isave);
	ULONG SaveGeoEdgeSelection(ISave *isave);
	ULONG SaveFaceSelection(ISave *isave);

	ULONG LoadTVVertSelection(ILoad *iload);
	ULONG LoadGeoVertSelection(ILoad *iload);
	ULONG LoadTVEdgeSelection(ILoad *iload);
	ULONG LoadGeoEdgeSelection(ILoad *iload);
	ULONG LoadFaceSelection(ILoad *iload);


	ULONG SaveFaces(ISave *isave);
	ULONG SaveTVVerts(ISave *isave);
	ULONG SaveGeoVerts(ISave *isave);

	ULONG LoadFaces(ILoad *iload);
	ULONG LoadTVVerts(ILoad *iload);
	ULONG LoadGeoVerts(ILoad *iload);

	//theser are save/load functions for external file support
	int LoadTVVerts(FILE *file);
	void SaveTVVerts(FILE *file);

	void LoadFaces(FILE *file, int ver);
	void SaveFaces(FILE *file);

	//these are some basic selection methods for the tv data
	//mode = TVVERTMODE, TVEDGEMODE, TVFACEMODE
	void ClearSelection(int mode);
	void InvertSelection(int mode);
	void AllSelection(int mode);
	
	//This just syncs the handle selection with the knot selection
	// dir determines whether to remove the selection when set to 0 or add to the selection when set to 1
	void SelectHandles(int dir);
	//this just does an element select based on the current selection
	//mode is the current subobject mode
	//addSelection determines whether to add to the current selection or remove from the current selection
	void SelectElement(int mode,BOOL addSelection);
	//just a vertex specific version of the above
	void SelectVertexElement(BOOL addSelection);

	//these must be called in pairs
	//this transfer our current selection into vertex selection
	//these are used since alot of operation work on vertices and this a quick way 
	//to do thos operations from edge/face slection by just transferring the selection
	void	TransferSelectionStart(int currentSubObjectMode);
	//this transfer our vertex selection back into our current selection
	//partial if set and recomputeSelection is set will only transfer back the suobjects where all the components vertices are set 
	//for instance in edge if only 1 vertex of the edge was selected that edge would get selected if partial was on
	//recomputeSelection if false the cached selection from the TransferSelectionSatrt is used otherwise the current selection
	//is used to get the new selection
	void	TransferSelectionEnd(int currentSubObjectMode,BOOL partial,BOOL recomputeSelection);



	//Converts the edge selection into a vertex selection set
	void    GetVertSelFromEdge(BitArray &sel);
	//Converts the vertex selection into a edge selection set
	//PartialSelect determines whether all the vertices of a edge need to be selected for that edge to be selected
	void    GetEdgeSelFromVert(BitArray &sel,BOOL partialSelect);

	
	void    GetVertSelFromFace(BitArray &sel);		 //Converts the face selection into a vertex selection set
	void    GetGeomVertSelFromFace(BitArray &sel);	//Converts the face selection into a geom vertex selection set
	//PartialSelect determines whether all the vertices of a face need to be selected for that face to be selected
	void    GetFaceSelFromVert(BitArray &sel,BOOL partialSelect);

	//converts the face selection to the edge selection
	void	ConvertFaceToEdgeSel();


	//this returns whether a vertex is visible
	BOOL IsTVVertVisible(int index);

	//this returns whether a face is visible
	BOOL IsFaceVisible(int index);

//TV VERTEX METHODS********************************************
	//returns the number texture vertices
	int GetNumberTVVerts();
	//returns a copy of the texture vertices selections
	BitArray GetTVVertSelection();
	//returns a ptr to the texture vertices selections
	BitArray *GetTVVertSelectionPtr();
	//copy a new selection into our texture vertices selection
	void SetTVVertSelection(BitArray newSel);
	//clears all our texture vertex selectiom
	void ClearTVVertSelection();
	//returns the UVW position of a texture vertex
	Point3 GetTVVert(int index);

	//this adds a texture vert to our tv list
	//it returns the position that the vertex was inserted at
	int AddTVVert(TimeValue t,Point3 p, UnwrapMod *mod, Tab<int> *deadVertList = NULL);

	//this adds a point to our tv list and also assigns it to a face vertex 
	// t is the current time in case we need to key the controller
	// p it the point to add
	// faceIndex is the face to add it to 
	// ithVert is the ith vertex of that face
	// mod is the pointer to the unwrap modifie in case we need to key the conreoller
	// sel determines whether that vertex is selected or nor
	// returns the position of the new vertex in the tvert list
	int AddTVVert(TimeValue t, Point3 p, int faceIndex, int ithVert, UnwrapMod *mod, BOOL sel = FALSE, Tab<int> *deadVertList = NULL);
	//same as above but for a patch handle
	int AddTVHandle(TimeValue t,Point3 p, int faceIndex, int ithVert, UnwrapMod *mod, BOOL sel = FALSE, Tab<int> *deadVertList = NULL);
	//same as above but for a patch interior
	int AddTVInterior(TimeValue t,Point3 p, int faceIndex, int ithVert, UnwrapMod *mod, BOOL sel = FALSE, Tab<int> *deadVertList = NULL);

	//this deletes a tvert from the list, it is not actually deleted but just flagged
	void DeleteTVVert(int index, UnwrapMod *mod);

	//this creates controllers for every selected vert
	void PlugControllers( UnwrapMod *mod);

	//these get and set the tv vert properties
	//this gets/sets the position
	void SetTVVert(TimeValue t, int index, Point3 p, UnwrapMod *mod);
	//this gets/sets the soft selection influence
	float GetTVVertInfluence(int index);	
	void SetTVVertInfluence(int index, float influ);
	//this gets/sets the selection state
	BOOL GetTVVertSelected(int index);
	void SetTVVertSelected(int index, BOOL sel);
	//this gets/sets the hidden state
	BOOL GetTVVertHidden(int index);
	void SetTVVertHidden(int index, BOOL hidden);
	//this gets/sets the frozen state
	BOOL GetTVVertFrozen(int index);
	void SetTVVertFrozen(int index, BOOL frozen);
	//this gets/sets the controler index, if no control is linked to this vertex it is set to -1
	//the controller list is actually kept in the modifier
	void SetTVVertControlIndex(int index, int id);
	//this gets/sets the dead flag
	BOOL GetTVVertDead(int index);
	void SetTVVertDead(int index, BOOL dead);

	//this flag means that the system has locked this vertex and it should not be touched, displayed etc.
	BOOL GetTVSystemLock(int index);
	//this gets/sets whether the soft selection influnce has been modified.  If it is modified
	//it wont get recomputed.
	BOOL GetTVVertWeightModified(int index);
	void SetTVVertWeightModified(int index, BOOL modified);
	//this is just a generic flag access
	int GetTVVertFlag(int index);
	void SetTVVertFlag(int index, int flag);

	//this returns the first geo index vert that touches the tvvert
	//this is slow
	int GetTVVertGeoIndex(int index);
	

	//this makes a copy of the UV vertx channel to v
	void CopyTVData(Tab<UVW_TVVertClass> &v);
	//this paste the tv data back from v
	void PasteTVData(Tab<UVW_TVVertClass> &v);

	void CopyFaceData(Tab<UVW_TVFaceClass*> &f);
	//this paste the tv face data back from f
	void PasteFaceData(Tab<UVW_TVFaceClass*> &f);


//GEOM VERTEX METHODS******************************************************
	//gets/sets the number geometric vertices
	int GetNumberGeomVerts();
	void SetNumberGeomVerts(int ct);
	//gets/sets the geomvert position
	Point3 GetGeomVert(int index);
	void SetGeomVert(int index, Point3 p);

	//Geom vert selection methods
	BitArray GetGeomVertSelection();
	BitArray *GetGeomVertSelectionPtr();
	void SetGeomVertSelection(BitArray newSel);
	void ClearGeomVertSelection();
	BOOL GetGeomVertSelected(int index);
	void SetGeomVertSelected(int index, BOOL sel);


//TV EDGE METHODS***********************************************************
	//returns the number texture edges
	int GetNumberTVEdges();
	//returns the texture vertex index of the start or end vertex of the edge
	//if whichEnd is 0 it returns the start else it returns the end
	int GetTVEdgeVert(int edgeIndex, int whichEnd);
	//returns the geom vertex index of the start or end vertex of the edge
	//if whichEnd is 0 it returns the start else it returns the end
	int GetTVEdgeGeomVert(int edgeIndex, int whichEnd);

	//returns the vector handle index if there is none -1 is returned
	int GetTVEdgeVec(int edgeIndex, int whichEnd);
	//returns the number of TV faces connected to this edge
	int GetTVEdgeNumberTVFaces(int edgeIndex);
	//returns the TV face connected to an edge
	int GetTVEdgeConnectedTVFace(int edgeIndex,int ithFaceIndex);
	//return if the edge is hidden
	BOOL GetTVEdgeHidden(int edgeIndex);

	//this sets an invalid flag on the tv edges and the tv edges will be rebuilt on the next
	//evaluation
	void SetTVEdgeInvalid();
	//this sets an invalid flag on the geo edges and the tv edges will be rebuilt on the next
	//evaluation
	void SetGeoEdgeInvalid();

	//this forces an tv edge rebuilt immediately
	void BuildTVEdges();

	//TV Edge selection methods
	BitArray GetTVEdgeSelection();
	BitArray *GetTVEdgeSelectionPtr();
	void SetTVEdgeSelection(BitArray newSel);
	void ClearTVEdgeSelection();
	BOOL GetTVEdgeSelected(int index);
	void SetTVEdgeSelected(int index, BOOL sel);

	//this determines whether a point intersects a tv edge within the threshold
	int EdgeIntersect(Point3 p, float threshold, int i1, int i2);

//this turns a geometric edge selection into a UV edge selection
	void ConvertGeomEdgeSelectionToTV(BitArray geoSel, BitArray &uvwSel);


//GEOM EDGE METHODS **************************************************************
	//returns the number geometric edges
	int GetNumberGeomEdges();

	//Selection Methods
	BitArray *GetGeomEdgeSelectionPtr();
	BitArray GetGeomEdgeSelection();
	void SetGeomEdgeSelection(BitArray newSel);
	void ClearGeomEdgeSelection();
	BOOL GetGeomEdgeSelected(int index);
	void SetGeomEdgeSelected(int index, BOOL sel);

	//returns whether an edge is a hidden edge or not
	BOOL GetGeomEdgeHidden(int index);
	//returns the vertex index of the start or end vertex of the edge
	int GetGeomEdgeVert(int edgeIndex, int whichEnd);
	//returns the vector index of the start or end vertex of the edge, if the
	//object is not a patch -1 is returned
	int GetGeomEdgeVec(int edgeIndex, int whichEnd);
	//returns the number of faces connected to an edge
	int GetGeomEdgeNumberOfConnectedFaces(int edgeIndex);
	//returns the face connected to an edge
	int GetGeomEdgeConnectedFace(int edgeIndex,int ithFaceIndex);

	//this displays the geometry edge selection
	void DisplayGeomEdge(GraphicsWindow *gw, int edgeIndex, float size, BOOL thick, Color c);
	//this displays a geo edge using vertex indices.  If vecA and vecB are not -1 it assumes it is
	//bezier curved edge to be drawn
	void DisplayGeomEdge(GraphicsWindow *gw, int vertexA,int vecA,int vertexB,int vecB, float size, BOOL thick, Color c);


	

//TV Face Methods*********************************************************************
	//we don't differentiate between tv and geo faces since they are a 1 to 1 correspondance
	//this returns the number faces
	int GetNumberFaces();
	//this returns the degree of the face ie the number of border edges to the face
	int GetFaceDegree(int faceIndex);

	//this returns texture vert index of the ith border vert
	int GetFaceTVVert(int faceIndex, int ithVert);
	void SetFaceTVVert(int faceIndex, int ithVert, int id);

	//this returns the texture vertex index of the ith border vec if no vectors are present -1 is returned	
	int GetFaceTVHandle(int faceIndex, int vertexID);
	void SetFaceTVHandle(int faceIndex, int ithVert, int id);

	//this returns the texture  vertex index of the ith interior vec if no vectors are present -1 is returned
	int GetFaceTVInterior(int faceIndex, int vertexID);
	void SetFaceTVInterior(int faceIndex, int ithVert, int id);

	//this returns the geom vertex index of the ith border vert
	int GetFaceGeomVert(int faceIndex, int ithVert);
	//this returns the geom vertex index of the ith border vec if no vectors are present -1 is returned
	int GetFaceGeomHandle(int faceIndex, int ithVert);
	//this returns the geom  vertex index of the ith interior vec if no vectors are present -1 is returned
	int GetFaceGeomInterior(int faceIndex, int ithVert);


	//face property accessor
	//gets/sets whether a face is selected
	BOOL GetFaceSelected(int faceIndex);
	void SetFaceSelected(int faceIndex, BOOL sel);
	//gets/sets whether a face is frozen
	BOOL GetFaceFrozen(int faceIndex);
	void SetFaceFrozen(int faceIndex, BOOL frozen);
	//gets/sets whether a face is dead
	BOOL GetFaceDead(int faceIndex);
	void SetFaceDead(int faceIndex, BOOL dead);
	//gets/sets whether a face has curved mapping flag is set
	BOOL GetFaceCurvedMaping(int faceIndex);
	//gets/sets whether a face has curved mapping vectors are actually allocated
	BOOL GetFaceHasVectors(int faceIndex);
	//gets/sets whether a face has curved mapping interior vectors are actually allocated
	BOOL GetFaceHasInteriors(int faceIndex);
	BOOL GetFaceHidden(int faceIndex);


	//returns the material id of the face
	int GetFaceMatID(int faceIndex);

	//returns the geometric normal of a face
	Point3 GetGeomFaceNormal(int faceIndex);
	//returns the texture normal of a face
	Point3 GetUVFaceNormal(int faceIndex);

//given a face index return a tab of all the uvw vertex indices on this face
	void GetFaceTVPoints(int faceIndex, Tab<int> &vIndices);
//given a face index return a tab of all the geom vertex indices on this face
	void GetFaceGeomPoints(int faceIndex, Tab<int> &vIndices);

//given a face index OR all the uvw indices with the incoming bitarray
	void GetFaceTVPoints(int faceIndex, BitArray &vIndices);
//given a face index OR all the geom indices with the incoming bitarray
	void GetFaceGeomPoints(int faceIndex, BitArray &vIndices);


	//clones a particular face
	UVW_TVFaceClass *CloneFace(int faceIndex);
	


	//Face selection methods
	void ClearFaceSelection();
	BitArray GetFaceSelection();
	BitArray *GetFaceSelectionPtr();
	void SetFaceSelection(BitArray sel);
	//Selects faces based on material ID
	void SelectByMatID(int matID);

	//given 2 geometric points return the geo edge index between them
	int FindGeoEdge(int a, int b);
	//given geom face index and 2 vert index on the face return this ith edge index that touches a and b
	int FindGeomEdge(int faceIndex,int a, int b);

	//given 2 uv points return the uv edge index between them
	int FindUVEdge(int a, int b);
	//given geom face index and 2 vert index on the face return this ith edge index that touches a and b
	int FindUVEdge(int faceIndex,int a, int b);

	//returns the tv faces that intersects point p
	//i1, i2 are the UVW space to check
	int PolyIntersect(Point3 p, int i1, int i2, BitArray *ignoredFaces = NULL);

//hitests our geometry returning a list of faces that were hit
	void HitTestFaces(GraphicsWindow *gw, INode *node, HitRegion hr, Tab<int> &hitFaces);

	//Returns ths angle between 2 vectors
	float AngleFromVectors(Point3 vec1, Point3 vec2);
	//returns a tm based on a face
	Matrix3 GetTMFromFace(int faceIndex, BOOL useTVFace);

//returns the local bounding box of the selecetd faces
	Box3 GetSelFaceBoundingBox();
//returns the local bounding box of the whole mesh
	Box3 GetBoundingBox();

	//these are just some access to a temporary IPoint2 array of the tvs in dialog space
	//they are created and destroyed in the dialog paint method
	int TransformedPointsCount();
	void TransformedPointsSetCount(int ct);
	IPoint2 GetTransformedPoint(int i);
	void SetTransformedPoint(int i, IPoint2 pt);


	//this just returns whether the file was just loaded so we know to convert over old load data if need be
	BOOL IsLoaded() { return mLoaded; };
	void ClearLoaded() {mLoaded = FALSE;}
	void SetLoaded() {mLoaded = TRUE;}

	//this just takes old max data and put into our new format
	void ResolveOldData(UVW_ChannelClass &oldData);
	//this saves the old data back into the modifier so we can reload max 9.5 files into max9
	void SaveOldData(ISave *isave);


	void BuilMatIDFilter(int matID);
	void BuilFilterSelectedFaces(int filterSelFaces);

	//these are our snap buffers
	//This builds the buffer where w/h are the size of the window
	void BuildSnapBuffer(int w, int h);
	void FreeSnapBuffer();
	//this gets/sets the snap vertex buffer were each cell contains the vertex that reside in it
	void SetVertexSnapBuffer(int index, int value);
	int GetVertexSnapBuffer(int index);

	//this gets/sets the snap edge buffer were each cell contains the vertex that reside in it
	void SetEdgeSnapBuffer(int index, int value);
	int GetEdgeSnapBuffer(int index);
	//Allows direct access to the edge snap buffer
	Tab<int> &GetEdgeSnapBuffer();

	//this are accessors to a buffer which tells you if an edge is touching a vertex that was snapped
	void SetEdgesConnectedToSnapvert(int index, BOOL value);
	BOOL GetEdgesConnectedToSnapvert(int index);

	//break the selected edges
	void BreakEdges(UnwrapMod *mod);

//breaks the UV edges
	void BreakEdges(BitArray uvEdges);

	//break the selected verts
	void BreakVerts(UnwrapMod *mod);
	//selected the selected verts
	void WeldSelectedVerts(float threshold, UnwrapMod *mod);

	//this takes the associated geo faces an ddetaches them and copies them into uv space
	//faceSel is the selection of faces to be detached
	//vertSel is the selection of vertices for the new detached element
	//mod is the modifier that own the local data needed to update the controllers if animation is on
	void DetachFromGeoFaces(BitArray faceSel, BitArray &vertSel, UnwrapMod *mod);

	//given the vertex selection this returns all the opposing shared vertices
	void  GetEdgeCluster(BitArray &cluster);
	//this stitches the selection together
	void Stitch(BOOL bAlign, float fBias, BOOL bScale, UnwrapMod *mod);

//several relax algorithms
	//first generation just center based relax
	//only called from maxscript now
	void  Relax(int subobjectMode, int iteration, float str, BOOL lockEdges, BOOL matchArea,UnwrapMod *mod);
	//Experimental relax/fit algorithm that was never worked well
	//only called from maxscript now
	void  RelaxFit(int subobjectMode,int iteration, float str, UnwrapMod *mod);
	//a spring based relax only called from script
	void  RelaxBySprings(int subobjectMode,int frames, float stretch, BOOL lockEdges, UnwrapMod *mod);

	//relax from the mesh/mnmesh algorithms
	void  RelaxVerts2(int subobjectMode, float relax, int iter, BOOL boundary, BOOL saddle, UnwrapMod *mod, BOOL applyToAll = FALSE, BOOL updateUI = TRUE);
	
	//this a relax that uses the shape of the geo faces to align texture faces
	void RelaxByFaceAngle(int subobjectMode, int frames, float stretch, float str, BOOL lockEdges,HWND statusHWND, UnwrapMod *mod, BOOL applyToAll = FALSE);

	//this a relax algorithm based on the angles of edges that touch a vertex
	void RelaxByEdgeAngle(int subobjectMode, int frames, float stretch,float str, BOOL lockEdges,HWND statusHWND, UnwrapMod *mod, BOOL applyToAll = FALSE);

	void	UpdateClusterVertices(Tab<ClusterClass*> &clusterList);
	BOOL	BuildCluster( Tab<Point3> normalList, float threshold, BOOL connected, BOOL cleanUpStrayFaces, Tab<ClusterClass*> &clusterList, UnwrapMod *mod);
	void	AlignCluster(Tab<ClusterClass*> &clusterList, int baseCluster, int moveCluster, int innerFaceIndex, int outerFaceIndex,int edgeIndex,UnwrapMod *mod);
	
	//these are various mapping algorithms
	//this applies a world space planar mapping from a normal
	void PlanarMapNoScale(Point3 gNormal, UnwrapMod *mod );
	BOOL NormalMap(Tab<Point3*> *normalList, Tab<ClusterClass*> &clusterList, UnwrapMod *mod );
	void ApplyMap(int mapType, BOOL normalizeMap, Matrix3 tm, UnwrapMod *mod);


	//just a temporary hold/restore face selection
	//this does not use the undo buffer, just a quick method to store and restore the face selection
	void HoldFaceSel();
	void RestoreFaceSel();

	//these are the seam edges for the pelt mapper
	void EdgeListFromPoints(Tab<int> &selEdges, int a, int b, Point3 vec);
	BitArray mSeamEdges;
	void ExpandSelectionToSeams();
	void CutSeams(UnwrapMod *mod, BitArray seams);
	void BuildSpringData(Tab<EdgeBondage> &springEdges, Tab<SpringClass> &verts, Point3 &rigCenter, Tab<RigPoint> &rigPoints, Tab<Point3> &initialPointData, float rigStrength);

	//tools to help manage the sketch tool soft selection
	void InitReverseSoftData();
	void ApplyReverseSoftData(UnwrapMod *mod);


	//this builds our tv vert to geo vert list so we can look to see which geo vert own which tv vert
	void BuildVertexClusterList();

//these are our named selection sets for each local data and subobject
	GenericNamedSelSetList fselSet;
	GenericNamedSelSetList vselSet;
	GenericNamedSelSetList eselSet;
	
//this fixes up an issue with the system lock flags
//it goes through and recomputes these flags
	void FixUpLockedFlags();

//some tools to help intersect the mesh with a ray
//needs to be called before any checks
	void PreIntersect();
//intersects the ray with the mesh.  The ray needs to be in local space of the mesh
//returns whether hit amd the distance along the ray
	bool  Intersect(Ray r,bool checkFrontFaces,bool checkBackFaces, float &at, Point3 &bary, int &hitFace);
//needs to be called to clear out our memory after you are done intersecting
	void PostIntersect();

	//these are functions that tell meshtopodata that the user or system wants to cancel
	//their current operation, we used to just listen to the asynckeyboard state
	//but with threading we something more
	BOOL		GetUserCancel();
	void		SetUserCancel(BOOL cancel);

	//this builds a list of all the border verts for the visible faces.  the borderverts
	//is the order list of the border verts, each seperate border is seperated by a -1 entry 
	//in this list
	void GetBorders(Tab<int> &borderVerts);
private:
//does a quick intersect test on a specific face
	bool CheckFace(Ray ray, Point3 p1,Point3 p2,Point3 p3, Point3 n, float &at, Point3 &bary);
	Tab<Point3> mIntersectNorms;

	Tab<int> mSketchBelongsToList;
	Tab<Point3> mOriginalPos;


	void	AddVertsToCluster(int faceIndex, BitArray &processedVerts, ClusterClass *cluster);
	//a patch specific relax called from relaxverts2 
	void  RelaxPatch(int subobjectMode, int iteration, float str, BOOL lockEdges, UnwrapMod *mod);

	inline BOOL LineIntersect(Point3 p1, Point3 p2, Point3 q1, Point3 q2);

	UVW_ChannelClass TVMaps;
	Tab<UVW_TVFaceClass*> gfaces;
	VertexLookUpListClass gverts;
	BOOL mHasIncomingSelection;

	//this is an bitarray of all our hidden polygons incoming from the stack mesh
	//we use this list to prevent those polygon from being displayed in the UV editor and 
	//to make sure they are unselectable
	BitArray mHiddenPolygonsFromPipe;


	void SetCache(Mesh &mesh, int mapChannel);
	void SetCache(MNMesh &mesh, int mapChannel);
	void SetCache(PatchMesh &patch, int mapChannel);

	void BuildInitialMapping(Mesh *msh);
	void BuildInitialMapping(MNMesh *msh);
	void BuildInitialMapping(PatchMesh *patch);

	void ApplyMapping(Mesh &mesh, int mapChannel);
	void ApplyMapping(MNMesh &mesh, int mapChannel);
	void RemoveDeadVerts(PatchMesh *mesh,int CurrentChannel);
	void ApplyMapping(PatchMesh &patch, int mapChannel);



	//this is the texture vertex selection bitarray
	BitArray mVSel;

	//this is the geo vertex selection bitarray
	BitArray mGVSel;

	//these are the UVW edge selections	
	BitArray mESel;

	//these are the geometric vertex, edge selections
	BitArray mGESel;


	//this is the face selection
	//since faces are 1 to 1 we dont differeniate between geo and tv faces
	BitArray mFSel;

	//this is the previsous face selection flowing up the stack, if the current stack selection
	//is different from the previous we want to use that
	BitArray mFSelPrevious;

	//These are just some bittarray to hold temporary selections
	BitArray originalSel;
	BitArray holdFSel;
	BitArray holdESel;

	//this is a visiblity list for verts based on the matid filter;
	BitArray mVertMatIDFilterList;

	//this is the filter selected face bitarray used to keep track of visible verts atatched to selected faces
	BitArray mFilterFacesVerts;


	//this is just a temporary list of the tvs in dialog space used to speed up some operations
	Tab<IPoint2> mTransformedPoints;
	BOOL mLoaded;

	//these are cluster id list so we know which texture verts are assigned to which geom verts
	//this a list were each enrty points to the equivalent geom index so you can find out whihc 
	//geo vert is attached to which texture vert
	Tab<int> mVertexClusterList;
	//this is just a cout of the number of shared tvverts at each tvert
	Tab<int> mVertexClusterListCounts;


	Mesh *mesh;
	PatchMesh *patch;
	MNMesh *mnMesh;

	//Snap data buffers
	BitArray mEdgesConnectedToSnapvert;
	Tab<int> mVertexSnapBuffer;
	Tab<int> mEdgeSnapBuffer;

	BOOL		mUserCancel;
	

	


};

/*************************************
This is just a container class that contains
all the active local data for a specific unwrap
**************************************/
class MeshTopoDataNodeInfo
{
public:
	INode* mNode;
	MeshTopoData* mMeshTopoData;

};

class MeshTopoDataContainer
{
public:
		int Count();
		void SetCount(int ct);
		void Append(int ct, MeshTopoData* ld, INode *node, int extraCT);		
		MeshTopoData* operator[](int i) 
		{ 
			if ((i < 0) || (i >= mMeshTopoDataNodeInfo.Count()))
			{
				DbgAssert(0);
				return NULL;
			}
			return mMeshTopoDataNodeInfo[i].mMeshTopoData;  
		};

		Point3 GetNodeColor(int index);
		Matrix3 GetNodeTM(TimeValue t, int index);
		INode* GetNode(int index);

private:
	Tab<MeshTopoDataNodeInfo>  mMeshTopoDataNodeInfo;
};
#endif
