/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_pathfinding */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_REFINEGOAL_LPFCOMPATIBLE
#define KY_REFINEGOAL_LPFCOMPATIBLE

#include <kyruntime/pathfinding/irefinegoal.h>
#include <kyruntime/lpf/ilpfmanager.h>


namespace Kaim
{
/*!	This implementation of the IRefineGoal modifier is similar to RefineGoal_NearestInsideNavMesh. It tests the 
	destination point provided to the PathFinder::FindNextMove() method against the static NavMesh and dynamic
	\Obstacles reflected in the \LpfContext set for the PathFinder. It returns the nearest position to the original 
	destination point that is both within the boundaries of the static NavMesh and that is not inside any \LpfAreas. 
	\pp This refined goal is re-computed any time either of the following conditions occurs while the Bot is following 
	its Path:
	-	The destination point passed to PathFinder::FindNextMove() moves a distance greater than the distance threshold
		set through a call to MinimalGoalImportantMove(KyFloat32).
	-	Any \LpfAreas are updated in the vicinity of the refined goal. 
	-	A previously valid refined goal becomes covered by an LpfArea. 
	\pp Note that this class does not take into account any \Obstacles that are contained in other blocking 
	\ObstacleLayers, or that have not yet been merged into Local PathFinding aggregates. This is not a problem for the
	PathFinder, but if you use this refined goal for other purposes you should be aware that it is not guaranteed to
	be outside the boundaries of non-LPF \Obstacles.
	\ingroup kyruntime_pathfinding */
class RefineGoal_LpfCompatible: public IRefineGoal
{
public:
	KY_DECLARE_REFINEGOAL(RefineGoal_LpfCompatible)


public:
	/*!	\label_constructor */
	RefineGoal_LpfCompatible();

	virtual void ReInit();

	virtual void RefineGoal(const PointWrapper& goal, PointWrapper& refinedGoal);
	/*!	Called by RefineGoal(). For internal use. */
	virtual void RefineGoal_FromOutside(const PointWrapper& goal, PointWrapper& refinedGoal);
	/*!	This method is called automatically for each possible candidate position within the boundaries of the NavMesh. 
		The candidate with the least cost is adopted as the final refined goal.
		\pp The default implementation of this method returns the straight-line distance between \c intermediate and \c dest.
		This ensures that the refined goal will be as close as possible to the original destination point while remaining
		within the boundaries of the NavMesh. However, it does not perform any tests against the static geometry; therefore,
		the refined goal may be blocked from the final destination point by static obstacles that exist outside the
		boundaries of the NavMesh.
		\param intermediate				The candidate position to test.
		\param dest						The original destination point.
		\param radius					The radius of the Entity. Not used in the default implementation. */
	virtual KyFloat32 GetCost(const Vec3f& intermediate, const PointWrapper& dest, KyFloat32 radius);


	/*! Retrieves the value set through a call to MinimalGoalImportantMove(KyFloat32). */
	KyFloat32 MinimalGoalImportantMove() const { return sqrtf(m_sqMinimalGoalImportantMove); }

	/*! Specifies the minimum distance that the destination point must move in order to trigger a recomputation
		of the refined goal. 
		Note, however, that if the destination point moves enough that it triggers the IDetectGoalChanged modifier,
		the entire path is always recomputed, regardless of the value of this parameter. 
		\units			meters
		\acceptedvalues	any positive value
		\defaultvalue	\c 0.1f */
	void MinimalGoalImportantMove(KyFloat32 val) 
	{ 
		KY_FUNCTION("RefineGoal_LpfCompatible::MinimalGoalImportantMove");
		KY_ASSERT(val > 0.f, ("Invalid value (%.2f), 'MinimalGoalImportantMove' must be > 0.", val));
		m_sqMinimalGoalImportantMove = val; 
		m_sqMinimalGoalImportantMove *= m_sqMinimalGoalImportantMove;
	}

protected:
	virtual KyResult CheckModifierDependencies(const Modifiers&, const Kaim::ILpfManager::LpfContext*);
	virtual void OnSetPathFinder();

	KyFloat32 m_sqMinimalGoalImportantMove; // Square length of the minimal goal move before refined goal is reconsidered

	ILpfManager::LpfContext* m_lpfContext;
	Vec3f m_cacheGoal; // Previous row goal position
	Vec3f m_cacheRefinedGoal; // Previous valid refined goal position
	KyFloat32 m_lpfDate; // Date when refined goal was last reconsidered
	const ILpfManager::LpfArea* m_cachedLpfArea; // This is used to precisely know if movement on LpfContext has to be taken into acount
};

} // namespace Kaim

#endif // KY_REFINEGOAL_LPFCOMPATIBLE
