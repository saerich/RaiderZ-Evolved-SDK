/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


// primary contact: GUAL - secondary contact: NOBODY
#ifndef KyPdg_PathMap_H
#define KyPdg_PathMap_H


#include "kypdg/generator/squarepartitiontypes.h"
#include "kypathdata/containers/kystring.h"

namespace Kaim
{


class PathMap // class with static functions only
{
public:
	static KyString CellFileName(
		const char* containingDirectory,
		const CellPos& cellPos,
		const char* extension,
		bool inCellSubDir);
};


}


#endif

