#ifndef _MAXDOMCP_H_
#define _MAXDOMCP_H_

template <class T>
class CProxyIMaxMaterialCollectionEvents : public IConnectionPointImpl<T, &IID_IMaxMaterialCollectionEvents, CComDynamicUnkArray>
{
	//Warning this class may be recreated by the wizard.
public:
	HRESULT Fire_OnMaterialAssigned(IMaxMaterialCollection * pMtlClxn, IMaxMaterial * pMtl)
	{
		HRESULT ret = S_OK;
		T* pT = static_cast<T*>(this);
		int nConnectionIndex;
		int nConnections = m_vec.GetSize();
		
		for (nConnectionIndex = 0; nConnectionIndex < nConnections; nConnectionIndex++)
		{
			pT->Lock();
			CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
			pT->Unlock();
			IMaxMaterialCollectionEvents* pIMaxMaterialCollectionEvents = reinterpret_cast<IMaxMaterialCollectionEvents*>(sp.p);
			if (pIMaxMaterialCollectionEvents != NULL)
				ret = pIMaxMaterialCollectionEvents->OnMaterialAssigned(pMtlClxn, pMtl);
		}	return ret;
	
	}
	HRESULT Fire_OnMaterialRemoved(IMaxMaterialCollection * pMtlClxn, IMaxMaterial * pMtl)
	{
		HRESULT ret = S_OK;
		T* pT = static_cast<T*>(this);
		int nConnectionIndex;
		int nConnections = m_vec.GetSize();
		
		for (nConnectionIndex = 0; nConnectionIndex < nConnections; nConnectionIndex++)
		{
			pT->Lock();
			CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
			pT->Unlock();
			IMaxMaterialCollectionEvents* pIMaxMaterialCollectionEvents = reinterpret_cast<IMaxMaterialCollectionEvents*>(sp.p);
			if (pIMaxMaterialCollectionEvents != NULL)
				ret = pIMaxMaterialCollectionEvents->OnMaterialRemoved(pMtlClxn, pMtl);
		}	return ret;
	
	}
	HRESULT Fire_OnMaterialUnAssigned( IMaxMaterialCollection *  pMtlClxn,  IMaxMaterial *  pMtl)
	{
		HRESULT hr = S_OK;
		T * pThis = static_cast<T *>(this);
		int cConnections = m_vec.GetSize();

		for (int iConnection = 0; iConnection < cConnections; iConnection++)
		{
			pThis->Lock();
			CComPtr<IUnknown> punkConnection = m_vec.GetAt(iConnection);
			pThis->Unlock();

			IMaxMaterialCollectionEvents * pConnection = static_cast<IMaxMaterialCollectionEvents *>(punkConnection.p);

			if (pConnection)
			{
				hr = pConnection->OnMaterialUnAssigned(pMtlClxn, pMtl);
			}
		}
		return hr;
	}
	HRESULT Fire_OnMaterialChanged( IMaxMaterialCollection *  pMtlClxn,  IMaxMaterial *  pMtl)
	{
		HRESULT hr = S_OK;
		T * pThis = static_cast<T *>(this);
		int cConnections = m_vec.GetSize();

		for (int iConnection = 0; iConnection < cConnections; iConnection++)
		{
			pThis->Lock();
			CComPtr<IUnknown> punkConnection = m_vec.GetAt(iConnection);
			pThis->Unlock();

			IMaxMaterialCollectionEvents * pConnection = static_cast<IMaxMaterialCollectionEvents *>(punkConnection.p);

			if (pConnection)
			{
				hr = pConnection->OnMaterialChanged(pMtlClxn, pMtl);
			}
		}
		return hr;
	}
	HRESULT Fire_OnSaveScratchMaterials( IMaxMaterialCollection *  pMtlClxn,  IXMLDOMNode* node)
	{
		bool saveit = false;
		HRESULT hr = S_OK;
		T * pThis = static_cast<T *>(this);
		int cConnections = m_vec.GetSize();

		for (int iConnection = 0; iConnection < cConnections; iConnection++)
		{
			pThis->Lock();
			CComPtr<IUnknown> punkConnection = m_vec.GetAt(iConnection);
			pThis->Unlock();

			IMaxMaterialCollectionEvents * pConnection = static_cast<IMaxMaterialCollectionEvents *>(punkConnection.p);

			if (pConnection)
			{
				hr = pConnection->OnSaveScratchMaterials(pMtlClxn, node);
			}
			if (FAILED(hr))
				return hr;
			saveit |= hr == S_OK;
		}
		return saveit ? S_OK : S_FALSE;
	}

};
#endif