/**********************************************************************
 *<
    FILE: StdMaterialViewportShading.cpp

    DESCRIPTION:

    CREATED BY: Qilin.Ren

    HISTORY:

 *> Copyright (c) 2007, All Rights Reserved.
 **********************************************************************/

#include "mtlhdr.h"
#include "StdMaterialViewportShading.h"
#include "IViewportManager.h"
#include "icustattribcontainer.h"

StdMaterialViewportShading::StdMaterialViewportShading()
{
	m_pStdMaterial = NULL;
}

StdMaterialViewportShading::~StdMaterialViewportShading()
{
	m_pStdMaterial = NULL;
}

void StdMaterialViewportShading::SetMaterial(MtlBase* pStdMaterial)
{
	if (NULL != pStdMaterial && 
		pStdMaterial->ClassID() == Class_ID(DMTL_CLASS_ID,0))
	{
		m_pStdMaterial = pStdMaterial;
	}
	else
	{
		m_pStdMaterial = NULL;
	}
}

bool StdMaterialViewportShading::IsShadingModelSupported(
	IMaterialViewportShading::ShadingModel shadingModel) const
{
	IViewportShaderManager3* pViewportShaderManager = NULL;

	if (NULL != m_pStdMaterial)
	{
		ICustAttribContainer* l_pCustAttribContainer = m_pStdMaterial->GetCustAttribContainer();
		if (NULL != l_pCustAttribContainer)
		{
			pViewportShaderManager = (IViewportShaderManager3*)
				(l_pCustAttribContainer->FindCustAttribInterface(VIEWPORT_SHADER_MANAGER_INTERFACE3));
		}
	}

	if (NULL == pViewportShaderManager)
	{
		return shadingModel == IMaterialViewportShading::Standard;
	}

	switch (shadingModel)
	{
	case IMaterialViewportShading::Standard:
		if (!pViewportShaderManager->IsCurrentEffectEnabled())
		{
			return true;
		}
	case IMaterialViewportShading::Hardware:
		return pViewportShaderManager->IsDxStdMtlSupported();
	default:
		break;
	}

	return false;
}

IMaterialViewportShading::ShadingModel 
StdMaterialViewportShading::GetCurrentShadingModel() const
{
	IViewportShaderManager3* pViewportShaderManager = NULL;

	if (NULL != m_pStdMaterial)
	{
		ICustAttribContainer* l_pCustAttribContainer = m_pStdMaterial->GetCustAttribContainer();
		if (NULL != l_pCustAttribContainer)
		{
			pViewportShaderManager = (IViewportShaderManager3*)
				(l_pCustAttribContainer->FindCustAttribInterface(VIEWPORT_SHADER_MANAGER_INTERFACE3));
		}
	}

	if (NULL == pViewportShaderManager)
	{
		return IMaterialViewportShading::Standard;
	}
	else if (pViewportShaderManager->IsCurrentEffectEnabled())
	{
		return IMaterialViewportShading::OtherTypes;
	}
	else if (pViewportShaderManager->IsDxStdMtlEnabled())
	{
		return IMaterialViewportShading::Hardware;
	}
	else
	{
		return IMaterialViewportShading::Standard;
	}
}

bool StdMaterialViewportShading::SetCurrentShadingModel(
	IMaterialViewportShading::ShadingModel model)
{
	IViewportShaderManager3* pViewportShaderManager = NULL;

	if (NULL != m_pStdMaterial)
	{
		ICustAttribContainer* l_pCustAttribContainer = m_pStdMaterial->GetCustAttribContainer();
		if (NULL != l_pCustAttribContainer)
		{
			pViewportShaderManager = (IViewportShaderManager3*)
				(l_pCustAttribContainer->FindCustAttribInterface(VIEWPORT_SHADER_MANAGER_INTERFACE3));
		}
	}

	if (NULL == pViewportShaderManager)
	{
		return model == IMaterialViewportShading::Standard;
	}

	switch (model)
	{
	case IMaterialViewportShading::Standard:
		if (!pViewportShaderManager->IsCurrentEffectEnabled())
		{
			pViewportShaderManager->SetDxStdMtlEnabled(false);
			return true;
		}
		break;

	case IMaterialViewportShading::Hardware:
		if (!pViewportShaderManager->IsCurrentEffectEnabled())
		{
			pViewportShaderManager->SetDxStdMtlEnabled(true);
			return pViewportShaderManager->IsDxStdMtlEnabled();
		}
		break;

	default:
		break;
	}

	return false;
}

int StdMaterialViewportShading::GetSupportedMapLevels() const
{
	return 1;
}
