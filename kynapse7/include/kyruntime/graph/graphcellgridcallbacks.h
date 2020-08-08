/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_graph */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KyPathData_GrahCellCallbacks_H
#define KyPathData_GrahCellCallbacks_H

#include <kypathdata/generic/memory.h>
#include <kypathdata/containers/list.h>

namespace Kaim
{

class StitchedGraphCell;

/*! Used internally by the GrahCell to manage the lists of callback functions it should invoke when a graphCell is activated 
	or deactivated in the GraphCellGrid. */
class GraphCellGridCallBacks
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	/*! \label_constructor For internal use. */
	GraphCellGridCallBacks() {}


	~GraphCellGridCallBacks();

	/*! \label_clear */
	void Clear();

	/*!	Defines a type for a callback function that you can use to process notifications each time a StitchedGraphCell
		is activated in the graphCellGrid. See also GraphManager::RegisterOnActivateGraphCellCB().
		\param stitchedGraphCell	the stitchedGraphCell that has been activated.
		\param userData				A void pointer that corresponds to the \c userData argument provided to the
									RegisterOnActivateGraphCellCB() method when you added your callback function. */
	typedef void (*OnActivateGraphCellCB)(StitchedGraphCell& stitchedGraphCell, void* userData);

	/*!	Defines a type for a callback function that you can use to process notifications each time a StitchedGraphCell
		is deactivated in the graphCellGrid. See also GraphManager::RegisterOnDeActivateGraphCellCB().
		\param stitchedGraphCell	the stitchedGraphCell that is going to be deActivated.
		\param userData				A void pointer that corresponds to the \c userData argument provided to the
									RegisterOnDeActivateGraphCellCB() method when you added your callback function. */
	typedef void (*OnDeActivateGraphCellCB)(StitchedGraphCell& stitchedGraphCell, void* userData);



	/*!	Called transparently by Database::RegisterOnActivateGraphCellCB(). */
	KyBool RegisterOnActivateGraphCellCB(OnActivateGraphCellCB cb, void* userData);

	/*!	Called transparently by Database::RegisterOnDeActivateGraphCellCB(). */
	KyBool RegisterOnDeActivateGraphCellCB(OnDeActivateGraphCellCB cb, void* userData);

	/*!	Called transparently by Database::RegisterOnActivateGraphCellCB(). */
	KyBool UnRegisterOnActivateGraphCellCB(OnActivateGraphCellCB cb, void* userData);

	/*!	Called transparently by Database::RegisterOnDeActivateGraphCellCB(). */
	KyBool UnRegisterOnDeActivateGraphCellCB(OnDeActivateGraphCellCB cb, void* userData);


	/*!	Called transparently by GrahCell::CallOnAddGraphCBs(). */
	void CallOnActivateGraphCellCBs(StitchedGraphCell& stitchedGraphCell) const
	{
		for (List<CbRecord<OnActivateGraphCellCB> >::ConstIterator it = m_onActivateGraphCellCallbacks.GetFirst(); it.IsValid(); ++it)
			it->m_cb(stitchedGraphCell, it->m_userData);
	}

	/*!	Called transparently by GrahCell::CallOnRemoveGraphCBs(). */
	void CallOnDeActivateGraphCellCBs(StitchedGraphCell& stitchedGraphCell) const
	{
		for (List<CbRecord<OnDeActivateGraphCellCB> >::ConstIterator it = m_onDeActivateGraphCellCallbacks.GetFirst(); it.IsValid(); ++it)
			it->m_cb(stitchedGraphCell, it->m_userData);
	}


	template <typename CB> struct CbRecord: public VirtualMemObject
	{
		CbRecord(): m_cb(0), m_userData(0) {}
		CbRecord(CB cb, void* userData): m_cb(cb), m_userData(userData) {}
		CB m_cb;
		void* m_userData;
	};


	List<CbRecord<OnActivateGraphCellCB> > m_onActivateGraphCellCallbacks;
	List<CbRecord<OnDeActivateGraphCellCB> > m_onDeActivateGraphCellCallbacks;
};

} // namespace Kaim

#endif // KyPathData_GrahCellCallbacks_H
