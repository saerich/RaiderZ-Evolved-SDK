/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_pathobjects */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_RUNTIMEPATHOBJECTCONNEXIONLINK_H
#define KY_RUNTIMEPATHOBJECTCONNEXIONLINK_H

#include <kyruntime/graph/stitchedgraphcell.h>
#include <kyruntime/graph/graphedgeptr.h>
#include <kyruntime/graph/graphedgesafeptr.h>
#include <kypathdata/pathobject/pathobjectdatatypes.h>

namespace Kaim
{

class RuntimePathObjectTopology;
class IPathObjectConnexionLink;

/*! A runtime representation of a PathObject connection link, created transparently when a PathObject
	is added to or removed from a World. The methods specific to this class are intended for internal
	use, except for IsConnectedToThisEdge().
	\ingroup kyruntime_pathobject */
class RuntimePathObjectConnexionLink
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	RuntimePathObjectConnexionLink(
		RuntimePathObjectTopology* parentTopology,
		IPathObjectConnexionLink* userConnexionLink);

	~RuntimePathObjectConnexionLink();

	const PathObjectConnexionSpaceIdx& GetStartConnexionSpaceIdx() const;
	const PathObjectConnexionSpaceIdx& GetEndConnexionSpaceIdx() const;

	/*! Read accessor for #m_parentTopology. */
	const RuntimePathObjectTopology* GetTopology() const { return m_parentTopology; }

	/*! Read accessor for #m_userConnexionLink. */
	IPathObjectConnexionLink* GetUserConnexionLink() { return m_userConnexionLink; }

	//////////////////////////////////////////////////////////////////////////
	// Runtime specific methods
	//////////////////////////////////////////////////////////////////////////

	void SetEdgePtr(GraphEdgePtr& edgePtr) { m_edgePtr = edgePtr; }

	/*! Tests whether or not this connection link is connected to the specified Graph edge.*/
	bool IsConnectedToThisEdge(const GraphEdgeSafePtr& edgeSafePtr) const;

	GraphEdgeSafePtr GetEdgeSafePtr() const { return m_edgePtr.ConvertToSafeEdgePtr(); }
	void ClearGraphEdgePtr() { m_edgePtr.Invalidate(); }
	
public:
	RuntimePathObjectTopology* m_parentTopology; /*! Stores a pointer to the RuntimePathObjectTopology using this object. */
	IPathObjectConnexionLink* m_userConnexionLink; /*!< Stores a pointer to the IPathObjectConnexionLink used to construct this object. */
	GraphEdgePtr m_edgePtr;
};

} // namespace Kaim

#endif // #ifndef KY_RUNTIMEPATHOBJECTCONNEXIONLINK_H
