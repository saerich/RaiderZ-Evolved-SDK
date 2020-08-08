/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_core */

// primary contact: LAPA - secondary contact: NOBODY
#ifndef INFOLIB_H
#define INFOLIB_H

#include <kypathdata/generic/basetypes.h>

namespace Kaim
{
/*!	Outputs the build mode, target platform, library name, version and build date of this library.
	The text is logged using debug messages.
	\return #KY_TRUE if the text was successfully logged, or #KY_FALSE if the BaseSystem has not yet been
	initialized.
	\ingroup kyruntime_core */
KyBool ShowInfoAboutLib();

/*!	Retrieves a string that contains the version number of this library.
	\ingroup kyruntime_core*/
const char* GetKynapseVersion();

}

#endif // INFOLIB_H

