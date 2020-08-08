/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*! \file
	\ingroup kyruntime_mesh */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_OBSTACLELAYER_H
#define KY_OBSTACLELAYER_H

#include <kyruntime/mesh/inavmeshlayer.h>
#include <kyruntime/mesh/obstacleprojectionvolume.h>

#include <kypathdata/generic/types.h>
#include <kypathdata/generic/memory.h>
#include <kypathdata/navmesh/navmeshtypes.h>
#include <kypathdata/navmesh/navfloorptr.h>
#include <kypathdata/containers/list.h>


namespace Kaim
{

class INavMeshAccessor;
class IObstacleAccessor;
class ObstacleAccessorClass;
class NavMeshAccessorClass;
class IVolume;
class Database;
class NavMeshQueryServerWrapper;
class ObstacleProjectionVolume;


/////////////////////////////////////////////////////////////////////////////////
// ObstacleLayer
/////////////////////////////////////////////////////////////////////////////////

/*! This class is used internally by ObstacleLayer to store all information related to each obstacle added to the ObstacleLayer. 
	You should not need to interact with this class directly.
	\ingroup kyruntime_mesh */
class ObstacleLayerObstacleRecord
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	/*! \label_constructor For internal use. */
	inline ObstacleLayerObstacleRecord();

	/*! \label_constructor Called transparently by ObstacleLayer::AddObstacle(). 
		For details on the parameters, see ObstacleLayer::AddObstacle(). */
	inline ObstacleLayerObstacleRecord(Body *body, Vec3f* points, KyUInt32* pointCount, KyFloat32* thickness,
		NavFloorPtr* navFloors, KyUInt32 navFloorMax, SharedPoolList<ObstacleProjectionVolume*>::Pool* projectionVolumePool,
		KyBool* hasChanged, void* userData);

	/*! \label_constructor For internal use. */
	inline ObstacleLayerObstacleRecord(const ObstacleLayerObstacleRecord& ob);

	/*! Copies all class members from the specified ObstacleLayerObstacleRecord to this object. */
	inline ObstacleLayerObstacleRecord& operator=(const ObstacleLayerObstacleRecord& ob);

	inline KyResult AddProjectionVolume(ObstacleProjectionVolume* projectionVolume);
	inline KyResult RemoveProjectionVolume(ObstacleProjectionVolume* projectionVolume);
	inline void ClearProjectionVolumes();

public:
	Body *m_body; /*!< Stores the pointer passed to the \c body parameter of ObstacleLayer::AddObstacle(). */
	Vec3f* m_points; /*!< Stores the pointer passed to the \c points parameter of ObstacleLayer::AddObstacle(). */// outline of the obstacle
	KyUInt32* m_pointCount; /*!< Stores the pointer passed to the \c pointCount parameter of ObstacleLayer::AddObstacle(). */// number of points in m_points (can change dynamically)
	KyFloat32* m_thickness; /*!< Stores the pointer passed to the \c thickness parameter of ObstacleLayer::AddObstacle(). */// thickness of the obstacle
	NavFloorPtr* m_navFloorPtrs; /*!< Stores the value passed to the \c navFloors parameter of ObstacleLayer::AddObstacle(). */// accessors to NavFloors in which the obstacle is projected
	KyUInt32 m_navFloorCount; /*!< Stores the number of NavFloorPtr objects in the #m_navFloorPtrs array that are currently used to store the \NavFloors on which this obstacle has been projected. */// number of elements in m_navFloorPtrs
	KyUInt32 m_navFloorMax; /*!< Stores the value passed to the \c navFloorCount parameter of ObstacleLayer::AddObstacle(). */// maximum number of elements in m_navFloorPtrs
	KyBool* m_hasChanged; /*!< Stores the pointer passed to the \c hasChanged parameter of ObstacleLayer::AddObstacle(). */// indicates whether the obstacle has changed since the last world update
	void* m_userData; /*!< Stores the pointer passed to the \c userData parameter of ObstacleLayer::AddObstacle(). */// This allow the user to transmit information about the obstacle, such as LOD...
	SharedPoolList<ObstacleProjectionVolume*> m_projectionVolumes;
	Box2f m_aabb2d;
};


