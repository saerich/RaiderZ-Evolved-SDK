// MaxMaterial.h : Declaration of the CMaxMaterial

#ifndef __MAXMATERIAL_H_
#define __MAXMATERIAL_H_

#include "resource.h"       // main symbols
#include "resOverride.h"    // alias some symbols for specific versions
#include "max.h"

#ifdef EXTENDED_OBJECT_MODEL
#define DID486021


#include "stdmat.h"
#include <map>
//some macros for use by refmaker methods
#define NUMREFS 1
#define REFIDX_MTL 0
class CMaxMaterialCollection;
#include "notify.h" //need NotifyInfo
#include "comsrv.h"

/////////////////////////////////////////////////////////////////////////////
// CMaxMaterial
class ATL_NO_VTABLE CMaxMaterial : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CMaxMaterial, &CLSID_MaxMaterial>,
	public IDispatchImpl<IMaxMaterial, &IID_IMaxMaterial, &LIBID_COMSRVLib>,
	public ReferenceMaker,
	public IMaxSerializeXml
{
	static DWORD sdwPutMtlNoPrompt;
	static bool sbClassInitialized;
	static void LoadClassDefaults();
	static void ReadINI();
	static void WriteINI();
	static void GetINIFilename(TCHAR* filename);
public:
	CMaxMaterial();
	virtual ~CMaxMaterial();


	DECLARE_REGISTRY_RESOURCEID(IDR_MAXMATERIAL)

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	BEGIN_COM_MAP(CMaxMaterial)
		COM_INTERFACE_ENTRY(IMaxMaterial)
		COM_INTERFACE_ENTRY(IDispatch)
		COM_INTERFACE_ENTRY(IMaxSerializeXml)
	END_COM_MAP()

	friend static void MtlRefAddedProc(void *param, NotifyInfo *info);
	friend static void MtlRefDeletedProc(void *param, NotifyInfo *info);



	// IMaxMaterial
public:
	STDMETHOD(loadXML)(/*[in]*/IUnknown *pXMLNode);
	STDMETHOD(saveXML)(/*[in]*/IUnknown* pUnk, /*[in]*/VARIANT_BOOL noPrompts);
	STDMETHOD(getDynPropManagers)(/*[in]*/ VARIANT *pObjUnkArray, /*[out]*/ VARIANT *pDynUnkArray);
	STDMETHOD(assignSelected)();
	STDMETHOD(getThumbnailBytes)(/*[in]*/ int sz, /*[out, retval]*/SAFEARRAY **psa);
	STDMETHOD(get_ID)(/*[out, retval]*/ GUID *pVal);
	STDMETHOD(get_ThumbnailFile)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(get_Name)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_Name)(/*[in]*/ BSTR newVal);

	// ReferenceMaker
public:
	RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID,RefMessage message);
	int NumRefs();
	RefTargetHandle GetReference(int i);
	void SetReference(int i, RefTargetHandle rtarg);
	void CheckReference(bool added = true, bool removed = true);

	void setMtl(Mtl *pMat);
	Mtl *getMtl(){return mpMtl;}
	void setCollection(CMaxMaterialCollection *pCol);
