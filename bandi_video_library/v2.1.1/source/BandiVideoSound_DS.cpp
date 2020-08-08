////////////////////////////////////////////////////////////////////////////////////////////////////
/// 
/// BandiVideoLibrary 2.0
/// 
/// Copyright(C) 2008-2009 BandiSoft.com All rights reserved.
///
/// Visit http://www.bandisoft.com for more information.
/// 
////////////////////////////////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <process.h>
#include <math.h>
#include "BandiVideoSound_DS.h"


#define BVL_DSWND_CLASS_NAME	_T("CBandiVideoSound_DS")


CBandiVideoSound_DS::CBandiVideoSound_DS() : m_ref_count(0)
{
	m_hwnd = NULL;
	m_hinst = NULL;
	m_ds = NULL;
	m_dsb_primary = NULL;
	m_dsb_secondary = NULL;

	m_channels = m_samplerate = 0;	
	m_last_play_pos = m_last_write_pos = 0;	
	m_inbuf_samples = m_total_samples = 0;

	memset(&m_crit_sec, NULL, sizeof(m_crit_sec));
	m_crit_sec_inited = FALSE;
	
	m_hevent = NULL;
	m_hthread = NULL;
	m_thread_id = 0;

	m_playing = FALSE;
	m_callback = NULL;
}


CBandiVideoSound_DS::~CBandiVideoSound_DS()
{
	Close();
}


STDMETHODIMP_(ULONG) CBandiVideoSound_DS::AddRef()
{                             
    LONG lRef = InterlockedIncrement(&m_ref_count);
	ASSERT(lRef >= 0);

    return m_ref_count;
}


