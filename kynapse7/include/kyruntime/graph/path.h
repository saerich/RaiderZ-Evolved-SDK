/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_graph */

// primary contact: JUBA - secondary contact: NOBODY
#ifndef KY_PATH_H
#define KY_PATH_H

#include <kypathdata/graph/graphcell.h>

#include <kypathdata/generic/types.h>
#include <kypathdata/containers/array.h>
#include <kypathdata/containers/list.h>
#include <kypathdata/math/vec3f.h>
#include <kyruntime/graph/graphvertexsafeptr.h>
#include <kyruntime/graph/graphedgesafeptr.h>


namespace Kaim
{

class IPathObject;


/*! Enumerates the different elements of data that can be rendered in a call to Path::Render() or PathFinder::Render().
	\pp Note that only the #PathRender_Nodes and #PathRender_NodeUids values are read by Path::Render(). The other elements
	in this enumeration are read only by PathFinder::Render().
	\ingroup kyruntime_graph */
enum PathRenderFlags
{
	// Path rendering options
	PathRender_Nodes = 1<<0, /*!< Renders \PathNodes as blue points. */
	PathRender_NodeUids = 1<<1, /*! Writes the index value of the vertex that corresponds to each PathNode in white. */

	// Path following
	PathRender_CurrentPathNode = 1<<8, /*!< Draws an orange line from the current position of the Bot to its current PathNode. */
	PathRender_TargetPoint = 1<<9, /*!< Draws a red line from the current position of the Bot to its current target point. */
	PathRender_GoTo = 1<<10, /*!< If the Bot is using the Goto_GapDynamicAvoidance modifier, this renders the current data maintained by that modifier. */

	// LPF
	PathRender_LocalPath = 1<<16, /*!< Renders the edges in the local path currently being followed by the Bot, if any. */
	/*! Renders the current passability status of each edge in the taboo edge list maintained by the Local PathFinding system. 
		-	Edges drawn in cyan are passable, but are still considered to be blocked by the IEdgeStatusAwareness modifier.
		-	Edges drawn in red are impassable, blocked by Local PathFinding obstacles.
		-	Edges drawn in yellow have an unknown passability status. */
	PathRender_EdgeStatus = 1<<17, 

	// Localization
	PathRender_BotLocalization = 1<<24, /*!< Renders any PathObject control spaces, connection spaces, and NavMesh triangles that contain the position of the Bot. */
	PathRender_PathObjects = 1<<25, /*!< Renders any edge in the Path that is linked to a PathObject in orange. */

	PathRender__dummy__forceEnumSize = KYFORCEENUMSIZEINT
};


///////////////////////////////////////////////////////////////////////////////////////////
// PathNode
///////////////////////////////////////////////////////////////////////////////////////////

/*! The PathNode class represents a single point within a Path.
	\ingroup kyruntime_graph */
class PathNode
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	/*! \label_constructor */
	PathNode(): m_firstPathObject(KY_NULL) {}
	/*! \label_destructor */
	~PathNode()
	{
		m_vertexSafePtr.Invalidate();
		m_nextEdgeSafePtr.Invalidate();
	}

public:
	GraphVertexSafePtr m_vertexSafePtr; /*!< The GraphVertexSafePtr that identifies the vertex in the spatial graph that lies at the position of this PathNode, if any. Do not modify. */
	Vec3f m_position; /*!< The position of the PathNode. Do not modify. */
	GraphEdgeSafePtr m_nextEdgeSafePtr; /*!< The GraphEdgeSafePtr of the edge in the spatial graph that corresponds to the edge in the Path that leads to the next PathNode, or an invalid GraphEdgeSafePtr if this is the last PathNode in the Path. */
	IPathObject* m_firstPathObject; /*!< A pointer to the first PathObject linked to the edge that leads to this PathNode. */
};

/*!	A bi-directional iterator that allows you to iterate through the \PathNodes stored in a Path.
	\ingroup kyruntime_graph */
typedef List<PathNode>::Iterator PathNodeIterator;

///////////////////////////////////////////////////////////////////////////////////////////
// Path
///////////////////////////////////////////////////////////////////////////////////////////

/*! This class stores a path through a spatial graph, computed through a call to GraphManager::Astar().
	It is a list of \PathNodes, each of which typically corresponds to a vertex in the Graph.
	\pp You can iterate through the \PathNodes in a Path as follows: \code
	for (Kaim::PathNodeIterator it = path.GetFirst(); it.IsValid(); ++it)
	{
		Kaim::Draw3DPoint(it->m_position, 255,255,255); // use each PathNode here
	} \endcode
	\ingroup kyruntime_graph */
class Path
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	typedef List<PathNode>::ConstIterator ConstIterator;

	/*! \label_constructor */
	Path() {}

	/*! \label_destructor */
	~Path() { }

	/*! \label_clear */
	void Clear() { m_pathNodes.Clear(); }

	/*! Renders the Path for debugging purposes.
		\param flags		A bitmask composed of elements from the #PathRenderFlags enumeration, which identifies
							the elements of data that should be rendered. Edges in the path are always drawn in green. */
	void Render(KyUInt32 flags = PathRender_PathObjects) const;

	// This set of functions allows to go through the content of the path.

	/*! Retrieves an iterator that points to the first PathNode in the Path. */
	PathNodeIterator GetFirst() { return m_pathNodes.GetFirst(); }

	/*! Retrieves an iterator that points to the last PathNode in the Path. */
	PathNodeIterator GetLast() { return m_pathNodes.GetLast(); }

	/*! Retrieves a const iterator that points to the first PathNode in the Path. */
	ConstIterator GetFirst() const { return m_pathNodes.GetFirst(); }

	/*! Retrieves a const iterator that points to the last PathNode in the Path. */
	ConstIterator GetLast() const { return m_pathNodes.GetLast(); }

	/*! Retrieves the number of \PathNodes in the Path. */
	KyUInt32 GetCount() const { return m_pathNodes.GetCount(); }

	/*! Adds a new PathNode to the end of the Path, re-allocating memory if necessary. */
	PathNodeIterator PushBack(const PathNode& node)
	{
		return m_pathNodes.Insert(node);
	}

	/*! Adds a new PathNode to the beginning of the Path, re-allocating memory if necessary. */
	void PushFront(const PathNode& node)
	{
		m_pathNodes.Insert(node, m_pathNodes.GetFirst());
	}

	/*! Removes the specified PathNode from the Path. */
	void Erase(PathNodeIterator it) { m_pathNodes.Erase(it); }

protected:
	List<PathNode> m_pathNodes; /*< Stores the list of PathNodes in the Path. */
};

} // namespace Kaim

#endif // KY_PATH_H
