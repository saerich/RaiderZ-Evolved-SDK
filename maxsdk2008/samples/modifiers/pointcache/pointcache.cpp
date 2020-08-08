/**********************************************************************
 *<
	FILE: PointCache.h

	DESCRIPTION:	Base class for OSM and WSM

	CREATED BY:

	HISTORY:

 *>	Copyright (c) 1997, All Rights Reserved.
 **********************************************************************/

#include "PointCache.h"
#include "IAssetAccessor.h"
#include "IPathConfigMgr.h"

#include <fbxsdk.h>
#include <fbxfilesdk_nsuse.h>

///////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
#define ENABLE_DEBUGPRINT
#else
// Uncomment for DebugPrints in release builds
//#define ENABLE_DEBUGPRINT
#endif

#ifdef ENABLE_DEBUGPRINT
#pragma message("Debug prints are enabled!")
#define DEBUGPRINT	DebugPrint
#else
void DoNothing(...) { }
#define DEBUGPRINT	DoNothing
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
// PointCache util stuff //////////////////////////////////////////////////////////////////////////

#define DELETE_BUFF(buff) if (buff) { delete[] buff; buff = NULL; }

TSTR GetLastErrorString()
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR) &lpMsgBuf,
		0,
		NULL
	);

	TSTR err;
	err.printf("%s", (LPCTSTR)lpMsgBuf);

	LocalFree(lpMsgBuf);

	return err;
}

// private namespace
namespace
{
	class sMyEnumProc : public DependentEnumProc 
	{
	public :
		virtual int proc(ReferenceMaker *rmaker); 
		INodeTab Nodes;              
	};

	int sMyEnumProc::proc(ReferenceMaker *rmaker) 
	{ 
		if (rmaker->SuperClassID()==BASENODE_CLASS_ID)    
		{
			Nodes.Append(1, (INode **)&rmaker);  
			return DEP_ENUM_SKIP;
		}

		return DEP_ENUM_CONTINUE;
	}
}


/////////////////////////////////////////////
// MCBufferIO
/////////////////////////////////////////////
MCBufferIO::MCBufferIO() :
	mbReadDouble(true),
	mpReadBufferFloat(NULL),
	mpReadBufferDouble(NULL),
	mpWriteBufferFloat(NULL)
{

}

MCBufferIO::~MCBufferIO()
{
	FreeAllBuffers();
}

void MCBufferIO::SetReadToDouble( bool bDouble )
{
	mbReadDouble = bDouble;
}

bool MCBufferIO::AllocReadBuffer( size_t size )
{
	FreeReadBuffer();
	if( mbReadDouble )
		mpReadBufferDouble = new double[size];
	else
		mpReadBufferFloat = new float[size];

	return !IsReadBufferNULL();
}

bool MCBufferIO::AllocWriteBuffer( size_t size )
{
	FreeWriteBuffer();

	mpWriteBufferFloat = new float[size];

	return !IsWriteBufferNULL();
}

void MCBufferIO::FreeAllBuffers()
{
	FreeReadBuffer();
	FreeWriteBuffer();
}

void MCBufferIO::FreeReadBuffer()
{
	if( mpReadBufferDouble ) {
		delete [] mpReadBufferDouble;
		mpReadBufferDouble = NULL;
	}
	if( mpReadBufferFloat ) {
		delete [] mpReadBufferFloat;
		mpReadBufferFloat = NULL;
	}
}

void MCBufferIO::FreeWriteBuffer()
{
	if( mpWriteBufferFloat ) {
		delete [] mpWriteBufferFloat;
		mpWriteBufferFloat = NULL;
	}
}

bool MCBufferIO::IsReadBufferNULL()
{
	if( mbReadDouble )
		return (mpReadBufferDouble == NULL);
	else
		return (mpReadBufferFloat == NULL);
}

bool MCBufferIO::IsWriteBufferNULL()
{
	return (mpWriteBufferFloat == NULL);
}

bool MCBufferIO::SetWriteBuffer( unsigned int iIndex, Point3 *pt )
{
	if( IsWriteBufferNULL() )
		return false;

	mpWriteBufferFloat[(iIndex*3)] = pt->x;
	mpWriteBufferFloat[(iIndex*3)+1] = pt->y;
	mpWriteBufferFloat[(iIndex*3)+2] = pt->z;

	return true;
}

bool MCBufferIO::ConvertReadBuffer( Point3 *pt, unsigned int iNumPoints )
{
	if( IsReadBufferNULL() )
		return false;

	int j=0;
	for( int i=0; i<iNumPoints*3; i+=3, j++ ) {
		if( mbReadDouble ) {
			pt[j].x = (float)mpReadBufferDouble[i];
			pt[j].y = (float)mpReadBufferDouble[i+1];
			pt[j].z = (float)mpReadBufferDouble[i+2];
		}
		else {
			pt[j].x = mpReadBufferFloat[i];
			pt[j].y = mpReadBufferFloat[i+1];
			pt[j].z = mpReadBufferFloat[i+2];
		}
	}

	return true;
}

bool MCBufferIO::Read( FBXFILESDK_NAMESPACE::KFbxCache* pCache, int iChannel, TimeValue t, unsigned int iNumPoints )
{
	if( IsReadBufferNULL() )
		return false;

	bool res;
	if( mbReadDouble )
		res = pCache->Read(iChannel, TimeValueToKTime(t), mpReadBufferDouble, iNumPoints);
	else
		res = pCache->Read(iChannel, TimeValueToKTime(t), mpReadBufferFloat, iNumPoints);

	return res;
}

bool MCBufferIO::Write( FBXFILESDK_NAMESPACE::KFbxCache* pCache, int iChannel, TimeValue t, unsigned int iNumPoints )
{
	if( IsWriteBufferNULL() )
		return false;

	bool res = pCache->Write( iChannel, TimeValueToKTime(t), mpWriteBufferFloat, iNumPoints );
	return res;
}

KTime MCBufferIO::TimeValueToKTime( TimeValue t )
{
	//    KTime:
	//        1 sec = 46 186 158 000 Ticks (1/46186158000 sec)
	//    Max:
	//        1 sec. = 4 800 Ticks (1/4800 sec)
	//    Maya (point cache component)
	//        1 sec. = 6 000 Ticks (1/6000 sec)
	// 
	//	Then:
	//    1 Maya Tick = 7 697 693 KTime Ticks.
	// 
	//    1 Max Tick = 9 622 116.25 KTime Ticks.
	//

	KTime kt;
	kLongLong kll = (kLongLong)((double)t * (double)9622116.25);
	kt.Set(kll);
	return kt;
}


// Static objects/
static PointCacheInstances g_PointCacheInstances;


// PointCache /////////////////////////////////////////////////////////////////////////////////////

IObjParam* PointCache::m_iCore = NULL;
KFbxSdkManager* PointCache::m_pFbxSdkManager = NULL;

PointCache::PointCache() :
	m_pblock			(NULL),
	m_hWnd				(NULL),
	m_hCacheFile		(NULL),

	m_firstSample		(NULL),
	m_sampleA			(NULL),
	m_sampleB			(NULL),
	m_samplea			(NULL),
	m_sampleb			(NULL),

	m_preloadBuffer		(NULL),
	m_recordBuffer		(NULL),

	m_fileVersion		(POINTCACHE_FILEVERSION),
	m_numPoints			(0),

	m_errorFlags		(ERROR_CACHEMISSING),
	m_doLoad			(true),
	m_tempDisable		(false),
	m_recordMode		(false),
	m_fileCount			(eOneFile),
	m_cacheChannel		(0),
	m_channelDataType	(eDoubleVectorArray),
	m_PTSPointCache		(NULL),
	m_pFbxCache			(NULL),
	m_fileType			(ePC2),
	m_forcedChannel		(-1),
	m_modifierVersion	(POINTCACHE_MODIFIERVERSION)
{
	sprintf(m_signature,"POINTCACHE2");

	RegisterNotification(NotifyDelete, this, NOTIFY_SYSTEM_PRE_NEW);
	RegisterNotification(NotifyDelete, this, NOTIFY_FILE_PRE_OPEN);
	RegisterNotification(NotifyDelete, this, NOTIFY_SYSTEM_PRE_RESET);

}

PointCache::~PointCache()
{
	if(m_PTSPointCache)
	{
		delete m_PTSPointCache;
		m_PTSPointCache = NULL;
	}
	UnRegisterNotification(NotifyDelete, this, NOTIFY_SYSTEM_PRE_NEW);
	UnRegisterNotification(NotifyDelete, this, NOTIFY_FILE_PRE_OPEN);
	UnRegisterNotification(NotifyDelete, this, NOTIFY_SYSTEM_PRE_RESET);

	CloseCacheFile(true);

	if( m_pFbxCache ) {
		m_pFbxCache->Destroy();
		m_pFbxCache = NULL;
	}
}

bool PointCache::InitFbxSdk()
{
	if( m_pFbxSdkManager == NULL )
		m_pFbxSdkManager = KFbxSdkManager::CreateKFbxSdkManager();

	if( m_pFbxSdkManager == NULL )
		return false;

	if( m_pFbxCache == NULL )
		m_pFbxCache = KFbxCache::Create(m_pFbxSdkManager, _T("Point Cache"));

	return (m_pFbxCache != NULL);
}

void PointCache::CloneCache( PointCache *pNewPointCache )
{
	if( !pNewPointCache )
		return;

	// Clone members that might not be recalculated or reside in PB2s
	pNewPointCache->m_cacheChannel = m_cacheChannel;
	pNewPointCache->m_fileType = m_fileType;
	pNewPointCache->m_channelDataType = m_channelDataType;
	pNewPointCache->m_forcedChannel = m_forcedChannel;
}

// Animatable /////////////////////////////////////////////////////////////////////////////////////

void PointCache::GetClassName(TSTR& s)
{
	s = GetString(IDS_CLASS_NAME);
}

void PointCache::DeleteThis()
{
	delete this;
}

int PointCache::NumSubs()
{
	return 1;
}

Animatable* PointCache::SubAnim(int i)
{
	return m_pblock;
}

TSTR PointCache::SubAnimName(int i)
{
	return GetString(IDS_PARAMS);
}

class PointCacheAssetAccessor : public IAssetAccessor	{
public:

	PointCacheAssetAccessor(PointCache* aPointCache, bool bMCFileList);

	// path accessor functions
	virtual MaxSDK::Util::Path GetPath() const;
	virtual void SetPath(const MaxSDK::Util::Path& aNewPath);
	virtual bool IsInputAsset() const ;

	// asset client information
	virtual int GetAssetType() const ;
	virtual const TCHAR* GetAssetDesc() const;
	virtual const TCHAR* GetAssetClientDesc() const ;
	virtual bool IsAssetPathWritable() const;

protected:
	PointCache* mPointCache;
	bool mbMCFileList;
	TSTR mStrAsset;
	TSTR mStrClientDesc;
};

PointCacheAssetAccessor::PointCacheAssetAccessor(PointCache* aPointCache, bool bMCFileList) : 
mPointCache(aPointCache),
mbMCFileList(bMCFileList)
{
	if( mbMCFileList ) {
		mPointCache->GetMCCacheFilename(0, mStrAsset);
		if( mPointCache->GetNumMCCacheFiles() > 1 ) {
			TSTR strLast;
			if( mPointCache->GetMCCacheFilename(mPointCache->GetNumMCCacheFiles()-1, strLast) )
				mStrClientDesc.printf(GetString(IDS_ATS_DESC), strLast.data());
		}
	}
	else {
		TCHAR *fname = NULL;
		Interval iv;
		mPointCache->m_pblock->GetValue(pc_cache_file,0,fname,iv);
		if( fname )
			mStrAsset = fname;
	}
}

