/*
* Copyright 2010 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download,
* or which otherwise accompanies this software in either electronic or hard copy form.
*/


/*! \file
	\ingroup kypdg */

// primary contact: LASI - secondary contact: GUAL
#ifndef KyPdg_PdgAdvancedParameters_H
#define KyPdg_PdgAdvancedParameters_H

#include <kypathdata/generic/types.h>
#include <kypathdata/blob/blobfieldarray.h>
#include "kypdg/generator/pdggraphadvancedparameters.h"

namespace Kaim
{


/*! The GeneratorAdvancedParameters class is used by the PdgGlobalConfig class to maintain a set of configuration parameters
	that control detailed technical aspects of the PathData generation process.
	\ingroup kypdg */
class PdgAdvancedParameters
{
	KY_DEFINE_NEW_DELETE_OPERATORS
public:
	/*! \label_constructor */
	PdgAdvancedParameters() :
		m_jumpDownHeight(0.0f),
		m_altitudeToleranceSamplingStep(0.3f),
		m_minConnectedSurface(10.0f),
		m_terrainTypeRasterMergeTolerance(0.1f)
	{
		Clear();
	}

	/*! \label_clear */
	void Clear()
	{
		m_jumpDownHeight = 0.0f;
		m_altitudeToleranceSamplingStep = 0.3f;
		m_minConnectedSurface = 10.0f;
		m_terrainTypeRasterMergeTolerance = 0.1f;
		m_graphAdvancedParameters.SetDefaultValues();
	}

	/*! For internal use. */
	void AddMapping(BlobFieldsMapping& mapping)
	{
		mapping.AddFloat32("JumpDownHeight", m_jumpDownHeight);
		mapping.AddFloat32("MinConnectedSurface", m_minConnectedSurface);
		mapping.AddFloat32("TerrainTypeRasterMergeTolerance", m_terrainTypeRasterMergeTolerance);
		mapping.AddFloat32("AltitudeToleranceSamplingStep", m_altitudeToleranceSamplingStep);
		m_graphAdvancedParameters.AddMapping(mapping);
	}

	/*! Returns \c true if all data values in this object match the values in the specified instance of PdgAdvancedParameters. */
	bool operator==(const PdgAdvancedParameters& other) const
	{
		return m_jumpDownHeight                  == other.m_jumpDownHeight
			&& m_minConnectedSurface             == other.m_minConnectedSurface
			&& m_terrainTypeRasterMergeTolerance == other.m_terrainTypeRasterMergeTolerance
			&& m_altitudeToleranceSamplingStep   == other.m_altitudeToleranceSamplingStep
			&& m_graphAdvancedParameters         == other.m_graphAdvancedParameters;
	}

	/*! Returns \c true if at least one data value in this object does not match the value in the specified instance of PdgAdvancedParameters. */
	bool operator!=(const PdgAdvancedParameters& other) const { return !operator==(other); }

public:
	/*! Determines whether or not jump edge candidates are generated, and the maximum vertical height for a jump edge candidate. 
		\pp If a non-zero value is set, the Graph is post-processed to find vertical drops up to this threshold, where a character 
		can descend but not ascend. Note that in order to generate jump edge candidates, you must also provide your 
		Generator with a collision interface that it will use to evaluate potential jumping spots.
		\pp These jump edge candidates are not included directly in the Graph. They are saved separately in the final \c .PathData file.
		\pp If this value is set to 0.0f, no jump edge candidates are generated.
		\units			meters
		\defaultvalue	\c 0.0f */
	KyFloat32 m_jumpDownHeight; // in meters

	/*! This parameter is used only when the Generator is configured to generate vertically accurate PathData: i.e. when
		PdgGlobalConfig::m_doUseAltitudeTolerance is set to #VERTICAL_ACCURACY_ENABLED.
		It determines the resolution used to sample the NavMesh and compare it to the original raster.
		\pp Lower values typically result in PathData with more triangles that matches the altitude of the original terrain mesh
		more closely, but may increase generation time.
		\pp Note that the sampling step is internally snapped to an integer value to make sure that #m_altitudeToleranceSamplingStep
		is in fact a multiple of the original rasterization pixel size (see PdgGlobalConfig::m_rasterPrecision). Therefore, any
		value less than twice the pixel size will be clamped to the pixel size, values between 2.0f and 3.0f times the pixel size
		will be clamped to twice the pixel size, etc.
		\units 			meters
		\defaultvalue	\c 0.30f */
	KyFloat32 m_altitudeToleranceSamplingStep; //in m

	/*! Sets the minimum surface area that any isolated area of NavMesh may occupy. Any NavMesh with a total surface area smaller
		than this value is discarded, along with its Graph.
		\units 			square meters
		\defaultvalue	\c 10.0f */
	KyFloat32 m_minConnectedSurface; //in m2

	/*! Sets the vertical tolerance used to select the terrain type to be applied to a raster pixel. When multiple triangles
		with different terrain types are merged into the same block in the raster, this parameter controls the vertical
		distance that is checked to determine the best terrain type to apply. If set to \c 0.0f, the terrain type of the
		uppermost triangle is always selected.
		\units 			square meters
		\defaultvalue	\c 0.1f */
	KyFloat32 m_terrainTypeRasterMergeTolerance; //in meters

	/*! Contains advanced configuration parameters related to Graph generation. */
	PdgGraphAdvancedParameters m_graphAdvancedParameters;
};


} // namespace Kaim


#endif

