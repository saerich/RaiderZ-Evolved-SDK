////////////////////////////////////////////////////////////////////////////////////////////////////
/// 
/// BandiAudioLibrary
/// 
/// Copyright(C) 2008 BandiSoft.com Allrights reserved.
///
/// 이 소스 및 프로그램 사용시 LICENSE.TXT 및 홈페이지에 기재된 저작권 정보를 확인후 준수해 주시기 바랍니다.
///
/// http://www.bandisoft.com/bandi_audio_library/
///
///
////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////
/// 
/// definitions for bandi audio
/// 
/// @author   parkkh
/// @date     Monday, August 11, 2008  7:20:29 PM
/// 
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// options
#define BAOPT_NONE		0x0000
#define BAOPT_LOOP		0x0001

/// status
enum BASTATUS
{
	BASTATUS_STOP,
	BASTATUS_PLAY,
};

// notify message
extern UINT	WM_BANOTIFY_EOF;			// END OF FILE NOTIFICATION, WPARAM: DWORD id 

// version info
#define BANDI_AUDIO_LIBRARY_DLL_VERSION		0x20080828


////////////////////////////////////////////////////////////////////////////////////////////////////
///         
///         BandiAudio controller interface
///         
/// @date   Monday, August 11, 2008  7:25:28 PM
/// 
////////////////////////////////////////////////////////////////////////////////////////////////////
interface IBaCon
{
	virtual HRESULT		Release() PURE;											/// release instance
	virtual HRESULT		Create(HINSTANCE hInstance, HWND hWndParent) PURE;		/// create object
	virtual HRESULT		Play(LPCWSTR szPathName) PURE;
	virtual HRESULT		Play(LPCSTR szPathName) PURE;
	virtual HRESULT		Stop() PURE;
	virtual HRESULT		SetVolume256(int nVolume) PURE;
	virtual HRESULT		SetOption(DWORD dwOption) PURE;
	virtual HRESULT		GetStatus(BASTATUS* pStatus) PURE;
	virtual HRESULT		GetID(DWORD* pID) PURE;
	virtual HRESULT		SetThreadPriority(int nPriority) PURE;

	virtual LPCSTR		GetName() PURE;											/// get library name + compile date
	virtual HRESULT		GetLibraryVersion(DWORD* pVersion) PURE;				/// get library version for check interface compatibility

};


////////////////////////////////////////////////////////////////////////////////////////////////////
///         
///         utility class for create bandi audio object
///         
/// @date   Friday, August 08, 2008  10:05:13 AM
/// 
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef ASSERT
# include <assert.h>
# define ASSERT(x) assert(x)
#endif;
interface IBaCon;


#ifdef _WIN64
#	ifdef _DEBUG
#		define BANDIAUDIO_DLL_FILE_NAME		_T("bdaudio64_d.dll")
#	else
#		define BANDIAUDIO_DLL_FILE_NAME		_T("bdaudio64.dll")
#	endif
#	define BANDIAUDIO_RELEASE_DLL_FILE_NAME	_T("bdaudio64.dll")
#else
#	ifdef _DEBUG
#		define BANDIAUDIO_DLL_FILE_NAME		_T("bdaudio32_d.dll")
#	else
#		define BANDIAUDIO_DLL_FILE_NAME		_T("bdaudio32.dll")
#	endif
#	define BANDIAUDIO_RELEASE_DLL_FILE_NAME	_T("bdaudio32.dll")
#endif

typedef HRESULT (*LPCREATEBANDIAUDIO)(int version, void **p);


////////////////////////////////////////////////////////////////////////////////////////////////////
// BA ERR CODES
#define _FACBCD  0x666
#define MAKE_BAHRESULT( code )	MAKE_HRESULT( 1, _FACBCD, code )
#define MAKE_BASTATUS( code )	MAKE_HRESULT( 0, _FACBCD, code )


#define BAERR_GENERAL							MAKE_BAHRESULT(1000)
#define BAERR_INVALID_VERSION					MAKE_BAHRESULT(1001)
#define BAERR_OUT_OF_MEMORY						MAKE_BAHRESULT(1001)


////////////////////////////////////////////////////////////////////////////////////////////////////
///         utility class for load library and create instance for audio library
/// @param  
/// @return 
/// @date   Thursday, August 14, 2008  3:48:15 PM
////////////////////////////////////////////////////////////////////////////////////////////////////
class CBandiAudioFactory
{
public :
	CBandiAudioFactory()
	{
		m_hDll = NULL;
		m_pCreateBandiAudio = NULL;
	}

	~CBandiAudioFactory()
	{
		Destroy();
	}

	BOOL Create(LPCTSTR szDllPathName)		/// load library and get function pointer for class factory
	{
		if(m_hDll) {ASSERT(0); return FALSE;}

		m_hDll = LoadLibrary(szDllPathName);
		if(m_hDll==NULL)
		{
			ASSERT(0);
			return FALSE;
		}

		m_pCreateBandiAudio = (LPCREATEBANDIAUDIO)GetProcAddress(m_hDll, "CreateBandiAudio");

		if(m_pCreateBandiAudio==NULL)
		{
			ASSERT(0);
			FreeLibrary(m_hDll);
			m_hDll = NULL;
			return FALSE;
		}
		return TRUE;
	}

	IBaCon*	CreateBaCon()				/// create music player instance
	{
		IBaCon* pBaCon = NULL;
		if(FAILED(m_pCreateBandiAudio(BANDI_AUDIO_LIBRARY_DLL_VERSION, (void**)&pBaCon)))
		{
			ASSERT(0);
			return NULL;
		}
		return pBaCon;
	}

	void	Destroy()					/// free library
	{
		m_pCreateBandiAudio = NULL;
		if(m_hDll) FreeLibrary(m_hDll);
		m_hDll = NULL;
	}

private :
	HMODULE				m_hDll;
	LPCREATEBANDIAUDIO	m_pCreateBandiAudio;
};