/*! This implementation of INavMeshLayer provides a ready-to-use system for managing dynamic, movable obstacles that
	can block the movements of \Bots through the NavMesh. It is integrated with the PathFinder, and is used by the ObstacleManager
	to supply Obstacle outlines for the CanGo_NavMesh modifier and for the LpfManager.
	\pp Obstacle outlines are added and removed to the ObstacleLayer through the AddObstacle() and RemoveObstacle() methods.
	Typically, these methods are called transparently by the ObstacleManager. They may also be called by objects whose classes
	derive from IObstacleAccessor, which can be be created and assigned to the ObstacleLayer through a call to CreateAccessor().
	\pp Each obstacle is "projected" onto a NavFloor, to associate the obstacle with a particular area of the static NavMesh.
	This increases the performance of the collision testing methods in the ObstacleLayer class (e.g. CanGo(), TraceLine(), etc.)
	by allowing them to consider only obstacles in the ObstacleLayer that lie in the areas of the static NavMesh covered by the
	collision test.
	\pp The \NavMeshes on which the ObstacleLayer projects its obstacles are managed through the AddMesh() and RemoveMesh() methods.
	Typically, these methods are called by an object that derives from INavMeshAccessor, which can be be created and assigned to the
	ObstacleLayer through a call to CreateAccessor(). Most projects will use the default NavMeshAccessor_Database class.
	\ingroup kyruntime_mesh */
class ObstacleLayer : public INavMeshLayer
{
public:
	/*! Enumerates the flags that can be used in a call to Render(). */
	enum RenderFlag
	{
		RF_Mapping = 1, /*!< Renders the link between each projected obstacle and the NavFloor on which that obstacle is projected. */
		RF_dummy_forceenumsize = KYFORCEENUMSIZEINT
	};

public:
	/*! Defines a type for a bi-directional iterator that can be used to iterate through a List composed of ObstacleLayerObstacleRecord objects.
		See the List and List::Iterator classes for usage. */
	typedef List<ObstacleLayerObstacleRecord>::Iterator ObstacleIt;
	typedef List<ObstacleLayerObstacleRecord>::ConstIterator ConstObstacleIt;

	class IObserver : public VirtualMemObject
	{
	public:
		virtual ~IObserver() {}

		virtual void OnAddObstacle(const ObstacleLayerObstacleRecord& obstacle) = 0;
		virtual void OnRemoveObstacle(const ObstacleLayerObstacleRecord& obstacle) = 0;
		virtual void OnUpdateObstacle(const ObstacleLayerObstacleRecord& obstacle) = 0;
	};
	typedef List<ObstacleLayer::IObserver*> ObserverContainer;

	/*! Registers an IObserver that will be invoked each time an Obstacle is added, removed or updated in the World. */
	void RegisterObstacleLayerObserver(IObserver* observer);
	
	/*! Unregisters an IObserver previously registered using RegisterObstaclelayerObserver(). */
	void UnregisterObstacleLayerObserver(IObserver* observer);

public:
	/*! This class is used internally by ObstacleLayer to store all information related to each NavMesh
		on which the obstacle outlines in the ObstacleLayer will be projected.
		\ingroup kyruntime_mesh */
	class LayerNavMeshRecord
	{
		KY_DEFINE_NEW_DELETE_OPERATORS

	public:
		/*! \label_constructor For internal use. */
		inline LayerNavMeshRecord();

		/*! \label_constructor Called transparently by ObstacleLayer::AddMesh(). 
			For details on the parameters, see ObstacleLayer::AddMesh(). */
		LayerNavMeshRecord(const NavMesh* navMesh, Database* db);

		/*! \label_constructor For internal use. */
		inline LayerNavMeshRecord(const LayerNavMeshRecord& ob) { *this = ob; }

		/*! Copies all class members from the specified LayerNavMeshRecord to this object. */
		inline LayerNavMeshRecord& operator=(const LayerNavMeshRecord& ob);

	public:
		const NavMesh* m_navMesh; /*!< Stores the pointer passed to the \c meshPtr parameter of ObstacleLayer::AddMesh. */
		NavMeshQueryServerWrapper* m_queryServer; /*!< Stores a pointer to the NavMeshQueryServerWrapper retrieved from the Database specified in the call to ObstacleLayer::AddMesh. */

		// For each mesh we store information about projected obstacles in m_projections.
		// m_projections[m_beginId] contains information about the first obstacle that is
		// projected in this mesh. m_projections[m_endId] contains information about the
		// last obstacle that is projected in this mesh.
		// NB: if m_beginId == m_endId == KyUInt32MAXVAL, there is no obstacle projected in that mesh.
		KyUInt32 m_beginId, m_endId; /*!< For internal use. Do not modify. */

