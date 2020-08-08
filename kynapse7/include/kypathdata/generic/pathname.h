/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kypathdata_generic */

#ifndef KY_PATHNAME_H
#define KY_PATHNAME_H

#include <kypathdata/generic/basetypes.h>

namespace Kaim
{
	/*!	Offers utilities for managing file and path names.
		\ingroup kypathdata_generic */
	struct PathName
	{
		/*!	Formats a specified path and file name for use on the current target platform. This 
			function converts between forward-slash and back-slash characters, and removes illegal
			characters from the path.
			\param in			The path and file name to convert.
			\param[out] out		A pre-allocated memory buffer in which the generated path name is
								written.
			\return #KY_TRUE if the path name was generated and written successfully, or #KY_FALSE otherwise. */
		static KyBool ConvertToNative(const char* in, char* out);
	};
}

#endif // KY_PATHNAME_H
