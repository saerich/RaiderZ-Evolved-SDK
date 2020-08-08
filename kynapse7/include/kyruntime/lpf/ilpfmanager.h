/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/


/*! \file
	\ingroup kyruntime_lpf */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_ILPFMANAGER_H
#define KY_ILPFMANAGER_H

#include <kyruntime/asyncmanagement/asyncmodule.h>
#include <kyruntime/core/timemanager.h>
#include <kypathdata/basesystem/taskregistry.h>
#include <kyruntime/mesh/obstaclelayer.h>
#include <kyruntime/mesh/polygonmerger_polygon.h>

#include <kypathdata/containers/list.h>
#include <kypathdata/containers/sortedarray.h>
#include <kyruntime/core/contiguousarray.h>
#include <kypathdata/navmesh/navfloor.h>


namespace Kaim
{

class PolygonMerger_Polygon;
class ILpfModule;
class ILpfPreMerger;
class ILpfPreMergerConfig;
class LpfAStarData;
class NavMeshManager;
class ObstacleLayer;


/*!	ILpfManager is an abstract base class for a WorldService that makes \Bots find and follow local paths that
	bypass aggregations of \Obstacles. Its main responsibility is to maintain a list of \LpfContexts, each of
	which computes and stores a set of \LpfAreas: outlines that represent aggregates of \Obstacles in the
	\gameOrSim world.
	\pp This class cannot be used directly. See the derived LpfManager class.
	\pp See also :USERGUIDE:"Using Obstacles" and :USERGUIDE:"Local PathFinding".
	\ingroup kyruntime_lpf */
class ILpfManager: public WorldService
{
public:
	class LpfContext;
	class LpfArea;
	class LpfPreAggregate;
	class LpfObstacleRecord;
	class LpfNavFloorRecord;


	//////////////////////////////////////////////////////////////////////////
	// LPF-specific constants
	///////////////////////////////////////////////////////
	/*! Sets the maximum number of intersection results that can be stored by LpfArea::GetAllPolylineIntersections(). */
	static const KyUInt32 PolylineIntersectionMaxCount = 1024;


	//////////////////////////////////////////////////////////////////////////
	// enums
	///////////////////////////////////////////////////////

	/*!	Used to indicate the result of a local path computation around an LpfArea. See LpfArea::GetLocalPath(). */
	enum LocalPathComputationResult {
		LPCR_ERROR, /*!< Indicates that an error occurred, preventing the calculation from being completed successfully. */
		LPCR_NOTFOUND, /*!< Indicates that no valid local path could be found to bypass the LpfArea. */
		LPCR_FOUND, /*!< Indicates that a valid local path was found to bypass the LpfArea. */
		LPCR_NOTAVAILABLE, /*!< Indicates that a local path may exist around the LpfArea, but it cannot currently be used. This may occur if a Bot is currently inside the LpfArea. */
		LPCR_DUNMMY__ = KYFORCEENUMSIZEINT
	};

	/*!	Used to indicate what information should be rendered in call to the Render() method of objects used in
		Local PathFinding. */
	enum RenderFlags {
		RF_BOUNDINGBOX = 1, /*!< Renders the bounding box of the object. Used by LpfArea::Render() and LpfPreAggregate::Render(). */
		RF_PASSABILITY = 2, /*!< Renders the passability of \LpfAreas: green if the LpfArea can be bypassed, red if not, or grey if the passability is unknown. Used by LpfArea::Render(). */
		RF_THIRDDIMENSION = 4, /*!< Renders the three-dimensional extents of \LpfAreas, if any. Used by LpfArea::Render(). */
		RF_PREAGGREGATES = 8, /*!< Renders the pre-aggregates that make up each LpfArea. Used by LpfArea::Render(). */
		RF_WORKINPROGRESS = 16, /*!< Renders \LpfObstacleRecords and \LpfPreAggregates that have not yet been merged into \LpfAreas, and \LpfAreas that have not yet been finalized. Used by ILpfManager::Render(). */
		RF_DUNMMY__ = KYFORCEENUMSIZEINT
	};

	/*!	Used to indicate the result of a polyline intersection computation around an LpfArea.
		See LpfArea::GetAllPolylineIntersections(). */
	enum GetAllPolylineIntersectionsResult {
		GAPIR_NOINTERSECTION, /*!< Indicates that the polyline does not intersect the LpfArea. */
		GAPIR_INTERSECTIONFOUND, /*!< Indicates that the polyline does intersect the LpfArea. */
		GAPIR_ERROR_INVALIDPOLYLINE, /*!< Indicates that the polyline is invalid. */
		GAPIR_ERROR_TOOMUCHINTERSECTIONS, /*!< Indicates that more intersections were found than #PolylineIntersectionMaxCount, and the reserved buffer has no more space. */
		GAPIR_DUMMY__ = KYFORCEENUMSIZEINT
	};



	/************************************************************************/
	/*                                                                      */
	/* Nested classes                                                       */
	/*                                                                      */
	/************************************************************************/

	//////////////////////////////////////////////////////////////////////////
	// PolylineNode
	///////////////////////////////////////////////////////

	/*!	Represents a single point in an outline around an LpfArea, LpfPreAggregate, or LpfObstacleRecord.
		Each instance of this class maintains its position in 3D space, and information about the passability
		between it and the next point in its outline.
		\ingroup kyruntime_lpf */
	class LpfPolylineNode: public VirtualMemObject
	{
	public:
		/*!	Used by LpfPolylineNode to store information about whether or not the direct line to a neighboring LpfPolylineNode
			in a polyline is clear. */
		class LpfEdgePassability: public VirtualMemObject
		{
		public:
			/*!	Used to indicate whether or not the direct line to a neighboring LpfPolylineNode is clear. */
			enum EdgePassability {
				EP_UNKNOWN, /*!< Indicates that the LpfPolylineNode does not know whether or not the direct line to its neighboring LpfPolylineNode is clear. */
				EP_PASSABLE, /*!< Indicates that the neighboring LpfPolylineNode in the polyline can be reached directly. */
				EP_IMPASSABLE, /*!< Indicates that the neighboring LpfPolylineNode in the polyline cannot be reached directly. */
				EP_DUNMMY__ = KYFORCEENUMSIZEINT
			};

		public:
			/*!	For internal use. */
			LpfEdgePassability(): m_passability(EP_UNKNOWN), m_terrainTypeMask(NavTerrainTypeMask_All), m_validityDate(0.0f) {}

			/*!	For internal use. */
			~LpfEdgePassability() {}

			/*!	Indicates whether or not the direct line to the neighboring LpfPolylineNode is clear. */
			EdgePassability GetPassability(LpfContext* cxt, NavTerrainTypeMask allowedTerrainTypes) const;

			/*!	Sets the passability to the neighboring LpfPolylineNode. For internal use. */
			void Update(EdgePassability passability, NavTerrainTypeMask allowedTerrainTypes, KyFloat32 validityDate)
			{
				m_passability = passability;
				m_terrainTypeMask = allowedTerrainTypes;
				m_validityDate = validityDate;
			}

		protected:
			EdgePassability m_passability;
			NavTerrainTypeMask m_terrainTypeMask;
			KyFloat32 m_validityDate;
		};

	public:
		/*!	For internal use. */
		LpfPolylineNode(): m_position(Vec3f(KyFloat32MAXVAL, KyFloat32MAXVAL, KyFloat32MAXVAL)) {}

		/*!	For internal use. */
		LpfPolylineNode(const Vec3f& position): m_position(position) {}

	public:
		Vec3f m_position; /*!< Stores the position of the point. */
		LpfEdgePassability m_clockwisePassability; /*!< Indicates whether or not the direct line is clear to reach the next LpfPolylineNode in the polyline that contains this LpfPolylineNode, moving clockwise. */
		LpfEdgePassability m_counterclockwisePassability; /*!< Indicates whether or not the direct line is clear to reach the next LpfPolylineNode in the polyline that contains this LpfPolylineNode, moving counter-clockwise. */
	};
	typedef SharedPoolList<LpfPolylineNode> LpfPolyline;
	typedef ChunkBasedPool<LpfPolyline>::Handle LpfPolylinePtr;


	//////////////////////////////////////////////////////////////////////////
	// LpfPolylineIntersection
	///////////////////////////////////////////////////////

	/*! For internal use. */
	struct LpfPolylineIntersection
	{
		LpfPolyline::Iterator m_edgeEndPointIt;
		Vec3f m_position;
		KyBool m_isNearBorder;
	};


	//////////////////////////////////////////////////////////////////////////
	// LpfPolylineIntersectionCompareNearestFrom
	///////////////////////////////////////////////////////

	/*! For internal use. */
	class CLpfPolylineIntersectionCompareNearestFrom
	{
		KY_DEFINE_NEW_DELETE_OPERATORS

	public:
		CLpfPolylineIntersectionCompareNearestFrom(const Vec3f& referencePosition) : m_referencePosition(referencePosition) {}

		inline bool operator()(const LpfPolylineIntersection& lpi1, const LpfPolylineIntersection& lpi2) const
		{
			return (SquareDistance(m_referencePosition, lpi1.m_position) < SquareDistance(m_referencePosition, lpi2.m_position));
		}

	protected:
		Vec3f m_referencePosition;
	};


	//////////////////////////////////////////////////////////////////////////
	// LpfPreAggregate
	///////////////////////////////////////////////////////

	/*!	This class represents an intermediate stage in the process of building \LpfAreas out of obstacle outlines.
		Within each NavFloor, any obstacles whose extended outlines overlap are merged together into an LpfPreAggregate
		by a pre-merger modifier that implements the ILpfPreMerger interface. These \LpfPreAggregates are then converted
		into \LpfAreas and merged with any overlapping \LpfAreas in adjacent \NavFloors.
		\pp For details, see :USERGUIDE:"Building LpfAreas".
		\ingroup kyruntime_lpf */
	class LpfPreAggregate: public VirtualMemObject
	{
	public:
		/*!	For internal use. */
		LpfPreAggregate();

