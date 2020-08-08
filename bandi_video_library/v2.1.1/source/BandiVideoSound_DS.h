////////////////////////////////////////////////////////////////////////////////////////////////////
/// 
/// BandiVideoLibrary 2.1
/// 
/// Copyright(C) 2008-2010 BandiSoft.com All rights reserved.
///
/// Visit http://www.bandisoft.com for more information.
/// 
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#define DIRECTSOUND_VERSION 0x0800

#include <dsound.h>
#include "bandivid.h"


class CBandiVideoSound_DS : public IBandiVideoSound
{
public:
	static const UINT MAX_NUM_SAMPLE	= 4800*2;
	static const UINT MAX_NUM_CHANNEL	= 2;
	static const UINT PLAY_THRESHOLD	= 4800;
	static const UINT PLAY_SAMPLE_BLOCK = 80;

	CBandiVideoSound_DS();
	virtual ~CBandiVideoSound_DS();

	// IUnknown
	STDMETHODIMP QueryInterface(REFIID riid, void **ppv) { return E_NOTIMPL; };
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();

	// IBandiVideoSound
	HRESULT		Open(INT channels, INT samplerate, IBandiVideoSoundCB *callback);
	void		Close();
	HRESULT		Start();
	HRESULT		Stop();
	HRESULT		Pause(BOOL pause);
	HRESULT		GetTime(INT64 &time);
	HRESULT		SetVolume(INT volume);

protected:
	static unsigned WINAPI InitialThreadProc(LPVOID param)
	{
		CBandiVideoSound_DS *p = (CBandiVideoSound_DS*)param;
		return p->ThreadProc();		
	};
	UINT	ThreadProc();
	HRESULT	PlayDS();
	void	UpdatePlayPos();

	LONG				m_ref_count;

	HWND				m_hwnd;				// Window Handle
	HINSTANCE			m_hinst;			// Instance Handle
	LPDIRECTSOUND8		m_ds;				// DirectSound Interface
	LPDIRECTSOUNDBUFFER	m_dsb_primary;		// Primary buffer Interface
	LPDIRECTSOUNDBUFFER m_dsb_secondary;	// DirectSound secondary sound buffer

	UINT				m_channels;			// Number of Channnel
	UINT				m_samplerate;		// Sample rate;
	
	UINT				m_last_play_pos;	// Last play offset (in samples)
	UINT				m_last_write_pos;	// Last write offset (in samples)
	
	INT					m_inbuf_samples;	// Number of samples in the buffer
	INT					m_total_samples;	// Total number of written samples

	CRITICAL_SECTION	m_crit_sec;
	BOOL				m_crit_sec_inited;
	
	HANDLE				m_hevent;
	HANDLE				m_hthread;
	UINT				m_thread_id;

	BOOL				m_playing;

	IBandiVideoSoundCB*	m_callback;
};
