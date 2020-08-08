/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kypathdata_pathobject */

#ifndef KyPathData_PathObjectDataTypes_H
#define KyPathData_PathObjectDataTypes_H

#include <kypathdata/generic/basetypes.h>
#include <kypathdata/generic/kyguid.h>

namespace Kaim
{

typedef KyUInt32 PathObjectConnexionSpaceIdx; /*!< Defines a type used for a unique index that identifies a single PathObject connection space. \ingroup kypathdata_pathobject */
typedef KyUInt32 PathObjectConnexionLinkIdx; /*!< Defines a type used for a unique index that identifies a single PathObject connection link. \ingroup kypathdata_pathobject */
typedef KyUInt32 PathObjectControlSpaceIdx; /*!< Defines a type used for a unique index that identifies a single PathObject control space. \ingroup kypathdata_pathobject */
typedef KyUInt32 PathObjectId; /*!< Defines a type used for a unique index that identifies a single PathObject. \ingroup kypathdata_pathobject */

static const PathObjectConnexionSpaceIdx INVALID_PATHOBJECT_CONNEXION_SPACE_IDX = KyUInt32MAXVAL;     /*!< Represents an invalid index for a PathObject connexion space. \ingroup kypathdata_pathobject */
static const PathObjectConnexionLinkIdx INVALID_PATHOBJECT_CONNEXION_LINK_IDX = KyUInt32MAXVAL;       /*!< Represents an invalid index for a PathObject connexion link. \ingroup kypathdata_pathobject */
static const PathObjectControlSpaceIdx INVALID_PATHOBJECT_CONTROL_SPACE_IDX = KyUInt32MAXVAL;         /*!< Represents an invalid index for a PathObject control space. \ingroup kypathdata_pathobject */
static const PathObjectId INVALID_PATHOBJECT_ID = KyUInt32MAXVAL;                                     /*!< Represents an invalid index for a PathObject. \ingroup kypathdata_pathobject */

}

#endif // #ifndef KyPathData_PathObjectDataTypes_H