		/*!	For internal use. */
		~LpfPreAggregate();

		/*!	Re-initializes the LpfPreAggregate. This releases all polylines and obstacle records assigned to the
			LpfPreAggregate. */
		void ReInit();

		/*!	Initializes #m_context, #m_polylines and #m_obstacles from the parameters provided. For internal use. */
		void Create(LpfContext *context, SharedPoolList<LpfPolylinePtr>::Pool& polylinesPool, SharedPoolList<LpfObstacleRecord*>::Pool& obstacleRecordPool);

		/*!	Adds the specified LpfObstacleRecord to the list of obstacles that make up the LpfPreAggregate. */
		KyBool AddObstacle(LpfObstacleRecord* obstacleRecord);

		/*!	Removes the specified LpfObstacleRecord from the list of obstacles that make up the LpfPreAggregate. */
		KyBool RemoveObstacle(LpfObstacleRecord* obstacleRecord);

		/*!	Adds the specified polyline to the list of polylines that make up the external contours of the LpfPreAggregate. */
		KyBool AddPolyline(LpfPolylinePtr polyline);

		/*!	Indicates whether or not the specified point lies within the area covered by the outline of the LpfPreAggregate.
			\param p				The point to test.
			\param check3D			Indicates whether or not the test takes into account the 3D complexity of the context.
									If #KY_FALSE, the test will consider the location of the tested point on the horozontal X and
									Y axes only. If #KY_TRUE, the test will consider also the location of the tested point on the
									vertical Z axis.
			\return #KY_TRUE if the point lies within the pre-aggregate outline, or #KY_FALSE otherwise. */
		KyBool IsInObstacleArea(const Vec3f& p, KyBool check3D) const;

		/*!	Draws the information contained in the LpfPreAggregate for debugging purposes.
			\param flags			A bit mask composed of elements from the ILpfManager::RenderFlags enumeration, which
									determines the information to be displayed. */
		void Render(KyUInt32 flags = 0) const;

		/*!	Draws the obstacles that make up the LpfPreAggregate for debugging purposes.
			\param colorRed			\label_redval
			\param colorGreen		\label_greenval
			\param colorBlue		\label_blueval */
		void RenderObstacles(KyUInt32 colorRed, KyUInt32 colorGreen, KyUInt32 colorBlue) const;

		/*! \label_getworld */
		World& GetWorld() { return m_context->GetWorld(); }

		/*! \label_getworld */
		const World& GetWorld() const { return m_context->GetWorld(); }

	private:
		const LpfPreAggregate& operator = (const LpfPreAggregate&); // No implementation.
		LpfPreAggregate(const LpfPreAggregate&); // No implementation.
	public:
		LpfContext* m_context; /*!< A pointer to the LpfContext using this LpfPreAggregate. Do not modify. */
		SharedPoolList<LpfPolylinePtr> m_polylines; /*!< The list of external contours of this LpfPreAggregate. Do not modify. */
		SharedPoolList<LpfObstacleRecord*> m_obstacles; /*!< The list of \LpfObstacleRecords used to build this LpfPreAggregate. Do not modify. */
		LpfNavFloorRecord* m_navFloorRecord; /*!< The NavFloor that contains the obstacles recorded in #m_obstacles. Do not modify. */
		LpfArea* m_area; /*!< The LpfArea that has been built from this LpfPreAggregate, if any. Do not modify. */
		Vec3f m_aabbMin; /*!< The minima of the bounding box around this LpfPreAggregate. Do not modify. */
		Vec3f m_aabbMax; /*!< The maxima of the bounding box around this LpfPreAggregate. Do not modify. */
		KyFloat32 m_timeStamp; /*!< The time at which the LpfPreAggregate was last modified. */
	};
	typedef ChunkBasedPool<LpfPreAggregate>::Handle LpfPreAggregatePtr;


	//////////////////////////////////////////////////////////////////////////
	// LpfArea
	///////////////////////////////////////////////////////

	/*!	Each instance of the LpfArea class represents an aggregation of dynamic obstacles that block Bots from moving freely.
		Each LpfArea maintains a set of polylines that indicate its external and internal contours, built by the LpfManager from
		the outlines contained in one or more \ObstacleLayers. The LpfArea class also offers an interface for testing the
		passibility of the contours: i.e. whether or not a Bot can successfully follow its external contour in order to get from
		one side of the LpfArea to the other.
		\pp See also :USERGUIDE:"Local PathFinding".
		\ingroup kyruntime_lpf */
	class LpfArea: public VirtualMemObject
	{
	public:
		/*!	Used to indicate the three-dimensional complexity of the LpfArea. */
		enum ThirdDimensionComplexity {
			TDC_2D, /*!< Indicates that the LpfArea is a two-dimensional structure. */
			TDC_2D5, /*!< Indicates that the LpfArea is a two-dimensional structure with a specific height. */
			//TDC_3D,
			TDC_DUNMMY__ = KYFORCEENUMSIZEINT
		};

	public:
		/*!	For internal use. */
		LpfArea(): m_context(KY_NULL), m_polylines(KY_NULL), m_preAggregates(KY_NULL), m_3dComplexity(TDC_2D), m_aabbMin(Vec3f(KyFloat32MAXVAL, KyFloat32MAXVAL, KyFloat32MAXVAL)), m_aabbMax(Vec3f(-KyFloat32MAXVAL, -KyFloat32MAXVAL, -KyFloat32MAXVAL)), m_timeStamp(KyFloat32MAXVAL) {}

		/*!	For internal use. */
		~LpfArea();

		/*!	Re-initializes the LpfArea. This releases all \LpfPreAggregates and polylines, which effectively
			erases the outline of the LpfArea, and resets the bounding box of the LpfArea. */
		virtual void ReInit();

		/*!	Initializes #m_context, #m_preAggregates and #m_polylines from the parameters provided. For internal use. */
		void Create(LpfContext* context, SharedPoolList<LpfPolylinePtr>::Pool& polylinesPool, SharedPoolList<LpfPreAggregatePtr>::Pool& preAggregatePool);

		/*!	Initializes a new, empty LpfArea to the match the outline of the specified LpfPreAggregate. For internal use. */
		KyBool InitWith(LpfPreAggregatePtr preAggregate);

		/*!	Removes the specified LpfPreAggregate from this LpfArea. For internal use. */
		KyBool RemovePreAggregate(LpfPreAggregatePtr preAggregate);

		/*!	Compute the nearest intersection between polyline \c pl of this LpfArea and the provided segment. For internal use. */
		KyBool GetFirstPolylineIntersection(
			const SharedPoolList<LpfPolylinePtr>::Iterator& pl,
			const Vec3f& from, const Vec3f& to, ILpfModule* lpfModule, LpfPolylineIntersection& intersection);

		/*!	Computes all intersections between polyline \c pl of this LpfArea and the provided segment. For internal use. */
		GetAllPolylineIntersectionsResult GetAllPolylineIntersections(
			const SharedPoolList<LpfPolylinePtr>::Iterator& pl,
			const Vec3f& from, const Vec3f& to, ILpfModule* lpfModule, KyUInt32& intersectionCount);

		/*!	Computes a local path that bypasses this LpfArea along its external contour. For internal use. */
		LocalPathComputationResult GetLocalPath(
			const SharedPoolList<LpfPolylinePtr>::Iterator& pl,
			const LpfPolyline::Iterator& entryEdgeEndPointIt,
			const LpfPolyline::Iterator& exitEdgeEndPointIt,
			const Vec3f& from, const Vec3f& to, ILpfModule* lpfModule, NavTerrainTypeMask allowedTerrainTypes,
			Vec3fList* localPath, KyFloat32& localPathCost, KyBool& didShortCut);

		/*!	Indicates whether or not the LpfArea can be bypassed by a Bot using the specified ILpfModule. For internal use. */
		KyBool CheckPassability(ILpfModule* lpfModule, NavTerrainTypeMask allowedTerrainTypes, KyBool updateFirst, KyBool updateLast);

		/*!	Indicates whether or not the specified point lies inside the outline of the LpfArea. */
		KyBool IsInObstacleArea(const Vec3f& p) const;

		/*!	Draws the information maintained by the LpfArea for debugging purposes.
			\param flags			A bit mask composed of elements from the ILpfManager::RenderFlags enumeration, which
									determines the information to be displayed. */
		void Render(KyUInt32 flags = 0) const;

		/*! \label_getworld */
		World& GetWorld()
		{
			return m_context->GetWorld();
		}

		/*! \label_getworld */
		const World& GetWorld() const
		{
			return m_context->GetWorld();
		}

		/*!	\label_getengine */
		Engine& GetEngine()
		{
			return GetWorld().GetEngine();
		}

		/*!	\label_getengine */
		const Engine& GetEngine() const
		{
			return GetWorld().GetEngine();
		}

	public:
		LpfContext* m_context; /*!< A pointer to the context using this LpfArea. Do not modify. */
		SharedPoolList<LpfPolylinePtr>* m_polylines; /*!< The list of external contours of this LpfArea. Do not modify. */
		SharedPoolList<LpfPreAggregatePtr>* m_preAggregates; /*!< The list of \LpfPreAggregates used to build this LpfArea. Do not modify. */
		ThirdDimensionComplexity m_3dComplexity; /*!< The three-dimensionality of this LpfArea. Do not modify. */
		Vec3f m_aabbMin; /*!< The minima of the bounding box around this LpfArea. Do not modify. */
		Vec3f m_aabbMax; /*!< The maxima of the bounding box around this LpfArea. Do not modify. */
		KyFloat32 m_timeStamp; /*!< The time at which the LpfArea was last modified. */
	};


	//////////////////////////////////////////////////////////////////////////
	// LPF NavFloorRecord
	///////////////////////////////////////////////////////