		// When a mesh is loaded, existing obstacles must be projected in that mesh, even
		// if their m_hasChanged attribute is false. Of course these projections must be
		// time-sliced. To do that we set m_curObstItWhenAdded to m_curObstIt-- when a mesh is 
		// loaded. Then, we project obstacle into this mesh (even if their m_hasChanged
		// attribute is false) until we hit m_curObstItWhenAdded. Once this process is over, we
		// invalidate m_curObstItWhenAdded.
		ObstacleIt m_curObstItWhenAdded; /*!< For internal use. Do not modify. */
	};

protected:
	/*! This class is used internally by ObstacleLayer to store all information
		related to each ObstacleProjectionVolume on which the obstacle outlines
		in the ObstacleLayer will be projected.
		\ingroup kyruntime_mesh */
	class LayerObstacleProjectionVolumeRecord
	{
		KY_DEFINE_NEW_DELETE_OPERATORS

	public:
		/*! \label_constructor For internal use. */
		inline LayerObstacleProjectionVolumeRecord();

		/*! \label_constructor Called transparently by
			ObstacleLayer::RegisterObstacleProjectionVolume().
			For details on the parameters, see ObstacleLayer::AddMesh(). */
		LayerObstacleProjectionVolumeRecord(ObstacleProjectionVolume* obstacleProjectionVolume);

		/*! \label_constructor For internal use. */
		inline LayerObstacleProjectionVolumeRecord(const LayerObstacleProjectionVolumeRecord& other) { *this = other; }

		/*! Copies all class members from the specified LayerObstacleProjectionVolumeRecord to this object. */
		inline LayerObstacleProjectionVolumeRecord& operator=(const LayerObstacleProjectionVolumeRecord& other);

	public:
		ObstacleProjectionVolume* m_projectionVolume; /*!< Stores the pointer passed to the \c pathObject parameter of ObstacleLayer::RegisterObstacleProjectionVolume. */

		// When a ObstacleProjectionVolume is added, existing obstacles must be projected in that ObstacleProjectionVolume,
		// even if their m_hasChanged attribute is false. Of course these projections must be
		// time-sliced. To do that we set m_curObstItWhenAdded to m_curObstIt-- when a ObstacleProjectionVolume is
		// added. Then, we project obstacle into this ObstacleProjectionVolume (even if their m_hasChanged
		// attribute is false) until we hit m_curObstItWhenAdded. Once this process is over, we
		// invalidate m_curObstItWhenAdded.
		ObstacleIt m_curObstItWhenAdded; /*!< For internal use. Do not modify. */
	};

public:
	/*! Defines a type for a List composed of LayerNavMeshRecord objects. */
	typedef List<LayerNavMeshRecord> MeshRecordList;

	/*! Defines a type for a bi-directional iterator that can be used to iterate through a List composed of LayerNavMeshRecord objects.
		See the List and List::Iterator classes for usage. */
	typedef List<LayerNavMeshRecord>::Iterator MeshIt;
	/*! Defines a type for a bi-directional iterator that can be used to iterate through a List composed of LayerNavMeshRecord objects.
		See the List and List::Iterator classes for usage. */
	typedef List<LayerNavMeshRecord>::ConstIterator ConstMeshIt;
public:
	/*! This class is used internally by ObstacleLayer to store information related to the projection
		of a specific obstacle onto a specific NavFloor.
		\ingroup kyruntime_mesh */
	class LayerProjectionRecord
	{
		KY_DEFINE_NEW_DELETE_OPERATORS
	public:
		/*! \label_constructor For internal use. */
		inline LayerProjectionRecord() {}

		/*! \label_constructor For internal use. */
		inline LayerProjectionRecord(const LayerProjectionRecord& ob) { *this = ob; }

		/*! Copies all class members from the specified LayerProjectionRecord to this object. */
		inline LayerProjectionRecord& operator=(const LayerProjectionRecord& ob);

	public:
		NavFloorPtr m_navFloorPtr; /*!< Stores the NavFloor on which the obstacle is projected. */ 
		ObstacleIt m_obsIt; /*!< Stores an iterator that points to the projected obstacle within the ObstacleLayer::m_obstacles list. */
	};