private:
	//nested class which manages cache of thumbnail images
	//these are maintaned in a map keyed by frame number
	class ThumbnailCache
	{
	public:
		int getByteCount();
		HRESULT updatePSByteCache(int sz,  Mtl *pMtl);
		ThumbnailCache():mPSByteCache(NULL){mPSByteCount = 0;};
		~ThumbnailCache();
		HRESULT updateCache(TSTR& basePath, Mtl *pMtl, const GUID& id, unsigned int frame = 0);
		//		bool isValid(){return mValid;}
		bool getPath(BSTR *path, unsigned int frame = 0);
		void Invalidate();
		SAFEARRAY *getBytes(){return mPSByteCache;}
		void Init(BSTR path);
	private:
		int mPSByteCount;
		//		bool mValid;
		typedef std::map<unsigned int, CComBSTR> framePathType;
		framePathType mframePaths;
		//		CComBSTR mBSTRPSpath;
		SAFEARRAY * mPSByteCache; //keep a cache of the bytes
	} mPSCache;
	GUID mID;
	Mtl* mpMtl;
	DWORD_PTR mdwKey;//mpMtl as a DWORD, we may need this key after the material has self-deleted
	CMaxMaterialCollection *mpCollection;
	CComQIPtr<IXMLDOMNode> mpXML; //cache of data
	unsigned int m_currentFrame;
	static const unsigned int sFrameCount;
	//TODO fix up our inscene and isreferenced methods to actually count
	//right now the methods just return 0 or 1
	bool	mbSuspended;
	USHORT mcNodeRefs;//total # max node references to wrapped mtl 
	USHORT mcMaxRefs;//total # max references to wrapped mtl (excluding this wrapper, matlibs and such)
	void updateRefCounts();//computes the previous two values.
	bool confirmEditing();//see if we're being edited
	bool mbEditing;
	bool mbSubMtl;//indicates that our mtl is part of a multi
	USHORT mcPersistCt;//count of the number of persistence requests.

public:
	STDMETHOD(OnDrop)(SHORT nFlag, DWORD_PTR hWnd, DWORD point, DWORD dwKeyState);
	STDMETHOD(BeginEdit)(VARIANT_BOOL bForceShow);
	STDMETHOD(EndEdit)(void);
	STDMETHOD(NextThumbnailFrame)(BSTR* filename);

	STDMETHOD(get_DiffuseColor)(OLE_COLOR* pVal);
	STDMETHOD(put_DiffuseColor)(OLE_COLOR newVal);
	STDMETHOD(get_DiffuseMapAmount)(FLOAT* pVal);
	STDMETHOD(put_DiffuseMapAmount)(FLOAT newVal);
	STDMETHOD(get_Shininess)(FLOAT* pVal);
	STDMETHOD(put_Shininess)(FLOAT newVal);
	STDMETHOD(get_Transparency)(FLOAT* pVal);
	STDMETHOD(put_Transparency)(FLOAT newVal);
	STDMETHOD(get_IdxRefraction)(FLOAT* pVal);
	STDMETHOD(put_IdxRefraction)(FLOAT newVal);
	STDMETHOD(get_Luminance)(FLOAT* pVal);
	STDMETHOD(put_Luminance)(FLOAT newVal);
	STDMETHOD(get_TwoSided)(VARIANT_BOOL* pVal);
	STDMETHOD(put_TwoSided)(VARIANT_BOOL newVal);
	STDMETHOD(DoColorPicker)(void);
	STDMETHOD(InScene)(VARIANT_BOOL* res);
	STDMETHOD(IsReferenced)(VARIANT_BOOL* res);
	STDMETHOD(utilExecute)(SHORT command, VARIANT arg1);
	STDMETHOD(selectNodes)(void);
	STDMETHOD(clone)(IMaxMaterial ** ppNew);
	STDMETHOD(CanAssignSelected)(VARIANT_BOOL* vResult);
	STDMETHOD(selectFace)(USHORT idxSelection);
	STDMETHOD(getFaceSelectionNames)(VARIANT* vFaceSelNames);

	bool IsSubMtl(){return mbSubMtl;}
	void CheckReference(Mtl *pMtl);
	void RecursiveAddMtl(Mtl *pMtl, Tab<Mtl*>& mtls);

	USHORT AddPersistence(){return ++mcPersistCt;}
	USHORT RemovePersistence(){return --mcPersistCt;}
	int WhichSlot(MtlBase *pMtl, bool * needPut);

	// IMaxSerializeXml Methods
public:
	STDMETHOD(QueryXmlSupport)( BSTR *  pbstrLog,  VARIANT_BOOL  fullReport);
	STDMETHOD(SerializeXml)( IUnknown *  pUnk);
	STDMETHOD(LoadXml)( IUnknown *  pUnk);
};

#endif	// EXTENDED_OBJECT_MODEL

#endif //__MAXMATERIAL_H_