	/*!	Each instance of this class represents a single NavFloor that is known to contain one or more obstacles
		reflected in a LpfContext. Use of this class is mostly transparent.
		\ingroup kyruntime_lpf */
	class LpfNavFloorRecord: public VirtualMemObject
	{
	public:
		/*!	For internal use. */
		LpfNavFloorRecord();

		/*!	For internal use. */
		~LpfNavFloorRecord();

		/*!	Initializes #m_preAggregates. For internal use. */
		void Create(SharedPoolList<LpfPreAggregatePtr>::Pool& preAggregatePool);

		/*!	Releases #m_preAggregates. For internal use. */
		KyBool ReleasePreAggregates(LpfContext* context);

	public:
		NavFloorPtr m_navFloorPtr; /*!< The Ptr of the NavFloor related to this record. Do not modify. */
		SharedPoolList<LpfPreAggregatePtr> m_preAggregates; /*!< A list of \LpfPreAggregates that reflect the obstacles projected in this NavFloor. Do not modify. */
	};


	//////////////////////////////////////////////////////////////////////////
	// LpfObstacleRecord
	///////////////////////////////////////////////////////

	/*!	Each instance of this class represents a single obstacle reflected in a LpfContext.
		\pp The LpfContext creates a pool of these objects at initialization time. Each time an obstacle is added
		to one of the ObstacleLayers configured for the LpfContext, an instance of this class is automatically
		retrieved from the pool and updated according to the information stored about the obstacle in the corresponding
		instance of the ObstacleLayer::ObstacleRecord class maintained in the ObstacleLayer. These obstacles are
		then merged into LpfPreAggregate objects by a pre-merger modifier (a class that implements the
		ILpfPreMerger interface).
		\pp Use of this class is mostly transparent, unless you create a custom implementation of ILpfPreMerger.
		\ingroup kyruntime_lpf */
	class LpfObstacleRecord: public VirtualMemObject
	{
	public:
		/*!	For internal use. */
		LpfObstacleRecord():
			m_points(KY_PTRMAXVAL<Vec3f>()),
			m_pointCount(0),
			m_userData(KY_NULL),
			m_firstProjNavFloor(KY_NULL),
			m_extendedPolyline(),
			m_preAggregate(KY_NULL) {}

		/*!	For internal use. */
		~LpfObstacleRecord() {}

	public:
		/*!	A pointer to the ObstacleLayerObstacleRecord::m_points array for the obstacle in the ObstacleLayer.
			Do not modify. */
		const Vec3f* m_points;
		/*!	A pointer to the ObstacleLayerObstacleRecord::m_pointCount member for the obstacle in the ObstacleLayer.
			Do not modify. */
		KyUInt32 m_pointCount;
		/*!	A pointer to the ObstacleLayerObstacleRecord::m_userData member for the obstacle in the ObstacleLayer.
			Do not modify. */
		void* m_userData;
		/*!	A pointer to the NavFloor that contains the obstacle. */
		LpfNavFloorRecord* m_firstProjNavFloor;
		/*!	The extended outline of the obstacle: a polyline that follows the contour of the obstacle at a distance from
			the obstacle equal to the value of LpfContext::m_edgeRadius. This
			extended outline should be used by the pre-merger modifier to build pre-aggregates from this obstacle. This
			extended outline is calculated automatically. Do not modify. */
		LpfPolylinePtr m_extendedPolyline;
		/*!	The exact position of the first corner when the extended polyline is computed. It should be used by
			the pre-merger modifier to dispatch a posteriori the obstacles into the pre-aggregates accordingly
			to the contours obtained there if the merge tool is not able to keep this information.
			This position is calculated automatically. Do not modify. */
		Vec3f m_firstCornerPositionAtPolylineComputationTime;
		/*!	A pointer to the LpfPreAggregate that represents this obstacle. Updated automatically when the obstacle is assigned
			to an LpfPreAggregate through a call to ILpfManager::LpfPreAggregate::AddObstacle(). Do not modify. */
		LpfPreAggregate* m_preAggregate;
	};


	//////////////////////////////////////////////////////////////////////////
	// LpfPreAggregateComputationRequest
	///////////////////////////////////////////////////////

	/*!	When the LpfContext determines that it needs to merge the obstacle outlines contained in its \ObstacleLayers into
		\LpfAreas, it creates instances of this class to represent discrete requests that it sends to a pre-merger modifier
		(a class that derives from IPreMergerModifier). The pre-merger modifier is responsible for retrieving the LpfObstacleRecord
		objects stored in #m_obstacleRecords, merging any overlapping outlines, and saving the results as LpfPreAggregatePtr objects
		in the #m_preMergeResults list.
		See also :USERGUIDE:"Building LpfAreas".
		\ingroup kyruntime_lpf */
	class LpfPreAggregateComputationRequest
	{
		KY_DEFINE_NEW_DELETE_OPERATORS
	public:
		/*! \label_constructor For internal use. */
		LpfPreAggregateComputationRequest(): m_context(KY_NULL), m_navFloorRecord(KY_NULL), m_obstacleRecords(KY_NULL), m_preMergeResults(KY_NULL), m_completed(KY_TRUE) {}
		/*! \label_destructor */
		~LpfPreAggregateComputationRequest();

		/*! For internal use. */
		void Init(LpfContext* context);

		/*! \label_clear For internal use. */
		void Clear();

		/*! Clears the polylines maintained by the list of obstacle records. For internal use. */
		void ReleaseObstacleRecordPolylines();

		/*! Clears the list of pre-aggregates. For internal use. */
		void ReleasePreAggregates();

		/*! Called by LpfContext::Render() to render merges in process. */
		void Render();

	public:
		LpfContext* m_context; /*!< The LpfContext making this LpfPreAggregateComputationRequest. */
		LpfNavFloorRecord* m_navFloorRecord; /*!< Indicates the NavFloor that contains the obstacle records in this request. */
		SharedPoolList<LpfObstacleRecord*>* m_obstacleRecords; /*!< Provides the list of all obstacles that need to be merged. */
		SharedPoolList<LpfPreAggregatePtr>* m_preMergeResults; /*!< Stores a list of LpfPreAggregatePtr objects that contain the merged outlines computed by the ILpfPreMerger. */
		KyBool m_completed; /*!< Indicates whether or not the ILpfPreMerger modifier has completed merging all obstacle records in this request. */
	};


	//////////////////////////////////////////////////////////////////////////
	// LpfContext
	///////////////////////////////////////////////////////

	/*!	Each instance of this class represents a single context for Local PathFinding: an object that computes
		and stores a coherent set of \LpfAreas that represent aggregates of intersecting \Obstacles in the \gameOrSim.
		The PathFinder takes these \LpfAreas into account during path computation and path following, to make \Bots
		bypass complex arrangements of dynamic obstacles that lie in their way.
		\pp See also :USERGUIDE:"Using Obstacles" and :USERGUIDE:"Local PathFinding".
		\ingroup kyruntime_lpf */
	class LpfContext
	{
		KY_DEFINE_NEW_DELETE_OPERATORS
	protected:
		class ObstacleLayerObserver_UsedInLpf : public ObstacleLayer::IObserver
		{
		public:
			ObstacleLayerObserver_UsedInLpf() : m_lpfContext(KY_NULL) {}
			void Init(LpfContext& lpfContext) { m_lpfContext = &lpfContext; }

			virtual void OnAddObstacle(const ObstacleLayerObstacleRecord& obstacle) { m_lpfContext->AddNavFloorEvents(obstacle); }
			virtual void OnRemoveObstacle(const ObstacleLayerObstacleRecord& obstacle) { m_lpfContext->AddNavFloorEvents(obstacle); }
			virtual void OnUpdateObstacle(const ObstacleLayerObstacleRecord& obstacle) { m_lpfContext->AddNavFloorEvents(obstacle); }
		protected:
			LpfContext* m_lpfContext;
		};
		friend class ObstacleLayerObserver_UsedInLpf;

		class ObstacleLayerObserver_NotUsedInLpf : public ObstacleLayer::IObserver
		{
		public:
			ObstacleLayerObserver_NotUsedInLpf() : m_lpfContext(KY_NULL) {}
			void Init(LpfContext& lpfContext) { m_lpfContext = &lpfContext; }

			virtual void OnAddObstacle(const ObstacleLayerObstacleRecord& ) { m_lpfContext->SetPathDataUpdateDate(); }
			virtual void OnRemoveObstacle(const ObstacleLayerObstacleRecord& ) { m_lpfContext->SetPathDataUpdateDate(); }
			virtual void OnUpdateObstacle(const ObstacleLayerObstacleRecord& ) { m_lpfContext->SetPathDataUpdateDate(); }
		protected:
			LpfContext* m_lpfContext;
		};
		friend class ObstacleLayerObserver_NotUsedInLpf;

	public:
		/*!	\label_constructor For internal use. */
		LpfContext();
		/*!	\label_destructor For internal use. */
		~LpfContext();

		/*!	Used by MergePolygons() to indicate the result of a requested merge. */
		enum MergeResult {
			MR_NoInteraction, /*!< Indicates that the polylines provided to MergePolygons() do not intersect at all. No merging was performed. */
			MR_BothIntervienes, /*!< Indicates that the polylines provided to MergePolygons() intersect in part, and that both contribute to the final merged polyline. */
			MR_FirstSubsumesSecond, /*!< Indicates that the second polyline provided to MergePolygons() is wholly contained inside the first. The merged polyline therefore reflects only the first polyline provided. */
			MR_SecondSubsumesFirst, /*!< Indicates that the first polyline provided to MergePolygons() is wholly contained inside the second. The merged polyline therefore reflects only the second polyline provided. */
			MR_Error, /*!< Indicates that an error occurred that prevented the polygons from being merged correctly. */
			MR_DUMMY__ = KYFORCEENUMSIZEINT
		};

