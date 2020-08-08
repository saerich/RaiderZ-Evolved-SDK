//**************************************************************************/
// Copyright (c) 1998-2007 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//**************************************************************************/
/**********************************************************************
 *<
	FILE: WeatherFileDialog.h

	DESCRIPTION:	Weather File Dialog header

	CREATED BY: Michael Zyracki

	HISTORY: Created Nov 2007

 *>	Copyright (c) 2007, All Rights Reserved.
 **********************************************************************/


#ifndef _WEATHERFILEDIALOG_H
#define _WEATHERFILEDIALOG_H

#include "WeatherData.h"

// Adding the following definition is the only way to get the linker to not issue this warning:
// warning LNK4248: unresolved typeref token (01000011) for '_IMAGELIST'; image may not run
struct _IMAGELIST {};

class WeatherFileDialog
{
	friend INT_PTR CALLBACK WeatherFileDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	friend INT_PTR CALLBACK SetPeriodDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	public:
		WeatherFileDialog():mHWND(0){}
		void GetWeatherData(WeatherUIData &data);
		void SetWeatherData(WeatherUIData);

		BOOL LaunchDialog(HWND hParent,WeatherUIData &data);
		void InitDialog(HWND hDlg);
		void GetResults(HWND hDlg);
		void GetSpinnerValues(HWND hDlg);
		void SetDateWindows(HWND hDlg,BOOL valid = TRUE);
		void SetTotalFrames(HWND hDlg,BOOL valid = TRUE);
		TSTR GetTimeString(Int3 &time);
		TSTR GetDateString(Int3 &date);
		void MatchTimeLine();
		void SetPeriod(HWND hParent,Int3 &MDY,Int3 &HMS);
		void SetPeriodInit(HWND hDlg, Int3 &MDY,Int3 &HMS);
		void SetDaysInMonth(HWND hDlg,Int3 &MDY);
		void CheckMonthsValue(HWND hDlg, Int3 &MDY,Int3 &HMS,BOOL checkIter=TRUE);
		void CheckDaysValue(HWND hDlg, Int3 &MDY,Int3 &HMS,BOOL checkIter = TRUE);
		void CheckHoursValue(HWND hDlg, Int3 &MDY,Int3 &HMS,BOOL checkIter = TRUE);
		void CheckMinutesValue(HWND hDlg, Int3 &MDY,Int3 &HMS,BOOL checkIter = TRUE);
		void ResetTimePeriods(HWND hDlg,BOOL valid);
		void SetSliderValue(int location,HWND hDlg, Int3 &MDY,Int3 &HMS);
		void SpinValuesSet(HWND hDlg, Int3 &MDY,Int3 &HMS);
private:
		WeatherUIData mData;
		WeatherFileCache		mWeatherFileCache;
		//use the following for tracking the file
		int mCurrentPeriod;
		DaylightWeatherDataEntries::iterator mIter;
		HWND mHWND; //will be o if not open
		static HIMAGELIST hIconImages;
		struct SetPeriodData
		{
			WeatherFileDialog *mDlg;
			Int3 MDY;
			Int3 HMS;
		};
		void GetWeatherFile(HWND hWnd);
		void ClearWeatherFile(HWND hWnd);
		BOOL GetInfoFromWeatherFile();
};




#endif