STDMETHODIMP_(ULONG) CBandiVideoSound_DS::Release()
{
    LONG lRef = InterlockedDecrement(&m_ref_count);
    ASSERT(lRef >= 0);
   
	if(lRef == 0) 
	{
        delete this;
        return ULONG(0);
    }

	return m_ref_count;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
/// 
/// Open sound device.
/// 
////////////////////////////////////////////////////////////////////////////////////////////////////
HRESULT		CBandiVideoSound_DS::Open(INT channels, INT samplerate, IBandiVideoSoundCB *callback)
{
	Close();

	HRESULT hr = E_FAIL;
	
	m_hinst = (HINSTANCE)GetModuleHandle(NULL);
	if(m_hinst == NULL) goto error;
	
	WNDCLASS wc;
	ZeroMemory(&wc, sizeof(wc));
	wc.lpfnWndProc = DefWindowProc;
	wc.lpszClassName = BVL_DSWND_CLASS_NAME;
	if(RegisterClass(&wc) == 0) goto error;
	
	// Create dummy window
	m_hwnd = CreateWindow(BVL_DSWND_CLASS_NAME, _T(""), 0, 0, 0, 0, 0, NULL, NULL, m_hinst, 0L);
	if(m_hwnd == NULL)
		goto error;
	
	// Create Direct Sound Instance
	hr = DirectSoundCreate8(NULL, &m_ds, NULL);
	if(FAILED(hr)) 
		goto error;

	hr = m_ds->SetCooperativeLevel(m_hwnd, DSSCL_PRIORITY);
	if(FAILED(hr)) 
		goto error;
	
	// Create a primary buffer
	DSBUFFERDESC dsbdesc;
	ZeroMemory(&dsbdesc, sizeof(DSBUFFERDESC));
	dsbdesc.dwSize = sizeof(DSBUFFERDESC);
	dsbdesc.dwFlags = DSBCAPS_PRIMARYBUFFER;
	hr = m_ds->CreateSoundBuffer(&dsbdesc, &m_dsb_primary, NULL);
	if(FAILED(hr)) 
		goto error;

	// Set format of the primary buffer
	WAVEFORMATEX wfx;
	ZeroMemory(&wfx, sizeof(WAVEFORMATEX));
	wfx.wFormatTag		= WAVE_FORMAT_PCM;
	wfx.nChannels = channels;
	wfx.nSamplesPerSec = samplerate;
	wfx.wBitsPerSample	= 16;
	wfx.nBlockAlign		= wfx.wBitsPerSample / 8 * wfx.nChannels;
	wfx.nAvgBytesPerSec	= wfx.nSamplesPerSec * wfx.nBlockAlign;
	hr = m_dsb_primary->SetFormat(&wfx);
	if(FAILED(hr)) 
		goto error;

	// Create secondary sound buffer
	ZeroMemory(&dsbdesc, sizeof(DSBUFFERDESC));
	dsbdesc.dwSize = sizeof(DSBUFFERDESC);
	dsbdesc.dwFlags = DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_GLOBALFOCUS
					| DSBCAPS_LOCDEFER | DSBCAPS_CTRLFREQUENCY | DSBCAPS_CTRLVOLUME;
	dsbdesc.dwBufferBytes = MAX_NUM_SAMPLE * wfx.nBlockAlign;
	dsbdesc.lpwfxFormat = &wfx;
	hr = m_ds->CreateSoundBuffer(&dsbdesc, &m_dsb_secondary, NULL);
	if(FAILED(hr)) 
		goto error;
	
	InitializeCriticalSection(&m_crit_sec);
	m_crit_sec_inited = TRUE;
	
	// Create an event object for periodical processing
	m_hevent = CreateEvent(NULL, FALSE, FALSE, NULL);	// auto reset event
	if(m_hevent == NULL)
		goto error;

	// Create thread for playback
	m_hthread = (HANDLE)_beginthreadex(NULL, 0, InitialThreadProc, this, CREATE_SUSPENDED, &m_thread_id);
	if(m_hthread == NULL)
		goto error;

	m_channels = channels;
	m_samplerate = samplerate;

	m_callback = callback;

	SetThreadPriority(m_hthread, THREAD_PRIORITY_TIME_CRITICAL);
	ResumeThread(m_hthread);

	return S_OK;

error:
	Close();

	return FAILED(hr) ? hr : E_FAIL;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// 
/// Close sound device.
/// 
////////////////////////////////////////////////////////////////////////////////////////////////////
void		CBandiVideoSound_DS::Close()
{
    HANDLE hThread = (HANDLE)InterlockedExchangePointer(&m_hthread, 0);
    if(hThread) 
	{
		if(m_hevent) SetEvent(m_hevent);

		WaitForSingleObject(hThread, INFINITE);
		CloseHandle(hThread);
	}
	
	if(m_hevent)
	{
		CloseHandle(m_hevent);
		m_hevent = NULL;
	}
	
	if(m_crit_sec_inited)
	{
		DeleteCriticalSection(&m_crit_sec);
		m_crit_sec_inited = FALSE;
	}
	
	if(m_dsb_secondary) 
	{
		m_dsb_secondary->Release();
		m_dsb_secondary = NULL;
	}

	if(m_dsb_primary)
	{
		m_dsb_primary->Release();
		m_dsb_primary = NULL;
	}

	if(m_ds)
	{
		m_ds->Release();
		m_ds = NULL;
	}

	if(m_hwnd)
	{
		DestroyWindow(m_hwnd);
		UnregisterClass(BVL_DSWND_CLASS_NAME, m_hinst);
		m_hwnd = NULL;
	}

	m_channels = m_samplerate = 0;	
	m_last_play_pos = m_last_write_pos = 0;	
	m_inbuf_samples = m_total_samples = 0;

	m_playing = FALSE;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// 
/// Start 
/// 
////////////////////////////////////////////////////////////////////////////////////////////////////
HRESULT		CBandiVideoSound_DS::Start()
{
	EnterCriticalSection(&m_crit_sec);

	m_playing = TRUE;

	LeaveCriticalSection(&m_crit_sec);

	return S_OK;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// 
/// Stop
/// 
////////////////////////////////////////////////////////////////////////////////////////////////////
HRESULT		CBandiVideoSound_DS::Stop()
{
	EnterCriticalSection(&m_crit_sec);

	m_playing = FALSE;

	if(m_dsb_secondary)
	{
		m_dsb_secondary->Stop();
		m_dsb_secondary->SetCurrentPosition(0);
		m_last_play_pos = m_last_write_pos = 0;	
		m_total_samples = m_inbuf_samples = 0;
	}

	LeaveCriticalSection(&m_crit_sec);

	return S_OK;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// 
/// Stop
/// 
////////////////////////////////////////////////////////////////////////////////////////////////////
HRESULT		CBandiVideoSound_DS::Pause(BOOL pause)
{
	EnterCriticalSection(&m_crit_sec);

	if(pause)
	{	
		m_playing = FALSE;
		if(m_dsb_secondary) m_dsb_secondary->Stop();
	}
	else
	{
		m_playing = TRUE;
	}

	LeaveCriticalSection(&m_crit_sec);

	return S_OK;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// 
/// Get play time (100nsec units)
/// 
////////////////////////////////////////////////////////////////////////////////////////////////////
HRESULT	CBandiVideoSound_DS::GetTime(INT64 &time)
{
	EnterCriticalSection(&m_crit_sec);

	UpdatePlayPos();
	time = max((m_total_samples - m_inbuf_samples)*10000000i64/m_samplerate, 0);
	
	LeaveCriticalSection(&m_crit_sec);

	return S_OK;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// 
/// Set sound volume
/// 
////////////////////////////////////////////////////////////////////////////////////////////////////
HRESULT		CBandiVideoSound_DS::SetVolume(INT volume)
{
	if(m_dsb_secondary == NULL)
		return E_POINTER;

	double lfPower10 = pow(10.0,-10.0); // 10^(-10);
	double lfVolume = (1 - lfPower10)*((double)volume/255) + lfPower10;
	
	lfVolume = 10*log10(lfVolume);
	lfVolume *= 300;
	
	if(lfVolume < -10000)
		lfVolume = -10000;

	return m_dsb_secondary->SetVolume((LONG)lfVolume);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// 
/// ThreadProc
/// 
////////////////////////////////////////////////////////////////////////////////////////////////////
UINT CBandiVideoSound_DS::ThreadProc()
{
	BOOL bWorking = FALSE;
	while(m_hthread)
	{
		EnterCriticalSection(&m_crit_sec);
		{
			if(m_playing)
				PlayDS();
		}
		LeaveCriticalSection(&m_crit_sec);
		
		WaitForSingleObject(m_hevent, 10);
	}
	
	return 0;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// 
/// Play DirectSound
/// 
////////////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CBandiVideoSound_DS::PlayDS()
{
	HRESULT hr;
	int samples_to_write;
	int samples_written;
	int sample_bytes = m_channels*sizeof(INT16);
	DWORD dsb_status;

	m_dsb_secondary->GetStatus(&dsb_status);
	
	//	Update play position
	if(dsb_status & DSBSTATUS_PLAYING)
		UpdatePlayPos();

	samples_to_write = min(MAX_NUM_SAMPLE - m_inbuf_samples, MAX_NUM_SAMPLE);
	samples_to_write = samples_to_write / PLAY_SAMPLE_BLOCK * PLAY_SAMPLE_BLOCK;

	//	Transfer PCM data to Direct Sound Buffer
	if(m_callback)
	{
		void *lock1_ptr = NULL;
		void *lock2_ptr = NULL;
		DWORD lock1_bytes = 0;
		DWORD lock2_bytes = 0;
		DWORD lock1_samples = 0;
		DWORD lock2_samples = 0;


		if(samples_to_write > 0) 
		{
			hr = m_dsb_secondary->Lock(m_last_write_pos*sample_bytes, samples_to_write*sample_bytes,
											&lock1_ptr, &lock1_bytes,
											&lock2_ptr, &lock2_bytes, 0);

			if(hr == DSERR_BUFFERLOST) 
			{
				m_dsb_secondary->Restore();
				hr = m_dsb_secondary->Lock(m_last_write_pos*sample_bytes, samples_to_write*sample_bytes,
											&lock1_ptr, &lock1_bytes,
											&lock2_ptr, &lock2_bytes, 0);
			}
			
			if(FAILED(hr)) return hr;

			lock1_samples = lock1_bytes/sample_bytes;
			lock2_samples = lock2_bytes/sample_bytes;

			lock1_samples = m_callback->GetPcmData((INT16*)lock1_ptr, lock1_samples);
			if(lock1_bytes/sample_bytes == lock1_samples)
			{
				if(lock2_ptr) lock2_samples = m_callback->GetPcmData((INT16*)lock2_ptr, lock2_samples);
				else  lock2_samples  = 0;
			}
			else lock2_samples  = 0;

			samples_written = lock1_samples + lock2_samples;

			lock1_bytes = lock1_samples*sample_bytes;
			lock2_bytes = lock2_samples*sample_bytes;

			m_dsb_secondary->Unlock(lock1_ptr, lock1_bytes, lock2_ptr, lock2_bytes);
		}
		else
			samples_written = 0;
	}
	else 
	{
		samples_written = samples_to_write;
	}
	
	//	Update Write Pointer
	m_total_samples += samples_written;
	m_inbuf_samples += samples_written;
	m_last_write_pos += samples_written;
	if (m_last_write_pos >= MAX_NUM_SAMPLE)
		m_last_write_pos = m_last_write_pos % MAX_NUM_SAMPLE;
	
	//	PLAYBACK control
	if((dsb_status & DSBSTATUS_PLAYING) == 0) 
	{ 
		if (m_inbuf_samples >= PLAY_THRESHOLD) 
		{
			m_dsb_secondary->Play(0, 0, DSBPLAY_LOOPING);
		}
	}
	else
	{
		if (m_inbuf_samples <= 0) 
		{
			m_dsb_secondary->Stop();
			m_dsb_secondary->SetCurrentPosition(0);
			m_last_play_pos = m_last_write_pos = 0;	
		}
	}

	return S_OK;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
/// 
/// Update play position
/// 
////////////////////////////////////////////////////////////////////////////////////////////////////
void CBandiVideoSound_DS::UpdatePlayPos()
{
	if(m_dsb_secondary == NULL) return;

	EnterCriticalSection(&m_crit_sec);
	{
		DWORD play_pos;
		if(SUCCEEDED(m_dsb_secondary->GetCurrentPosition(&play_pos, NULL)))
		{
			play_pos /= m_channels*sizeof(INT16);	// bytes -> samples
			if(play_pos >= m_last_play_pos)
				m_inbuf_samples -= (play_pos - m_last_play_pos);
			else
				m_inbuf_samples -= MAX_NUM_SAMPLE - (m_last_play_pos - play_pos);

			if(m_inbuf_samples < 0)
				m_inbuf_samples = 0;

			m_last_play_pos = play_pos;
		}
	}
	LeaveCriticalSection(&m_crit_sec);
}