	public:
		/*!	This class is used by LpfContext to represent an event that is fired when the obstacles in an NavFloor
			have changed. For internal use. */
		class LpfNavFloorEvent: public VirtualMemObject
		{
		public:
			LpfNavFloorEvent(): m_nextUpdate(0.0f) {}
			LpfNavFloorEvent(const NavFloorPtr& navFloorPtr, KyFloat32 nextUpdate): m_navFloorPtr(navFloorPtr), m_nextUpdate(nextUpdate) {}
			~LpfNavFloorEvent() {}

		public:
			NavFloorPtr m_navFloorPtr;
			KyFloat32 m_nextUpdate;
		};


	public:
		/*! This class gathers together the task IDs for the estimations involved in the Local PathFinding system.
			Used in deterministic time measurement mode only. See :USERGUIDE:"Using Deterministic Time Measurement".
			\ingroup kyruntime_lpf */
		struct EstimatedTasks
		{
			/*! Provides the task ID for the \c LpfMergePolygons estimation. */
			TaskRegistry::TaskId m_mergePolygons;

			/*! Provides the task ID for the \c LpfGetLocalPath estimation. */
			TaskRegistry::TaskId m_getLocalPath;

			/*! Provides the task ID for the \c LpfPolylineIntersection estimation. */
			TaskRegistry::TaskId m_polylineIntersection;
			
			/*! \label_constructor
				\param mergePolygons		Sets the value of #m_mergePolygons.
				\param getLocalPath			Sets the value of #m_getLocalPath.
				\param polylineIntersection	Sets the value of #m_polylineIntersection. */
			EstimatedTasks(TaskRegistry::TaskId mergePolygons = TaskRegistry::InvalidTaskId,
				TaskRegistry::TaskId getLocalPath = TaskRegistry::InvalidTaskId,
				TaskRegistry::TaskId polylineIntersection = TaskRegistry::InvalidTaskId)
				:m_mergePolygons(mergePolygons), m_getLocalPath(getLocalPath), m_polylineIntersection(polylineIntersection)
			{}
		};

		/*! This class gathers together the task IDs for the aperiodic tasks that can be used to govern the time per
			frame allotted to the Local PathFinding system.
			\ingroup kyruntime_lpf */
		struct TimeSlicedTasks
		{
			/*! Provides the task ID for an aperiodic task used to control how much time is allotted per frame to the
				merging of \LpfObstacleRecords into \LpfPreAggregates. */
			TimeManager::TaskId m_preAggregateComputation;

			/*! Provides the task ID for an aperiodic task used to control how much time is allotted per frame to the
				merging of \LpfPreAggregates into \LpfAreas. */
			TimeManager::TaskId m_areaComputation;

			/*! \label_constructor
				\param preAggregateComputation		Sets the value of #m_preAggregateComputation.
				\param areaComputation				Sets the value of #m_preAggregateComputation. */
			TimeSlicedTasks(TimeManager::TaskId preAggregateComputation = TimeManager::InvalidTaskId,
				TimeManager::TaskId areaComputation = TimeManager::InvalidTaskId)
				:m_preAggregateComputation(preAggregateComputation), m_areaComputation(areaComputation)
			{}
		};

		/*! \label_init
			\param mgr							The ILpfManager for the World.
			\param navMeshLayerMgr				The NavMeshLayerManager for the World.
			\param premergerConfig				An instance of a class that derives from ILpfPreMergerConfig, which the LpfContext will
												use to create an instance of a class that derives from ILpfPreMerger.
			\param name							A unique name for the LpfContext.
			\param timeslicedTasks				An instance of TimeSlicedTasks that sets the aperiodic tasks that govern the time per frame allotted to this LpfContext.
			\param edgeRadius					Sets the value of #m_edgeRadius.
			\param minDeltaHeight				Sets the value of #m_minDeltaHeight.
			\param inhibitionPeriod				Sets the value of #m_inhibitionPeriod.
			\param maxNavFloorProcessPerFrame	Specifies the maximum number of \NavFloors that can have their obstacles merged in any given frame.
												\acceptedvalues	Any positive value.
												\defaultvalue	\c 50
			\param use3D						Sets the value of #m_use3D.
			\param estimatedTasks				An instance of EstimatedTasks that sets time estimations for the tasks involved in Local PathFinding. */
		KyResult Initialize(ILpfManager& mgr, NavMeshLayerManager& navMeshLayerMgr, const ILpfPreMergerConfig& premergerConfig, const char* name, TimeSlicedTasks timeslicedTasks, 
			KyFloat32 edgeRadius=0.5f, KyFloat32 minDeltaHeight=1.8f, KyFloat32 inhibitionPeriod=0.5f, KyUInt32 maxNavFloorProcessPerFrame=50, KyBool use3D=KY_FALSE, 
			EstimatedTasks estimatedTasks=EstimatedTasks());

		/*! Registers the specified ObstacleLayer as a source of Obstacle outlines for this LpfContext. */
		KyResult RegisterLayer(ObstacleLayer& layer);

		/*!	\label_reinit This method calls ResetProcessedData() to clear all \LpfAreas from this LpfContext,
			then clears all \LpfNavFloorRecords, performance and memory usage statistics. */
		void ReInit();

		/*!	This method calls ResetProcessedData() to clear all \LpfAreas from this LpfContext, and launches the process
			of re-merging obstacle records into \LpfAreas. You should call this method each time you change the value of
			a major configuration parameter that should affect all \LpfAreas, such as #m_edgeRadius.*/
		void ForceRefresh();

		/*!	This method clears all \LpfAreas from this LpfContext, and abandons all work in progress. */
		void ResetProcessedData();

		/*!	This method is transparently registered at initialization time with the DatabaseManager as a NavCell deactivation callback.
			When a NavCell is deactivated, this method is called to evaluate what action needs to be taken in response.
			It calls RemoveNavCellReferences() to remove all internal references on the provided NavCell.*/
		static void OnDeactivateNavCellCB(StitchedNavCell& stitchedNavCell, void* userData);

		/*!	For internal use. */
		void RemoveNavCellReferences(StitchedNavCell& stitchedNavCell);

		/*!	For internal use. */
		void AddNavFloorEvents(const ObstacleLayerObstacleRecord& obstacle);

		/*!	For internal use. */
		void AddNavFloorEvent(const NavFloorPtr& navFloorPtr);

		/*!	Updates \LpfPreAggregates and \LpfAreas. For internal use. */
		void Update();

		/*!	Retrieves the unique name assigned to this LpfContext. */
		const char* GetName() { return m_name; }

		/*!	Draws the information stored in the LpfContext for debugging purposes.
			\param flags			A bit mask composed of elements from the ILpfManager::RenderFlags enumeration, which
									determines the information to be displayed. */
		void Render(KyUInt32 flags = 0) const;

		/*!	Calculates an extended outline around the specified obstacle. For internal use. */
		void CreateExtendedPolyline(LpfPolyline& obstaclePolyline, const Vec3f* points, KyUInt32 pointCount);

		/*!	Updates \LpfPreAggregates in the \NavFloors that need to be updated. For internal use. */
		void Update_PreAggregates();

		/*!	Called by Update_PreAggregates(). For internal use. */
		bool Update_PreAggregates_CreateRequest(const NavFloorPtr& navFloorPtr);

		/*!	Called by Update_PreAggregates(). For internal use. */
		void Update_PreAggregates_GetResult(LpfPreAggregateComputationRequest* pacr);

		/*!	Merges the specified polylines using the PolygonMerger_Polygon instance maintained in m_polygonMerger. For internal use. */
		MergeResult MergePolygons(
			const SharedPoolList<LpfPolylinePtr>& in1,
			const SharedPoolList<LpfPolylinePtr>& in2,
			SharedPoolList<LpfPolylinePtr>& out);

		/*!	Called by MergePolygons() to retrieve merged polylines from m_polygonMerger. For internal use. */
		KyBool CopyPolylinesFromPlgMerger(KyUInt32 extContourIndex, SharedPoolList<LpfPolylinePtr>& out);

		/*!	Called transparently to merge together overlapping \LpfAreas in neighboring NavFloors. For internal use. */
		void Update_Areas();

		/*!	Called transparently by Update_Areas() to merge the next LpfArea that needs to be treated. For internal use. */
		KyBool TreatNextArea();

		/*!	Called transparently by TreatNextArea() to merge the outlines of the specified \LpfAreas. For internal use. */
		KyBool MergeAreas(LpfArea* area0, LpfArea* area1, bool& stopMerge);

		// Retrieve a list of LpfArea by browsing
		//   - the NavFloors around the preAggregates in inLpfArea's NavFloors
		//   - the NavFloors (in other NavMeshes) linked from the preAggregates in inLpfArea's NavFloors
		// \param inLpfArea the LpfArea that we want to find the neighbors of
		// \param outArrayLpfAreas the array to store the retrieved LpfAreas
		// \param maxSizeOutArrayLpfAreas "outArrayLpfAreas"'s maximum size
		// \param outCountArrayLpfAreas number of LpfArea retrieved
		/*!	For internal use. */
		void GetLpfAreasFromLpfAreaBasedOnNavFloors(const LpfArea& inLpfArea,
			LpfArea** outArrayLpfAreas, const KyUInt32 maxSizeOutArrayLpfAreas, KyUInt32& outCountArrayLpfAreas);

		/*! Removes duplicate elements from an array. Size of elements must be sizeof(void*) (a pointer). For internal use.
			\return KY_TRUE if successful. */
		KyBool RemoveDuplicateAreas(LpfArea** inOutArray, KyUInt32& inOutArrayCount);

		/*! Removes duplicate elements from an array of NavFloorPtr objects. For internal use.
			\return KY_TRUE if successful. */
		KyBool RemoveDuplicateNavFloorPtrs(NavFloorPtr* inOutArray, KyUInt32& inOutArrayCount);

		/*! Indicates whether or not a specified element is present in an array. For internal use. */
		KyBool IsPresent(void* element, void** inArray, const KyUInt32 countInArray);