MaxSDK::Util::Path PointCacheAssetAccessor::GetPath() const	{
	return MaxSDK::Util::Path(mStrAsset.data());
}


void PointCacheAssetAccessor::SetPath(const MaxSDK::Util::Path& aNewPath)	{
	mPointCache->m_pblock->SetValue(
		pc_cache_file, 
		0, 
		const_cast<TCHAR*>(aNewPath.GetCStr()));
}

bool PointCacheAssetAccessor::IsInputAsset() const	{
	return true;
}

int PointCacheAssetAccessor::GetAssetType() const	{
	return IAssetAccessor::kAnimationAsset;
}

const TCHAR* PointCacheAssetAccessor::GetAssetDesc() const	{
	return NULL;
}
const TCHAR* PointCacheAssetAccessor::GetAssetClientDesc() const		{
	return (mStrClientDesc.Length() > 0) ? mStrClientDesc.data() : NULL;
}
bool PointCacheAssetAccessor::IsAssetPathWritable() const	{
	return !mbMCFileList;
}

void PointCache::EnumAuxFiles(NameEnumCallback& nameEnum, DWORD flags)
{
	if ((flags&FILE_ENUM_CHECK_AWORK1)&&TestAFlag(A_WORK1)) return; // LAM - 4/11/03

	if(flags & FILE_ENUM_ACCESSOR_INTERFACE)	{
		if( (m_fileType == eMC) && (GetNumMCCacheFiles() > 0) ) {
			PointCacheAssetAccessor accessorCacheXML(this, false);
			if(!accessorCacheXML.GetPath().IsEmpty())	{
				PointCacheAssetAccessor accessorCacheMC(this, true);
				IEnumAuxAssetsCallback* callback = static_cast<IEnumAuxAssetsCallback*>(&nameEnum);
				callback->DeclareGroup(accessorCacheXML);
				callback->DeclareAsset(accessorCacheMC);
				callback->EndGroup();
			}

		}
		else {
			PointCacheAssetAccessor accessor(this, false);
			if(!accessor.GetPath().IsEmpty())	{
				IEnumAuxAssetsCallback* callback = static_cast<IEnumAuxAssetsCallback*>(&nameEnum);
				callback->DeclareAsset(accessor);
			}
		}
	}
	else	{
		TCHAR *fname = NULL;
		Interval iv;
		m_pblock->GetValue(pc_cache_file,0,fname,iv);
		if(NULL != fname) 
		{
			MaxSDK::Util::Path cachePath(fname);
			IPathConfigMgr::GetPathConfigMgr()->RecordInputAsset(
				cachePath,
				nameEnum, 
				flags);
		}

		Modifier::EnumAuxFiles( nameEnum, flags ); // LAM - 4/21/03

	}

}

int PointCache::NumParamBlocks()
{
	return 1;
}

IParamBlock2* PointCache::GetParamBlock(int i)
{
	return m_pblock;
}

IParamBlock2* PointCache::GetParamBlockByID(BlockID id)
{
	return (m_pblock->ID() == id) ? m_pblock : NULL;
}

// ReferenceMaker /////////////////////////////////////////////////////////////////////////////////

int PointCache::NumRefs()
{
	return 1;
}

RefTargetHandle PointCache::GetReference(int i)
{
	return m_pblock;
}

void PointCache::SetReference(int i, RefTargetHandle rtarg)
{
	m_pblock = (IParamBlock2*)rtarg;
}

RefResult PointCache::NotifyRefChanged
(
	Interval changeInt, RefTargetHandle hTarget,
	PartID& partID,  RefMessage message
) {
	return REF_SUCCEED;
}

// BaseObject /////////////////////////////////////////////////////////////////////////////////////

CreateMouseCallBack* PointCache::GetCreateMouseCallBack()
{
	return NULL;
}

BOOL PointCache::ChangeTopology()
{
	return FALSE;
}

BOOL PointCache::HasUVW()
{
	return TRUE;
}

void PointCache::SetGenUVW(BOOL sw)
{
	if (sw==HasUVW()) return;
}






// Modifier ///////////////////////////////////////////////////////////////////////////////////////

Interval PointCache::LocalValidity(TimeValue t)
{
	if(m_tempDisable)
		return FOREVER;
	int playType;
	m_pblock->GetValue(pc_playback_type,t,playType,FOREVER);
	switch (playType)
	{
		case playback_original:
		{
			int tpf = GetTicksPerFrame();

			TimeValue startTime = (TimeValue)m_cacheStartFrame * tpf;

			float frameLength = (m_cacheNumSamples-1) / (1.0f/m_realCacheSampleRate);
			TimeValue endTime = startTime + (TimeValue)(tpf * frameLength);

			if (t < startTime)
				return Interval(TIME_NegInfinity, startTime-1);
			else if (t > endTime)
				return Interval(endTime + 1, TIME_PosInfinity);
			else
				return Interval(t,t);
		}
		case playback_start:
		{
			int tpf = GetTicksPerFrame();

			TimeValue startTime = (TimeValue)m_pblock->GetFloat(pc_playback_start, 0) * tpf;

			float frameLength = (m_cacheNumSamples-1) / (1.0f/m_realCacheSampleRate);
			TimeValue endTime = startTime + (TimeValue)(tpf * frameLength);

			if (t < startTime)
				return Interval(TIME_NegInfinity, startTime-1);
			else if (t > endTime)
				return Interval(endTime+1, TIME_PosInfinity);
			else
				return Interval(t,t);
		}
		case playback_range:
		{
			int tpf = GetTicksPerFrame();
			Interval iv(t,t);

			float sf, ef;
			m_pblock->GetValue(pc_playback_start,t,sf,FOREVER);
			m_pblock->GetValue(pc_playback_end,t,ef,FOREVER);
			if (ef < sf)
			{
				float tmp = sf;
				sf = ef;
				ef = tmp;
			}
			TimeValue s = int(sf * tpf);
			TimeValue e = int(ef * tpf);


			int ort;
			m_pblock->GetValue(pc_playback_before,0,ort,FOREVER);
			if (ort != ort_hold)
				s = TIME_NegInfinity;
			m_pblock->GetValue(pc_playback_after,0,ort,FOREVER);
			if (ort != ort_hold)
				e = TIME_PosInfinity;

			if (t > e)
				iv.Set(e+1,TIME_PosInfinity);
			else if (t < s)
				iv.Set(TIME_NegInfinity,s-1);

			return iv;
		}
		case playback_graph:
		{
			Interval iv = FOREVER;

			float v;
			m_pblock->GetValue(pc_playback_frame,t,v,iv);

			return iv;
		}
	}

	Interval iv(t,t);
	return iv;
}

ChannelMask PointCache::ChannelsUsed()
{
	return (PART_GEOM | PART_TOPO);
}

ChannelMask PointCache::ChannelsChanged()
{
	return GEOM_CHANNEL;
}