	/*! This class stores a pair of KyUInt32 values, and is used internally by ObstacleLayer to represent a range of elements
		within the ObstacleLayer::m_projections list. The #a member is used to represent the first element in a range, and the
		#b member is used to represent the last element in a range.
		\ingroup kyruntime_mesh */
	class UInt32Pair
	{
		KY_DEFINE_NEW_DELETE_OPERATORS
	public:
		inline UInt32Pair() : a(KyUInt32MAXVAL), b(KyUInt32MAXVAL) {}
		inline UInt32Pair(KyUInt32 a, KyUInt32 b) : a(a), b(b) {}
		inline UInt32Pair(const UInt32Pair& ob) { *this = ob; }

		inline UInt32Pair& operator=(const UInt32Pair& ob) { a=ob.a; b=ob.b; return *this; }
	public:
		KyUInt32 a, b; /*!< Stores a KyUInt32 maintained by this class. */
	};

	/*! This class is used internally by ObstacleLayer to compare the NavFloorPtr values assigned to different 
		instances of LayerProjectionRecord.
		\ingroup kyruntime_mesh */
	struct ProjectionComparator
	{
		KY_DEFINE_NEW_DELETE_OPERATORS
	public:
		/*! Returns #KY_TRUE if the NavFloorPtr assigned to \c r1 is less than the NavFloorPtr assigned to \c r2, or
			#KY_FALSE otherwise. */
		bool operator()(const LayerProjectionRecord& r1, const LayerProjectionRecord& r2) const
		{
			return r1.m_navFloorPtr < r2.m_navFloorPtr;
		}
		ProjectionComparator& operator=(const ProjectionComparator&) { return *this; }
	};

public:
	KY_DECLARE_NAVMESHLAYER(ObstacleLayer)

public:
	/*! \label_constructor */
	ObstacleLayer(NavMeshLayerManager& manager, const char* name);

	/*! \label_virtualdestructor */
	virtual ~ObstacleLayer();

	/*! Creates a new IObstacleAccessor or INavMeshAccessor of the specified class, which will provide obstacle outlines or \NavMeshes 
		to the ObstacleLayer. The lifespan of the newly created object is managed by the ObstacleLayer.
		\tparam DerivedAccessor		The class name of the INavMeshAccessor or IObstacleAccessor to create.
		\param[out] createdAccessor	Stores a pointer to the newly created object.
		\return A #KyResult that indicates the success or failure of the request. */
	template <class DerivedAccessor> KyResult CreateAccessor(DerivedAccessor* & createdAccessor) 
	{ 
		KyResult error = Kaim::Result::Unknown;
		createdAccessor = static_cast<DerivedAccessor*>(CreateAccessor(DerivedAccessor::Class, error)); 
		return error;
	}

	/*! Creates a new IObstacleAccessor of the specified class, which will provide obstacle outlines
		to the ObstacleLayer. The lifespan of the newly created object is managed by the ObstacleAccessor.
		\param accessorClass		The MetaClass of the IObstacleAccessor to create.
		\param[out] error			Updated to indicate the success or failure of the request.
		\return A pointer to the newly created object. */
	IObstacleAccessor* CreateAccessor(ObstacleAccessorClass& accessorClass, KyResult & error);

	/*! Creates a new INavMeshAccessor of the specified class, which will provide \NavMeshes
		to the ObstacleLayer. The lifespan of the newly created object is managed by the ObstacleAccessor.
		\param accessorClass		The MetaClass of the INavMeshAccessor to create.
		\param[out] error			Updated to indicate the success or failure of the request.
		\return A pointer to the newly created object. */
	INavMeshAccessor* CreateAccessor(NavMeshAccessorClass& accessorClass, KyResult & error);


	//////////////////////////////////////////////////////////////////////////
	// INavMeshLayer interface
	//////////////////////////////////////////////////////////////////////////

	/*! \copydoc WorldService::Update()
		\pp This implementation of WorldService::Update() first calls the IObstacleAccessor::GetInput() method of each IObstacleAccessor
		maintained by the ObstacleLayer, in order to update the list of obstacles maintained by the ObstacleLayer. Then, it updates the
		obstacle projections maintained by the ObstacleLayer: newly created obstacles are projected onto the necessary \NavFloors, obstacles
		that have been removed from the ObstacleLayer have their projections removed, and obstacles whose outlines have changed are re-projected. */
	virtual void Update();

	virtual KyBool CanGo(const NavFloorPtr& navFloorPtr, const Vec3f& startPoint, const Vec3f& destPoint, KyFloat32 radius) const;