		/*!	Indicates whether or not the specified point is inside an LpfArea within the specified NavFloor.
			\param p					The point to test.
			\param lm					Specifies the NavFloor to search.
			\param[out] inArea			Stores the LpfArea that contains the point.
			\return #KY_TRUE if \c p is within any LpfArea within the specified NavFloor, or #KY_FALSE otherwise. */
		KyBool IsInsideLpfAreas(const Vec3f& p, const NavFloorPtr& lm, LpfArea*& inArea);

		/*!	Indicates whether or not the specified point is inside the specified polyline.
			\param pl					The polyline to test.
			\param p					The point to test.
			\return #KY_TRUE if \c pt is within the area defined by \c pl, or #KY_FALSE otherwise. */
		KyBool IsInside(const LpfPolyline& pl, const Vec3f& p);

		/*!	Indicates whether or not the specified point is inside the specified LpfArea or polyline, taking into account
			the three-dimensional area of the LpfArea defined by the \c Use3D and \c MinDeltaHeight configuration
			parameters. If a valid LpfArea is provided, the test is conducted against that LpfArea. If no LpfArea is
			provided, the test is conducted against the polyline provided.
			\param plArea				The LpfArea to test.
			\param pl					The polyline to test, if no valid LpfArea is specified in \c p1Area.
			\param pt					The point to test.
			\return #KY_TRUE if \c pt is within the area defined by \c plArea or \c pl, or #KY_FALSE otherwise. */
		KyBool IsInside(const LpfArea* plArea, const LpfPolyline& pl, const Vec3f& pt);

		/*!	Indicates whether or not the specified point lies within any LpfArea maintained by the context.
			\return #KY_TRUE if \c p is within the outline of any LpfArea in the context, or #KY_FALSE otherwise. */
		KyBool IsInsideLpfAreas(const Vec3f& p);

		/*!	Indicates whether or not the specified point lies within any LpfArea maintained by the context, and
			provides access to the LpfArea that contains the point.
			\param p					The point to test.
			\param[out] area			Updated to store the LpfArea that contains \c p, if any.
			\return #KY_TRUE if \c p is within the outline of any LpfArea in the context, or #KY_FALSE otherwise. */
		KyBool IsInsideLpfAreas(const Vec3f& p, LpfArea*& area);

		/*!	Retrieves an LpfObstacleRecord from the specified memory pointer. For internal use. */
		LpfObstacleRecord* GetObstacleRecordFromAddr(const Vec3f* addr);

		/*!	For internal use. */
		void SortObstacleRecords_Remove(LpfObstacleRecord* obstacleRecord);

		/*!	Retrieves an LpfNavFloorRecord from the specified NavFloorPtr. For internal use. */
		LpfNavFloorRecord* GetNavFloorRecordFromNavFloorPtr(const NavFloorPtr& navFloorPtr);

		/*!	For internal use. */
		void SortNavFloorRecordRecords_Remove(LpfNavFloorRecord* navFloorRecord);

		// Time management
		// - time-slicing
		/*!	Retrieves the numeric ID of the \c LpfManager::PreAggregateComputation aperiodic task
			used to manage the CPU time allocated within each frame for creating \LpfPreAggregates from
			extended obstacle outlines. */
		TimeManager::TaskId GetPreAggregateComputationTaskId() {return m_timeSlicedTasks.m_preAggregateComputation;}

		// - deterministic timer
		/*!	Indicates the last time at which an LpfArea in this LpfContext was updated. */
		KyFloat32 GetLastMoveDate() {return m_timeStamp;}

		/*!	Indicates the last time at which an obstacle in the ObstacleLayer was updated. */
		KyFloat32 GetLastPathDataUpdateDate() {return m_lastPathDataUpdateDate;}

		/*!	Sets the last time at which an obstacle in the ObstacleLayer was updated. For internal use. */
		void SetPathDataUpdateDate() {m_lastPathDataUpdateDate = GetWorld().GetTimeManager().GetCurrentFrameStart(); }

		/*!	Returns the time taken during the last call to LpfContext::Update(). */
		KyFloat32 GetLastUpdateCpuConsumption() {return m_lastUpdateCpuConsumption;}

		/*!	Retrieves the numeric ID of the \c LpfGetLocalPath estimated task, used in deterministic time measurement
			mode to provide an estimation for the time required to compute local Paths. */
		TaskRegistry::TaskId GetGetLocalPathEstimatedTaskId() {return m_estimatedTask.m_getLocalPath;}

		/*!	Retrieves the numeric ID of the \c LpfPolylineIntersection estimated task, used in deterministic time measurement
			mode to provide an estimation for the time required to test for polyline intersections. */
		TaskRegistry::TaskId GetPolylineIntersectionEstimatedTaskId() {return m_estimatedTask.m_polylineIntersection;}


		/*!	Retrieves an LpfArea from the pool of unused \LpfAreas maintained by the LpfContext. For internal use. */
		LpfArea* GetNewArea();

		/*!	Releases the specified LpfArea back to the pool of unused \LpfAreas when it is no longer needed. For internal use. */
		void ReleaseArea(LpfArea* area);

		/*!	Splits the specified LpfArea into a set of \LpfAreas, each of which represents one of the \LpfPreAggregates
			initially used to construct the specified LpfArea. For internal use. */
		KyBool SplitArea(LpfArea* area);

		/*!	Retrieves an LpfPreAggregate from the pool of unused \LpfPreAggregates maintained by the LpfContext. */
		LpfPreAggregatePtr GetNextFreePreAggregate();

		/*!	Releases the specified LpfPreAggregate back to the pool of unused \LpfPreAggregates when it is no longer needed.
			\return #KY_TRUE if the LpfPreAggregate was successfully released, or #KY_FALSE otherwise. */
		KyBool ReleasePreAggregate(LpfPreAggregatePtr preAggregate);

		/*!	Retrieves a polyline from the pool of unused polylines maintained by the LpfContext. */
		LpfPolylinePtr GetNextFreePolyline();

		/*!	Copies \c inPolylines to \c outPolylines.
			\param inPolylines			A list of polylines to be copied.
			\param[out] outPolylines	Stores the polylines copied from \c inPolylines.
			\return #KY_TRUE if all polylines were successfully copied, or #KY_FALSE if one or more polylines could not be copied.
		*/
		KyBool CopyPolylines(const SharedPoolList<LpfPolylinePtr>& inPolylines, SharedPoolList<LpfPolylinePtr>& outPolylines);

		/*!	Releases the specified polyline back to the pool of unused polylines when it is no longer needed. */
		void ReleasePolyline(LpfPolylinePtr polyline);

		/*!	Releases the specified list of polylines back to the pool of unused polylines when they are no longer needed. */
		void ReleasePolylines(SharedPoolList<LpfPolylinePtr>& polylines);

		/*!	Adds a new LpfPolylineNode to the specified polyline.
			\param polyline				The polyline to which the new LpfPolylineNode will be added.
			\param pn					An LpfPolylineNode that contains the position of the new point.
			\return #KY_TRUE if the point was successfully added, or #KY_FALSE otherwise. */
		KyBool AddPointToPolyline(LpfPolyline& polyline, const LpfPolylineNode& pn);

		/*!	Removes all \LpfPolylineNodes from the specified polyline. */
		void ClearPolyline(LpfPolyline& polyline);

		/*!	Creates a new LpfObstacleRecord that will be included in the \LpfAreas built by this LpfContext. For internal use. */
		LpfObstacleRecord* CreateObstacleRecord(const Vec3f* points, KyUInt32 pointCount, void* userData);

		/*!	Creates a new LpfNavFloorRecord to represent an area that contains obstacles considered by this LpfContext. For internal use. */
		LpfNavFloorRecord* CreateNavFloorRecord(const NavFloorPtr& navFloorPtr);

		/*!	Releases the specified LpfNavFloorRecord back to the pool of unused \LpfNavFloorRecords. For internal use. */
		void ReleaseNavFloorRecord(LpfNavFloorRecord* navFloorRecord);

		// PreMerger accessors
		/*!	Retrieves the pre-merger modifier currently used by the LpfContext. */
		ILpfPreMerger* GetLpfPreMergerModifier() const { return m_preMerger; }

		/*!	Sets the pre-merger modifier the LpfContext will use. */
		void SetLpfPreMergerModifier(ILpfPreMerger* pm) { m_preMerger = pm; }

		/*!	\label_getworld */
		World& GetWorld()
		{
			return m_manager->GetWorld();
		}

		/*!	\label_getworld */
		const World& GetWorld() const
		{
			return m_manager->GetWorld();
		}

		/*!	\label_getengine */
		Engine& GetEngine()
		{
			return GetWorld().GetEngine();
		}

		/*!	\label_getengine */
		const Engine& GetEngine() const
		{
			return GetWorld().GetEngine();
		}

		// areas
		const ContiguousPtrArray<LpfArea>& AreasUpdatedThisFrame() const { return m_areasUpdatedThisFrame; }

		// Double-check stats values
		/*! Retrieves from #m_stats a set of information about the number of objects used by the LpfContext.
			\param[out] vectorCurCount				The current number of LpfPolylineNode objects.
			\param[out] polylineCurCount			The current number of LpfPolyline objects.
			\param[out] preAggregateCurCount		The current number of LpfPreAggregate objects.
			\param[out] areaCurCount				The current number of LpfArea objects.
			\param[out] vectorFreeCount				The number of free LpfPolylineNode objects remaining in the pool.
			\param[out] polylineFreeCount			The number of free LpfPolyline objects remaining in the pool.
			\param[out] preAggregateFreeCount		The number of free LpfPreAggregate objects remaining in the pool.
			\param[out] areaFreeCount				The number of free LpfArea objects remaining in the pool.
			\param[out] sortedNavRecordArraySize	The total number of NavFloorRecord objects allocated in the pool.
			\param[out] validNavRecordCount			The number of NavFloorRecord objects currently used in the pool. */
		void CountActualPoolSizes(
			KyUInt32& vectorCurCount, KyUInt32& polylineCurCount, KyUInt32& preAggregateCurCount, KyUInt32& areaCurCount,
			KyUInt32& vectorFreeCount, KyUInt32& polylineFreeCount, KyUInt32& preAggregateFreeCount, KyUInt32& areaFreeCount,
			KyUInt32& sortedNavRecordArraySize, KyUInt32& validNavRecordCount);

