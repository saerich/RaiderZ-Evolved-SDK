/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_gapda */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_GAPMANAGER_H
#define KY_GAPMANAGER_H

#include <kyruntime/core/service.h>

#include <kypathdata/containers/list.h>


namespace Kaim
{
///////////////////////////////////////////////////////////////////////////////////////////
// Gap
///////////////////////////////////////////////////////////////////////////////////////////

/*! Used by the GapManager to store information about each nearby Body. For internal use only.
	\ingroup kyruntime_gapda */
class Gap
{
	KY_DEFINE_NEW_DELETE_OPERATORS
	friend class GapManager;

public:
	Gap() {}
	Gap(KyFloat32 a, KyFloat32 b, KyFloat32 tti = KyFloat32MAXVAL, KyFloat32 frontalSpeed = KyFloat32MAXVAL, KyBool isInnerGap = KY_FALSE):
		m_a(a),
		m_b(b),
		m_tti(tti),
		m_frontalSpeed(frontalSpeed),
		m_isInnerGap(isInnerGap) {}
	~Gap() {};

	KyFloat32 GetBegin() const {return m_a;}
	KyFloat32 GetEnd() const {return m_b;}
	KyFloat32 GetTTI() const {return m_tti;}
	KyFloat32 GetFrontalSpeed() const {return m_frontalSpeed;}
	KyBool IsInnerGap() const {return m_isInnerGap;}

protected:
	KyFloat32 m_a;
	KyFloat32 m_b;
	KyFloat32 m_tti;
	KyFloat32 m_frontalSpeed;
	KyBool m_isInnerGap;
};


///////////////////////////////////////////////////////////////////////////////////////////
// GapManager
///////////////////////////////////////////////////////////////////////////////////////////

/*! This class defines a WorldService that manages the computation of the collision diagrams
	used by the Goto_GapDynamicAvoidance pathfinding modifier. You should not interact with the
	GapManager directly, but you must activate it for your World any time you
	use the PathFinder with the Goto_GapDynamicAvoidance pathfinding modifier.
	\ingroup kyruntime_gapda */
class GapManager: public WorldService
{
public:
	KY_DECLARE_WORLDSERVICE(GapManager)

public:


	/* ! Resets the diagram by clearing the gap list and adding a new gap
		the full width of the diagram. For internal use only.
		\param a Minimal abscissa of the diagramm
		\param b Maximal abscissa of the diagramm
		\param tti Time-depth of the diagramm
	*/
	void Reset(KyFloat32 a, KyFloat32 b, KyFloat32 tti = KyFloat32MAXVAL);

	/* ! Inserts the specified gap into the current diagram.
		If the diagram is almost full, only new gaps with time to impact lower than
		the current minimal time to impact are inserted.
		\param gap The gap to be inserted have to be coherent and compatible with
		the diagramm, i.e. its limits have to respect the following inequalities :
		1. diagramm.minAbscissa <= gap.minAbscissa
		2. gap.minAbscissa < gap.maxAbscissa
		3. gap.maxAbscissa <= diagramm.maxAbscissa
		\return KY_TRUE if the gap have been inserted, KY_FALSE otherwise.
	*/
	KyBool AddGap(const Gap& gap);

	/* ! Accessor to the gap list.
		\return The gap list reference.
	*/
	const List<Gap>& GetGaps() const { return m_gaps; }

	/* ! \return The current minimal time to impact of the gap list. */
	inline KyFloat32 GetMinTTI() const { return m_minTTI; }

	/* !
		\return The maximal time to impact of the gap manager.
		(NB: this value is valid until next call to Reset(...) member function.)
	*/
	inline KyFloat32 GetMaxTTI() const { return m_maxTTI; }

protected:
	explicit GapManager(Kaim::World& world);
	~GapManager();

	KyBool SearchNewGapBegin(KyFloat32 a, KyFloat32 b, KyFloat32 tti, List<Gap>::Iterator gStart, List<Gap>::Iterator gEnd, KyFloat32 &nxtBegin, List<Gap>::Iterator &nxtG);
	KyBool SearchNewGapEnd(KyFloat32 a, KyFloat32 b, KyFloat32 tti, List<Gap>::Iterator gStart, List<Gap>::Iterator gEnd, KyFloat32 &nxtEnd, List<Gap>::Iterator &nxtG);

protected:
	List<Gap> m_gaps;
	KyFloat32 m_aMin;
	KyFloat32 m_bMax;
	KyFloat32 m_minTTI;
	KyFloat32 m_maxTTI; //< TTI corresponding to maximal gap TTI value (i.e. TTI of top diagram gap)
};

} // namespace Kaim

#endif // KY_GAPMANAGER_H
