/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_core */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_VARRESTORER_H
#define KY_VARRESTORER_H

#include <kypathdata/generic/memory.h>


namespace Kaim
{

/*!	This class sets a variable immediately to a specified value, and to a different specified value 
	when the VarRestorer object is destroyed. This is typically used to reset a variable to its 
	previous value automatically when a code block or function is exited and the VarRestorer object
	is removed from the stack.
	\tparam T		The type of the variable to be managed by this object.
	\ingroup kyruntime_core */
template<typename T>
class VarRestorer : public VirtualMemObject
{
public:
	/*!	Constructs a new instance of VarRestorer for a specified variable, sets the 
		value of the variable to \c v1 immediately, and sets the value of the variable
		to \c v2 when the VarRestorer is destroyed.
		\param var				The variable to be managed by this object.
		\param v1				The value to set for \c var immediately.
		\param v2				The value to set for \c var when the VarRestorer is destroyed. */
	VarRestorer(T &var, const T& v1, const T& v2) : m_var(var), m_v2(v2) { m_var = v1; }
	
	/*!	Constructs a new instance of VarRestorer for a specified variable, sets the 
		value of the variable to \c v1 immediately, and resets the value of the variable
		to its previous value when the VarRestorer is destroyed.
		\param var				The variable to be managed by this object.
		\param v1				The value to set for \c var immediately. */
	VarRestorer(T &var, const T& v1) : m_var(var), m_v2(var) { m_var = v1; }
	
	/*!	Default destructor. */
	~VarRestorer() { m_var = m_v2; }

private:
	// to avoid warning C4512 ("assignment operator could not be generated")
	const VarRestorer& operator = (const VarRestorer&);


protected:
	T& m_var; //< Variable to handle.
	T m_v2;   //< Value to restore.
};

} // namespace Kaim

#endif // KY_VARRESTORER_H