		/*! Retrieves the maximum number of \NavFloors that can be processed in a single frame, as set in the call to Initialize(). */
		KyUInt32 GetMaxNavFloorProcessPerFrame() const { return m_maxNavFloorProcessPerFrame; }

	public:
		/*! Sets the state of this LpfContext to match \c rhs. For internal use. */
		const LpfContext& operator = (const LpfContext& rhs); // needed by m_contexts.Resize
	public:
		// Manager
		ILpfManager* m_manager; /*!< A pointer to the LpfManager that contains this LpfContext. Do not modify. */

		// PathData
		INavMeshLayerPtrList m_layers; /*!< The \ObstacleLayers that supply obstacle outlines for this LpfContext. Do not modify. */

		// obstacles
		typedef SortedArray<const Vec3f*, LpfObstacleRecord> SortedArray_Obstacle;
		SortedArray_Obstacle m_sortedObstacleRecords; /*!< The pool of \LpfObstacleRecords, sorted by memory address. Do not modify. */
		SharedPoolList<LpfObstacleRecord*>::Pool m_obstacleRecordPtrPool; /*!< The pool of LpfObstacleRecord pointers. Do not modify. */

		// navFloors
		typedef SortedArray<NavFloorPtr, LpfNavFloorRecord> SortedArray_NavFloor;
		SortedArray_NavFloor m_sortedNavFloorRecords; /*!< The pool of \LpfNavFloorRecords. Do not modify. */

		// polylines
		LpfPolyline::Pool m_vectorPool; /*!< The pool of \LpfPolylineNodes. Do not modify. */
		ChunkBasedPool<LpfPolyline> m_polylinePool; /*!< The pool of polylines. Do not modify. */
		SharedPoolList<LpfPolylinePtr>::Pool m_polylinePtrPool; /*!< The pool of polyline pointers. Do not modify. */

		// PreAggregates
		ChunkBasedPool<LpfPreAggregate> m_preAggregatePool; /*!< The pool of free \LpfPreAggregates. Do not modify. */
		SharedPoolList<LpfPreAggregatePtr>::Pool m_preAggregatePtrPool; /*!< The pool of LpfPreAggregate pointers. Do not modify. */

		// Areas
		ContiguousPtrArray<LpfArea> m_areas;
		ContiguousPtrArray<LpfArea> m_areasUpdatedThisFrame;
		List<LpfArea*> m_freeAreas; /*!< The free \LpfAreas that can be recycled. Do not modify. */

		// events
		List<LpfNavFloorEvent> m_events; /*!< A list of events fired when the obstacles in a NavFloor have changed. Do not modify. */
		List<LpfNavFloorEvent> m_inhibitedNavFloors; /*!< A list of \NavFloors that have been updated more recently than the #m_inhibitionPeriod value. Do not modify. */

		ObstacleLayerObserver_UsedInLpf m_observer_UsedInlpf;
		ObstacleLayerObserver_NotUsedInLpf m_observer_NotUsedInlpf;

		// Parameters
		/*!	Determines whether or not the LpfContext takes the vertical distance between obstacles into account when creating \LpfAreas.
			\defaultvalue	#KY_FALSE */
		KyBool m_use3D;

		/*!	Specifies the distance the LpfContext leaves between the obstacle outlines it retrieves from the ObstacleLayer and the
			bypass polylines it generates for each \LpfArea.
			\pp Set this value to the radius of the largest Bot expected to use this LpfContext, plus an extra margin equal to the
			value of DetectPathNodeReached_2d5::MaxDist() or DetectPathNodeReached_3d::MaxDist().
			\units			meters
			\acceptedvalues	any positive value
			\defaultvalue	\c 0.5f */
		KyFloat32 m_edgeRadius;

		/*!	Specifies the minimum height differential that must exist between two overlapping outlines in order for the LpfContext
			to treat them as two separate \LpfAreas.
			\pp If the vertical distance between two overlapping obstacles is less than the value of this parameter, the LpfContext
			combines the two obstacles into a single \LpfArea. 
			\pp If the vertical distance between two overlapping obstacles is greater than the value of this parameter, the LpfContext
			typically creates a separate LpfArea for each obstacle. Note, however, that in this case the two obstacles may still be
			combined into a single LpfArea if linked together through other overlapping obstacles. 
			\units			meters
			\acceptedvalues	any positive value
			\defaultvalue	\c 1.8f */
		KyFloat32 m_minDeltaHeight;

		/*!	Specifies the minimum time that must elapse between successive pre-merging calculations carried out on a single NavFloor. 
			\pp Each time the LpfContext retrieves obstacles from a NavFloor and calls a pre-merger modifier to merge these obstacles into
			pre-aggregates, it adds the value of this parameter to the current time in order to obtain the soonest possible time at which
			the obstacles in that NavFloor can be reconsidered. When obstacles in the NavFloor are created, moved or destroyed, those
			hanges will only be reflected in the LpfContext once the inhibition period has elapsed. 
			\pp This parameter implies a tradeoff between CPU consumption on one hand and the responsiveness of your LpfContexts to changes
			in your obstacle configurations on the other. The lower the value of this parameter, the more frequently your LpfContext will
			be updated, which can provide greater fidelity for your pathfinding. However, lower values also entail a corresponding increase
			in CPU requirements. 
			\units			seconds
			\acceptedvalues	any positive value
			\defaultvalue	\c 0.5f */
		KyFloat32 m_inhibitionPeriod;

		/*! For each class of object used by the LpfContext, this structure stores the current number of objects of that class maintained
			by the LpfContext, and the maximum number of objects of that class that have been maintained by the LpfContext at any point
			since its initialization.
			\ingroup kyruntime_lpf */
		struct Stats
		{
			/*! Enumerates the types of count maintained by the Stats structure. */
			enum Kind
			{
				current, /*!< The current number of objects that are maintained by the LpfContext. */
				maximum, /*!< The maximum number of objects that have been maintained by the LpfContext at any point in time. */
				_Kind_Count
			};

			/*! Enumerates the types of object counted by the Stats structure. */
			enum StatData
			{
				ObstacleRecord = 0, /*!< LpfObstacleRecord objects. */
				ObstacleRecordPtrs, /*!< Pointers to LpfObstacleRecord objects. */
				NavFloorRecord, /*!< LpfNavFloorRecord objects. */
				Vector, /*!< LpfPolylineNode objects. */
				Polyline, /*!< LpfPolyline objects. */
				PolylinePtrs, /*!< Pointers to LpfPolyline objects. */
				PreAggregate, /*!< LpfPreAggregate objects. */
				PreAggregatePtrs, /*!< Pointers to LpfPreAggregate objects. */
				Area, /*!< LpfArea objects. */
				_StatData_Count
			};

			/*! \label_constructor For internal use. */
			Stats()
			{ 
				Clear(current);
				Clear(maximum);
			}

			/*! \label_clear */
			void Clear(Kind w)
			{
				memset(m_uint32[w], 0,_StatData_Count*sizeof(KyUInt32));
				if(w == maximum)
				{
					m_maxPointCount_Polyline = 0;
					m_maxPointCount_Area = 0;
				}
			}

			/*! Increases the count recorded for the specified type of object.
				\param member			The type of object whose count will be increased.
				\param count			The amount of the increase to apply to the count. */
			void Increment(StatData member, KyUInt32 count=1)
			{
				KyUInt32& currentValue = m_uint32[current][member];
				KyUInt32& maximumValue = m_uint32[maximum][member];
				currentValue += count;
				maximumValue = Kaim::Max(maximumValue, currentValue);
			}

			/*! Decreases the count recorded for the specified type of object.
				\param member			The type of object whose count will be decreased.
				\param count			The amount of the decrease to apply to the count. */
			void Decrement(StatData member, KyUInt32 count=1)
			{
				//KY_FUNCTION("Stats::Decrement");
				KyUInt32& currentValue = m_uint32[current][member];
				//KY_ASSERT(currentValue >= count, ("Decrementing more than possible, StatData(%u)'s value is %u, trying to decrement by %u", member, currentValue, count));
				currentValue -= (currentValue >= count ? count : currentValue); //decrement or set to 0
			}

			/*! Retrieves the current or maximum count recorded for the specified type of object.
				\param member			The type of object whose count will be retrieved.
				\param kind				Indicates whether to retrieve the current count or maximum count. */
			KyUInt32 Get(StatData member, Kind kind)
			{
				return m_uint32[kind][member];
			}

			//read only (written internally by the LpfManager and the LpfContext)
			KyUInt32 m_maxPointCount_Polyline; /*!< The number of LpfPolylineNode points used in the most complex LpfPolyline to be created within the LpfContext (the polyline composed of the largest number of nodes). Do not modify. */
			KyUInt32 m_maxPointCount_Area; /*!< The number of LpfPolylineNode points contained in the most complex LpfArea outline within the LpfContext (the outline composed of the largest number of nodes). Do not modify. */

	protected:
			//they should not be modified out of Stats functions
			KyUInt32 m_uint32[_Kind_Count][_StatData_Count];
		};

		/*! Stores information about the number of objects of different types used by the LpfContext. */
		Stats m_stats;

	protected:
		char* m_name;

		ILpfPreMerger* m_preMerger;

		// the id of the estimated tasks
		EstimatedTasks m_estimatedTask;
		TaskRegistry::TaskId m_mergePolygonsEstimatedTaskId;
		TaskRegistry::TaskId m_getLocalPathEstimatedTaskId;
		TaskRegistry::TaskId m_polylineIntersectionEstimatedTaskId;

