/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download,
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

/*! \file
	\ingroup kypdg */

// primary contact: JODA - secondary contact: NONE
#ifndef KyPdg_PdgFlyGlobalConfig_H
#define KyPdg_PdgFlyGlobalConfig_H

#include <kypathdata/math/interval.h>


namespace Kaim
{

/*! This class contains configuration parameters that control whether or not the Generator creates PathData
	for flying characters, and that control the characteristics of that PathData.
	\pp An instance of this class is maintained in the PdgGlobalConfig::m_pdgFlyGlobalConfig class member.
	\ingroup kypdg */
class PdgFlyGlobalConfig
{
	friend class PdgGlobalConfig;
public:
	/*! \label_constructor */
	PdgFlyGlobalConfig() : m_generateFly(0), m_altitudeRange(0.f, 100.f) {}

	/*! \label_clear */
	void Clear()
	{
		m_generateFly = 0;
		m_altitudeRange.m_low = 0.f;
		m_altitudeRange.m_high = 100.f;
	}

	/*! Returns \c true if all data values in this object match the values in the specified instance of PdgFlyGlobalConfig. */
	bool operator == (const PdgFlyGlobalConfig& rhs) const
	{
		return m_generateFly == rhs.m_generateFly && m_altitudeRange == rhs.m_altitudeRange;
	}

private:
	/*! For internal use. */
	void AddMapping(BlobFieldsMapping& mapping)
	{
		mapping.AddUInt32("GenerateFly", m_generateFly);
		mapping.AddFloat32("FlyAltitudeMin", m_altitudeRange.m_low);
		mapping.AddFloat32("FlyAltitudeMax", m_altitudeRange.m_high);
	}
public:
	/*! Determines whether or not the Generator creates PathData intended for use by flying creatures. Note that in order to 
		generate PathData for flying creatures, you must also provide your Generator with a collision interface that it will 
		use to evaluate collisions.
		\acceptedvalues	\c 0 to generate PathData for ground-based characters, or \c 1 to generate PathData for flying characters.
		\defaultvalue	\c 0 */
	KyUInt32 m_generateFly; // bool for blob

	/*! Sets the minimum and maximum altitudes within which the Generator will create PathData for flying creatures.
		\units			meters
		\defaultvalue	\c 0.0f to \c 100.0f */
	Interval m_altitudeRange; // min/max altitude for flyGeneration.
};

} // namespace Kaim

#endif
