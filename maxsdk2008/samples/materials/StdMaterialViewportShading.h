/**********************************************************************
 *<
    FILE: StdMaterialViewportShading.h

    DESCRIPTION:

    CREATED BY: Qilin.Ren

    HISTORY:

 *> Copyright (c) 2007, All Rights Reserved.
 **********************************************************************/

#ifndef STD_MATERIAL_SHADING_PROPERTY_H
#define STD_MATERIAL_SHADING_PROPERTY_H

#include "IMaterialViewportShading.h"
#include "imtl.h"

class StdMaterialViewportShading : public IMaterialViewportShading
{
public:
	StdMaterialViewportShading();
	virtual ~StdMaterialViewportShading();

public:
	void SetMaterial(MtlBase* pStdMaterial);

	virtual bool IsShadingModelSupported(
		IMaterialViewportShading::ShadingModel model) const;

	virtual IMaterialViewportShading::ShadingModel 
		GetCurrentShadingModel() const;

	virtual bool SetCurrentShadingModel(
		IMaterialViewportShading::ShadingModel model);

	virtual int GetSupportedMapLevels() const;

private:
	MtlBase* m_pStdMaterial;
};

#endif // #ifndef STD_MATERIAL_SHADING_PROPERTY_H