	virtual KyBool TraceLine(const NavFloorPtr& navFloorPtr, const Vec3f& startPoint, const Vec3f& destPoint, KyFloat32 radius,
								Vec3f& lastValidPoint,NavMeshLayerLocation* location) const;

	virtual KyBool GetAllLocations(const NavFloorPtr& navFloorPtr, const Vec3f& startPoint, const Vec3f& destPoint, KyFloat32 radius,
								NavMeshLayerLocationList& locations) const;

	virtual KyBool IsValidPosition(const NavFloorPtr& navFloorPtr, const Vec3f& pos, NavMeshLayerLocation* location) const;


	//////////////////////////////////////////////////////////////////////////
	// NavMesh & Database management
	//////////////////////////////////////////////////////////////////////////

	/*! Adds a new NavMesh to the list of \NavMeshes on which this ObstacleLayer projects its obstacles.
		This method is typically called by an INavMeshAccessor configured for this ObstacleLayer.
		\param navMesh				The NavMesh to be added to this ObstacleLayer.
		\param db					The Database that contains the NavMesh.
		\return #KY_TRUE if the NavMesh was successfully added, or #KY_FALSE otherwise. */
	virtual KyBool AddMesh(const NavMesh* navMesh, Database* db = KY_NULL);

	/*! Removes the specified NavMesh from the list of \NavMeshes on which this ObstacleLayer projects its obstacles. This method
		is typically called by an INavMeshAccessor configured for this ObstacleLayer.
		\return #KY_TRUE if the NavMesh was successfully removed, or #KY_FALSE otherwise. */
	virtual KyBool RemoveMesh(const NavMesh* navMesh);

	/*! If the specified axis-aligned bounding box intersects the bounding box around an obstacle, this method marks that obstacle
		as having been updated. For internal use. */
	void TouchObstaclesInAABB(const Box2f& aabb);


	//////////////////////////////////////////////////////////////////////////
	// Obstacle management
	//////////////////////////////////////////////////////////////////////////

	/*! Adds a new ObstacleLayerObstacleRecord to the ObstacleLayer to represent a new obstacle outline. Because most of the obstacle characteristics
		accepted by this method are pointers to data maintained in another location or object (such as in a ProjectedOutline assigned to an Obstacle),
		you do not have to update the ObstacleLayer directly when the points defining an obstacle's outline change. For example, the outline of
		the obstacle is retrieved from the pointer you specify in the \c points parameter; therefore, when the obstacle moves, you do not have to
		update the ObstacleLayer explicitly. The new outline is retrieved from the original points.
		\pp If you use the Obstacle management system provided by the ObstacleManager, this method is called transparently by the ObstacleManager
		when your \Obstacles need to be added to an ObstacleLayer. You do not have to call it yourself.
		\param body				A pointer to a Body to be stored in the ObstacleLayerObstacleRecord. Use a pointer to any valid Body, or #KY_NULL.
		\param points			A pointer to an array of Vec3f objects that reflects the current outline of the obstacle.
		\param pointCount		A pointer to a KyUInt32 that the ObstacleLayer can retrieve at each frame to determine the number of Vec3f objects
								contained in the \c points array.
		\param thickness		A pointer to a KyFloat32 that maintains the vertical thickness of the obstacle.
		\param navFloorPtrs		A pointer to an array of NavFloorPtr objects, which are used by the ObstacleLayer to store the \NavFloors in
								which this obstacle is projected. You do not need to update this array yourself; you need only to dimension the
								array and pass it to the ObstacleLayer. This array should be as large as the maximum number of \NavFloors the
								obstacle might possibly intersect. If your obstacle is relatively small, less than the width of a NavFloor in
								diameter, an array size of four is typically acceptable, as each small obstacle can intersect a maximum of four
								different \NavFloors if it lies at a corner where four NavFloor meet. Larger obstacles need larger arrays. In addition,
								the position and shape of the obstacles become increasingly relevant as the size of the obstacles increases. For example,
								in the diagram below, the blue obstacle intersects a total of six \NavFloors, indicated by the filled squares. However,
								this is just an accident of position; if it is offset to the position indicated by the magenta outline, it will
								intersect 14. The array must be sized to the largest value that can occur at runtime.
								\image html graphics\obstacleslayer_localmaps.png
								You should count any \NavFloors that are entirely enclosed by the obstacle outline. For example, in the diagram above,
								the brown obstacle requires an array of 12 NavFloorPtr objects: 10 for the maps intersected by the outline, and
								two for the enclosed \NavFloors. Note that if this obstacle can be rotated freely at runtime, it may require an even
								larger array that allows up to 17 elements, as shown by the orange outline.
		\param navFloorCount	The number of NavFloorPtr objects in the \c navFloors array.
		\param hasChanged		A pointer to a KyBool that the ObstacleLayer consults at each frame to determine whether or not the outline has changed
								in the current frame. If the value of this KyBool is #KY_TRUE, the ObstacleLayer re-projects the outline of the obstacle, and re-sets
								this flag to #KY_FALSE when it has finished. If this value is set to #KY_FALSE when you add your obstacle, it will not be projected
								in the ObstacleLayer until the value changes to #KY_TRUE. If you want your new obstacle to projected immediately, make sure that
								this value is already set to #KY_TRUE before you call AddObstacle().
		\param userData			An optional void pointer that provides a unique identifier for the obstacle. If you conduct a TraceLine() or GetAllLayerLocations()
								query against this ObstacleLayer, or against a LayerQuery or NavMeshQueryServerWrapper to which this ObstacleLayer has
								been added, and if a collision is detected, the NavMeshLayerLocation wrapper you pass to the collision test will be
								updated to contain this same pointer. You can use this value to identify the obstacle encountered by the collision test.
								In addition, if you write a custom pre-merger modifier for the LpfManager, you can retrieve this value in order to
								determine how you treat each obstacle during the pre-merge operation.
		\return A bi-directional iterator that points to the new obstacle within the #m_obstacles list. */
	ObstacleIt AddObstacle(Body *body, Vec3f* points, KyUInt32* pointCount, KyFloat32* thickness,
							NavFloorPtr* navFloorPtrs, KyUInt32 navFloorCount, KyBool* hasChanged, void* userData = KY_NULL);

