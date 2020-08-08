/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_mesh */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_OBSTACLESACCESSOR_PATHOBJECTOUTLINE_H
#define KY_OBSTACLESACCESSOR_PATHOBJECTOUTLINE_H

#include <kyruntime/mesh/iobstacleaccessor.h>
#include <kyruntime/mesh/obstaclelayer.h>


namespace Kaim
{

class PathObjectOutline;
class RuntimePathObjectControlSpace;


/*! This implementation of IObstacleAccessor is set up and used transparently by the PathObjectManager in order to
	add the control spaces of each blocking PathObject to the PathObject layer maintained by the
	INavMeshLayerManager.
	\pp The outline of the obstacle is set to point directly to the outline maintained
	in an instance of PathObjectOutline that represents each PathObject. If the control space of the
	PathObject changes, the PathObjectOutline and the obstacle outlines maintained in the ObstacleLayer
	are updated automatically.
	\ingroup kyruntime_mesh */
class ObstacleAccessor_PathObjectOutline: public IObstacleAccessor
{
public:
	/*! This class is used internally by ObstacleAccessor_PathObjectOutline to
		record each PathObject it adds to the ObstacleLayer, so that those
		"obstacles" can be removed when their associated \PathObjects are
		removed from the World. Each instance of this class associates the
		instance of ObstacleLayer::ObstacleIt returned by a call to
		ObstacleLayer::AddObstacle() with the instance of PathObjectOutline
		used to provide the obstacle outline in that call to
		ObstacleLayer::AddObstacle().
		\ingroup kyruntime_mesh */
	class Record: public VirtualMemObject
	{
	public:
		/*! Default constructor. For internal use. */
		Record(): m_outline(KY_NULL), m_controlSpace(KY_NULL) {}

		/*! Constructs a new instance of Record from the specified information. For internal use. */
		Record(PathObjectOutline* outline, RuntimePathObjectControlSpace* controlSpace, ObstacleLayer::ObstacleIt obstacleIt): m_outline(outline), m_controlSpace(controlSpace), m_obstacleIt(obstacleIt) {}

		/*! Copy constructor. For internal use. */
		Record(const Record& r): VirtualMemObject(), m_outline(r.m_outline), m_controlSpace(r.m_controlSpace), m_obstacleIt(r.m_obstacleIt) {}

		/*! Sets the state of this object to match the specified instance of Record. */
		Record& operator=(const Record& r) { m_outline = r.m_outline; m_controlSpace = r.m_controlSpace; m_obstacleIt = r.m_obstacleIt; return *this; }

	public:
		PathObjectOutline* m_outline; /*!< A pointer to the PathObjectOutline instance associated with this Record. */
		RuntimePathObjectControlSpace* m_controlSpace; /*!< A pointer to the PathObject ControlSpace whose outline is represented in #m_outline. */
		ObstacleLayer::ObstacleIt m_obstacleIt; /*!< A pointer to the ObstacleLayer::ObstacleIt instance associated with this Record. */
	};

public:
	KY_DECLARE_OBSTACLEACCESSOR(ObstacleAccessor_PathObjectOutline)

public:
	/*! \label_constructor For internal use. */
	ObstacleAccessor_PathObjectOutline(ObstacleLayer* layer): IObstacleAccessor(layer) {}

	/*! \label_destructor For internal use. */
	~ObstacleAccessor_PathObjectOutline() {}

	/*! This implementation of IObstacleAccessor::GetInput() is empty, since
		this class adds and removes PathObjectOutlines from the ObstacleLayer
		in its AddPathObjectOutline() and RemovePathObjectOutline() methods. */
	virtual void GetInput() {}

	/*! Adds the specified outline to the ObstacleLayer as a new obstacle, and adds a new Record to the #m_outlines member to track the registration.
		Called transparently each time a PathObject is activated in the World. For internal use. */
	void AddPathObjectOutline(PathObjectOutline* poOutline);

	/*! Removes the specified outline from the ObstacleLayer. Called transparently each time a PathObject is de-activated in the World. For internal use. */
	void RemovePathObjectOutline(PathObjectOutline* poOutline);

protected:
	List<Record> m_outlines; /*!< Maintains a record of all outlines added to the ObstacleLayer by this instance of ObstacleAccessor_PathObjectOutline. */
};

} // namespace Kaim

#endif // KY_OBSTACLESACCESSOR_PATHOBJECTOUTLINE_H