void PointCache::ModifyObject(TimeValue t, ModContext& mc, ObjectState* os, INode* node)
{
	Object* obj = os->obj;
	m_numPoints = obj->NumPoints();

	// check if applied to more than one object
    sMyEnumProc dep;
	DoEnumDependents(&dep);
	if (dep.Nodes.Count()>1)
	{
		SetError(ERROR_INSTANCE);
	} else {
		ClearError(ERROR_INSTANCE);
	}

	int loadType = (GetCOREInterface()->InSlaveMode()) ?	m_pblock->GetInt(pc_load_type_slave, 0) :
															m_pblock->GetInt(pc_load_type, 0);

	if (m_recordMode)
	{
		DEBUGPRINT("PointCache::ModifyObject: In record mode at time %d\n", t);

		if ((IsCacheFileOpen()) && (m_recordTime == t))
		{
			if (m_numPoints == m_cacheNumPoints)
			{
				for (int i = 0; i < m_cacheNumPoints; i++)
				{
					if( m_fileType == ePC2 )
						m_recordBuffer[i] = os->obj->GetPoint(i);
					else if( m_fileType == eMC )
						m_mcBufferIO.SetWriteBuffer(i, &(os->obj->GetPoint(i)));
				}

				DWORD numWrite;
				DEBUGPRINT("PointCache::ModifyObject: Writing File: m_hCacheFile: %d\n", m_hCacheFile);
				BOOL res(false);
				if( m_fileType == ePC2 ) {
					res = WriteFile(m_hCacheFile, m_recordBuffer, sizeof(Point3)*m_cacheNumPoints, &numWrite, NULL);
				}
				else if( m_fileType == eMC ) {
					res = m_mcBufferIO.Write( m_pFbxCache, m_cacheChannel, t, m_cacheNumPoints );
				}
				if (!res)
				{
					DEBUGPRINT("Failed: %s\n", GetLastErrorString());
				}
				//TODO: if !res, handle error
			}
			else
			{
				// The point count changed underneath us... so skip dumping data.  The outer record loop will
				// see this and deal with the rest...
			}
		}
		++m_recordTime;
	}
	else if (!m_tempDisable && m_doLoad)
	{
		TCHAR* fname = m_pblock->GetStr(pc_cache_file, 0);

		if (IsValidFName(fname))
		{
			int interpType = m_pblock->GetInt(pc_interp_type, 0);
			BOOL relativeOffset = m_pblock->GetInt(pc_relative, 0);
			float strength = m_pblock->GetFloat(pc_strength, t);
			// Go into file opening loop if:
			// - we're pre-loading and haven't actually loaded anything yet, or
			// - we're not pre-loading and the cacheFile needs to be opened still

			if(m_fileType==ePTS)
			{
				m_hCacheFile = NULL;
				if(m_PTSPointCache==NULL)
					m_PTSPointCache = new OLDPointCacheBase();
				float startTime = m_pblock->GetFloat(pc_playback_start, 0);

				int error = m_PTSPointCache->PerformPTSMod(fname,startTime,relativeOffset,strength,t,
					m_cacheNumPoints,m_cacheSampleRate,m_cacheNumSamples,mc,os,node);
				m_realCacheSampleRate = m_cacheSampleRate; //we use this and it's the same for old files mz defect #806998
				m_cacheStartFrame = startTime;
				if(error==2)
					SetError(ERROR_NUMPOINTS);
				else if(error==3)
					SetError(ERROR_CACHEINVALID);
				else if (error==0)
				{
					ClearError(ERROR_CACHEMISSING);
					ClearError(ERROR_NUMPOINTS);
					ClearError(ERROR_CACHEINVALID);
				}
				Interval iv = LocalValidity(t);
				iv = iv & os->obj->ChannelValidity(t, GEOM_CHAN_NUM);
				os->obj->UpdateValidity(GEOM_CHAN_NUM, iv);
				return;
			}
			else if( m_fileType == eMC ) {

				bool bApplySample = true;

				// If we don't have a cache open, open one
				if( !IsCacheFileOpen() ) {
					bApplySample = false;

					// Open Cache
					OpenCacheForRead(fname);

					if( IsCacheFileOpen() ) {
						ClearError(ERROR_CACHEMISSING);
						ClearError(ERROR_NUMPOINTS);

						// Read Header information
						if( ReadHeader() ) {
							ClearError(ERROR_CACHEINVALID);
							ClearError(ERROR_NO_DEFORM_IN_FILE);

							// Set up buffers
							FreeBuffers();
							m_firstSample = new Point3[m_cacheNumPoints];
							m_sampleA = new Point3[m_cacheNumPoints];
							m_mcBufferIO.AllocReadBuffer(m_cacheNumPoints*3);

							// Read first sample
							if( ReadSampleMC(0, m_firstSample) == false ) {
								SetError(ERROR_CACHEINVALID);
								CloseCacheFile(true);
							}
							else
								bApplySample = true;
						}
						else {
							// If it isn't the number of points, set the error
							// to an invalid cache
							if( !TestError(ERROR_NUMPOINTS) && !TestError(ERROR_NO_DEFORM_IN_FILE) )
								SetError(ERROR_CACHEINVALID);
							CloseCacheFile(true);
						}
					}
					else
						SetError(ERROR_CACHEMISSING);
				}

				// Apply sample to object
				if( bApplySample && m_sampleA && m_firstSample ) {

					// Make certain the number of points hasn't changed
					if( m_numPoints == m_cacheNumPoints ) {
						// Calc TimeValue based on playback options
						float fIndex = GetSampleIndex(t);
						TimeValue tSample = (TimeValue)( ((fIndex * m_cacheSampleRate)+m_cacheStartFrame) * (float)GetTicksPerFrame() );

						DEBUGPRINT("t=%d, tSample=%d findex=%f\n", t, tSample, fIndex);

						// Read new sample
						ReadSampleMC(tSample, m_sampleA);

						bool useSubObjectSelection = (obj->GetSubselState() != 0) && (!m_pblock->GetInt(pc_apply_to_whole_object));
						for( int i=0; i<m_numPoints; ++i ) {
							if( i < m_cacheNumPoints ) {
								Point3 p = obj->GetPoint(i);
								Point3 newPoint = m_sampleA[i];
								float selWeight = (useSubObjectSelection) ? obj->PointSelection(i) : 1.0f;
								if( relativeOffset ) {
									p += (newPoint - m_firstSample[i]) * strength * selWeight;
								} 
								else {
									p += (newPoint - p) * strength * selWeight;
								}
								obj->SetPoint(i, p);
							}
						}

						obj->PointsWereChanged();
					}
					else {
						SetError(ERROR_NUMPOINTS);
					}

					if (m_hWnd&&m_pblock->GetMap()) 
						m_pblock->GetMap()->Invalidate(); 

					Interval iv = LocalValidity(t);
					iv = iv & os->obj->ChannelValidity(t, GEOM_CHAN_NUM);
					os->obj->UpdateValidity(GEOM_CHAN_NUM, iv);
				}
				return;
			}
			else if (	// PC2
				((loadType == load_preload) && !m_preloadBuffer) ||
				((loadType != load_preload) && !IsCacheFileOpen())
			) {
				m_hCacheFile = OpenCacheForRead(fname);
				//read header and first frame
				if (IsCacheFileOpen())
				{
					ClearError(ERROR_CACHEMISSING);

					if (ReadHeader())
					{
						ClearError(ERROR_CACHEINVALID);

						// try to preload cache if desired...
						if (loadType == load_preload)
						{
							FreeBuffers();
							GetPointCacheManager().PreLoadPointCache(this);

							CloseFile(m_hCacheFile);
							m_hCacheFile = NULL;
						}

						if ((loadType == load_preload) && m_preloadBuffer)
						{
							m_firstSample =
							m_sampleA = m_sampleB =
							m_samplea = m_sampleb = m_preloadBuffer;
						} else {
							FreeBuffers();

							m_sampleA = new Point3[m_cacheNumPoints];
							m_sampleB = new Point3[m_cacheNumPoints];
							m_firstSample = new Point3[m_cacheNumPoints];

							if (interpType == interp_cubic)
							{
								m_samplea = new Point3[m_cacheNumPoints];
								m_sampleb = new Point3[m_cacheNumPoints];
							}

							//load first frame
							ClearError(ERROR_NO_DEFORM_IN_FILE);
							if(ReadSample(0, m_firstSample)==false)
							{
								SetError(ERROR_NO_DEFORM_IN_FILE);
								CloseCacheFile(true);
							}
						}
					} else {
						SetError(ERROR_CACHEINVALID);

						CloseCacheFile(true);
					}
				} else {
					SetError(ERROR_CACHEMISSING);
				}
			}

			BOOL applyToSpline = m_pblock->GetInt(pc_apply_to_spline, 0);

			//load up cache data
			if (
				// worlds ugliest if statement...
				(
					// check that all our normal samples are allocated...
					(IsCacheFileOpen() || m_preloadBuffer) && m_firstSample && m_sampleA && m_sampleB
				) &&
				(
					// ...and if using cubic interpolation, check that our extra samples are allocated...
					(interpType == interp_cubic) ? (m_samplea && m_sampleb) : true
				) &&
				(
					// ...and lastly check that the points count matches
					(!applyToSpline && (m_numPoints == m_cacheNumPoints)) ||
					(applyToSpline && (m_numPoints == m_cacheNumPoints*3))
				)
			) {
				ClearError(ERROR_NUMPOINTS);

				float fIndex = GetSampleIndex(t);
				//fix for fIndex near integer but less than, we want to make sure it is equal to one!
				float iIndex = (float)(int) fIndex;
				if(fabs(iIndex-fIndex)<1e-6f)
					fIndex =iIndex;
				fIndex = fmodf(fIndex, (float)m_cacheNumSamples);
				if (fIndex < 0.0f) fIndex += m_cacheNumSamples;

				int indexA = (int)fmodf(fIndex, (float)m_cacheNumSamples);
				int indexB = (int)fmodf(float(indexA+1), (float)m_cacheNumSamples);
				float perc = fIndex - (int)fIndex;

				DEBUGPRINT("t: %d, indexA: %d, indexB: %d, perc: %f\n", t, indexA, indexB, perc);

				// load up the buffers based on the computed indexes
				if (m_preloadBuffer)
				{
					m_sampleA = m_preloadBuffer + (indexA * m_cacheNumPoints);
					m_sampleB = m_preloadBuffer + (indexB * m_cacheNumPoints);
				} else {
					ReadSample(indexA, m_sampleA);

					if (indexB == indexA)
					{
						m_sampleB = m_sampleA;
					} else {
						ReadSample(indexB, m_sampleB);
					}
				}

				bool useSubObjectSelection = (obj->GetSubselState() != 0) && (!m_pblock->GetInt(pc_apply_to_whole_object));

				// now apply the samples
				switch (interpType)
				{
					case interp_linear:
					{
						for (int i=0; i<m_numPoints; ++i)
						{
							if (applyToSpline)
							{
								if (((i-1) % 3) != 0) // check if i is a knot point
									continue; // if not, bail (don't mess with tangents)
								else
									i = int(float(i-1) / 3.0f); // otherwise, turn knot index into point index
							}

							if (i < m_cacheNumPoints)
							{
								Point3 p = obj->GetPoint(i);

								Point3 newPoint = m_sampleA[i] + (m_sampleB[i] - m_sampleA[i]) * perc;

								float selWeight = (useSubObjectSelection) ? obj->PointSelection(i) : 1.0f;

								if (relativeOffset)
								{
									p += (newPoint - m_firstSample[i]) * strength * selWeight;
								} else {
									p += (newPoint - p) * strength * selWeight;
								}

								obj->SetPoint(i, p);
							}
						}
						break;
					}
					case interp_cubic:
					{
						// using cubic, so we need two extra samples...
						int indexa = indexA - 1;
						if (indexa < 0) indexa = 0; else if (indexa > (m_cacheNumSamples-1)) indexa = (m_cacheNumSamples-1);
						int indexb = indexB + 1;
						if (indexb < 0) indexb = 0; else if (indexb > (m_cacheNumSamples-1)) indexb = (m_cacheNumSamples-1);

						// read our extra samples (but only if really necessary)
						if (m_preloadBuffer)
						{
							m_samplea = m_preloadBuffer + (indexa * m_cacheNumPoints);
							m_sampleb = m_preloadBuffer + (indexb * m_cacheNumPoints);
						} else {
							if (indexa == indexA)
							{
								m_samplea = m_sampleA;
							} else {
								ReadSample(indexa, m_samplea);
							}

							if (indexb == indexB)
							{
								m_sampleb = m_sampleB;
							} else {
								ReadSample(indexb, m_sampleb);
							}
						}

						// ...and deform the object points
						for (int i=0; i<m_numPoints; i++)
						{
							if (applyToSpline)
							{
								if (((i-1) % 3) != 0) // check if i is a knot point
									continue; // if not, bail (don't mess with tangents)
								else
									i = int(float(i-1) / 3.0f); // otherwise, turn knot index into point index
							}

							if (i < m_cacheNumPoints)
							{
								Point3 p = obj->GetPoint(i);

								Point3 P = (m_sampleb[i] - m_sampleB[i]) - (m_samplea[i] - m_sampleA[i]);
								Point3 Q = (m_samplea[i] - m_sampleA[i]) - P;
								Point3 R = m_sampleB[i] - m_samplea[i];
								Point3 S = m_sampleA[i];
								float a = powf(perc, 3);
								float b = powf(perc, 2);

								Point3 newPoint = (P * a) + (Q * b) + (R * perc) + S;

								float selWeight = (useSubObjectSelection) ? obj->PointSelection(i) : 1.0f;

								if (relativeOffset)
								{
									p += (newPoint - m_firstSample[i]) * strength * selWeight;
								} else {
									p += (newPoint - p) * strength * selWeight;
								}

								obj->SetPoint(i, p);
							}
						}
						break;
					}
				}

				obj->PointsWereChanged();
			} else {
				if(IsCacheFileOpen())
					SetError(ERROR_NUMPOINTS);
			}
		} else {
			SetError(ERROR_CACHEMISSING);
		}

		if (loadType == load_persample)
		{
			CloseCacheFile(true);
		}
	}
	if (m_hWnd&&m_pblock->GetMap()) m_pblock->GetMap()->Invalidate(); //fix for 791963 mz. Also don't want to completely update the UI, but just want to invalidate it
												  //so the key brackets are correctly redrawn

	Interval iv = LocalValidity(t);
	iv = iv & os->obj->ChannelValidity(t, GEOM_CHAN_NUM);
	os->obj->UpdateValidity(GEOM_CHAN_NUM, iv);
}

Class_ID PointCache::InputType()
{
	return defObjectClassID;
}

// Handle old versions of the modifier ////////////////////////////////////////////////////////////

class FixUpModifierVersionPLCB : public PostLoadCallback
{
public:
	PointCache* m_pc;
	FixUpModifierVersionPLCB(PointCache* pc) { m_pc = pc; }

	void proc(ILoad *iload)
	{
		if (m_pc->m_modifierVersion < 1)
		{
			int oldType = m_pc->m_pblock->GetInt(pc_playback_type, 0);
			int newType = playback_original;
			switch (oldType)
			{
				case old_playback_range: newType = playback_range; break;
				case old_playback_graph: newType = playback_graph; break;
			}
			m_pc->m_pblock->SetValue(pc_playback_type, 0, newType);
		}

		if (m_pc->m_modifierVersion < 2)
		{
			// old versions were always clamped... new default is to be not clamped
			m_pc->m_pblock->SetValue(pc_clamp_graph, 0, TRUE);
		}

		if (m_pc->m_modifierVersion < 3)
		{
			int loadType = (m_pc->m_pblock->GetInt(pc_preload_cache)) ? load_preload : load_stream;
			m_pc->m_pblock->SetValue(pc_load_type, 0, loadType);
		}

		if (m_pc->m_modifierVersion < 4)
		{
			m_pc->m_pblock->SetValue(pc_load_type_slave, 0, load_persample);
		}

		m_pc->m_modifierVersion = POINTCACHE_MODIFIERVERSION;
	}
};

#define MODIFIERVERSION_CHUNK	0x0000
#define ENABLE_CHUNK		0x2120
#define ENABLEINRENDER_CHUNK		0x2130


