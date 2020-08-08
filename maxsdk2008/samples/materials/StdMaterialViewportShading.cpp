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

// This is a duplicated function, it should be cleared in Johnson.
/*! Find an interface from the CustAttribs of the material.  This would typically
be used to find the DirectX Manager or Mental Ray attributes that are hosted as
Custom Attributes on a material.
\param pMaterial the material, can be a NULL pointer.
\param iid the interface id to be search for.
\return returns the first interface pointer found.
*/
static BaseInterface* FindCustAttribInterface(
	MtlBase* pMaterial, 
	const Interface_ID& iid)
{
	if (NULL == pMaterial)
	{
		return NULL;
	}

	ICustAttribContainer* l_pAttribContainer = pMaterial->GetCustAttribContainer();

	if (NULL != l_pAttribContainer)
	{
		int l_numCustAttribs = l_pAttribContainer->GetNumCustAttribs();
		for (int i = 0; i < l_numCustAttribs; ++i)
		{
			CustAttrib* l_pCustAttrib = l_pAttribContainer->GetCustAttrib(i);
			BaseInterface* l_pInterface = l_pCustAttrib->GetInterface(iid);
			if (NULL != l_pInterface)
			{
				return l_pInterface;
			}
		}
	}

	return NULL;
}

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
	IViewportShaderManager3* pViewportShaderManager = 
		(IViewportShaderManager3*)(FindCustAttribInterface(
			m_pStdMaterial, 
			VIEWPORT_SHADER_MANAGER_INTERFACE3));
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
	IViewportShaderManager3* pViewportShaderManager = 
		(IViewportShaderManager3*)(FindCustAttribInterface(
		m_pStdMaterial, 
		VIEWPORT_SHADER_MANAGER_INTERFACE3));
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
	IViewportShaderManager3* pViewportShaderManager = 
		(IViewportShaderManager3*)(FindCustAttribInterface(
		m_pStdMaterial, 
		VIEWPORT_SHADER_MANAGER_INTERFACE3));
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
