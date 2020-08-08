// MaxMaterialCollection.h : Declaration of the CMaxMaterialCollection

#ifndef __MAXMATERIALCOLLECTION_H_
#define __MAXMATERIALCOLLECTION_H_

#include "resource.h"       // main symbols
#include "resOverride.h"    // alias some symbols for specific versions
#include "max.h" //for referencemaker

#define TP_SUSPEND_FOR_FILELINK
#ifdef EXTENDED_OBJECT_MODEL

#include "MaxDOMCP.H"
#include <map>
#include "notify.h"
#include "comsrv.h"

class CMaxMaterial;
struct IVIZPointerClient;

#define SCRATCH_LIB_CLASS_ID Class_ID(MTLBASE_LIB_CLASS_ID,0xf7256c9e)
class ScratchLib : public MtlBaseLib {
	Class_ID ClassID(){return SCRATCH_LIB_CLASS_ID;}
};

/////////////////////////////////////////////////////////////////////////////
// CMaxMaterialCollection
class ATL_NO_VTABLE CMaxMaterialCollection : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CMaxMaterialCollection, &CLSID_MaxMaterialCollection>,
	public IDispatchImpl<IMaxMaterialCollection, &IID_IMaxMaterialCollection, &LIBID_COMSRVLib>,
	public CProxyIMaxMaterialCollectionEvents< CMaxMaterialCollection >,
	public IConnectionPointContainerImpl<CMaxMaterialCollection>,
	public ReferenceMaker
{
public:
	typedef CComObject<CMaxMaterial> MtlWrapper;
	CMaxMaterialCollection():mpSceneMtlLib(0)
	{
		m_suspendCount = 0;
	}
	STDMETHOD(FinalConstruct)();
	void FinalRelease();

	static HRESULT GetXMLImpExp(IVIZPointerClient** ppClient);
	static HMODULE shXmlMtlDllModule;
	static HRESULT CreatePluglet(REFCLSID rclsid, REFIID riid, void** ppPluglet, LPCTSTR sModule);

DECLARE_REGISTRY_RESOURCEID(IDR_MAXMATERIALCOLLECTION)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CMaxMaterialCollection)
	COM_INTERFACE_ENTRY(IMaxMaterialCollection)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
END_COM_MAP()

// IMaxMaterialCollection
public:
	STDMETHOD(createNew)(/*[out, retval]*/ IMaxMaterial **ppMtl);
	STDMETHOD(createFromMaxID)(/*[in]*/ VARIANT partA, /*[in]*/ VARIANT partB, /*[out,retval]*/ IMaxMaterial ** ppNew);
	STDMETHOD(getMaterial)(/*[in]*/ int which, /*[out, retval]*/ IMaxMaterial **ppMtl);
	STDMETHOD(get_count)(/*[out, retval]*/ short *pVal);
	STDMETHOD(FindItem)(/*[in]*/ GUID id, /*[out, retval]*/ IMaxMaterial **ppMtl);
	STDMETHOD(FindItemKey)(/*[in]*/ DWORD_PTR key, /*[out, retval]*/ IMaxMaterial **ppMtl);
public :

BEGIN_CONNECTION_POINT_MAP(CMaxMaterialCollection)
	CONNECTION_POINT_ENTRY(IID_IMaxMaterialCollectionEvents)
END_CONNECTION_POINT_MAP()

// ReferenceMaker
public:
	RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID,RefMessage message);
	int NumRefs();
	RefTargetHandle GetReference(int i);
	void SetReference(int i, RefTargetHandle rtarg);

	// These are ReferenceMaker methods but they are called when
	// the GUP is saved, which is not in the reference hierarchy.
	// These methods cannot use the reference hierarchy for
	// saving and loading data.
	virtual IOResult Save(ISave *isave);
	virtual IOResult Load(ILoad *iload);

	//3/6/03
	//making this member public so that material wrappers can create PersistMtlRestore undo objects
	//not clear if these undo objects will remain. If they do, we should consider exposing this member
	//through friendship
	ScratchLib* mpScratchMtlLib; //the scratch materials
	//note that we must derive from MtlBaseLib and provide 
	//a new class id so that the ref is considered relevant
	//by the maxmaterial dependency enumerators


