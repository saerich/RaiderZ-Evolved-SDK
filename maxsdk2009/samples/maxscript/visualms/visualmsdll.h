// ============================================================================
// VisualMSDll.h
// Copyright ©2000, Discreet
// Created by Simon Feltman
// ----------------------------------------------------------------------------
#ifndef __VISUALMSDLL_H__
#define __VISUALMSDLL_H__

#include "VisualMSThread.h"

// ============================================================================
// For MFC DLL's you must use a global instance of a CWinApp derived class to do
// initialization instead of using the DllEntry function.
class CVisualMSDLL : public CWinApp
{
public:
	CPtrArray m_threads;

	BOOL InitInstance();
	BOOL Exit();

	void RegisterThread(CVisualMSThread *pThread);
	void RemoveThread(CVisualMSThread *pThread);
};

extern CVisualMSDLL theDLL;


#endif //__VISUALMSDLL_H__

