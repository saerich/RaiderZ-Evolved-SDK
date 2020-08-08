/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_lpf */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_LPFCONSTRAINT_H
#define KY_LPFCONSTRAINT_H

#include <kyruntime/pathfinding/pathfinder.h>

#include <kyruntime/graph/astarcontext.h>
#include <kyruntime/graph/traversalmanager.h>


namespace Kaim
{

class LpfConstraint;


/*!	Used by LpfConstraint to maintain temporary Local PathFinding data during an A* calculation. 
	For internal use.
	\ingroup kyruntime_lpf */
class AstarContextLpfAdditionalData: public VirtualMemObject
{
protected:
	class AStarContextObserverForLpfConstraintData : public AStarContextObserver
	{
	public:
		AStarContextObserverForLpfConstraintData() : m_asCxtAddData(KY_NULL) {}

		void Init(AstarContextLpfAdditionalData& asCxtAddData) { m_asCxtAddData = &asCxtAddData; }

		void OnResize(const AstarContext& astarContext, KyUInt32 oldSize)
		{
			m_asCxtAddData->Resize(astarContext, oldSize);
		}

	protected:
		AstarContextLpfAdditionalData* m_asCxtAddData;
	};

public:
	AstarContextLpfAdditionalData():
		m_lpfConstraint(KY_NULL),
		m_context(KY_NULL),
		m_dataCellCount(0),
		m_data(KY_NULL),
		m_observer()
	{
		m_observer.Init(*this);
	}
	~AstarContextLpfAdditionalData();

	KyBool Init(LpfConstraint* lpfConstraint, AstarContext* astarContext);
	void ReInit();

	void DeAllocate();

	void Resize(const AstarContext& astarContext, KyUInt32 oldSize);


protected:
	LpfConstraint* m_lpfConstraint; // used to update its m_currentData when resized

public:
	AstarContext* m_context;
	KyUInt32 m_dataCellCount;
	LpfAStarData* m_data;
	AStarContextObserverForLpfConstraintData m_observer;
};



/*!	This class represents a special kind of IConstraint used internally by the LpfManager and PathFinder 
	to take \Obstacles into account transparently during A* calculations and from within pathfinding 
	modifiers. 
	\pp The GetCost() method internally invokes the current IConstraint set for the 
	PathFinder to determine the cost of moving between the specified start and end points. 
	In addition, when one or more \LpfAreas are detected along the line segment provided, the 
	LpfConstraint adds a supplementary cost to indicate the extra cost of bypassing those \Obstacles. 
	If the \Obstacles cannot be bypassed (for instance, crates that fill the entire width a corridor), 
	GetCost() returns #KY_FALSE to indicate that the corresponding edge is blocked.
	\pp You should never need to derive this class yourself. However, you may want to use an LpfConstraint 
	yourself any time you need to evaluate the cost of moving from one point to another, 
	in order to take into account the added cost of \Obstacles that lie on that line segment. 
	\pp You can retrieve an instance of this class by calling PathFinder::GetLpfConstraint(). Note, however, 
	that if Local PathFinding is not correctly set up for that instance of the PathFinder, 
	PathFinder::GetLpfConstraint() instead returns the current IConstraint set for the 
	PathFinder.
	\ingroup kyruntime_lpf */
class LpfConstraint: public IConstraint
{
public:
	KY_DECLARE_CONSTRAINT(LpfConstraint);


public:
	/*!	\label_baseconstructor */
	LpfConstraint();
	
	/*!	\label_virtualdestructor */
	~LpfConstraint();

	//////////////////////////////////////////////////////////////////////////
	// Specific methods
	//////////////////////////////////////////////////////////////////////////

	/*!	Initializes the LpfConstraint. Called automatically during the initialization of the LpfManager. */
	virtual KyBool Init(TraversalManager& traversalManager);

	//////////////////////////////////////////////////////////////////////////
	// Inherited methods
	//////////////////////////////////////////////////////////////////////////
	virtual KyResult SetCurrentAstarContext(AstarContext* astarContext);
	virtual KyResult UnsetCurrentAstarContext();
	virtual void OnStartAstar(const GraphVertexPtr& start, const GraphVertexPtr& dest);
	virtual void OnFinishAstar(const GraphVertexPtr& start, const GraphVertexPtr& dest);
	virtual void OnPauseAstar(const GraphVertexPtr& start, const GraphVertexPtr& dest);
	virtual void OnResumeAstar(const GraphVertexPtr& start, const GraphVertexPtr& dest);
	virtual KyBool GetCost(Bot* entity, const PointWrapper& from, const PointWrapper& to, KyFloat32& cost);

	//////////////////////////////////////////////////////////////////////////
	// Internal use only !
	//////////////////////////////////////////////////////////////////////////
	/*! For internal use. */
	void UpdateCurrentAddDataPointer(LpfAStarData* oldData, LpfAStarData* newData)
	{
		if (m_currentAddData == oldData)
			m_currentAddData = newData;
	}

protected:
	KyResult SetupCurrentInternals();
	void UnsetCurrentInternals();
	virtual KyBool GetCost_LpfConstraint(const PointWrapper& from, const PointWrapper& to, KyFloat32& cost);
	virtual KyBool GetCost_DefaultConstraint(Bot* entity, const PointWrapper& from, const PointWrapper& to, KyFloat32& cost);


protected:
	Array<AstarContextLpfAdditionalData> m_asCxtAddData; /*< Temporary storage of Local PathFinding data during an A* calculation. For internal use. Do not modify. */
	LpfAStarData* m_currentAddData; /*< For internal use. Do not modify. */
	Bot* m_currentUser; /*< The Bot carrying out the current A* calculation. For internal use. Do not modify. */
	PathFinder* m_currentPF; /*< The PathFinder carrying out the current A* calculation. For internal use. Do not modify. */
	IConstraint* m_userConstraint; /*< The pathfinding constraint set for the PathFinder. Do not modify. */
};

} // namespace Kaim

#endif // KY_LPFCONSTRAINT_H
