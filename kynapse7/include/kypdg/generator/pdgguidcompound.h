/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*! \file
	\ingroup kypdg */

// primary contact: GUAL - secondary contact: NOBODY
#ifndef KyPdg_PdgGuidCompound_H
#define KyPdg_PdgGuidCompound_H


#include <kypathdata/generic/memory.h>
#include <kypathdata/generic/guidcompound.h>
#include <kypathdata/containers/array.h>


namespace Kaim
{

/*! This class is a wrapper around an array of KyGuid objects.
	\ingroup kypdg */
class PdgGuidCompound
{
	KY_DEFINE_NEW_DELETE_OPERATORS

public:
	/*! \label_constructor Creates a blank array with no KyGuid objects. */
	PdgGuidCompound() : m_timeStamp(0) {}

	/*! Retrieves the first KyGuid in the array, or KyGuid::s_defaultGuid if none. */
	KyGuid GetMainGuid() const { return m_guids.GetCount() == 0 ?  KyGuid::s_defaultGuid : m_guids[0]; }

	/*! Returns \c true if this object has more KyGuid objects in its array than the specified PdgGuidCompound, or false if it has fewer.
		If they have the same number of KyGuid objects, iterates through each KyGuid in the arrays. For internal use. */
	bool operator<(const PdgGuidCompound& other) const; // ignore timestamp !

	/*! Returns \c true if all GUIDs in the array maintained by this object match the GUIDs in the specified PdgGuidCompound. */
	bool AreGuidsEqual(const PdgGuidCompound& other) const;

	/*! Adds the specified KyGuid to the array maintained by this object. */
	void AddGuid(KyGuid guid);

	/*! Sets the last modified timestamp for this object to the specified value. For internal use. */
	void SetTimeStamp(KyUInt32 timeStamp) { m_timeStamp = timeStamp; }

	/*! Sorts the KyGuid objects in the array. For internal use. */
	void Sort();

	/*! For internal use. */
	void Init(const GuidCompound& guidCompound);

private:
	bool operator==(const PdgGuidCompound&);
	bool operator!=(const PdgGuidCompound&);

public:
	Array<KyGuid> m_guids; /*!< The array of KyGuid objects. Do not modify directly. */
	KyUInt32 m_timeStamp; /*!< The last modification date of this object. Do not modify directly. */
};


}


#endif