	/*! Removes the specified outline from this ObstacleLayer.
		\param ObstacleIt			The ObstacleIt returned by the call to AddObstacle() at the time this obstacle was added to the ObstacleLayer. */
	void RemoveObstacle(ObstacleIt);

	/*! Removes the specified outline from all projections in this ObstacleLayer. For internal use.
		\param ObstacleIt			The ObstacleIt returned by the call to AddObstacle() at the time this obstacle was added to the ObstacleLayer. */
	void RemoveObstacleFromProjection(ObstacleIt);

	/*! Retrieves the number of outlines maintained by this ObstacleLayer. */
	KyUInt32 GetObstacleCount() const { return m_obstacles.GetCount(); }


	//////////////////////////////////////////////////////////////////////////
	// Misc
	//////////////////////////////////////////////////////////////////////////

	/*! Clears and recomputes all projections. */
	virtual void ResetAll();

	// Rendering function
	/*! Renders all projected obstacles in dark green, and all obstacles that have not been projected in dark red.
		\param mask					A bit mask composed of the flags in the #RenderFlag enumeration. Default renders only the obstacles. */
	virtual void Render(KyUInt32 mask=0) const;


	/*! Determines the maximum number of \NavFloors that the ObstacleLayer can update at each frame. When this limit is reached, any
		obstacles that have not yet been re-projected in the current frame will be treated first in the next frame.
		\pp If the update of your ObstacleLayer takes too much CPU time at each frame, you can lower the value of this parameter to
		force the projections to be time-sliced more aggressively.
		\acceptedvalues	any positive value
		\defaultvalue	\c 256 */
	void SetMaxProjectionsPerFrame(KyUInt32 maxProjectionsPerFrame = 256) { m_maxProjectionsPerFrame = maxProjectionsPerFrame; }

	/*! Retrieves the value set through a call to SetMaxProjectionsPerFrame(). */
	KyUInt32 GetMaxProjectionsPerFrame() const { return m_maxProjectionsPerFrame; }


	/*! Retrieves the first LayerProjectionRecord in the specified NavFloor. You can use this method to loop through all obstacles
		projected onto that NavFloor.
		For example: \code
		for (LayerProjectionRecord* proj=GetFirstProj(floor, meshIt); proj; proj=GetNextProj()) { ... } \endcode
		\param navFloorPtr			Identifies the NavFloor whose projections you want to retrieve.
		\param meshIt				A MeshIt that points to the NavMesh that contains the \c navFloorPtr. */
	LayerProjectionRecord* GetFirstProj(const NavFloorPtr& navFloorPtr, ConstMeshIt meshIt) const;

