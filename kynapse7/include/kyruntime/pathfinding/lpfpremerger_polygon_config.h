/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

	


/*!	\file
	\ingroup kyruntime_pathfinding */

#ifndef KY_LPFPREMERGER_POLYGON_CONFIG_H
#define KY_LPFPREMERGER_POLYGON_CONFIG_H

#include <kyruntime/lpf/ilpfmanager.h>

namespace Kaim
{

/*!	The LpfPreMerger_Polygon_Config class allows an \LpfContext to create an instance of the LpfPreMerger_Polygon class
	and make it available to other components in the World.
	\ingroup kyruntime_pathfinding */
	class LpfPreMerger_Polygon_Config : public ILpfPreMergerConfig
	{
	public:
		/*! \label_constructor */
		LpfPreMerger_Polygon_Config() {};

		virtual ILpfPreMerger* CreateLpfPreMerger(ILpfManager::LpfContext& context) const;
	};


} // namespace Kaim

#endif // KY_LPFPREMERGER_POLYGON_CONFIG_H