class ErrorPLCB : public PostLoadCallback {
	public:
		PointCache *obj;
		ErrorPLCB(PointCache *o) {obj=o;}
		void proc(ILoad *iload);
	};

void ErrorPLCB::proc(ILoad *iload)
{
	TCHAR *fname = NULL;
	Interval iv;
	obj->m_pblock->GetValue(pc_cache_file,0,fname,iv);
	obj->SetFileType(fname);

	if(obj->m_pblock && 
		( (obj->m_pcacheEnabled && obj->m_pcacheEnabledInRender) ||
		  (obj->m_pcacheEnabled && !obj->m_pcacheEnabledInRender) ) )
	{
		if (fname)
		{
			MaxSDK::Util::Path cachePath(fname);
			IPathConfigMgr* pathMgr = IPathConfigMgr::GetPathConfigMgr();
			if (!pathMgr->SearchForExternalFiles(cachePath)) 
			{
				obj->SetError(ERROR_CACHEMISSING);
			}
			
		}	
	}
	delete this;
}


IOResult PointCache::Load(ILoad* iload)
{
	ULONG nb;
	IOResult res;

	res = Modifier::Load(iload);
    if (res != IO_OK) return res;

	int loadVer = 0;

	m_pcacheEnabled = TRUE;
	m_pcacheEnabledInRender = TRUE;



	while (IO_OK==(res=iload->OpenChunk()))
	{
		switch(iload->CurChunkID())
		{
			case MODIFIERVERSION_CHUNK:
			{
				iload->Read(&loadVer, sizeof(int), &nb);
				break;
			}
			case ENABLE_CHUNK:
				iload->Read(&m_pcacheEnabled,sizeof(m_pcacheEnabled), &nb);
				break;
			case ENABLEINRENDER_CHUNK:
				iload->Read(&m_pcacheEnabledInRender,sizeof(m_pcacheEnabledInRender), &nb);
				break;


		}
		iload->CloseChunk();
		if (res!=IO_OK)
			return res;
	}

	if (loadVer != POINTCACHE_MODIFIERVERSION)
	{
		m_modifierVersion = loadVer;
		iload->RegisterPostLoadCallback(new FixUpModifierVersionPLCB(this));
	}
	iload->RegisterPostLoadCallback(new ErrorPLCB(this));	

#ifdef DO_MISSING_FILES_PLCB //these are older missing files routines that are no longer used. Left in code in case we want to revisit.
	iload->RegisterPostLoadCallback(new GetMissingFilesPLCB(this));
	iload->RegisterPostLoadCallback(new NotifyMissingFilesPLCB());
#endif

	return IO_OK;
}

IOResult PointCache::Save(ISave* isave)
{
	ULONG nb;
	IOResult res;

	res = Modifier::Save(isave);
	if (res != IO_OK) return res;

	m_pcacheEnabled = IsEnabled();
	m_pcacheEnabledInRender = IsEnabledInRender();

	isave->BeginChunk(ENABLE_CHUNK);
	isave->Write(&m_pcacheEnabled,sizeof(m_pcacheEnabled),&nb);
	isave->EndChunk();

	isave->BeginChunk(ENABLEINRENDER_CHUNK);
	isave->Write(&m_pcacheEnabledInRender,sizeof(m_pcacheEnabledInRender),&nb);
	isave->EndChunk();


	isave->BeginChunk(MODIFIERVERSION_CHUNK);
	isave->Write(&m_modifierVersion, sizeof(m_modifierVersion), &nb);
	isave->EndChunk();

	return IO_OK;
}

// PointCache /////////////////////////////////////////////////////////////////////////////////////

void PointCache::SetError(DWORD error)
{
	m_errorFlags |= error;
}

void PointCache::ClearError(DWORD error)
{
	m_errorFlags &= ~error;
}

bool PointCache::TestError(DWORD error)
{
	return ((m_errorFlags & error) ? true : false);
}

void PointCache::UpdateCacheInfoStatus( HWND hWnd, const TCHAR* szForceMsg )
{
	TSTR str;
	bool bShowData = false;

	// Cache info/error messages
	if (szForceMsg != NULL )
		str = szForceMsg;
	else if (TestError(ERROR_CACHEMISSING))
		str = GetString(IDS_ERROR_CACHEMISSING);
	else if (TestError(ERROR_CACHEINVALID))
		str = GetString(IDS_ERROR_CACHEINVALID);
	else if (TestError(ERROR_NUMPOINTS))
		str.printf(GetString(IDS_ERROR_NUMPOINTS), m_cacheNumPoints);
	else if (TestError(ERROR_POINTCOUNTCHANGED))
		str.printf(GetString(IDS_ERROR_POINTCOUNTCHANGED));
	else if (TestError(ERROR_NONODEFOUND))
		str.printf(GetString(IDS_ERROR_NONODEFOUND));
	else if(TestError(ERROR_NO_DEFORM_IN_FILE))
		str.printf(GetString(IDS_NO_DEFORM_IN_FILE));
	else 
		bShowData = true;

	ShowWindow(GetDlgItem(hWnd,IDC_MESSAGE_LABEL),			!bShowData?SW_SHOW:SW_HIDE);
	ShowWindow(GetDlgItem(hWnd,IDC_MSG_FILES),				bShowData?SW_SHOW:SW_HIDE);
	ShowWindow(GetDlgItem(hWnd,IDC_MSG_POINTCOUNT),			bShowData?SW_SHOW:SW_HIDE);
	ShowWindow(GetDlgItem(hWnd,IDC_MSG_EVAL),				bShowData?SW_SHOW:SW_HIDE);
	ShowWindow(GetDlgItem(hWnd,IDC_MSG_SAMPLERATE),			bShowData?SW_SHOW:SW_HIDE);
	ShowWindow(GetDlgItem(hWnd,IDC_MSG_STARTEND),			bShowData?SW_SHOW:SW_HIDE);
	ShowWindow(GetDlgItem(hWnd,IDC_MSG_FILES_DATA),			bShowData?SW_SHOW:SW_HIDE);
	ShowWindow(GetDlgItem(hWnd,IDC_MSG_POINTCOUNT_DATA),	bShowData?SW_SHOW:SW_HIDE);
	ShowWindow(GetDlgItem(hWnd,IDC_MSG_EVAL_DATA),			bShowData?SW_SHOW:SW_HIDE);
	ShowWindow(GetDlgItem(hWnd,IDC_MSG_SAMPLERATE_DATA),	bShowData?SW_SHOW:SW_HIDE);
	ShowWindow(GetDlgItem(hWnd,IDC_MSG_STARTEND_DATA),		bShowData?SW_SHOW:SW_HIDE);

	if( !bShowData ) {
		SetWindowText(GetDlgItem(hWnd,IDC_MESSAGE_LABEL), str);
	}
	else {
		// Num Files
		int iNumFiles = 1;
		if( m_fileType == eMC ) {
			if( m_fileCount == eOneFile )
				iNumFiles = 2;
			else 
				iNumFiles = ((int)(m_cacheSampleRate * (float)m_cacheNumSamples))+1;
		}

		str.printf("%d", iNumFiles );
		SetWindowText(GetDlgItem(hWnd,IDC_MSG_FILES_DATA), str);

		// Point Count
		str.printf("%d", m_cacheNumPoints);
		SetWindowText(GetDlgItem(hWnd,IDC_MSG_POINTCOUNT_DATA), str);

		// Evals
		str.printf("%d", m_cacheNumSamples);
		SetWindowText(GetDlgItem(hWnd,IDC_MSG_EVAL_DATA), str);

		// Sample Rate
		str.printf("%.3f", m_cacheSampleRate);
		SetWindowText(GetDlgItem(hWnd,IDC_MSG_SAMPLERATE_DATA), str);

		// Start End
		float cacheEndFrame = m_cacheStartFrame + ((m_cacheNumSamples-1) / (1.0f / m_cacheSampleRate));
		str.printf("%d/%d", (int)m_cacheStartFrame, (int)cacheEndFrame);
		SetWindowText(GetDlgItem(hWnd,IDC_MSG_STARTEND_DATA), str);
	}

	// Cache Info Group label
	if (m_doLoad)
		str = GetString(IDS_CACHEINFO_GROUP);
	else
		str = GetString(IDS_CACHEINFOUNLOADED_GROUP);

	SetWindowText(GetDlgItem(hWnd,IDC_CACHEINFO_GROUP),str);

}

void PointCache::UpdateUI()
{
	TCHAR* fname = NULL;
	m_pblock->GetValue(pc_cache_file,0,fname,FOREVER);
	TSTR str(fname);
	SetFileName(str);
	if (m_hWnd) m_pblock->GetMap()->Invalidate();
}

bool PointCache::IsValidFName(TCHAR* fname)
{
	if (fname && (*fname != _T('\0')) )
		return true;
	else
		return false;
}

void PointCache::SetForcedChannelNumber( int iChannelNum )
{
	m_forcedChannel = iChannelNum;
}

void PointCache::ClearForcedChannelNumber()
{
	m_forcedChannel = -1;
}

int PointCache::GetNumMCCacheFiles()
{
	if( !m_pFbxCache )
		return 0;

	return m_pFbxCache->GetCacheDataFileCount();
}

bool PointCache::GetMCCacheFilename( int iIndex, TSTR &strFile )
{
	if( !m_pFbxCache )
		return false;

	if( (iIndex < 0) || (iIndex >= GetNumMCCacheFiles()) )
		return false;

	KString kStrAbs, kStrRel;
	if( !m_pFbxCache->GetCacheDataFileName( iIndex, kStrRel, kStrAbs ) )
		return false;

	strFile = kStrAbs;

	return true;
}


bool PointCache::IsMultipleChannelCache( const TCHAR* fname )
{
	if( !fname )
		return false;

	// file type should be set at this point
	if( m_fileType != eMC )
		return false;

	// Even thought we don't use m_pFbxCache, let's still init fbx here
	if( !InitFbxSdk() )
		return false;

	KFbxCache* pTempFbxCache = NULL;
	pTempFbxCache = KFbxCache::Create(m_pFbxSdkManager, _T("Point Cache Temp"));

	if( !pTempFbxCache )
		return false;

	TSTR strPath;
	SplitFilename(TSTR(fname), &strPath, NULL, NULL);
	pTempFbxCache->SetCacheFileName( strPath, fname );
	pTempFbxCache->SetCacheFileFormat( KFbxCache::eMC );
	pTempFbxCache->OpenFileForRead();

	int iNumTotalChannels = pTempFbxCache->GetChannelCount();

	pTempFbxCache->CloseFile();
	pTempFbxCache->Destroy();
	pTempFbxCache = NULL;

	return iNumTotalChannels > 1;
}

bool PointCache::GetCompatibleChannelInfo( int iNumPoints, int &iNumChannels, int &iFirstChannel )
{
	iFirstChannel = -1;
	iNumChannels = 0;
	int iNumTotalChannels = m_pFbxCache->GetChannelCount();

	if( iNumTotalChannels < 1 )
		return false;

	// Iterate over channels to find matching type and point count
	for( int i=0; i<iNumTotalChannels; i++ ) {
		KTime ktStartTime, ktEndTime;
		if( !m_pFbxCache->GetAnimationRange(i, ktStartTime, ktEndTime) )
			continue;

		unsigned int lNumPoints = 0;
		if( !m_pFbxCache->GetChannelPointCount(i, ktStartTime, lNumPoints) )
			continue;

		// Check point count
		if( lNumPoints == iNumPoints ) {
			// We have a match
			iNumChannels++;
			if( iFirstChannel == -1 )
				iFirstChannel = i;
		}
	}

	return true;
}