	/*!	Retrieves the next projected obstacle in the NavFloor identified in the last call to GetFirstProj(). Must be preceded by a call to GetFirstProj(). */
	LayerProjectionRecord* GetNextProj() const;

	/*!	Flags the specified obstacle for immediate projection in the next call to ObstacleLayer::Update(), instead of allowing the time-slicing
		mechanism of the ObstacleLayer to defer the obstacle projection to a subsequent frame.
		\param it					The ObstacleIt returned by the call to AddObstacle() at the time the obstacle was added to the ObstacleLayer. */
	KyBool AddObstacleToUpdateImmediately(ObstacleIt it)
	{
		if (m_obstaclesToUpdateImmediately.Insert(it).IsValid())
			return KY_TRUE;
		return KY_FALSE;
	}


	/************************************************************************/
	/* ObstacleProjectionVolumes                                            */
	/************************************************************************/

	/*! For internal use. */
	virtual KyResult RegisterObstacleProjectionVolume(ObstacleProjectionVolume* obstacleProjectionVolume);

	/*! For internal use. */
	virtual KyResult UnregisterObstacleProjectionVolume(ObstacleProjectionVolume* obstacleProjectionVolume);

	/*! Retrieves the list of IObstacleAccessor objects that have been created for this ObstacleLayer. */
	const List<IObstacleAccessor*>& GetObstaclesAccessor() const { return m_obstaclesAccessor; }

	/*! Retrieves a bi-directional iterator that points to the first IObstacleAccessor in the list of IObstacleAccessor objects
		that have been created for this ObstacleLayer. */
	List<IObstacleAccessor*>::Iterator GetObstaclesAccessorIterator() { return m_obstaclesAccessor.GetFirst(); }
	/*! Retrieves a const bi-directional iterator that points to the first IObstacleAccessor in the list of IObstacleAccessor objects
		that have been created for this ObstacleLayer. */
	List<IObstacleAccessor*>::ConstIterator GetObstaclesAccessorIterator() const { return m_obstaclesAccessor.GetFirst(); }

	/*! Retrieves a bi-directional iterator that points to the first obstacle record added to this ObstacleLayer. */
	ObstacleIt GetFirstObstacle() { return m_obstacles.GetFirst(); }
	/*! Retrieves a const bi-directional iterator that points to the first obstacle record added to this ObstacleLayer. */
	ConstObstacleIt GetFirstObstacle() const { return m_obstacles.GetFirst(); }

	/*! Retrieves a bi-directional iterator that points to the first NavMesh added to this ObstacleLayer. */
	MeshIt GetFirstNavMesh() { return m_meshes.GetFirst(); }
	/*! Retrieves a const bi-directional iterator that points to the first NavMesh added to this ObstacleLayer. */
	ConstMeshIt GetFirstNavMesh() const { return m_meshes.GetFirst(); }

	/*! Retrieves a const bi-directional iterator that points to the specified NavMesh within the list of \NavMeshes added to this ObstacleLayer. */
	ConstMeshIt FindMeshIt(const NavMesh* navMesh) const;


protected:
	virtual KyBool UpdateObstacle(ObstacleIt curObsIt, KyUInt32& projectionCount);
	virtual KyBool ProjectObstacle(ObstacleIt curObsIt, MeshIt meshIt);
	KyBool AddNavFloor(ObstacleIt curObsIt, MeshIt meshIt, const NavFloorPtr& navFloorPtr);
	KyUInt32 UpdateFreeSlots_Get(LayerNavMeshRecord& layerNavMeshRecord);
	KyUInt32 UpdateFreeSlots_Enlarge(MeshIt meshIt);
	void UpdateFreeSlots_Remove(KyUInt32 id, KyBool merge);
	inline KyInt32 GetSortStatus(MeshIt meshIt, KyUInt32 recordId, KyUInt32 newId) const;
	inline KyInt32 IsSorted(MeshIt meshIt, KyUInt32 recordId) const;
	void Sort(MeshIt meshIt, KyUInt32 recordId);

	void ResizeMaxProjection(KyUInt32 newSize);

#ifdef KYDEBUG
	void CheckIntegrity(const char* context, KyBool dumpTables = KY_FALSE, KyBool checkSort = KY_FALSE);
	bool CheckIntegrity_1(List<UInt32Pair>& check_freeSlots, const LayerNavMeshRecord& mIt) const;
	bool CheckIntegrity_Sort(const char* context, const LayerNavMeshRecord& mIt) const;
	bool CheckIntegrity_ListEqual(const List<UInt32Pair>& freeSlots, const List<UInt32Pair>& check_freeSlots) const;
	void CheckIntegrity_Dump(const char* context, bool errorDetected, const List<UInt32Pair>& check_freeSlots) const;
	List<UInt32Pair> m_check_freeSlots;
#endif // KYDEBUG

protected:
	ObserverContainer m_observers;

