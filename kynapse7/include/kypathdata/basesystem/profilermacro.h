/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*! \file
	\ingroup kypathdata_basesystem */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef KY_PROFILERMACRO_H
#define KY_PROFILERMACRO_H

#include "kypathdata/basesystem/basesystem.h"

namespace Kaim
{

	/*	This class is used internally by the #KY_PROFILE macro to represent each block of 
		profiled code. You should not need to interact with this class directly. */
	struct ProfileObj
	{
		ProfileObj(const char* blockName);
		~ProfileObj();

		const char* m_blockName; //< name of the block profiled.
		KyFloat32 m_t0;               //< start time of the block.
		bool m_inspect;          //< KY_TRUE if we profile the block.
	};
#if defined(KY_PROFILE_ENABLED)
/*!	This macro identifies a block of code for performance profiling. For details
	on the profiling framework and the usage of this macro, see 
	:USERGUIDE:"Profiling".
	\ingroup kypathdata_basesystem */
#define KY_PROFILE(name) Kaim::ProfileObj profileObj___(name);
#else
#define KY_PROFILE(name) /* empty */
#endif

} // namespace Kaim

#endif // KY_PROFILER_MACRO_H