private:
	MtlBaseLib* mpSceneMtlLib; //pointer to the scene materials
	int mLastSceneMtlLibSize; //keep count of the scene materials lib

	int mLastScratchMtlLibSize;


	//consider refactoring to store c++ objects
	//typedef std::map<DWORD, IMaxMaterial*> ifxClxnType;
	typedef std::map<DWORD_PTR, MtlWrapper*> ifxClxnType;
	ifxClxnType mMtlMap;//the collection of material objects.indexed by pointer to the inner Material
//	ifxClxnType mRecycledMtlMap;//the collection of material objects.indexed by pointer to the inner Material
	ULONG m_suspendCount;
	static bool s_InMakeWrapper;
	static bool s_PreventRecursion;

#ifdef TP_SUSPEND_FOR_FILELINK
	//2/9/05 // if we're going to suspend across link operations, we need
	//to collect any linked materials, so that we can update their ref counts when 
	//the operation completes
	ifxClxnType mCacheLinkedMtls;
#endif


	HRESULT deleteMaterials(bool recycle, bool scene, bool scratch);
	void deleteReferencedScratchMaterials();

protected:
	void OnSceneLibChanged();
	void OnScratchLibChanged();
	void OnFileLinkMtlsChanged();
	void OnMeditVisible();
#ifdef TP_SUSPEND_FOR_FILELINK
	void OnPreFileLinkEvent();
#endif


	void OnMaxNotify(NotifyInfo *info);
	static void NotifyProc(void *param, NotifyInfo *info);

	HRESULT MakeWrapperObject(Mtl* pMtl);
	HRESULT RecursiveMakeWrapperObject(Mtl* pMtl);
	HRESULT MakeWrapperObjectLeaf(Mtl *pMtl);

public:
	bool IsSuspended() { return m_suspendCount > 0; }
	void Suspend(){
		if(m_suspendCount++ == 0)
			mLastSceneMtlLibSize = mLastScratchMtlLibSize = 0;//process all on resume
	}
	bool Resume(){ return m_suspendCount == 0 || --m_suspendCount == 0; }
	CMaxMaterial* findItem(const GUID& id);
	CMaxMaterial* findItemKey(DWORD_PTR key);
	void switchMaterial(CMaxMaterial* wrapper, Mtl* mtl);
	//called when a material node references goes from 0 to 1
	void OnMtlAddedToScene(IMaxMaterial * pMtl);
	//called when a material node references drop to zero
	void OnMtlRemovedFromScene(IMaxMaterial * pMtl);
	//called when we suspsect that we are no longer being edited
	void OnMtlEndEdit(IMaxMaterial *pMtl);
	void OnMtlChanged(IMaxMaterial *pMtl);
	void OnMtlDeleted(IMaxMaterial *pMtl, DWORD_PTR key);

	//consider refactoring to take c++ type rather than ip
	bool AddMtl(MtlWrapper* pMtl, DWORD_PTR key);

	STDMETHOD(emptyTrash)(void);
	STDMETHOD(AddPersistence)(IMaxMaterial * pMtl);
	STDMETHOD(RemovePersistence)(IMaxMaterial * pMtl);
	STDMETHOD(IsSelectionValid)(VARIANT_BOOL* vRes);
};


class PersistMtlRestore: public RestoreObj  {
	SingleRefMaker refmkr;
	ScratchLib *mlib;
	public:
		PersistMtlRestore() { }
		PersistMtlRestore(ScratchLib *lib, Mtl* m){
			refmkr.SetRef(m);
			mlib = lib;
			}
		~PersistMtlRestore(){}
		void Restore(int isundo)
		{
			MtlBase *mtl = (MtlBase*)refmkr.GetRef();
			if(mlib->FindMtl(mtl)>0)
				mlib->Remove(mtl);
			else
				mlib->Add(mtl);
		}
		void Redo() {
			Restore(0);
			}
		TSTR Description() {
			TCHAR buf[100];
			_stprintf(buf,_T("PersistMtlRestore"));
			return(TSTR(buf));
			}
	};

#endif // EXTENDED_OBJECT_MODEL

#endif //__MAXMATERIALCOLLECTION_H_