	List<INavMeshAccessor*> m_navMeshAccessor; /*< The list of INavMeshAccessor objects that manage the \NavMeshes considered by this ObstacleLayer. */
	List<IObstacleAccessor*> m_obstaclesAccessor; /*< The list of IObstacleAccessor objects that manage the obstacles maintained by this ObstacleLayer. */

	MeshRecordList m_meshes; /*< The list of \NavMeshes currently considered by this ObstacleLayer to project obstacles onto. Do not modify directly. */
	SharedPoolList<ObstacleProjectionVolume*>::Pool m_projectionVolumePointerPool;
	List<LayerObstacleProjectionVolumeRecord> m_projectionVolumes; /*< The list of \ObstacleProjectionVolumes currently considered by this ObstacleLayer to project obstacles into. Do not modify directly. */

	List<ObstacleLayerObstacleRecord> m_obstacles; /*< The list of obstacles currently maintained by this ObstacleLayer. Do not modify directly. */

	/* Projected obstacles are stored in m_projections. They are grouped by mesh
		(please refer to LayerNavMeshRecord::m_beginId/m_endId). m_freeSlots contains all the ranges of
		ids that are currently not used by any mesh. For each element,
		'a' is the index of the first free element, 'b' is the index of the last one.
		eg:
			m_projections = {m1_o1, m1_o2, 0, 0, m2_o1, 0, 0 }
			(m1_o1 is the projection of obstacle o1 in mesh m1)
			For m1, m_beginId = 0 and m_endId = 1
			For m2, m_beginId = 4 and m_endId = 4
			m_freeSlots = {{a = 2, b = 3}, {a = 5, b = 6}}
	*/
	List<UInt32Pair> m_freeSlots; /*< Identifies all ranges of free elements within the ::m_projections array. For internal use. */
	LayerProjectionRecord* m_projections; /*< The list of all obstacle projections maintained by this ObstacleLayer, grouped by NavMesh. */

	KyUInt32 m_maxPOvolumeProjections;
	KyUInt32 m_maxProjections;
	KyUInt32 m_maxProjectionsPerFrame; /*< Accessor for the \c MaxProjectionsPerFrame configuration parameter. */
	/* When the maximum number of obstacle projections has been reached in a frame, this member is set to the next obstacle that needs to be treated.
		This allows the ObstacleLayer to resume projecting obstacles in the next frame where it left off. */
	ObstacleIt m_curObsIt;

	List<ObstacleIt> m_obstaclesToUpdateImmediately; /*< A list of obstacles that must be updated immediately, in the next call to Update(). */
	List<MeshIt> m_recentlyAddedMeshes; /*< Stores a list of recently added \NavMeshes, upon which the ObstacleLayer must project all existing obstacles. */
	List<LayerObstacleProjectionVolumeRecord*> m_recentlyMovedProjectionVolumes; /*< Stores a list of recently added \ObstacleProjectionVolumes, in which the ObstacleLayer must project all existing obstacles. */

	/*	Used by IsValidPosition(). If the altitude of the point provided to IsValidPosition() is closer to an obstacle than the value of this member,
		then a collision is detected and the tested point is considered invalid. Set to 2.0f by default. */
	KyFloat32 m_maxAltitudeDifference;

protected:
	// m_findProj_xxx attributes are used internally in GetFirstProj/GetNextProj functions
	mutable NavFloorPtr m_findProj_navFloorPtr;
	mutable ConstMeshIt m_findProj_meshIt;
	mutable KyUInt32 m_findProj_index;
	mutable KyUInt32 m_findProj_index0;
	mutable KyBool m_findProj_negativeInc;

private:
	/* defined internally to generate the body of the two member functions CreateAccessor */
	template <class TAccessor, class TAccessorClass>
	TAccessor* CreateAccessor(KyResult & error, TAccessorClass& accessorClass, List<TAccessor*>& memberVarlist);
};


#include <kyruntime/mesh/obstaclelayer.inl>

} // namespace Kaim

#endif // KY_OBSTACLELAYER_H
