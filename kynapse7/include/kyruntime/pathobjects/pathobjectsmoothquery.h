/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download,
* or which otherwise accompanies this software in either electronic or hard copy form.
*/



/*!	\file
	\ingroup kyruntime_pathobjects */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KyRuntime_PathObjectSmoothQuery_H
#define KyRuntime_PathObjectSmoothQuery_H


namespace Kaim
{

/*!	Enumerates the possible results of a PathObjectSmoothQuery. Set internally by the PathObjectSmoothQuery.
	\ingroup kyruntime_pathobjects */
enum PathObjectSmoothResult
{
	PathObjectSmooth_Unknown = 0, /*!< Indicates that the query has not been carried out, or the result cannot be determined. */
	PathObjectSmooth_DoesNotIntervene, /*!< Indicates that the PathObject should not affect the movement of a Bot passing along the "from-to" line segment defined in the query. */
	PathObjectSmooth_NotPassable, /*!< Indicates that the PathObject considers the "from-to" line segment defined in the query to be impassable. */
	PathObjectSmooth_Passable, /*!< Indicates that the PathObject considers the "from-to" line segment defined in the query to be passable. */
	PathObjectSmoothResult_dummy_forceenumsize = KYFORCEENUMSIZEINT
};

/*! This class is used by the PathFinder and the CanGo_NavMesh modifier to provide input to IPathObject::SmoothQuery(), and
	to retrieve output values set by IPathObject::SmoothQuery().
	\pp The PathFinder or CanGo_NavMesh modifier set up this query with:
	-	A Bot that should be taken into account during the query.
	-	A "from-to" line segment. During path smoothing, this line segment represents a potential shortcut to a path node farther along the
		path of the Bot, to a node that lies on the other side of a smoothable PathObject.
	\pp The implementation of IPathObject::SmoothQuery() in the PathObject will determine whether or not that PathObject should allow the
	Bot to follow that potential shortcut. If so, it should call SetPassable(), and specify the points at which it will take control of the 
	Bot and cede control of the Bot back to the PathFinder. If not, it should call SetNotPassable().
	\pp For more information, see also :USERGUIDE:"PathObjects and Path Smoothing".
	\ingroup kyruntime_pathobjects */
class PathObjectSmoothQuery
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	/*! \label_constructor */
	PathObjectSmoothQuery():
		m_bot(KY_NULL),
		m_result(PathObjectSmooth_Unknown)
	{}

	/*! \label_constructor
		\param bot				The Bot attempting to smooth its path through the PathObject.
		\param from				The starting point of the line segment that defines the potential smoothed path through the PathObject.
		\param to				The ending point of the line segment that defines the potential smoothed path through the PathObject. */
	PathObjectSmoothQuery(Bot* bot, const Vec3f& from, const Vec3f& to):
		m_bot(bot),
		m_from(from),
		m_to(to),
		m_result(PathObjectSmooth_Unknown)
	{}

	/*! \label_destructor */
	~PathObjectSmoothQuery() {}

	/*! Can be called by IPathObject::SmoothQuery() to indicate that the PathObject will not affect a Bot that smooths its path by passing along the 
		"from-to" line segment defined in this query. */
	void SetDoesNotIntervene() { m_result = PathObjectSmooth_DoesNotIntervene; }

	/*! Can be called by IPathObject::SmoothQuery() to indicate that the PathObject does not allow a Bot to smooth its path by passing along the
		"from-to" line segment defined in this query. */
	void SetNotPassable() { m_result = PathObjectSmooth_NotPassable; }

	/*! Can be called by IPathObject::SmoothQuery() to indicate that the PathObject will allow a Bot to smooth its path by passing along the 
		"from-to" line segment defined in the query.
		\param entryPosition			The point at which the PathObject will take control of the Bot as it moves along its smoothed path.
		\param exitPosition				The point at which the PathObject will return control of the Bot to the PathFinder. */
	void SetPassable(const Vec3f& entryPosition, const Vec3f& exitPosition)
	{
		m_result = PathObjectSmooth_Passable;
		m_entryPosition = entryPosition;
		m_exitPosition = exitPosition;
	}

	/*! Retrieves the Bot set for this query. */
	Bot* GetBot() const { return m_bot; }
	/*! Retrieves the "from" position set for this query. */
	const Vec3f& GetFrom() const { return m_from; }
	/*! Retrieves the "to" position set for this query. */
	const Vec3f& GetTo() const { return m_to; }

	/*! Retrieves the result of this query. */
	PathObjectSmoothResult GetResult() const { return m_result; }
	/*! Indicates whether or not the IPathObject::SmoothQuery() method determined the "from-to" line segment to be passable. */
	bool IsPassable() const { return m_result == PathObjectSmooth_Passable; }
	/*! Retrieves the entry position set by IPathObject::SmoothQuery(), if any. Only valid when IsPassable() returns true. */
	const Vec3f& GetEntryPosition() const { return m_entryPosition; }
	/*! Retrieves the exit position set by IPathObject::SmoothQuery(), if any. Only valid when IsPassable() returns true. */
	const Vec3f& GetExitPosition() const { return m_exitPosition; }


private:
	// inputs
	Bot* m_bot;
	Vec3f m_from;
	Vec3f m_to;

	// outputs
	Vec3f m_entryPosition;
	Vec3f m_exitPosition;
	PathObjectSmoothResult m_result;
};

} // namespace Kaim

#endif // KyRuntime_PathObjectSmoothQuery_H