bool PointCache::ReadHeader()
{
	if( m_fileType == eMC ) {

		// Get channel number
		m_cacheChannel = 0;

		// If we have a forced channel #, try using it
		if( (m_forcedChannel >= 0) && (m_forcedChannel < m_pFbxCache->GetChannelCount()) ) {
			m_cacheChannel = m_forcedChannel;
		}
		else {
			// else, find the first compatible channel
			int iNumChannels = 0;
			int iFirstChannel = 0;
			if( !GetCompatibleChannelInfo( m_numPoints, iNumChannels, iFirstChannel ) )
				return false;

			if( iFirstChannel >= 0 )
				m_cacheChannel = iFirstChannel;
		}

		// Get sample count
		unsigned int lNumSamples = 0;
		if( !m_pFbxCache->GetChannelSampleCount(m_cacheChannel, lNumSamples) )
			return false;
		m_cacheNumSamples = lNumSamples;

		if( m_cacheNumSamples == 0 ) {
			SetError(ERROR_NO_DEFORM_IN_FILE);
			return false;
		}

		// Get time values
		KTime ktTimePerFrame = m_pFbxCache->GetCacheTimePerFrame();
		KTime ktStartTime, ktEndTime, ktSampleRate;
		if( !m_pFbxCache->GetAnimationRange(m_cacheChannel, ktStartTime, ktEndTime) )
			return false;
		if( !m_pFbxCache->GetChannelSamplingRate(m_cacheChannel, ktSampleRate) ) {
			// 934889
			// If we don't have a sample rate for the channel, we'll need to calculate it
			// ourself.  This might be because the channel sample rate is irregular.
			if( m_cacheNumSamples <= 1 )
				return false;

			kLongLong kLongSample = (kLongLong)((double)(ktEndTime.Get() - ktStartTime.Get()) / (double)(m_cacheNumSamples-1));
			ktSampleRate.Set(kLongSample);
		}

		// Check validity
		if( (ktTimePerFrame.Get() == 0) || (ktSampleRate.Get() == 0) )
			return false;

		// Calc members
		m_cacheStartFrame = ktStartTime.Get() / ktTimePerFrame.Get();
		m_cacheSampleRate = (float)ktSampleRate.Get() / (float)ktTimePerFrame.Get();

		// Get Num of data points
		unsigned int lNumPoints = 0;
		if( !m_pFbxCache->GetChannelPointCount(m_cacheChannel, ktStartTime, lNumPoints) )
			return false;
		m_cacheNumPoints = (int)lNumPoints;
		if( m_cacheNumPoints != m_numPoints ) {
			SetError(ERROR_NUMPOINTS);
			return false;
		}

		// Get File Count type
		KFbxCache::EMCFileCount pFileCount;
		m_pFbxCache->GetCacheType(pFileCount);
		m_fileCount = pFileCount == KFbxCache::eMC_ONE_FILE ? eOneFile : eOneFilePerFrame;

		// Evaluate data types
		KFbxCache::EMCDataType dataType = KFbxCache::kDoubleVectorArray;
		if( !m_pFbxCache->GetChannelDataType(m_cacheChannel, dataType) )
			return false;

		if( dataType == KFbxCache::kDoubleVectorArray )
			m_channelDataType = eDoubleVectorArray;
		else if( dataType == KFbxCache::kFloatVectorArray )
			m_channelDataType = eFloatVectorArray;
		else
			return false;
	}
	else {
		SetFilePointer(m_hCacheFile, 0, NULL, FILE_BEGIN);

		DWORD numRead;
		if (!ReadFile(m_hCacheFile, &m_cacheSignature, sizeof(m_cacheSignature), &numRead, NULL) || numRead == 0) return false;
		if (strcmp(m_signature,m_cacheSignature) != 0) return false;

		if (!ReadFile(m_hCacheFile, &m_cacheVersion, sizeof(m_cacheVersion), &numRead, NULL) || numRead == 0) return false;
		if (!ReadFile(m_hCacheFile, &m_cacheNumPoints, sizeof(m_cacheNumPoints), &numRead, NULL) || numRead == 0) return false;
		if (!ReadFile(m_hCacheFile, &m_cacheStartFrame, sizeof(m_cacheStartFrame), &numRead, NULL) || numRead == 0) return false;
		if (!ReadFile(m_hCacheFile, &m_cacheSampleRate, sizeof(m_cacheSampleRate), &numRead, NULL) || numRead == 0) return false;
		if (!ReadFile(m_hCacheFile, &m_cacheNumSamples, sizeof(m_cacheNumSamples), &numRead, NULL) || numRead == 0) return false;

		m_fileCount = eOneFile;
		m_channelDataType = eFloatVectorArray;
	}

	TimeValue tpf = GetTicksPerFrame();

	TimeValue recordInc = (m_cacheSampleRate * tpf);
	if( recordInc == 0 )
		recordInc = 1;
	TimeValue endFrame = m_cacheStartFrame*tpf + (recordInc * m_cacheNumSamples); //cachestart frame is in frames need to make sure in ticks
	TimeValue numFrames = 	endFrame/tpf - m_cacheStartFrame ;
	m_realCacheSampleRate = (float)numFrames/(float)(m_cacheNumSamples);

	return true;
}

bool PointCache::IsCacheFileOpen()
{
	if( m_fileType == eMC )
		return ((m_pFbxCache != NULL) && m_pFbxCache->IsOpen());
	else
		return (m_hCacheFile != NULL) && (m_hCacheFile != INVALID_HANDLE_VALUE);
}

HANDLE PointCache::OpenCacheForRead(TCHAR* fname)
{
	DEBUGPRINT("PointCache::OpenCacheForRead: %s\n", fname);

	HANDLE hFile = NULL;
	if (IsValidFName(fname))
	{
		MaxSDK::Util::Path path(fname);
		IPathConfigMgr* pathMgr = IPathConfigMgr::GetPathConfigMgr();
		if (pathMgr->SearchForExternalFiles(path)) 
		{
			g_PointCacheInstances.OpenCacheForRead(this,path.GetCStr());
			if( m_fileType == eMC ) {
				if( !InitFbxSdk() )
					return INVALID_HANDLE_VALUE;
				TSTR strPath;
				SplitFilename(TSTR(path.GetCStr()), &strPath, NULL, NULL);
				m_pFbxCache->SetCacheFileName( strPath, fname );
				m_pFbxCache->SetCacheFileFormat( KFbxCache::eMC );
				m_pFbxCache->OpenFileForRead();
				return NULL;
			}
			else {
				hFile = ::CreateFile(	path.GetCStr(),
										GENERIC_READ,
										FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,	// allow file to be overwritten or deleted out from under us!
										0,
										OPEN_EXISTING,
										0,
										NULL);
			}
		}
		else
			hFile = INVALID_HANDLE_VALUE;

		if (hFile == INVALID_HANDLE_VALUE)
		{
			hFile = NULL;
			DEBUGPRINT("\tfail: '%s' - %s\n", fname, GetLastErrorString());
		} else {
			DEBUGPRINT("\tsuccess: %d - '%s'\n", hFile, fname);
			
		}
	} else {
		DEBUGPRINT("\tfail: Invalid fname: '%s'\n", fname);
	}

	return hFile;
}

HANDLE PointCache::OpenCacheForRecord(TCHAR* fname,HWND hWnd)
{
	DEBUGPRINT("PointCache::OpenCacheForRecord: %s\n", fname);

	if(m_fileType==ePTS)
	{
		Interface *iCore = GetCOREInterface();
		if (iCore->IsNetServer() || GetCOREInterface()->GetQuietMode())
		{
				iCore->Log()->LogEntry(	SYSLOG_ERROR,
										DISPLAY_DIALOG,
										_T("Point Cache"),
										_T("Can't record .pts files.  Need to save as .pc2"));
			return NULL;
		} 
		else
		{
			TSTR error = GetString(IDS_PTS_TO_PC2);
			TSTR errorTitle = GetString(IDS_CLASS_NAME);
			MessageBox(iCore->GetMAXHWnd(),error,errorTitle,MB_OK|MB_ICONERROR);
			SaveFilePrompt(hWnd);
			m_pblock->GetValue(pc_cache_file,0,fname,FOREVER);

		}
	}

	//first we try to create given the passed in file name.. if that doesn't work we then
	//try to find it using searching for an external path..
	HANDLE hFile = INVALID_HANDLE_VALUE;
	MaxSDK::Util::Path path(fname);
	path.ConvertToAbsolute();

	g_PointCacheInstances.OpenCacheForRecord(this,path.GetCStr());

	if( m_fileType == ePC2 ) {
		hFile = ::CreateFile(	path.GetCStr(),
									GENERIC_WRITE,
									0,						// don't share it
									0,
									CREATE_ALWAYS,			// overwrite existing files
									FILE_ATTRIBUTE_ARCHIVE,
									NULL);
	}
	else if( m_fileType == eMC ) {
		if( OpenCacheForRecordMC(fname) )
			return NULL;
	}

	if (hFile == INVALID_HANDLE_VALUE)
	{
		//okay now we search along the 
		DEBUGPRINT("\tfail: but now searching along external path'%s' - %s\n",path.GetCStr(), GetLastErrorString());
		MaxSDK::Util::Path path(fname);		
		IPathConfigMgr* pathMgr = IPathConfigMgr::GetPathConfigMgr();

		if (pathMgr->SearchForExternalFiles(path)) //just use the orignal then
		{
			g_PointCacheInstances.OpenCacheForRecord(this,path.GetCStr());
			if( m_fileType == ePC2 ) {
				hFile = ::CreateFile(	path.GetCStr(),
											GENERIC_WRITE,
											0,						// don't share it
											0,
											CREATE_ALWAYS,			// overwrite existing files
											FILE_ATTRIBUTE_ARCHIVE,
											NULL);
			}
			else if( m_fileType == eMC ) {
				if( OpenCacheForRecordMC(path.GetCStr()) )
					return NULL;
			}

			if (hFile != INVALID_HANDLE_VALUE)
				return hFile;
			//else just print fail and exit
		}
		DEBUGPRINT("\tfail: '%s' - %s\n",path.GetCStr(), GetLastErrorString());
	} else {
		DEBUGPRINT("\tsuccess: %d - '%s'\n", hFile, fname);
	}

	return hFile;
}

bool PointCache::OpenCacheForRecordMC(const TCHAR* fname)
{
	if( !InitFbxSdk() )
		return false;

	TSTR strPath;
	SplitFilename(TSTR(fname), &strPath, NULL, NULL);
	m_pFbxCache->SetCacheFileName( strPath, fname );
	m_pFbxCache->SetCacheFileFormat( KFbxCache::eMC );

	// Get channel name
	TSTR strChannel;
	ModContextList mcList;
	INodeTab nodes;
	GetCOREInterface()->GetModContexts(mcList,nodes);
	if( nodes.Count() != 0 ) 
		strChannel = nodes[0]->GetName();
	else
		strChannel = GetString(IDS_CLASS_NAME);

	int iFileCount;
	m_pblock->GetValue(pc_file_count,0,iFileCount,FOREVER);
	if( iFileCount == eOneFile )
		m_fileCount = eOneFile;
	else
		m_fileCount = eOneFilePerFrame;
	bool bRes = m_pFbxCache->OpenFileForWrite( 
								(m_fileCount==eOneFile)? KFbxCache::eMC_ONE_FILE : KFbxCache::eMC_ONE_FILE_PER_FRAME,
								GetFrameRate(), strChannel.data() );

	return bRes;
}

