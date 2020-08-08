/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download,
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

// primary contact: LASI - secondary contact: NONE

#ifndef KyPdg_DynamicPointGraphBuilder_H
#define KyPdg_DynamicPointGraphBuilder_H

#include "kypdg/pdginput/indexedtrianglesoup.h"
#include "kypdg/common/performancetickcount.h"
#include "kypathdata/navmesh/navcell.h"

#include "kypathdata/graph/graphtypes.h"
#include "kypathdata/containers/stringstream.h"


namespace Kaim
{

class DynamicNavCell;
class DynamicNavFloor;
class DynamicPointGraphCell;
class PdgGraphVertex;
class PdgPointGraph;
class PdgSystem;

class IDynamicPointGraphBuilder
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	IDynamicPointGraphBuilder(const PdgSystem* sys) :
		m_sys(sys), m_startTime(0), m_endTime(0), m_profilingEnabled(false)
	{}

	virtual ~IDynamicPointGraphBuilder() {}

	virtual KyResult BuildPointGraphFromDynamicNavCell(DynamicNavCell&, DynamicPointGraphCell&) = 0;

	void ToggleProfiling(bool toggle) { m_profilingEnabled = toggle;}
	KyFloat32 GetLastBuildDuration() { return m_profilingEnabled? PerformanceTickCount::Seconds(m_startTime,m_endTime): 0; }
	virtual void GetProfilingReport(Kaim::String& report)
	{
		if (!m_profilingEnabled)
		{
			report += "profiling disabled\n";
			return;
		}
		Kaim::StringStream ss(report);
		ss << "Total duration : " << PerformanceTickCount::Seconds(m_startTime,m_endTime) * 1000.0 << " ms " << Kaim::Endl ;
	}

protected:
	const PdgSystem* m_sys;
	PerformanceTickCount m_startTime;
	PerformanceTickCount m_endTime;
	bool m_profilingEnabled;
};

} // namespace Kaim

#endif // #ifndef KyPdg_DynamicPointGraphBuilder_H