		// Time-slicing
		TimeSlicedTasks m_timeSlicedTasks;

		// Currently processed variables
		LpfArea* m_outArrayLpfAreas[1024];// HARDCODED : MUST BE IMPROVED. List instead?
		LpfPreAggregateComputationRequest* m_preAggregateComputationRequests;
		List<LpfPreAggregateComputationRequest*> m_pacrToProcess;
		List<LpfPreAggregateComputationRequest*> m_freePACR;
		List<LpfArea*> m_areasToMerge;
		LpfArea* m_tmpNewArea;
		SharedPoolList<LpfPolylinePtr> m_tmpMergedPolylines;
		List<NavFloorPtr> m_tmpNavFloorsToReupdateAfterReInit;

		KyFloat32 m_now;
		KyFloat32 m_timeStamp;
		KyFloat32 m_lastPathDataUpdateDate;
		KyFloat32 m_lastUpdateCpuConsumption;
		KyUInt32  m_maxNavFloorProcessPerFrame;

		Vec3f m_maxObstacleExtends;
	};


	//////////////////////////////////////////////////////////////////////////
	// Async module
	///////////////////////////////////////////////////////

	/*!	This class represents an AsyncModule that offloads the update of \LpfContexts to a secondary thread.
		\pp For more information on using this class, and the \SDKName asynchronous processing framework,
		see :USERGUIDE:"Asynchronous Processing".
		\ingroup kyruntime_lpf */
	class LpfManager_AsyncModule: public AsyncModule
	{
	public:
		/*!	\label_baseconstructor */
		LpfManager_AsyncModule(AsyncManager& asyncMgr): AsyncModule(asyncMgr),
			m_lpfManager(KY_NULL) {}

		/*!	\label_virtualdestructor */
		virtual ~LpfManager_AsyncModule() {}

		KY_DECLARE_ASYNC_MODULE(LpfManager_AsyncModule)

		virtual void BeginAsyncProcessing();

		virtual void EndAsyncProcessing();

		ILpfManager* m_lpfManager; /*!< Pointer to the LpfManager. Do not modify. */
	};



	/************************************************************************/
	/*                                                                      */
	/* ILpfManager itself                                                   */
	/*                                                                      */
	/************************************************************************/

public:
	KY_DECLARE_WORLDSERVICE(ILpfManager)

public:
	/*!	\label_baseconstructor */
	ILpfManager(Kaim::World& world);

	/*!	\label_virtualdestructor */
	virtual ~ILpfManager() = 0;

	virtual void ReInit();

	virtual void Update();

	/*!	Retrieves the time spent during the last call to the Update() method
		within the main thread running the \SDKName Engine, or 0 if using asynchronous
		processing. */
	KyFloat32 GetLastUpdateCpuConsumption() {return m_lastUpdateCpuConsumption;}

	/*!	Retrieves the LpfContext with the specified name. */
	LpfContext* GetContextByName(const char* name);

	/*!	Retrieves the LpfContext at the specified index within the list of \LpfContexts maintained by the LpfManager. */
	LpfContext& GetContext(KyUInt32 index) 
	{
		KY_FUNCTION("ILpfManager::GetContext");
		KY_ASSERT(index < m_contexts.GetCount(), ("No lpf context with index %u", index));
		return m_contexts[index];
	}

	/*!	Retrieves the number of \LpfContexts maintained by the LpfManager. */
	KyUInt32 GetContextCount() const { return m_contexts.GetCount(); }

	/*!	Draws debug information about Local PathFinding.
		This implementation calls the LpfContext::Render() method for each LpfContext, passing
		the value specified for the \c flags parameter. */
	void Render(KyUInt32 flags = 0) const;

	/*!	Retrieves the total number of \LpfPolylineNodes that have been merged since the creation of
		the World. When using deterministic time measurement mode, you can use this information to
		compute an accurate value for the \c LpfMergePolygons estimated task. For details, see
		:USERGUIDE:"Using Deterministic Time Measurement". */
	static KyUInt32 GetMergeAreasPointSumCounter();

	/*!	Retrieves the total number of edges that have been tested during the calculation of local paths
		since the creation of the World. When using deterministic time measurement mode, you can
		use this information to compute an accurate value for the \c LpfGetLocalPath estimated task. For
		details, see :USERGUIDE:"Using Deterministic Time Measurement". */
	static KyUInt32 GetLocalPathComputationEdgeCounter();

	/*!	Retrieves the total number of \LpfPolylineNodes that have been tested for intersections
		since the creation of the World. When using deterministic time measurement mode, you can
		use this information to compute an accurate value for the \c LpfPolylineIntersection estimated task. For
		details, see :USERGUIDE:"Using Deterministic Time Measurement". */
	static KyUInt32 GetPolylineIntersectionPointSumCounter();


protected:
	KyResult Initialize(KyUInt32 maxContexts=1, KyUInt32 maxAreaPoint=40, LpfManager_AsyncModule* lpfAsynModule=KY_NULL, KyInt32 tmpPathPointMax=32);

	/*	A pointer to the LpfManager_AsyncModule maintained by the AsyncManager,
		if any. Otherwise, #KY_NULL. */
	LpfManager_AsyncModule* m_asyncModule;

	// Mergers
	friend class LpfContext;
	/*	An instance of PolygonMerger_Polygon used to merge overlapping \LpfAreas that are contained in adjacent \NavFloors. */
	PolygonMerger_Polygon m_polygonMerger;

	// Temp passability test structures
	friend class LpfArea;
	KyUInt32 m_tmpPathPointMax;
	KyUInt32 m_tmpPathPointCount;
	Vec3f* m_tmpPathPoints;
	List<ILpfManager::LpfPolylineNode::LpfEdgePassability*> m_tmpPassabilityList;

	/*	Stores the list of LpfContexts. Use GetContextByName() to retrieve an LpfContext from the list. */
	Array<LpfContext> m_contexts;