bool PointCache::CloseFile(HANDLE hFile)
{
	if( m_fileType == eMC ) {
		if( m_pFbxCache ) {
			m_pFbxCache->CloseFile();
			m_pFbxCache->Destroy();
			m_pFbxCache = NULL;
		}
		return true;
	}

	DEBUGPRINT("PointCache::CloseFile: %d\n", hFile);
	if(hFile)
	{
		if (!CloseHandle(hFile))
		{
			DEBUGPRINT("\tfail: %d - %s\n", hFile, GetLastErrorString());
			return false;
		} else {
			DEBUGPRINT("\tsuccess: %d\n", hFile);
		}

		hFile = NULL;
	}
	return true;
}

bool PointCache::CloseCacheFile(bool notifyFileManager)
{
	FreeBuffers();

	CloseFile(m_hCacheFile);
	m_hCacheFile = NULL;
	if(notifyFileManager==true)
	{
		g_PointCacheInstances.DeletePointCache(this);
	}
	//also handle old point caches
	if(m_PTSPointCache)
	{
		delete m_PTSPointCache;
		m_PTSPointCache = NULL;
	}
	return true;
}

// Checks if t is in in the range defined by [a, b] (inclusive).
static bool InTimeRange(TimeValue a, TimeValue b, TimeValue t)
{
	return (a <= b) ? ( (t >= a) && (t <= b) ) : ( (t >= b) && (t <= a) );
}

//TODO: make sure this returns true/false properly
bool PointCache::RecordToFile(HWND hWnd,bool &canceled)
{
	canceled = false; //assume we aren't.
	DEBUGPRINT("PointCache::RecordToFile\n");

	ClearError(ERROR_POINTCOUNTCHANGED);
	ClearError(ERROR_NONODEFOUND);

	TCHAR* fname = NULL;
	m_pblock->GetValue(pc_cache_file,0,fname,FOREVER);
	if (!IsValidFName(fname))
	{
		if(SaveFilePrompt(hWnd)==false)
		{
			canceled = true;
			return false;
		}
		m_pblock->GetValue(pc_cache_file,0,fname,FOREVER);
	}

	if (IsValidFName(fname))
	{
		CloseCacheFile(true);

		// create the cacheFile
		float endFrame;
		m_pblock->GetValue(pc_record_start,0,m_cacheStartFrame,FOREVER);
		m_pblock->GetValue(pc_record_end,0,endFrame,FOREVER);
		m_pblock->GetValue(pc_sample_rate,0,m_cacheSampleRate,FOREVER);

		int tpf = GetTicksPerFrame();
		// Setup variables used to control the record loop.
		TimeValue recordBegin = TimeValue(m_cacheStartFrame * tpf);
		TimeValue recordEnd = TimeValue(endFrame * tpf);
		TimeValue recordInc = TimeValue(m_cacheSampleRate * tpf);
		if (recordInc == 0)
			recordInc = 1;
		if (recordBegin > recordEnd) recordInc = -recordInc;	// we're going backwards through time, so loop counter must be flipped
		if (recordBegin == recordEnd) recordEnd += recordInc;	// make sure we're recording at least two frames

		// Update m_numPoints and
		// Update objectstate to current time + 1. (If we didn't do this, and current time == start time, then the first frame wouldn't
		// be recorded because ModifyObject wouldn't get called due to the objectstate not being invalid).
		sMyEnumProc dep;
		DoEnumDependents(&dep);
		if (!dep.Nodes[0])
		{
			SetError(ERROR_NONODEFOUND);
			return false;
		}

		m_tempDisable = true;
		dep.Nodes[0]->EvalWorldState(GetCOREInterface()->GetTime() + 1);
		m_tempDisable = false;

		m_cacheNumPoints = m_numPoints;

		// Count the number of samples to take
		TimeValue sampleTime;
		m_cacheNumSamples = 0;
		for (sampleTime = recordBegin; InTimeRange(recordBegin, recordEnd, sampleTime); sampleTime += recordInc)
		{
			++m_cacheNumSamples;
		}
		TimeValue numFrames = 	endFrame- m_cacheStartFrame ;
		m_realCacheSampleRate = (float)numFrames/(float)(m_cacheNumSamples);

		m_hCacheFile = OpenCacheForRecord(fname,hWnd);

		if (!IsCacheFileOpen()) 
		{
			m_hCacheFile = NULL; //make sure we close it
			return false;
		}

		if( m_fileType == ePC2 )
			m_recordBuffer = new Point3[m_cacheNumPoints];
		else
			m_mcBufferIO.AllocWriteBuffer(m_cacheNumPoints*3);

		if (!m_recordBuffer && m_mcBufferIO.IsWriteBufferNULL())
		{
			CloseCacheFile(true);
			return false;
		}

		DWORD numWrite;
		if( m_fileType == ePC2 ) {
			//TODO: breaks if header changes
			WriteFile(m_hCacheFile, &m_signature, sizeof(m_signature), &numWrite, NULL);
			WriteFile(m_hCacheFile, &m_fileVersion, sizeof(m_fileVersion), &numWrite, NULL);
			WriteFile(m_hCacheFile, &m_cacheNumPoints, sizeof(m_cacheNumPoints), &numWrite, NULL);
			WriteFile(m_hCacheFile, &m_cacheStartFrame, sizeof(m_cacheStartFrame), &numWrite, NULL);
			WriteFile(m_hCacheFile, &m_cacheSampleRate, sizeof(m_cacheSampleRate), &numWrite, NULL);
			WriteFile(m_hCacheFile, &m_cacheNumSamples, sizeof(m_cacheNumSamples), &numWrite, NULL);
		}


		TSTR name;
		m_recordMode = true;
		bool abort = FALSE;
		int numRecorded = 0;

		for (sampleTime = recordBegin; InTimeRange(recordBegin, recordEnd, sampleTime) && !abort; sampleTime += recordInc)
		{
			m_recordTime = sampleTime;
			dep.Nodes[0]->EvalWorldState(m_recordTime);

			if (m_numPoints == m_cacheNumPoints)
			{
				numRecorded += 1;

				name.printf(GetString(IDS_RECORDING_TEXT),numRecorded,m_cacheNumSamples);
				UpdateCacheInfoStatus(hWnd, name.data());

				SHORT iret = GetAsyncKeyState(VK_ESCAPE);
				if (iret == -32767) abort = true;
			}
			else
			{
				abort = true;
				SetError(ERROR_POINTCOUNTCHANGED);
			}
		}

		if (abort)
		{
			if( m_fileType == ePC2 ) {
				// write the correct number of samples back to the file if they cancel
				// TODO: breaks if header changes
				int offset = sizeof(m_signature) + (sizeof(int)*2) + (sizeof(float)*2);
				SetFilePointer(m_hCacheFile, offset, NULL, FILE_BEGIN);
				WriteFile(m_hCacheFile, &numRecorded, sizeof(numRecorded), &numWrite, NULL);
			}
		}

		name.printf(" ");
		UpdateCacheInfoStatus(hWnd, name.data());

		CloseCacheFile(true);
		DELETE_BUFF(m_recordBuffer);
		m_mcBufferIO.FreeReadBuffer();

		m_recordMode = false;
		m_doLoad = true;

		NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);

		return true; //was returning false if we abort but we still get some frames so we are good!
	} else {
		return false;
	}
}




class PC2HistoryList : public bmmHistoryList
{
public:

	void Init();
};

TCHAR PointCache::m_InitialDir[MAX_PATH];


void PC2HistoryList::Init()
{
	_tcscpy(title,GetString(IDS_CLASS_NAME));
	initDir[0] = 0;
	bmmHistoryList::LoadDefaults();
	
	if (bmmHistoryList::DefaultPath()[0]) 
		_tcscpy(PointCache::m_InitialDir, bmmHistoryList::DefaultPath());
	else _tcscpy(PointCache::m_InitialDir,GetCOREInterface()->GetDir(APP_EXPORT_DIR));

};


class CustomData {
public:
	PC2HistoryList history;
	BOOL save;
	CustomData(BOOL s) {
		save = s; //if save we show the plus button
		history.Init();
		history.LoadDefaults();
	};
};



INT_PTR WINAPI PC2HookProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static CustomData* custom = 0;
	static int offset1 = 14;
	static int offset2 = 24;
	static int offset3 = 28;
	if (message == WM_INITDIALOG)
	{
		custom = (CustomData*)((OPENFILENAME *)lParam)->lCustData;
		if (custom) {
			custom->history.SetDlgInfo(GetParent(hDlg),hDlg,IDC_HISTORY);
			custom->history.LoadList();
		}

		WINDOWPLACEMENT wp;
		wp.length = sizeof(WINDOWPLACEMENT);
		HWND hParent = GetParent(hDlg);
		if ( GetDlgItem(hDlg, IDC_PLUS_BUTTON) ) {
			// push Save and Cancel buttons right, position plus button
			GetWindowPlacement(GetDlgItem(hParent,IDOK),&wp);
			wp.rcNormalPosition.left += offset1;
			wp.rcNormalPosition.right += offset1;
			SetWindowPlacement(GetDlgItem(hParent, IDOK), &wp);

			wp.rcNormalPosition.left -= offset3;
			wp.rcNormalPosition.right = wp.rcNormalPosition.left + offset2;
			if(!(custom && custom->save==TRUE))
				wp.showCmd = SW_HIDE ;
			SetWindowPlacement(GetDlgItem(hDlg, IDC_PLUS_BUTTON), &wp);

			GetWindowPlacement(GetDlgItem(hParent,IDCANCEL),&wp);
			wp.rcNormalPosition.left += offset1;
			wp.rcNormalPosition.right += offset1;
			SetWindowPlacement(GetDlgItem(hParent, IDCANCEL), &wp);
		}
	} else if (message == WM_COMMAND) {
		if (LOWORD(wParam) == IDC_HISTORY) {
			if (HIWORD(wParam) == CBN_SELCHANGE && custom)
				custom->history.SetPath();
		} else if (LOWORD(wParam) == IDC_PLUS_BUTTON) {
			TCHAR buf[1024],tmpbuf[32], ext[5];
			bool haveExt = false;
			HWND hParent = GetParent(hDlg);
			// russom - 04/09/02 defect 417924
			if( CommDlg_OpenSave_GetSpec(hParent, buf, 1024) < 1 )
				buf[0] = _T('\0');
			int len = static_cast<int>(_tcslen(buf));	// SR DCAST64: Downcast to 2G limit.
			// don't know extension, assume at most 3 letter extension (.dat)
			for (int i = 4; i >= 0; i--) {
				if(len > i && buf[len-i] == _T('.')) {
					_tcscpy(ext, &buf[len-i]);
					buf[len-i] = _T('\0');
					haveExt = true;
					break;
				}
			}
			len = static_cast<int>(_tcslen(buf));	// SR DCAST64: Downcast to 2G limit.
			if(len > 2 && _istdigit(buf[len-1]) && _istdigit(buf[len-2])) {
				int num = _ttoi(buf+len-2);
				buf[len-2] = _T('\0');
				_stprintf(tmpbuf, _T("%02d"), num+1);
				_tcscat(buf, tmpbuf);
			}
			else
				_tcscat(buf, _T("01"));
			if (haveExt)
				_tcscat(buf, ext);
			if(len)
			{
				int file_name_index = edt1;
				if (_WIN32_WINNT >= 0x0500)
				{
					// check for combo box and places bar. If both present, use the combo box
					// instead of the edittext.
					if (GetDlgItem(hDlg, cmb13) && GetDlgItem(hDlg, ctl1))
						file_name_index = cmb13;
				}
				CommDlg_OpenSave_SetControlText(hParent, file_name_index, buf);
			}
			PostMessage(hParent, WM_COMMAND, IDOK, 0);
			return TRUE;
		}
	}
	else if(message==WM_NOTIFY)
		 {
		     OFNOTIFY *ofnot = (OFNOTIFY *) lParam;
			 NMHDR *nmh = &(ofnot->hdr);

			 if (nmh->code == CDN_INITDONE && custom)
				 custom->history.NewPath(ofnot->lpOFN->lpstrInitialDir);

			 if (nmh->code == CDN_FILEOK && custom)
			 {
				 TCHAR buf[MAX_PATH];
				 if( CommDlg_OpenSave_GetFolderPath(GetParent(hDlg),buf,MAX_PATH) < 1 )
					 buf[0] = _T('\0');
				 custom->history.NewPath(buf);
				 custom->history.SaveList();
			 }
	}
	
	else if (message == WM_DESTROY)
		custom = NULL;
	else if (message == WM_SIZE) {
		// Reposition plus button and history combo
		HWND hParent = GetParent(hDlg);
		if ( hParent ) {
			WINDOWPLACEMENT wp;
			wp.length = sizeof(WINDOWPLACEMENT);
			if ( GetDlgItem(hDlg, IDC_PLUS_BUTTON) ) {
				GetWindowPlacement(GetDlgItem(hParent,IDOK),&wp);
				wp.rcNormalPosition.left -= offset3;
				wp.rcNormalPosition.right = wp.rcNormalPosition.left + offset2;
				if(!(custom && custom->save==TRUE))
					wp.showCmd = SW_HIDE ;
				SetWindowPlacement(GetDlgItem(hDlg, IDC_PLUS_BUTTON), &wp);
			}

			GetWindowPlacement(GetDlgItem(GetParent(hDlg),IDCANCEL),&wp);
			int right = wp.rcNormalPosition.right;
			GetWindowPlacement(GetDlgItem(hDlg,IDC_HISTORY),&wp);
			wp.rcNormalPosition.right = right;
			SetWindowPlacement(GetDlgItem(hDlg,IDC_HISTORY),&wp);
		}
	}
	return FALSE;
}


