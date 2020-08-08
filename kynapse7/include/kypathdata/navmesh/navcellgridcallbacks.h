/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_graph */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KyPathData_NavCellGridCallBacks_H
#define KyPathData_NavCellGridCallBacks_H

#include <kypathdata/generic/memory.h>
#include <kypathdata/containers/list.h>

namespace Kaim
{

class StitchedNavCell;

/*! Used internally by the NavMeshManager to manage the lists of callback functions it should invoke when a navCell is activated 
	or deactivated in the NavCellGrid. */
class NavCellGridCallBacks
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	/*! \label_constructor For internal use. */
	NavCellGridCallBacks() {}

	~NavCellGridCallBacks();


	/*! \label_clear */
	void Clear();

	/*!	Defines a type for a callback function that you can use to process notifications each time a StitchedNavCell
		is activated in the navCellGrid. See also Database::RegisterOnActivateNavCellCB().
		\param stitchedNavCell		the stitchedNavCell that has been activated.
		\param userData				A void pointer that corresponds to the \c userData argument provided to the
									RegisterOnActivateNavCellCB() method when you added your callback function. */
	typedef void (*OnActivateNavCellCB)(StitchedNavCell& stitchedNavCell, void* userData);

	/*!	Defines a type for a callback function that you can use to process notifications each time a StitchedNavCell
		is deactivated in the navCellGrid. See also Database::RegisterOnDeActivateNavCellCB().
		\param stitchedNavCell		the stitchedNavCell that is going to be deActivated.
		\param userData				A void pointer that corresponds to the \c userData argument provided to the
									RegisterOnDeActivateNavCellCB() method when you added your callback function. */
	typedef void (*OnDeActivateNavCellCB)(StitchedNavCell& stitchedNavCell, void* userData);



	/*!	Called transparently by Database::RegisterOnActivateNavCellCB(). */
	KyBool RegisterOnActivateNavCellCB(OnActivateNavCellCB cb, void* userData);

	/*!	Called transparently by Database::RegisterOnDeActivateNavCellCB(). */
	KyBool RegisterOnDeActivateNavCellCB(OnDeActivateNavCellCB cb, void* userData);


	/*!	Called transparently by Database::RegisterOnActivateNavCellCB(). */
	KyBool UnRegisterOnActivateNavCellCB(OnActivateNavCellCB cb, void* userData);

	/*!	Called transparently by Database::RegisterOnDeActivateNavCellCB(). */
	KyBool UnRegisterOnDeActivateNavCellCB(OnDeActivateNavCellCB cb, void* userData);


	/*!	Called transparently by NavCellGrid::CallOnAddGraphCBs(). */
	void CallOnActivateNavCellCBs(StitchedNavCell& stitchedNavCell) const
	{
		for (List<CbRecord<OnActivateNavCellCB> >::ConstIterator it = m_onActivateNavCellCallbacks.GetFirst(); it.IsValid(); ++it)
			it->m_cb(stitchedNavCell, it->m_userData);
	}

	/*!	Called transparently by NavCellGrid::CallOnRemoveGraphCBs(). */
	void CallOnDeActivateNavCellCBs(StitchedNavCell& stitchedNavCell) const
	{
		for (List<CbRecord<OnDeActivateNavCellCB> >::ConstIterator it = m_onDeActivateNavCellCallbacks.GetFirst(); it.IsValid(); ++it)
			it->m_cb(stitchedNavCell, it->m_userData);
	}


	template <typename CB> struct CbRecord: public VirtualMemObject
	{
		CbRecord(): m_cb(0), m_userData(0) {}
		CbRecord(CB cb, void* userData): m_cb(cb), m_userData(userData) {}
		CB m_cb;
		void* m_userData;
	};


	List<CbRecord<OnActivateNavCellCB> > m_onActivateNavCellCallbacks;
	List<CbRecord<OnDeActivateNavCellCB> > m_onDeActivateNavCellCallbacks;
};

} // namespace Kaim

#endif // KyPathData_NavCellGridCallBacks_H