	KyFloat32 m_lastUpdateCpuConsumption;

public:
	// Temp polyline intersections
	LpfPolylineIntersection m_tmpPolylineIntersections[PolylineIntersectionMaxCount];
};



///////////////////////////////////////////////////////////////////////////////////////////
// ILpfPreMerger
///////////////////////////////////////////////////////////////////////////////////////////
class ILpfPreMerger;

/*!	Provides an abstract base class for an object that the ILpfManager can use to create an object that derives
	from ILpfPreMerger. This class cannot be used directly. Use one of the derived classes supplied with the \SDKName,
	which are specialized for the classes of ILpfPreMerger supplied in the SDK. If you create your own class of
	ILpfPreMerger, you must also create your own corresponding class of ILpfPreMergerConfig. */
class ILpfPreMergerConfig : public VirtualMemObject
{
public:
	/*!	Called by the ILpfManager to create an instance of a class that derives from ILpfPreMerger for use in the
		specified \LpfContext. */
	virtual ILpfPreMerger* CreateLpfPreMerger(ILpfManager::LpfContext& context) const =0;
};

/*!	Defines the signature for the constructor function that is called automatically each time \SDKName 
	needs to create an instance of a class that derives from ILpfPreMerger. Each class that derives from 
	ILpfPreMerger must contain a constructor function that matches this type definition, and that passes
	its arguments on to the constructor function of its parent class.
	\param context		The \LpfContext context using the modifier.
	\ingroup kyruntime_lpf */
typedef ILpfPreMerger* (*LpfPreMergerConstructor)(ILpfManager::LpfContext& context);




/*!	LpfPreMergerClass is the MetaClass for ILpfPreMerger, created and used internally by
	the \SDKName object factory.
	\ingroup kyruntime_lpf */
class LpfPreMergerClass: public MetaClass<ILpfPreMerger, LpfPreMergerConstructor>
{
public:
	/*!	Constructor for the ILpfPreMerger MetaClass. For internal use. */
	LpfPreMergerClass(const char* className, LpfPreMergerConstructor ctr, LpfPreMergerClass* baseClass);
};

/*!	This macro must be included in the declaration of any class that derives from ILpfPreMerger, to set up the
	MetaClass used by the \SDKName object factory.
	\ingroup kyruntime_lpf */
#define KY_DECLARE_LPFPREMERGER(ky_class) \
	static Kaim::LpfPreMergerClass Class; \
	static Kaim::ILpfPreMerger* KyCreate(Kaim::ILpfManager::LpfContext& context); \
	Kaim::LpfPreMergerClass& GetClass() const { return ky_class::Class; }

/*!	This macro must be included in the implementation of any class that derives from ILpfPreMerger, to set up the
	MetaClass used by the \SDKName object factory.
	\ingroup kyruntime_lpf */
#define KY_IMPLEMENT_LPFPREMERGER(ky_class, ky_base_class) \
	Kaim::LpfPreMergerClass ky_class::Class(#ky_class,ky_class::KyCreate,ky_base_class); \
	Kaim::ILpfPreMerger* ky_class::KyCreate(Kaim::ILpfManager::LpfContext& context) \
	{ \
		KY_LOG_MEMORY(ky_class::Class.GetName()); \
		Kaim::ILpfPreMerger* ptr = (Kaim::ILpfPreMerger*) KY_NEW_CLASS(ky_class)(context); \
		KY_REGISTER_METACLASS_VPTR(ky_class::Class, (*(void**)ptr)); \
		return ptr; \
	}



/*!	ILpfPreMerger provides an abstract base interface for objects that are invoked by the ILpfManager::LpfContext
	to merge the outlines of individual obstacles into \LpfPreAggregates. These \LpfPreAggregates
	objects are then merged internally by the ILpfManager::LpfContext into \LpfAreas.
	\pp See :USERGUIDE:"Building LpfAreas".
	\pp If you want to merge obstacles using a custom approach that differs from the ILpfPreMerger classes supplied with 
	\SDKName, you can write your own implementation of ILpfPreMerger for use in your \LpfContexts. For example, you can create a 
	custom pre-merging scheme that uses different Levels of Detail, where obstacles in \NavFloors close to the player are 
	pre-merged with more accuracy than obstacles in \NavFloors at a greater distance.
	\section writingcustompremerger Writing a custom ILpfPreMerger class
	Writing a custom class that derives from ILpfPreMerger involves the following considerations.
	\subsection premergerconst Constructor
	All classes that derive from ILpfPreMerger must have a constructor that follows the #LpfPreMergerConstructor type definition. 
	\subsection premergervm Virtual methods
	All classes that derive from ILpfPreMerger must implement the ComputePreAggregates() method.
	\subsection premergeroverlap Recommendations for treating overlapping obstacles
	Your modifier can use any method you choose for determining which obstacle records overlap, and how best to create polyline
	outlines for the single pre-aggregate that you create to represent those overlapping obstacles. For example, due to performance
	considerations, your modifier may tolerate more inaccuracy in the final outline generated for the pre-aggregate. This is 
	particularly true if you intend to implement a modifier that can apply with different levels of detail; faster, lower-detail 
	merges may produce more blocky, inexact contours for the pre-aggregates. 
	\pp However, you should always ensure that the final area covered by the pre-aggregate entirely contains the area covered by 
	all of its composite obstacle records. Because \LpfAreas are created from the outlines of your pre-aggregates, and because the 
	local path calculation and path following processes assume that your \Bots can follow any polyline outlining an LpfArea, if a 
	pre-aggregate outline cuts through the extended outline of any of its composite obstacle records, your \Bots may not be able to 
	bypass that obstacle correctly at runtime.
	\subsection premergercontours Polyline contours
	\pp The list of polylines for each pre-aggregate must contain: 
	-	A single, closed polyline that represents the outside contours of the pre-aggregate. The points in this polyline must follow 
		the contour in clockwise order.
	-	Any number of closed polylines that represent inner contours of the pre-aggregate: spaces entirely enclosed by the overlapping
		obstacles. For example, if your pre-aggregate represents a circular grouping of obstacles, the open area in the center of the 
		circle can be defined by a polyline. Similarly, figure-8 groupings may have two internal polylines, pretzel shapes may have 
		three, etc. The points in these polylines must follow the contour in counter-clockwise order.
	\pp All polylines must be closed: the last polyline node in the list must be at exactly the same position as the first.
	\pp In order to ensure the correct usage of the object pools maintained by the LpfContext, your modifier must access and construct 
	polylines using the functions provided by the LpfContext:
	-	LpfContext::GetNextFreePolyline(), which retrieves a free polyline from the polyline pool maintained by the LpfContext.
	-	LpfContext::AddPointToPolyline(), which retrieves a new polyline node from the pool maintained by the LpfContext, and adds it to the polyline.
	-	LpfContext::RemovePointFromPolyline(), which removes a node from a polyline, and returns it to the pool of polyline nodes maintained by the LpfContext. 
	-	LpfContext::ClearPolyline(), which removes all node from a polyline, and returns them to the pool of polyline nodes maintained by the LpfContext. 
	-	LpfContext::ReleasePolyline(), which returns a polyline to the pool of free polylines. Call this method only for polylines that you want to discard 
		without saving them in your pre-aggregate. This method clears the polyline before returning it to the pool.
	\subsection premergertimeslicing Time slicing
	The pre-merging process can take a long time, especially for \NavFloors that contain many obstacles. It is highly recommended 
	that you make use of the aperiodic task named \c Kaim::LpfManager::PreAggregateComputation to govern the computations carried
	out by your modifier.
	\pp Before performing a long calculation during the course of your pre-merging algorithm, you should request time from this
	aperiodic task. If time is granted, you may continue with your calculations. If time is not granted, your ComputePreAggregates() 
	method should return #KY_FALSE to indicate that the computation is not over. Your modifier must store the state of its 
	calculations in such a way that it can resume the calculations from the same point at the following frame.
	\pp For more information on using aperiodic tasks in your custom component, see :USERGUIDE:"Using Periodic and Aperiodic Tasks", 
	or see the source code for the pre-merger modifiers supplied with \SDKName.
	\subsection premergermetaclass MetaClass macros
	In order for \SDKName to recognize your ILpfPreMerger class, you must include the following macro definitions:
	-	#KY_DECLARE_LPFPREMERGER: Place this macro in the declaration of your new class. This macro takes one argument: the 
		name of your new class.
	-	#KY_IMPLEMENT_LPFPREMERGER: Place this macro in the implementation file of your new class. This macro takes the following
		arguments:
		-	The name of your new class.
		-	A reference to the MetaClass of the class from which your new class derives, if different from ILpfPreMerger.
			For example, &Kaim::LpfPreMerger_Grid::Class. If your new class derives directly from ILpfPreMerger,
			the value of this argument should be #KY_NULL.
	\ingroup kyruntime_lpf */
class ILpfPreMerger: public Kaim::VirtualMemObject
{
	ILpfPreMerger& operator=(const ILpfPreMerger&);
	ILpfPreMerger(const ILpfPreMerger&);
public:
	/*!	\label_baseconstructor
		\param context				The \LpfContext that will own this object. */
	ILpfPreMerger(ILpfManager::LpfContext& context) : m_context(context)
	{
	}

	/*!	\label_virtualdestructor */
	~ILpfPreMerger() { }

	/*!	\label_getclass */
	virtual LpfPreMergerClass& GetClass() const = 0;


	/*!	Called by LpfContext::Init() to finalize and confirm the initialization of the pre-merger modifier.
		The default implementation returns #KY_TRUE in all cases.
		\return #KY_TRUE if the initialization was successfully completed, or #KY_FALSE otherwise. */
	virtual KyBool FinalizeInitialization() {return KY_TRUE;}

	/*!	Resets the state of the pre-merger modifier. This method is called transparently when
		ILpfManager::LpfContext::ReInit() is called on the LpfContext that owns this pre-merger
		modifier. */
	virtual void ReInit() {}


	/*!	Retrieves a reference to the LpfContext that owns this pre-merger modifier. */
	ILpfManager::LpfContext& GetContext() const { return m_context; }

	/*!	This method is called by the LpfContext for each NavFloor any time an obstacle has been created, moved or removed 
		within a NavFloor, to pre-merge the obstacles contained in the area covered by that NavFloor into a set of pre-aggregates. 
		The \c preAggregrateComputationRequests parameter contains a list of objects that encapsulate individual requests that the
		pre-merger should treat in sequence. Each request contains a list of obstacle outlines that need to be
		merged together, and a list that accepts the output pre-aggregates computed by the pre-merger modifier. This method must take the list of
		obstacle records contained in the request, determine where and how their extended outlines overlap (stored in the 
		ILpfManager::LpfObstacleRecord::m_extendedOutline class members of each LpfObstacleRecord), and produce a set 
		of outlines that represent the external contours of the merged obstacles. These output contours must be saved into the list of
		ILpfManager::LpfPreAggregate objects contained in the request.
		\pp In cases where the outlines of two or more obstacles overlap, the overlapping obstacles should typically be merged into
		a single pre-aggregate. In cases where an obstacle does not overlap any other obstacles, it should typically be placed in 
		its own pre-aggregate. However, your custom modifier may use any approach for determining which obstacles need to be merged,
		and for calculating the final outline of each pre-aggregate.
		\pp For each pre-aggregate that your method determines is necessary, you must:
		-#	Retrieve an instance of the LpfPreAggregate class from the shared pool maintained by the LpfContext. To do so, call the
			LpfContext::GetNextFreePreAggregate() method of the LpfContext maintained in your modifier’s #m_context member. 
		-#	Set the NavFloor of the pre-aggregate to the NavFloor currently under consideration. To do so, set the value of the 
			pre-aggregate’s LpfPreAggregate::m_navFloorRecord member to the LpfNavFloorRecord stored in the request. 
		-#	Add to the pre-aggregate each obstacle that the pre-aggregate represents. To do so, call the LpfPreAggregate::AddObstacle()
			method of the pre-aggregate for each obstacle that should be included as part of the pre-aggregate, and pass as an argument 
			the pointer to the corresponding obstacle record from the list contained in the request.
		-#	Update the outline of the pre-aggregate to indicate its contours. This outline is determined by the list of polylines maintained
			in the pre-aggregate’s LpfPreAggregate::m_polylines member. Typically, this outline is updated by retrieving a free polyline 
			(a list of PolylineNode objects) from the pool maintained by the LpfContext, adding to that polyline a polyline node to represent 
			each desired corner in the pre-aggregate’s contour, and adding the resulting polyline to the pre-aggregate by passing it to the 
			LpfPreAggregate::AddPolyline() method of the pre-aggregate. For more information on the polylines that you should create for each 
			pre-aggregate, see "Polyline contours" above.
		-#	Update the time-stamp of the pre-aggregate, maintained in its LpfPreAggregate::m_timeStamp member, to indicate the start time of
			the current frame. 
		-#	Store the pre-aggregate in the list of pre-aggregates contained in the request.
		\param[in, out] preAggregrateComputationRequests		The list of requests that the pre-merger modifier should treat.
		\return -	#KY_TRUE if the computation has completed all the requests.
				-	#KY_FALSE to defer any remaining computations to the following frame. If you return #KY_FALSE,
					this method will given the same parameters when it is called again in the following frame.
					Your implementation must be able to pick up the calculations where it left off in the
					previous frame. */
	virtual KyBool ComputePreAggregates(List<ILpfManager::LpfPreAggregateComputationRequest*>& preAggregrateComputationRequests) = 0;

protected:
	ILpfManager::LpfContext& m_context; /*!< The ILpfManager::LpfContext that owns this pre-merger. */
};

} // namespace Kaim

#endif // KY_ILPFMANAGER_H