bool PointCache::SaveFilePrompt(HWND hWnd)
{
	Interface* iCore = GetCOREInterface();
	if (!iCore) return true;

	OPENFILENAME ofn = {sizeof(OPENFILENAME)};
	ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400; // has OFN_ENABLEHOOK

	FilterList fl;

	// Maya Cache
	fl.Append(GetString(IDS_MAYA_CACHE));
	fl.Append(GetString(IDS_MAYA_CACHE_WILDCARD));

	// PC2
	fl.Append(GetString(IDS_PW_CACHEFILES));
	fl.Append(GetString(IDS_CACHE_EXT_WILDCARD));

	TSTR title = GetString(IDS_PW_SAVEOBJECT);

	TCHAR fname[MAX_PATH] = {'\0'};
	ModContextList mcList;
	INodeTab nodes;
	iCore->GetModContexts(mcList,nodes);
	if (nodes.Count() != 0)
	{
		TSTR str(nodes[0]->GetName());

		for (int ii=0; ii<str.Length(); ++ii)
		{
			// TODO: implement these restrictions (looser than the ones below):
			// - do not use a path separator, a character in the range 0 through 31, or
			//   any character explicitly disallowed by the file system. A name can contain
			//   characters in the extended character set (128-255).
			// - disallowed characters: < > : " / \ |
			// - The following reserved words cannot be used as the name of a file:
			//   CON, PRN, AUX, CLOCK$, NUL, COM1, COM2, COM3, COM4, COM5, COM6, COM7, COM8, COM9,
			//   LPT1, LPT2, LPT3, LPT4, LPT5, LPT6, LPT7, LPT8, and LPT9. Also, reserved words
			//   followed by an extension - for example, NUL.tx7 - are invalid file names.
			// - Process a path as a null-terminated string. The maximum length for a path, including a
			//   trailing backslash, is given by MAX_PATH.
			char c = str[ii];
			if ((c<'0' || c>'9') &&
				(c<'A' || c>'Z') &&
				(c<'a' || c>'z')
			) str[ii] = '_';
		}

		str += GetString(IDS_MAYA_CACHE_EXT);

		strncpy(fname, str.data(), str.length());
	}

	ofn.hwndOwner       = hWnd;
	ofn.lpstrFilter     = fl;
	ofn.lpstrFile       = fname;
	ofn.nMaxFile        = MAX_PATH;
	ofn.lpstrInitialDir = PointCache::m_InitialDir;
	ofn.Flags           = OFN_EXPLORER|OFN_HIDEREADONLY|OFN_FILEMUSTEXIST|OFN_ENABLETEMPLATE |OFN_ENABLEHOOK|OFN_PATHMUSTEXIST;
	ofn.FlagsEx         = OFN_EX_NOPLACESBAR;
	ofn.Flags |= OFN_ENABLESIZING;


	CustomData custom(TRUE);
	ofn.lCustData       = LPARAM(&custom);
	ofn.lpstrDefExt     = GetString(IDS_CACHE_EXT);
	ofn.lpTemplateName = MAKEINTRESOURCE(IDD_FILE_DIALOG);
	ofn.lpfnHook = (LPOFNHOOKPROC)PC2HookProc;
	ofn.hInstance = hInstance;

	ofn.lpstrTitle      = title;

	tryAgain:
	if (GetSaveFileName(&ofn))
	{
		if (DoesFileExist(fname))
		{
			TSTR buf1;
			TSTR buf2 = GetString(IDS_PW_SAVEOBJECT);
			buf1.printf(GetString(IDS_PW_FILEEXISTS),fname);
			if (IDYES!=MessageBox(hWnd,buf1,buf2,MB_YESNO|MB_ICONQUESTION))
				goto tryAgain;

			// Delete original file to avoid loading header
			CloseCacheFile(true);
			remove(fname);
		}
		//save stuff here
		MaxSDK::Util::Path path(fname);
		IPathConfigMgr::GetPathConfigMgr()->NormalizePathAccordingToSettings(path);
		m_pblock->SetValue(pc_cache_file,0,const_cast<TCHAR *>(path.GetCStr()));
		SetFileType(const_cast<TCHAR *>(path.GetCStr()));
		if( m_fileType == ePTS )
			m_fileType = ePC2;
		UpdateUI();
	}
	else 
		return false;  //if we cancel return false
	return true;
}


void PointCache::SetFileType(const TCHAR *filename)
{
	// Default to PC2
	m_fileType = ePC2;

	if( (filename != NULL) && (_tcslen(filename)>1) ) {
		// Extract extension
		TSTR ext;
		SplitFilename(TSTR(filename), NULL, NULL, &ext);

		if( ext.Length() > 0 ) {
			// Check for old pts or maya cache
			if( _tcsicmp( ext.data(), GetString(IDS_PTS_EXT)) == 0 )
				m_fileType = ePTS;
			else if( _tcsicmp( ext.data(), GetString(IDS_MAYA_CACHE_EXT)) == 0 )
				m_fileType = eMC;
		}
	}

	// Special cases for old point cache object
	if( m_fileType == ePTS ) {
		if(m_PTSPointCache==NULL)
			m_PTSPointCache = new OLDPointCacheBase();
	}
	else if( m_PTSPointCache ) {
		delete m_PTSPointCache;
		m_PTSPointCache = NULL;
	}

	// Set file type specific defaults
	if( m_fileType == ePTS  ) {
		m_pblock->SetValue(pc_file_count, 0, PointCache::eOneFile);
		m_pblock->SetValue(pc_playback_type, 0, playback_original);
		m_pblock->SetValue(pc_load_type, 0, load_stream);
	}
	else if( m_fileType == ePC2 ) {
		m_pblock->SetValue(pc_file_count, 0, PointCache::eOneFile);		
	}
	else if( m_fileType == eMC ) {
		m_pblock->SetValue(pc_load_type, 0, load_stream);
	}

}


void PointCache::OpenFilePrompt(HWND hWnd)
{
	CloseCacheFile(true);

	// set initial file/dir
	TCHAR fname[MAX_PATH] = {'\0'};
	TCHAR* str = m_pblock->GetStr(pc_cache_file);

	if(str&& _tcslen(str)>0)
	{
		MaxSDK::Util::Path cachePath(str);
		IPathConfigMgr* pathMgr = IPathConfigMgr::GetPathConfigMgr();
		if (pathMgr->SearchForExternalFiles(cachePath)) 
		{
			TSTR what = cachePath.GetString();
			str = what.data();
			if (IsValidFName(str))
				strcpy(fname, str);
		}
	}

	OPENFILENAME ofn = {sizeof(OPENFILENAME)};
	ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400; // has OFN_ENABLEHOOK

	FilterList fl;

	// All Cache Files
	fl.Append(GetString(IDS_ALL_CACHE_FILES));
	fl.Append(GetString(IDS_ALL_CACHE_FILES_WILDCARD));

	// Maya Cache
	fl.Append(GetString(IDS_MAYA_CACHE));
	fl.Append(GetString(IDS_MAYA_CACHE_WILDCARD));

	// PC2
	fl.Append(GetString(IDS_PW_CACHEFILES));
	fl.Append(GetString(IDS_CACHE_EXT_WILDCARD));

	// PC1
	fl.Append( GetString(IDS_OLD_CACHE_FILES));
	fl.Append( GetString(IDS_OLD_CACHE_WILDCARD));

	// All Files
	fl.Append( GetString(IDS_ALL_FILES));
	fl.Append( GetString(IDS_ALL_FILES_WILDCARD));

	TSTR title = GetString(IDS_PW_LOADOBJECT);

	ofn.hwndOwner       = hWnd;
 	ofn.lpstrFilter     = fl;
	ofn.lpstrFile       = fname;
	ofn.nMaxFile        = MAX_PATH;
	ofn.lpstrInitialDir = PointCache::m_InitialDir;
	ofn.Flags           = OFN_HIDEREADONLY| OFN_ENABLETEMPLATE |OFN_PATHMUSTEXIST |OFN_ENABLEHOOK|OFN_EXPLORER;
	ofn.FlagsEx         = OFN_EX_NOPLACESBAR;
	ofn.lpTemplateName = MAKEINTRESOURCE(IDD_FILE_DIALOG);
	ofn.lpfnHook = (LPOFNHOOKPROC)PC2HookProc;
	ofn.hInstance = hInstance;

	ofn.Flags |= OFN_ENABLESIZING;

	CustomData custom(FALSE);
	ofn.lCustData       = LPARAM(&custom);

	ofn.lpstrDefExt     = GetString(IDS_CACHE_EXT);
	ofn.lpstrTitle      = title;


	if (GetOpenFileName(&ofn))
	{
		SetFileType(fname);

		if( IsMultipleChannelCache(fname) ) {
			TSTR strTitle = GetString(IDS_CLASS_NAME);
			if( GetCOREInterface()->IsNetServer() || GetCOREInterface()->GetQuietMode() ) 	{
				GetCOREInterface()->Log()->LogEntry(SYSLOG_ERROR,
													DISPLAY_DIALOG,
													strTitle,
													GetString(IDS_MULTI_CHANNEL_WARNING));
			} 
			else {
				MessageBox(GetCOREInterface()->GetMAXHWnd(), 
							GetString(IDS_MULTI_CHANNEL_WARNING), 
							strTitle.data(), MB_OK|MB_ICONWARNING);
			}
		}

		MaxSDK::Util::Path path(fname);
		IPathConfigMgr::GetPathConfigMgr()->NormalizePathAccordingToSettings(path);
		TSTR what = path.GetString();
		m_pblock->SetValue(pc_cache_file,0,what.data());
		NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);

		ClearError(ERROR_CACHEMISSING);
	}
	else
	{
		DWORD val = CommDlgExtendedError();
		if(val==3)
		{
			int t=2;
		}
	}

	UpdateUI();
}

void PointCache::Unload()
{
	CloseCacheFile(true);
	m_doLoad = false;
	NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
	UpdateUI();
}

void PointCache::Reload()
{
	DEBUGPRINT("PointCache::Reload\n");

	CloseCacheFile(true);
	m_doLoad = true;
	NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
	UpdateUI();
}

bool PointCache::DeleteCacheFile(HWND hWnd)
{
	TCHAR* fname = NULL;
	m_pblock->GetValue(pc_cache_file,0,fname,FOREVER);
	Interface* iCore = GetCOREInterface();
	if (iCore)
	{
		HWND maxWnd = iCore->GetMAXHWnd();
		if (IsValidFName(fname))
		{
			TSTR buf1;
			TSTR buf2 = GetString(IDS_DELETE_TITLE);
			buf1.printf(GetString(IDS_DELETE_PROMPT),fname);

			if (IDYES==MessageBox(maxWnd,buf1,buf2,MB_YESNO|MB_ICONEXCLAMATION))
			{
				CloseCacheFile(true);
				return (remove(fname) == 0) ? TRUE : FALSE;
			}
		}
	}
	return FALSE;
}

void PointCache::EnableModsBelow(BOOL enable)
{
	int				i;
	SClass_ID		sc;
	IDerivedObject* dobj = NULL;

// return the indexed modifier in the mod chain
	INode* node = NULL;
	INodeTab nodes;
	if (!m_iCore)
		{
		sMyEnumProc dep;              
		DoEnumDependents(&dep);
		node = dep.Nodes[0];
		}
	else
		{
		ModContextList mcList;
		m_iCore->GetModContexts(mcList,nodes);
		assert(nodes.Count());
		node = nodes[0];
		}
	BOOL found = TRUE;

// then osm stack
	Object* obj = node->GetObjectRef();
	int ct = 0;

	if ((dobj = node->GetWSMDerivedObject()) != NULL)
		{
		for (i = 0; i < dobj->NumModifiers(); i++)
			{
			Modifier *m = dobj->GetModifier(i);
			BOOL en = m->IsEnabled();
			BOOL env = m->IsEnabledInViews();
			if (!enable)
				{
				if (!found)
					{
					m->DisableMod();
					}
				}
			else 
				{
				if (!found)
					{
					m->EnableMod();
					}
				}
			if (this == dobj->GetModifier(i))
				found = FALSE;

			}
		}

	if ((sc = obj->SuperClassID()) == GEN_DERIVOB_CLASS_ID)
		{
		dobj = (IDerivedObject*)obj;

		while (sc == GEN_DERIVOB_CLASS_ID)
			{
			for (i = 0; i < dobj->NumModifiers(); i++)
				{
				TSTR name;
				Modifier *m = dobj->GetModifier(i);
				m->GetClassName(name);


				BOOL en = m->IsEnabled();
				BOOL env = m->IsEnabledInViews();
				if (!enable)
					{
					if (!found)
						{
						m->DisableMod();
						}
					}
				else 
					{
					if (!found)
						{
						m->EnableMod();
						}
					}
				if (this == dobj->GetModifier(i))
					found = FALSE;


				}
			dobj = (IDerivedObject*)dobj->GetObjRef();
			sc = dobj->SuperClassID();
			}
		}

nodes.DisposeTemporary();

GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
NotifyDependents(FOREVER,PART_ALL,REFMSG_NUM_SUBOBJECTTYPES_CHANGED);
}

void PointCache::NotifyDelete(void* param, NotifyInfo* arg)
{
	PointCache* pc = (PointCache*)param;

	if (pc)
	{
		pc->CloseCacheFile(true);
		pc->m_doLoad = true;
	}
}

float PointCache::GetSampleIndex(TimeValue t)
{
	int playbackType;
	m_pblock->GetValue(pc_playback_type,0,playbackType,FOREVER);
	switch (playbackType)
	{
		case playback_original:
		{
			float startFrame = m_cacheStartFrame;
			float sampleminus = (float) m_cacheNumSamples-1;
			float endFrame = (float)startFrame + ((sampleminus) / (1.0f/m_realCacheSampleRate));
			float curFrame = (float)t / (float)GetTicksPerFrame();
			if (curFrame < startFrame) curFrame = startFrame;
			if (curFrame > endFrame) curFrame = endFrame;
			float normFrame = (float)(curFrame - startFrame) / (float)(endFrame - startFrame);
			return (normFrame * (sampleminus));
		}
		case playback_start:
		{
			float startFrame = m_pblock->GetFloat(pc_playback_start, 0);

			float endFrame = startFrame + ((m_cacheNumSamples-1) / (1.0f/m_realCacheSampleRate));
			float curFrame = (float)t / (float)GetTicksPerFrame();
			if (curFrame < startFrame) curFrame = startFrame;
			if (curFrame > endFrame) curFrame = endFrame;
			float normFrame = (curFrame - startFrame) / (endFrame - startFrame);
			return (normFrame * (m_cacheNumSamples-1));
		}
		case playback_range:
		{
			float sFrame, eFrame;
			m_pblock->GetValue(pc_playback_start,0,sFrame,FOREVER);
			m_pblock->GetValue(pc_playback_end,0,eFrame,FOREVER);

			if (sFrame == eFrame)
				return 0.0f;

			float cFrame = (float)t / (float)GetTicksPerFrame();

			float lo = (sFrame < eFrame) ? sFrame : eFrame;
			float hi = (eFrame > sFrame) ? eFrame : sFrame;
			if (cFrame < lo)
				cFrame = lo;
			else if (cFrame > hi)
				cFrame = hi;

			float normFrame = (cFrame - sFrame) / (eFrame - sFrame);

			float idx = normFrame * (m_cacheNumSamples-1);

			return idx;
		}
		case playback_graph:
		{
			float fCurFrame = m_pblock->GetFloat(pc_playback_frame, t);

			float fIdx = 0.0f;
			if (m_pblock->GetInt(pc_clamp_graph, 0))
			{
				fIdx = fCurFrame * (1.0f / m_realCacheSampleRate);

				if (fIdx > (m_cacheNumSamples-1))
					fIdx = float(m_cacheNumSamples-1);
				else if (fIdx < 0.0f)
					fIdx = 0.0f;
			} else {
				fIdx = fCurFrame * (1.0f / m_realCacheSampleRate);
			}

			return fIdx;
		}
	}

	return 0.0f;
}

bool PointCache::ReadSampleMC(TimeValue t, Point3* sampleBuffer)
{
	bool bRes = false;
	if( IsCacheFileOpen() ) {
		bRes = m_mcBufferIO.Read(m_pFbxCache, m_cacheChannel, t, m_cacheNumPoints );
		if( bRes )
			bRes = m_mcBufferIO.ConvertReadBuffer( sampleBuffer, m_cacheNumPoints );

		if( !bRes ) {
			memset((void*)sampleBuffer, 0, sizeof(Point3)*m_cacheNumPoints);
			return false;
		}
	}

	return bRes;
}

int PointCache::ComputeOffset(int index)
{
	//TODO: breaks if header changes
	int offset = sizeof(m_cacheSignature) + (sizeof(int)*2) + (sizeof(float)*2) + (sizeof(int));
	offset += sizeof(Point3) * m_cacheNumPoints * index;
	return offset;
}

bool PointCache::ReadSample(int sampleIndex, Point3* sampleBuffer)
{
	if (m_hCacheFile && m_hCacheFile != INVALID_HANDLE_VALUE)
	{
		int offset = ComputeOffset(sampleIndex);
		SetFilePointer(m_hCacheFile, offset, NULL, FILE_BEGIN);
		DWORD numRead = 0;
		DWORD res = ReadFile(m_hCacheFile, sampleBuffer, sizeof(Point3)*m_cacheNumPoints, &numRead, NULL);
		if (res && numRead != 0)
		{
			return true;
		} else {
			memset((void*)sampleBuffer, 0, sizeof(Point3)*m_cacheNumPoints);
			return false;
		}
	}

	return false;
}

void PointCache::FreeBuffers()
{
	if (m_preloadBuffer)
	{
		GetPointCacheManager().UnLoadPointCache(this);
	} else {
		DELETE_BUFF(m_firstSample)
		DELETE_BUFF(m_sampleA)
		DELETE_BUFF(m_sampleB)
		DELETE_BUFF(m_samplea)
		DELETE_BUFF(m_sampleb)
		m_mcBufferIO.FreeAllBuffers();
	}
}





PointCacheInstances::~PointCacheInstances()
{
	Delete();
}

void PointCacheInstances::Delete()
{
	for(int i=0;i<m_fileAndCaches.Count();++i)
	{
		if(m_fileAndCaches[i])
			delete m_fileAndCaches[i];
	}
	m_fileAndCaches.ZeroCount();
}

void PointCacheInstances::DeletePointCache(PointCache *pC)  //happens when cache closes too!
{
	FileAndCaches* fC = NULL;
	for(int i=0;i<m_fileAndCaches.Count();++i)
	{
		fC= m_fileAndCaches[i];
		if(fC)
		{
			for(int j=0;j<fC->m_PCs.Count();++j)
			{
				if(fC->m_PCs[j]==pC)
				{
					fC->m_PCs.Delete(j,1);
					if(fC->m_PCs.Count()==0)
					{
						delete fC;
						m_fileAndCaches.Delete(i,1);
					}
					return;
				}
			}
		}
	}
}


void PointCacheInstances::OpenCacheForRead(PointCache *pc, const TCHAR *fileName)
{
	FileAndCaches* fC = NULL;
	bool wasAdded = false;
	if(fileName==NULL||_tcslen(fileName)<=0)
		return;
	for(int i=0;i<m_fileAndCaches.Count();++i)
	{
		fC= m_fileAndCaches[i];
		if(fC)
		{
			if(_tcsicmp(fileName,fC->m_fileName)==0)
			{

				for(int j=0;j<fC->m_PCs.Count();++j)
				{
					if(fC->m_PCs[j]==pc)
					{
						return; //same file name.. no need to do anything... and we know it's only one!
					}
				}
				//so doesn't exist here.. we need to add it.. here.. note we don't return since it may still exists elsewhere
				//so we'll need to delete it if it does.
				fC->m_PCs.Append(1,&pc);
				wasAdded = true;
			}
			else
			{
				for(int j=0;j<fC->m_PCs.Count();++j)
				{
					if(fC->m_PCs[j]==pc)
					{
						//it's here but doesn't exist delete it
						fC->m_PCs.Delete(j,1);
						if(fC->m_PCs.Count()==0)
						{
							delete fC;
							m_fileAndCaches.Delete(i,1);
							--i;
							break;
						}
					}
				}
			}
		}
	}
	//okay we got here need to create a new one.
	if(wasAdded==false)
	{
		fC = new FileAndCaches;
		fC->m_PCs.Append(1,&pc);
		_tcscpy(fC->m_fileName,fileName);
		m_fileAndCaches.Append(1,&fC);
	}
}


void PointCacheInstances::OpenCacheForRecord(PointCache *pc, const TCHAR *fileName)
{
	FileAndCaches* fC = NULL;
	for(int i=0;i<m_fileAndCaches.Count();++i)
	{
		fC= m_fileAndCaches[i];
		if(fC)
		{
			if(_tcsicmp(fileName,fC->m_fileName)==0)
			{
				for(int j=0;j<fC->m_PCs.Count();++j)
				{
					PointCache *npc = fC->m_PCs[j];
					if(npc!=pc)
						npc->CloseCacheFile(false);
				}
			}
		}
	}
}


