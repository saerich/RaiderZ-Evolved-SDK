
//**************************************************************************/
// Copyright (c) 2008 Autodesk, Inc.
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
	FILE: weatherfiledialog.cpp

	DESCRIPTION:	Weather Data UI

	CREATED BY: Michael Zyracki

	HISTORY: Created Nov 2007

 *>	Copyright (c) 2008, All Rights Reserved.
 **********************************************************************/
#include "NativeInclude.h"
#include "WeatherFileDialog.h"
#include "sunlight.h"
#include "autovis.h"

#pragma managed

using namespace MaxSDK::AssetManagement;

HIMAGELIST WeatherFileDialog::hIconImages = NULL;
int  WeatherFileDialog::mPos[4] = {433,295,534,431};
bool WeatherFileDialog::mPreviouslyDisplayed = false;

int getTimeZone(float longi);
class WeatherHistoryList : public bmmHistoryList
{
public:

	void Init(TSTR &initDir);
};

void WeatherHistoryList::Init(TSTR &initDir)
{
	_tcscpy(title,MaxSDK::GetResourceString(IDS_WEATHER_FILE));
	bmmHistoryList::LoadDefaults();
	
	if (bmmHistoryList::DefaultPath()[0])
		initDir = TSTR( bmmHistoryList::DefaultPath());
	else 
		initDir = GetCOREInterface()->GetDir(APP_EXPORT_DIR);

};


class CustomData {
public:
	WeatherHistoryList history;
	BOOL save;
	TSTR initDir;
	CustomData(BOOL s) {
		save = s; //if save we show the plus button
		history.Init(initDir);
		history.LoadDefaults();
	};
};



INT_PTR WINAPI WeatherHookProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
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
		 LPNMHDR nmh = (LPNMHDR)lParam;

		 if (nmh->code == CDN_INITDONE && custom)
		 {
			 OFNOTIFY *ofnot = (OFNOTIFY *) lParam;
			 custom->history.NewPath(ofnot->lpOFN->lpstrInitialDir);
		 }

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

void WeatherFileDialog::ClearWeatherFile(HWND hWnd)
{
	mData.mFileName = TSTR("");
	mData.mResetTimes = true;
	InitDialog(hWnd);
}

void WeatherFileDialog::GetWeatherFile(HWND hWnd)
{
	// set initial file/dir
	TCHAR fname[MAX_PATH] = {'\0'};
	TCHAR* str = mData.mFileName.data();

	if(str&& _tcslen(str)>0)
	{
		MaxSDK::Util::Path cachePath(str);
		IFileResolutionManager* pFRM = IFileResolutionManager::GetInstance();
		DbgAssert(pFRM);  //TTD: consider replacing this with debug assert function in Systems::Diagnositics
		// TTD: determine the right assetType enum. For now default to bitmap
		if (pFRM && pFRM->GetFullFilePath(cachePath, kBitmapAsset)) 
		{
			TSTR what = cachePath.GetString();
			str = what.data();
			strcpy(fname, str);
		}
	}

	OPENFILENAME ofn = {sizeof(OPENFILENAME)};
	ofn.lStructSize = sizeof(OPENFILENAME);//_SIZE_VERSION_400; // has OFN_ENABLEHOOK

	FilterList fl;
	CustomData custom(FALSE);

	// All Cache Files
	fl.Append(MaxSDK::GetResourceString(IDS_ALL_WEATHER_FILES));
	fl.Append(MaxSDK::GetResourceString(IDS_ALL_WEATHER_FILES_WILDCARD));

	TSTR title = MaxSDK::GetResourceString(IDS_OPEN_WEATHER_FILE);

	ofn.hwndOwner       = hWnd;
 	ofn.lpstrFilter     = fl;
	ofn.lpstrFile       = fname;
	ofn.nMaxFile        = MAX_PATH;
	ofn.lpstrInitialDir = custom.initDir.data();
	ofn.Flags           = OFN_HIDEREADONLY| OFN_ENABLETEMPLATE |OFN_FILEMUSTEXIST |OFN_ENABLEHOOK|OFN_EXPLORER;
//	ofn.FlagsEx         = OFN_EX_NOPLACESBAR;
	ofn.lpTemplateName = MAKEINTRESOURCE(IDD_FILE_DIALOG);
	ofn.lpfnHook = (LPOFNHOOKPROC)WeatherHookProc;
	ofn.hInstance = MaxSDK::GetHInstance();

	ofn.Flags |= OFN_ENABLESIZING;

	ofn.lCustData       = LPARAM(&custom);

	ofn.lpstrDefExt     = MaxSDK::GetResourceString(IDS_WEATHER_EXT);
	ofn.lpstrTitle      = title;


	if (GetOpenFileName(&ofn))
	{
		TSTR tfname = TSTR(fname);
		if(mData.mFileName==tfname&&tfname.Length()>0)
			mData.mResetTimes = false;
		else
			mData.mResetTimes = true;
		mData.mFileName = fname;
		
	}
	InitDialog(hWnd);
}




BOOL WeatherFileDialog::LaunchDialog(HWND hParent,WeatherUIData &data)
{
	mData = data;
	if (DialogBoxParam(MaxSDK::GetHInstance(), MAKEINTRESOURCE(IDD_WEATHER_DATA_FILE),
		hParent, WeatherFileDlgProc, (LPARAM)this) == 1) 
	{
		data = mData;
		mHWND = NULL;
		return TRUE;
	}
	mHWND = NULL;

	return FALSE;
}


static void SetText(HWND hWnd, const TCHAR *text)
{
	SetWindowText( hWnd, text );
	InvalidateRect( hWnd, NULL, TRUE );
}

BOOL WeatherFileDialog::GetInfoFromWeatherFile()
{

	return (mWeatherFileCache.ReadWeatherData(mData.mFileName.data())>0) ? TRUE : FALSE;
	
}

// returns whether or not the string has been truncated
static bool GetTruncatedString(HWND hedit, const TCHAR* name, TCHAR *buf)
{
	// get the font
	HFONT Font = NULL;
	LOGFONT lf;
	Font = reinterpret_cast<HFONT>(SendMessage(hedit, WM_GETFONT, 0, 0));
	GetObject(Font, sizeof(LOGFONT), &lf);

	HDC	hdc = GetDC(hedit);
	HFONT oldFont = (HFONT)SelectObject(hdc,Font);
	Rect rect;
	GetClientRect(hedit,&rect);
	DPtoLP(hdc,(LPPOINT)&rect,2);
	int w = rect.right;

	bool truncated = false;
	const TCHAR *cur = name;
	_tcscpy(buf,name);
	SIZE sz;
	int len = static_cast<int>(_tcslen(name));   // SR DCAST64: Downcast to 2G limit.
	for (int i=0;i<len;i++) {
		BOOL b = DLGetTextExtent(hdc,buf,&sz);
		if (sz.cx <= w) 
			break;
		truncated = true;
		cur++;
		_tcscpy(buf,_T("..."));
		_tcscat(buf,cur);
	}
	//SetText(hedit, buf);
	SelectObject(hdc,oldFont);
	ReleaseDC(hedit,hdc);
	return truncated;
}

void WeatherFileDialog::InitDialog(HWND hDlg)
{
	mHWND = hDlg;//let's us know it's open.
	
	BOOL valid  = FALSE;
	bool truncated = false;
	if(mData.mFileName.Length()>0)
	{
		// set text to truncated string
		TSTR truncname = mData.mFileName.data();
		truncname += _T("....."); // make it a little bigger for the dots
		// need to get the truncated name from a hidden windows static text control, to get the right size
		truncated = GetTruncatedString(GetDlgItem(hDlg,IDC_WEATHER_FILENAME_HIDDEN),mData.mFileName.data(),truncname.data());
		if (truncated) // set text to the truncated name if necessary
			 SetText(GetDlgItem(hDlg,IDC_WEATHER_FILENAME),truncname.data());
		else SetText(GetDlgItem(hDlg,IDC_WEATHER_FILENAME),mData.mFileName.data());

		valid = TRUE;
	}
	else
	{
		// set text to default string - no file
		SetText(GetDlgItem(hDlg,IDC_WEATHER_FILENAME), MaxSDK::GetResourceString(IDS_NO_WEATHER_FILE_LOADED));
		valid = FALSE;
	}

	// set tooltip to the full string if truncated, turn off tooltip if not truncated
	ICustStatus *pEdit = GetICustStatus( GetDlgItem(hDlg,IDC_WEATHER_FILENAME));
	if( pEdit) 
	{
		pEdit->SetTooltip(truncated, const_cast<TCHAR*>(mData.mFileName.data())); //const_cast<TCHAR*>(szTooltip));
		ReleaseICustStatus( pEdit );
	}

	if(valid)
		valid = GetInfoFromWeatherFile();
	TCHAR str[32];
	if(valid)
	{
		//keeps current position in the file, used by both the weather controller and the UI for scrolling time.
		mCurrentPeriod = 0;
		mIter = mWeatherFileCache.mDataPeriods[mCurrentPeriod]->begin();
		Int3 startMDY(1,1,2007),endMDY(1,1,2007);
		Int3 startHMS(12,0,0), endHMS(12,0,0);
	
		DaylightWeatherDataEntries::iterator iter = mWeatherFileCache.mDataPeriods[0]->begin();
		if(iter!=mWeatherFileCache.mDataPeriods[0]->end())
		{
			startMDY.x = iter->mMonth;
			startMDY.y = iter->mDay;
			startMDY.z = iter->mYear;
			startHMS.x = iter->mHour;
			startHMS.y = iter->mMinute;
			startHMS.z = 0;
		}

		int numPeriods = mWeatherFileCache.mDataPeriods.Count();
		iter = --mWeatherFileCache.mDataPeriods[numPeriods-1]->end();
		if(iter!=mWeatherFileCache.mDataPeriods[numPeriods-1]->end())
		{
			endMDY.x = iter->mMonth;
			endMDY.y = iter->mDay;
			endMDY.z = iter->mYear;
			endHMS.x = iter->mHour;
			endHMS.y = iter->mMinute;
			endHMS.z = 0;
		}

		TSTR dateString = GetDateString(startMDY);
		SetText(GetDlgItem(hDlg,IDC_START_DATE),dateString.data());

		dateString = GetDateString(endMDY);
		SetText(GetDlgItem(hDlg,IDC_END_DATE),dateString.data());


		if(mData.mResetTimes==true)
		{
			//new file loaded so set up new start, end times for UI and data structure.
			mData.mSpecificTimeHMS = startHMS;
			mData.mAnimStartTimeHMS = startHMS;
			mData.mSpecificTimeMDY = startMDY;
			mData.mAnimStartTimeMDY = startMDY;
			mData.mAnimEndTimeHMS = endHMS;
			mData.mAnimEndTimeMDY = endMDY;
		}

		TSTR city = mWeatherFileCache.mWeatherHeaderInfo.mCity + TSTR(", ") +
			mWeatherFileCache.mWeatherHeaderInfo.mRegion + TSTR(", ") + mWeatherFileCache.mWeatherHeaderInfo.mCountry;
		SetText(GetDlgItem(hDlg,IDC_LOCATION),city.data());

		TCHAR lat[32], lon[32];
		if(mWeatherFileCache.mWeatherHeaderInfo.mLat>0)
			sprintf(lat,"(%4.3f %s, ",mWeatherFileCache.mWeatherHeaderInfo.mLat,MaxSDK::GetResourceString(IDS_NORTH_INITIAL));
		else
			sprintf(lat,"(%4.3f %s, ",-mWeatherFileCache.mWeatherHeaderInfo.mLat,MaxSDK::GetResourceString(IDS_SOUTH_INITIAL));

		if(mWeatherFileCache.mWeatherHeaderInfo.mLon>0)
			sprintf(lon,"%4.3f %s)",mWeatherFileCache.mWeatherHeaderInfo.mLon,MaxSDK::GetResourceString(IDS_EAST_INITIAL));
		else
			sprintf(lon,"%4.3f %s)",-mWeatherFileCache.mWeatherHeaderInfo.mLon,MaxSDK::GetResourceString(IDS_WEST_INITIAL));

		TSTR latlon = TSTR(lat) +TSTR(lon);
		SetText(GetDlgItem(hDlg,IDC_LOCATION_LAT),latlon.data());

//		SetText(GetDlgItem(hDlg,IDC_TIME_ZONE),getTimeZone(mWeatherFileCache.mWeatherHeaderInfo.mLon));
		if(mWeatherFileCache.mWeatherHeaderInfo.mNumRecordsPerHour>0.0f) //not -1 or zero
		{
			sprintf(str,"%3.1f %s",mWeatherFileCache.mWeatherHeaderInfo.mNumRecordsPerHour,MaxSDK::GetResourceString(IDS_PER_HOUR));
			SetText(GetDlgItem(hDlg,IDC_DATA_PERIOD),str);
		}
		else
		{
			SetText(GetDlgItem(hDlg,IDC_DATA_PERIOD),MaxSDK::GetResourceString(IDS_MULTIPLE));
		}

		sprintf(str,"%d",mWeatherFileCache.mNumEntries);
		SetText(GetDlgItem(hDlg,IDC_TOTAL_PERIODS),str);
	}
	else
	{
		TSTR error = TSTR("");
		SetText(GetDlgItem(hDlg,IDC_TIME_ZONE),MaxSDK::GetResourceString(IDS_ERROR_READING_WEATHER_FILE));
		//SetText(GetDlgItem(hDlg,IDC_TIME_ZONE),error.data());
		SetText(GetDlgItem(hDlg,IDC_LOCATION),error.data());
		SetText(GetDlgItem(hDlg,IDC_LOCATION_LAT),error.data());
		SetText(GetDlgItem(hDlg,IDC_LOCATION_LAT),error.data());
		SetText(GetDlgItem(hDlg,IDC_START_DATE),error.data());
		SetText(GetDlgItem(hDlg,IDC_END_DATE),error.data());
		SetText(GetDlgItem(hDlg,IDC_DATA_PERIOD),error.data());
		SetText(GetDlgItem(hDlg,IDC_TOTAL_PERIODS),error.data());
	}	


	ICustButton *setPeriod1 = GetICustButton(GetDlgItem(hDlg,IDC_SET_PERIOD));
	setPeriod1->SetTooltip(true,MaxSDK::GetResourceString(IDS_CHANGE_TIME_PERIOD));
	ICustButton *setPeriod2 = GetICustButton(GetDlgItem(hDlg,IDC_SET_PERIOD2));
	setPeriod2->SetTooltip(true,MaxSDK::GetResourceString(IDS_CHANGE_TIME_PERIOD));
	ICustButton *setPeriod3 = GetICustButton(GetDlgItem(hDlg,IDC_SET_PERIOD3));
	setPeriod3->SetTooltip(true,MaxSDK::GetResourceString(IDS_CHANGE_TIME_PERIOD));
	if(mData.mAnimated==TRUE)
	{
		CheckDlgButton(hDlg,IDC_ANIMATION,TRUE);
		CheckDlgButton(hDlg,IDC_USE_DATE_TIME,FALSE);
	}
	else
	{
		CheckDlgButton(hDlg,IDC_ANIMATION,FALSE);
		CheckDlgButton(hDlg,IDC_USE_DATE_TIME,TRUE);
	}

	if(valid==FALSE)
	{
		EnableWindow(GetDlgItem(hDlg,IDC_ANIMATION),FALSE);
		EnableWindow(GetDlgItem(hDlg,IDC_USE_DATE_TIME),FALSE);
		if(setPeriod1)
			setPeriod1->Enable(FALSE);
		if(setPeriod2)
			setPeriod2->Enable(FALSE);
		if(setPeriod3)
			setPeriod3->Enable(FALSE);
	}
	else
	{
		EnableWindow(GetDlgItem(hDlg,IDC_ANIMATION),TRUE);
		EnableWindow(GetDlgItem(hDlg,IDC_USE_DATE_TIME),TRUE);
		if(setPeriod1)
			setPeriod1->Enable(!mData.mAnimated);
		if(setPeriod2)
			setPeriod2->Enable(mData.mAnimated);
		if(setPeriod3)
			setPeriod3->Enable(mData.mAnimated);

	}

	static int perFrameTable[] = {
		IDS_PERIOD,	
		IDS_DAY,				
		IDS_WEEK,		
		IDS_MONTH,		
		IDS_SEASON,				
	};
	
	//init the combo box.
	HWND hCombo = GetDlgItem(hDlg,IDC_ONE_FRAME);
	SendMessage(hCombo,CB_RESETCONTENT,0,0);
	for (int i=0; i<WeatherCacheFilter::eEnd; i++)
	{
		const TCHAR *name = MaxSDK::GetResourceString(perFrameTable[i]);
		if(nullptr != name)
		{
			SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)name);
		}
	}
	SendMessage(hCombo, CB_SETCURSEL, mData.mFilter.mFramePer, 0);
	EnableWindow(hCombo,valid);

    CheckDlgButton(hDlg,IDC_SKIP_HOURS,mData.mFilter.mSkipHours);
	EnableWindow(GetDlgItem(hDlg,IDC_SKIP_HOURS),valid);
	ISpinnerControl *spin = GetISpinner(GetDlgItem(hDlg,IDC_SKIP_HOURS_START_SPIN));
	spin->LinkToEdit(GetDlgItem(hDlg,IDC_SKIP_HOURS_START),EDITTYPE_POS_INT);
	spin->SetScale(1.0f);
	spin->SetLimits(0,24);
	spin->SetValue(mData.mFilter.mSkipHoursStart,FALSE);
	
	if(valid==FALSE)
		spin->Enable(FALSE);
	else
		spin->Enable(mData.mFilter.mSkipHours);
	ReleaseISpinner(spin);
	spin = GetISpinner(GetDlgItem(hDlg,IDC_SKIP_HOURS_END_SPIN));
	spin->LinkToEdit(GetDlgItem(hDlg,IDC_SKIP_HOURS_END),EDITTYPE_POS_INT);
	spin->SetScale(1.0f);
	spin->SetLimits(0,24);
	spin->SetValue(mData.mFilter.mSkipHoursEnd,FALSE);
	if(valid==FALSE)
		spin->Enable(FALSE);
	else
		spin->Enable(mData.mFilter.mSkipHours);
	ReleaseISpinner(spin);

    CheckDlgButton(hDlg,IDC_SKIP_WEEKENDS,mData.mFilter.mSkipWeekends);
	EnableWindow(GetDlgItem(hDlg,IDC_SKIP_WEEKENDS),valid);

	SetTotalFrames(hDlg,valid);

	EnableWindow(GetDlgItem(hDlg,IDC_MATCH_TIMELINE),valid);

	// Set up weather clear icon
	if (!hIconImages)
	{
		hIconImages  = ImageList_Create(16, 15, ILC_COLOR24 | ILC_MASK, 6, 0);
		LoadMAXFileIcon("daylight", hIconImages, kBackground, FALSE);
	}
	ICustButton *pBut = GetICustButton(GetDlgItem(hDlg,IDC_CLEAR_WEATHER_FILE));
	if(pBut && hIconImages)
	{
		pBut->SetImage(hIconImages,2,2,3,3,16,14);
		ReleaseICustButton(pBut);
	}
	//if (pBut) pBut->SetTooltip(TRUE, MaxSDK::GetResourceString(IDS_CLEAR_WEATHER_FILE));

	if(setPeriod1)
		ReleaseICustButton(setPeriod1);
	if(setPeriod2)
		ReleaseICustButton(setPeriod2);
	if(setPeriod3)
		ReleaseICustButton(setPeriod3);

}

void WeatherFileDialog::GetSpinnerValues(HWND hDlg)
{
	ISpinnerControl *spin = GetISpinner(GetDlgItem(hDlg,IDC_SKIP_HOURS_START_SPIN));
	mData.mFilter.mSkipHoursStart = spin->GetIVal();
	ReleaseISpinner(spin);
	spin = GetISpinner(GetDlgItem(hDlg,IDC_SKIP_HOURS_END_SPIN));
	mData.mFilter.mSkipHoursEnd = spin->GetIVal();
	ReleaseISpinner(spin);

}
void WeatherFileDialog::GetResults(HWND hDlg)
{
	GetSpinnerValues(hDlg);
}

TSTR WeatherFileDialog::GetTimeString(Int3 &time)
{
	int hour = time.x;
	int minute = time.y;
	int second = time.z;
	TCHAR what[128];
	sprintf(what, "%02d:%02d:%02d",hour,minute,second);
	return TSTR(what);

}

TSTR WeatherFileDialog::GetDateString(Int3 &date)
{
	static int day[] = {
		IDS_SUNDAY,
		IDS_MONDAY,
		IDS_TUESDAY,
		IDS_WEDNESDAY ,
		IDS_THURSDAY ,
		IDS_FRIDAY ,
		IDS_SATURDAY };

	static int month [] = {
		IDS_JANUARY,
		IDS_FEBRUARY,
		IDS_MARCH ,
		IDS_APRIL ,
		IDS_MAY  ,
		IDS_JUNE ,
		IDS_JULY ,
		IDS_AUGUST ,
		IDS_SEPTEMBER ,
		IDS_OCTOBER ,
		IDS_NOVEMBER ,
		IDS_DECEMBER };                  

 
		int x = date.x;
		int y = date.y;
		int z = date.z;

		TCHAR what[256];
		int dayofWeek = mWeatherFileCache.FindDayOfWeek(z,x,y);

		TSTR dayStr = TSTR(MaxSDK::GetResourceString(day[dayofWeek]));
		TSTR monStr = TSTR(MaxSDK::GetResourceString(month[x-1]));
		sprintf(what,"%s, %s %d, %d ", dayStr.data(),monStr.data(),
			y,z);
		return TSTR(what);
		
}

void WeatherFileDialog::SetDateWindows(HWND hDlg,BOOL valid)
{
	TSTR animStart("");
	TSTR animEnd("");
	TSTR dateTime("");
	if(valid==TRUE)
	{
		if(mData.mAnimated==TRUE)
		{
			TSTR timeString =  GetTimeString(mData.mAnimStartTimeHMS);
			TSTR dateString = GetDateString(mData.mAnimStartTimeMDY);
			animStart = dateString +TSTR("; ") + timeString;
			SetWindowText(GetDlgItem(hDlg,IDC_ANIMATION_START),animStart.data());
			timeString =  GetTimeString(mData.mAnimEndTimeHMS);
			dateString = GetDateString(mData.mAnimEndTimeMDY);
			animEnd = dateString +TSTR("; ") + timeString;
			SetWindowText(GetDlgItem(hDlg,IDC_ANIMATION_END),animEnd.data());
			SetWindowText(GetDlgItem(hDlg,IDC_DATE_TIME),dateTime.data());
		}
		else
		{
			SetWindowText(GetDlgItem(hDlg,IDC_ANIMATION_START),animStart.data());
			SetWindowText(GetDlgItem(hDlg,IDC_ANIMATION_END),animEnd.data());
			TSTR timeString =  GetTimeString(mData.mSpecificTimeHMS);
			TSTR dateString = GetDateString(mData.mSpecificTimeMDY);
			dateTime = dateString +TSTR("; ") + timeString;
			SetWindowText(GetDlgItem(hDlg,IDC_DATE_TIME),dateTime.data());
		}
	}
	else
	{
		SetWindowText(GetDlgItem(hDlg,IDC_ANIMATION_START),animStart.data());
		SetWindowText(GetDlgItem(hDlg,IDC_ANIMATION_END),animEnd.data());
		SetWindowText(GetDlgItem(hDlg,IDC_DATE_TIME),dateTime.data());
	}
}

bool WeatherFileDialog::RestoreWindowLoc(HWND hWnd)
{
	if( !hWnd )
		return false;

	// center window if never shown
	if (!mPreviouslyDisplayed)
	{
		CenterWindow(hWnd, GetWindow(hWnd, GW_OWNER));
		mPreviouslyDisplayed = true;
		StoreWindowLoc(hWnd);
		return false;
	}
	// get stored position and check against screen boundaries
	else
	{
		int iWidth = GetScreenWidth();
		int iHeight = GetScreenHeight();
		if( (mPos[0] < 0) || (mPos[0] > iWidth) )
			mPos[0] = 0;
		if( (mPos[1] < 0) || (mPos[1] > iHeight) )
			mPos[1] = 0;
		if( mPos[2] > iWidth )
			mPos[2] = iWidth;
		if( mPos[3] > iHeight )
			mPos[3] = iHeight;
		SetWindowPos(hWnd, NULL, mPos[0], mPos[1], mPos[2], mPos[3], SWP_NOZORDER);
	}
	return true;
}

bool WeatherFileDialog::StoreWindowLoc(HWND hWnd)
{
	if( !hWnd )
		return false;

	Rect rect;
	GetWindowRect(hWnd,&rect);
	mPos[0] = rect.left;
	mPos[1] = rect.top;
	mPos[2] = rect.right-rect.left;
	mPos[3] = rect.bottom-rect.top;
	return true;
}

	
static INT_PTR CALLBACK WeatherFileDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	
	WeatherFileDialog *theDlg = DLGetWindowLongPtr<WeatherFileDialog*>( hWnd);

	Interval valid = FOREVER;
	
	switch (msg)
	{
	case WM_INITDIALOG:
		theDlg = (WeatherFileDialog *)lParam;
		DLSetWindowLongPtr( hWnd, theDlg);
		theDlg->InitDialog(hWnd);
		theDlg->RestoreWindowLoc(hWnd);
		break;
	
	case WM_COMMAND:

		// First check for ok or cancel.
		switch (LOWORD(wParam))
		{
		case IDOK:
			theDlg->StoreWindowLoc(hWnd);
			EndDialog(hWnd,1);
			break;

		case IDCANCEL:
			theDlg->StoreWindowLoc(hWnd);
			EndDialog(hWnd,0);
			break;
		case IDC_LOAD_WEATHER_DATA:
			theDlg->GetWeatherFile(hWnd);
			break;
		case IDC_CLEAR_WEATHER_FILE:
			theDlg->ClearWeatherFile(hWnd);
			break;

		case IDC_USE_DATE_TIME:
			{
				BOOL check = IsDlgButtonChecked(hWnd,IDC_USE_DATE_TIME);
				CheckDlgButton(hWnd,IDC_ANIMATION,!check);
				theDlg->mData.mAnimated = !check;
				ICustButton *setPeriod1 = GetICustButton(GetDlgItem(hWnd,IDC_SET_PERIOD));
				ICustButton *setPeriod2 = GetICustButton(GetDlgItem(hWnd,IDC_SET_PERIOD2));
				ICustButton *setPeriod3 = GetICustButton(GetDlgItem(hWnd,IDC_SET_PERIOD3));
				if(setPeriod1)
					setPeriod1->Enable(check);
				if(setPeriod2)
					setPeriod2->Enable(!check);
				if(setPeriod3)
					setPeriod3->Enable(!check);
				theDlg->SetTotalFrames(hWnd); //reset UI and stuff
				if(setPeriod1)
					ReleaseICustButton(setPeriod1);
				if(setPeriod2)
					ReleaseICustButton(setPeriod2);
				if(setPeriod3)
					ReleaseICustButton(setPeriod3);
			}
			break;
		case IDC_ANIMATION:
			{
				BOOL check = IsDlgButtonChecked(hWnd,IDC_ANIMATION);
				CheckDlgButton(hWnd,IDC_USE_DATE_TIME,!check);
				theDlg->mData.mAnimated = check;
				ICustButton *setPeriod1 = GetICustButton(GetDlgItem(hWnd,IDC_SET_PERIOD));
				ICustButton *setPeriod2 = GetICustButton(GetDlgItem(hWnd,IDC_SET_PERIOD2));
				ICustButton *setPeriod3 = GetICustButton(GetDlgItem(hWnd,IDC_SET_PERIOD3));
				if(setPeriod1)
					setPeriod1->Enable(!check);
				if(setPeriod2)
					setPeriod2->Enable(check);
				if(setPeriod3)
					setPeriod3->Enable(check);
				theDlg->SetTotalFrames(hWnd); //reset UI and stuff
				if(setPeriod1)
					ReleaseICustButton(setPeriod1);
				if(setPeriod2)
					ReleaseICustButton(setPeriod2);
				if(setPeriod3)
					ReleaseICustButton(setPeriod3);
			}
			break;

		case IDC_SKIP_HOURS:
			{ //need to handle spin control enable/disable.
				BOOL checked = IsDlgButtonChecked(hWnd,IDC_SKIP_HOURS);
				theDlg->mData.mFilter.mSkipHours = checked;
				ISpinnerControl *spin = GetISpinner(GetDlgItem(hWnd,IDC_SKIP_HOURS_START_SPIN));
				spin->Enable(checked);
				ReleaseISpinner(spin);
				spin = GetISpinner(GetDlgItem(hWnd,IDC_SKIP_HOURS_END_SPIN));
				spin->Enable(checked);
				ReleaseISpinner(spin);
				theDlg->SetTotalFrames(hWnd); //reset UI and stuff
			}
			break;

		case IDC_SKIP_WEEKENDS:
			theDlg->mData.mFilter.mSkipWeekends = IsDlgButtonChecked(hWnd,IDC_SKIP_WEEKENDS);
			theDlg->SetTotalFrames(hWnd); //reset UI and stuff
			break;
		case IDC_MATCH_TIMELINE:
			theDlg->MatchTimeLine();
			break;
		case IDC_SET_PERIOD:
			theDlg->SetPeriod(hWnd,theDlg->mData.mSpecificTimeMDY,theDlg->mData.mSpecificTimeHMS);
			theDlg->SetTotalFrames(hWnd);
			break;
		case IDC_SET_PERIOD2:
			theDlg->SetPeriod(hWnd,theDlg->mData.mAnimStartTimeMDY,theDlg->mData.mAnimStartTimeHMS);
			theDlg->SetTotalFrames(hWnd);
			break;
		case IDC_SET_PERIOD3:
			theDlg->SetPeriod(hWnd,theDlg->mData.mAnimEndTimeMDY,theDlg->mData.mAnimEndTimeHMS);
			theDlg->SetTotalFrames(hWnd);
			break;

		case IDC_ONE_FRAME:
			if (HIWORD(wParam) == CBN_SELENDOK) {
				int sel = SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0);
				theDlg->mData.mFilter.mFramePer = (WeatherCacheFilter::FramePer)sel;
				theDlg->SetTotalFrames(hWnd); //reset UI and stuff
			}
			break;

		}

		break;
		
	default:
		return 0;
	}	
	return 1;
}



void WeatherFileDialog::MatchTimeLine()
{
	int totalFrames = mWeatherFileCache.GetRealNumEntries(mData,TRUE);
	if(totalFrames>1)
	{
		TimeValue start  = GetCOREInterface10()->GetAutoKeyDefaultKeyTime();
		Interval range(start,start+ totalFrames*GetTicksPerFrame()
			-GetTicksPerFrame());
		GetCOREInterface()->SetAnimRange(range);
	}
}


void WeatherFileDialog::SetTotalFrames(HWND hDlg,BOOL valid)
{	
	ResetTimePeriods(hDlg,valid);
	int totalFrames =0;
	if(valid==TRUE)
	{
		totalFrames = mWeatherFileCache.GetRealNumEntries(mData,TRUE);
	}

	char str[32];
	sprintf(str,"%d",totalFrames);
	SetWindowText(GetDlgItem(hDlg,IDC_TOTAL_FRAMES),str);
	GetSpinnerValues(hDlg);

}



void WeatherFileDialog::ResetTimePeriods(HWND hDlg,BOOL valid)
{
	if(valid==TRUE)
	{
		
		mCurrentPeriod = 0;
		mIter = mWeatherFileCache.mDataPeriods[mCurrentPeriod]->begin();
		mWeatherFileCache.GetValidIter(mIter,mCurrentPeriod,
			mData.mSpecificTimeMDY,
			mData.mSpecificTimeHMS,
			mData.mFilter);
		mCurrentPeriod = 0;
		mIter = mWeatherFileCache.mDataPeriods[mCurrentPeriod]->begin();
		mWeatherFileCache.GetValidIter(mIter, mCurrentPeriod,
			mData.mAnimStartTimeMDY,
			mData.mAnimStartTimeHMS,
			mData.mFilter);
		

		//reset it to the last one!
		mCurrentPeriod = mWeatherFileCache.mDataPeriods.Count()-1;
		mIter =  mWeatherFileCache.mDataPeriods[mCurrentPeriod]->end();
		--mIter;
		mWeatherFileCache.GetValidIter(mIter, mCurrentPeriod,
			mData.mAnimEndTimeMDY,
			mData.mAnimEndTimeHMS,
			mData.mFilter);
		//now draw the windows correctly
		SetDateWindows(hDlg,valid);
	}
}


void WeatherFileDialog::SetPeriod(HWND hParent,Int3 &MDY,Int3 &HMS)
{
	WeatherFileDialog::SetPeriodData data;
	//reset the iterator to point here
	mWeatherFileCache.GetValidIter(mIter, mCurrentPeriod, MDY,HMS, 
		mData.mFilter);

	data.MDY = MDY;
	data.HMS = HMS;
	data.mDlg = this;

	if (DialogBoxParam(MaxSDK::GetHInstance(), MAKEINTRESOURCE(IDD_SET_PERIOD),
		hParent, SetPeriodDlgProc, (LPARAM)&data) == 1) 
	{
		MDY = data.MDY;
		HMS = data.HMS;
	}
}

void WeatherFileDialog::SetDaysInMonth(HWND hDlg,Int3 &MDY)
{

	int month = MDY.x;
	int modays;
	int which = mWeatherFileCache.mWeatherHeaderInfo.mLeapYearObserved == true ? 1 : 0;
	if (month == 12) modays = 31;
	else modays = GetMDays(which, month)-GetMDays(which, month-1);

	ISpinnerControl *spin = GetISpinner(GetDlgItem(hDlg,IDC_DAYS_SPIN));
	spin->SetScale(1.0f);
	spin->SetLimits(0,modays+1);
	spin->SetValue(MDY.y,FALSE);
	ReleaseISpinner(spin);

}

void WeatherFileDialog::SetPeriodInit(HWND hDlg, Int3 &MDY,Int3 &HMS)
{

	ISpinnerControl *spin = GetISpinner(GetDlgItem(hDlg,IDC_MONTHS_SPIN));
	spin->SetScale(1.0f);
	spin->SetLimits(0,13);
	spin->SetValue(MDY.x,FALSE);
	ReleaseISpinner(spin);

	SetDaysInMonth(hDlg,MDY);

	spin = GetISpinner(GetDlgItem(hDlg,IDC_HOURS_SPIN));
	spin->SetScale(1.0f);
	spin->SetLimits(-1,24);
	spin->SetValue(HMS.x,FALSE);
	ReleaseISpinner(spin);

	BOOL minutesOn = FALSE;
	float minuteStep = 1.0f;
	if(this->mWeatherFileCache.mWeatherHeaderInfo.mNumRecordsPerHour>1.0f)
	{
		minuteStep = 60.0f/(mWeatherFileCache.mWeatherHeaderInfo.mNumRecordsPerHour);
		minutesOn = TRUE;
	}

	spin = GetISpinner(GetDlgItem(hDlg,IDC_MINUTES_SPIN));
	spin->SetScale(1.0f);
	spin->SetLimits(-1,60);
	spin->SetValue(HMS.y,FALSE);
	spin->Enable(minutesOn);
	ReleaseISpinner(spin);


	TSTR timeString =  GetTimeString(HMS);
	TSTR dateString = GetDateString(MDY);
	TSTR str = dateString +TSTR("; ") + timeString;
	SetWindowText(GetDlgItem(hDlg,IDC_SELECTED_TIME_PERIOD),str.data());


	ISliderControl *pCustSlider = GetISlider(GetDlgItem(hDlg, IDC_TIME_SLIDER));
    if( pCustSlider )
	{
		int num =  mWeatherFileCache.GetRealNumEntries(mData,FALSE);
		int loc = mWeatherFileCache.FindNthEntry(MDY,HMS,mData.mFilter);
		pCustSlider->SetLimits(0, num, FALSE);
        pCustSlider->SetNumSegs(0);
		pCustSlider->SetValue(loc,FALSE);
        pCustSlider->SetResetValue(0);
        ReleaseISlider( pCustSlider );
   }

}

void WeatherFileDialog::CheckMonthsValue(HWND hDlg, Int3 &MDY,Int3 &HMS,BOOL checkIter)
{
	ISpinnerControl *spin = GetISpinner(GetDlgItem(hDlg,IDC_MONTHS_SPIN));
	int val = spin->GetIVal();
	if(val<1)
		val = 12;
	else if (val>12)
		val = 1;
	spin->SetValue(val,FALSE);
	ReleaseISpinner(spin);
	MDY.x = val;
	if(checkIter)
	{
		mWeatherFileCache.GetValidIter(mIter, mCurrentPeriod, MDY,HMS,
			mData.mFilter);
	}



	TSTR timeString =  GetTimeString(HMS);
	TSTR dateString = GetDateString(MDY);
	TSTR str = dateString +TSTR("; ") + timeString;
	SetWindowText(GetDlgItem(hDlg,IDC_SELECTED_TIME_PERIOD),str.data());
}

void WeatherFileDialog::CheckDaysValue(HWND hDlg, Int3 &MDY,Int3 &HMS,BOOL checkIter)
{
	ISpinnerControl *spin = GetISpinner(GetDlgItem(hDlg,IDC_DAYS_SPIN));
	int val = spin->GetIVal();
	//check it here.
	int month = MDY.x;
	int modays;
	int which = mWeatherFileCache.mWeatherHeaderInfo.mLeapYearObserved == true ? 1 : 0;
	if (month == 12) modays = 31;
	else modays = GetMDays(which, month)-GetMDays(which, month-1);
	if(val<1)
	{
		MDY.x -= 1;
		ISpinnerControl *newSpin = GetISpinner(GetDlgItem(hDlg,IDC_MONTHS_SPIN));
		newSpin->SetValue(MDY.x,FALSE);
		ReleaseISpinner(newSpin);
		CheckMonthsValue(hDlg,MDY,HMS,FALSE);
		int month = MDY.x;
		if (month == 12) modays = 31;
		else modays = GetMDays(which, month)-GetMDays(which, month-1);
		val = modays;//end day!
		spin->SetValue(val,FALSE);
	}
	else if(val>modays)
	{
		MDY.x += 1;
		ISpinnerControl *newSpin = GetISpinner(GetDlgItem(hDlg,IDC_MONTHS_SPIN));
		newSpin->SetValue(MDY.x,FALSE);
		ReleaseISpinner(newSpin);
		CheckMonthsValue(hDlg,MDY,HMS,FALSE);
		val = 1;
		spin->SetValue(val,FALSE);
	}
	MDY.y = val;
	ReleaseISpinner(spin);
	if(checkIter)
	{
		mWeatherFileCache.GetValidIter(mIter, mCurrentPeriod, MDY,HMS,
			mData.mFilter);
	}


	TSTR timeString =  GetTimeString(HMS);
	TSTR dateString = GetDateString(MDY);
	TSTR str = dateString +TSTR("; ") + timeString;
	SetWindowText(GetDlgItem(hDlg,IDC_SELECTED_TIME_PERIOD),str.data());
}

void WeatherFileDialog::CheckHoursValue(HWND hDlg, Int3 &MDY,Int3 &HMS,BOOL checkIter)
{
	ISpinnerControl *spin = GetISpinner(GetDlgItem(hDlg,IDC_HOURS_SPIN));
	int val = spin->GetIVal();
	//check it here.
	if(val<0)
	{
		MDY.y = -1;
		ISpinnerControl *newSpin = GetISpinner(GetDlgItem(hDlg,IDC_DAYS_SPIN));
		newSpin->SetValue(MDY.y,FALSE);
		ReleaseISpinner(newSpin);
		CheckDaysValue(hDlg,MDY,HMS,FALSE);
		val = 23;
		spin->SetValue(val,FALSE);
	}
	else if(val>23)
	{
		MDY.y += 1;
		ISpinnerControl *newSpin = GetISpinner(GetDlgItem(hDlg,IDC_DAYS_SPIN));
		newSpin->SetValue(MDY.y,FALSE);
		ReleaseISpinner(newSpin);
		CheckDaysValue(hDlg,MDY,HMS,FALSE);
		val = 0;
		spin->SetValue(val,FALSE);
	}
	HMS.x = val;
	if(checkIter)
	{
		mWeatherFileCache.GetValidIter(mIter, mCurrentPeriod, MDY,HMS,
			mData.mFilter);
	}
	ReleaseISpinner(spin);

	TSTR timeString =  GetTimeString(HMS);
	TSTR dateString = GetDateString(MDY);
	TSTR str = dateString +TSTR("; ") + timeString;
	SetWindowText(GetDlgItem(hDlg,IDC_SELECTED_TIME_PERIOD),str.data());
}


void WeatherFileDialog::CheckMinutesValue(HWND hDlg, Int3 &MDY,Int3 &HMS,BOOL checkIter)
{
	ISpinnerControl *spin = GetISpinner(GetDlgItem(hDlg,IDC_MINUTES_SPIN));
	int val = spin->GetIVal();
	spin->SetLimits(-1,60);
	if(val<0)
	{	
		HMS.x -= 1;
		ISpinnerControl *newSpin = GetISpinner(GetDlgItem(hDlg,IDC_HOURS_SPIN));
		newSpin->SetValue(HMS.x,FALSE);
		ReleaseISpinner(newSpin);

		CheckHoursValue(hDlg,MDY,HMS,FALSE);
		val = 59;
		spin->SetValue(val,FALSE);
	}
	else if (val>59.0f)
	{
		HMS.x += 1;
		ISpinnerControl *newSpin = GetISpinner(GetDlgItem(hDlg,IDC_HOURS_SPIN));
		newSpin->SetValue(HMS.x,FALSE);
		ReleaseISpinner(newSpin);

		CheckHoursValue(hDlg,MDY,HMS,FALSE);
		val = 0;
		spin->SetValue(val,FALSE);
	}
	//check it here.
	HMS.y= val;
	if(checkIter)
	{
		mWeatherFileCache.GetValidIter(mIter, mCurrentPeriod, MDY,HMS,
			mData.mFilter);
	}
	ReleaseISpinner(spin);

	TSTR timeString =  GetTimeString(HMS);
	TSTR dateString = GetDateString(MDY);
	TSTR str = dateString +TSTR("; ") + timeString;
	SetWindowText(GetDlgItem(hDlg,IDC_SELECTED_TIME_PERIOD),str.data());
}

 void WeatherFileDialog::SetSliderValue(int value,HWND hDlg, Int3 &MDY,Int3 &HMS)
 {
	DaylightWeatherDataEntries::iterator entry =  mWeatherFileCache.GetFilteredNthEntry(value,mData.mFilter);
	if(entry!= mWeatherFileCache.mDataPeriods[0]->end())
	{
		MDY.x = entry->mMonth;
		MDY.y = entry->mDay;
		MDY.z = entry->mYear;
		HMS.x = entry->mHour;
		HMS.y = entry->mMinute;
		HMS.z = 0.0f;
		ISpinnerControl *spin = GetISpinner(GetDlgItem(hDlg,IDC_MONTHS_SPIN));
		spin->SetValue(MDY.x,FALSE);
		ReleaseISpinner(spin);
		spin = GetISpinner(GetDlgItem(hDlg,IDC_DAYS_SPIN));
		spin->SetValue(MDY.y,FALSE);
		ReleaseISpinner(spin);
		spin = GetISpinner(GetDlgItem(hDlg,IDC_HOURS_SPIN));
		spin->SetValue(HMS.x,FALSE);
		ReleaseISpinner(spin);
		spin = GetISpinner(GetDlgItem(hDlg,IDC_MINUTES_SPIN));
		spin->SetValue(HMS.y,FALSE);
		ReleaseISpinner(spin);
		TSTR timeString =  GetTimeString(HMS);
		TSTR dateString = GetDateString(MDY);
		TSTR str = dateString +TSTR("; ") + timeString;
		SetWindowText(GetDlgItem(hDlg,IDC_SELECTED_TIME_PERIOD),str.data());

	}
 }

 void WeatherFileDialog::SpinValuesSet(HWND hDlg, Int3 &MDY,Int3 &HMS)
 {
	ISliderControl *pCustSlider = GetISlider(GetDlgItem(hDlg, IDC_TIME_SLIDER));
	if( pCustSlider )
	{
		int loc = mWeatherFileCache.FindNthEntry(MDY,HMS,mData.mFilter);
		pCustSlider->SetValue(loc,FALSE);
		ReleaseISlider( pCustSlider );
	}
	//and set the spinners they may have gotten changed.
	ISpinnerControl *spin = GetISpinner(GetDlgItem(hDlg,IDC_MONTHS_SPIN));
	spin->SetValue(MDY.x,FALSE);
	ReleaseISpinner(spin);
	spin = GetISpinner(GetDlgItem(hDlg,IDC_DAYS_SPIN));
	spin->SetValue(MDY.y,FALSE);
	ReleaseISpinner(spin);
	spin = GetISpinner(GetDlgItem(hDlg,IDC_HOURS_SPIN));
	spin->SetValue(HMS.x,FALSE);
	ReleaseISpinner(spin);
	spin = GetISpinner(GetDlgItem(hDlg,IDC_MINUTES_SPIN));
	spin->SetValue(HMS.y,FALSE);
	ReleaseISpinner(spin);
	

 }



static INT_PTR CALLBACK SetPeriodDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	
	WeatherFileDialog::SetPeriodData *theData = DLGetWindowLongPtr<WeatherFileDialog::SetPeriodData*>( hWnd);

	Interval valid = FOREVER;
	
	switch (msg)
	{
	case WM_INITDIALOG:
		theData= (WeatherFileDialog::SetPeriodData *)lParam;
		DLSetWindowLongPtr( hWnd, theData);
		theData->mDlg->SetPeriodInit(hWnd,theData->MDY,theData->HMS);
//		CenterWindow(hWnd,GetParent(hWnd));
		break;

	case CC_SPINNER_CHANGE:
		switch (LOWORD(wParam))
		{
		case IDC_MONTHS_SPIN:
			theData->mDlg->CheckMonthsValue(hWnd,theData->MDY,theData->HMS);
			theData->mDlg->SpinValuesSet(hWnd,theData->MDY,theData->HMS);
			break;
		case IDC_DAYS_SPIN:
			theData->mDlg->CheckDaysValue(hWnd,theData->MDY,theData->HMS);
			theData->mDlg->SpinValuesSet(hWnd,theData->MDY,theData->HMS);
			break;
		case IDC_HOURS_SPIN:
			theData->mDlg->CheckHoursValue(hWnd,theData->MDY,theData->HMS);
			theData->mDlg->SpinValuesSet(hWnd,theData->MDY,theData->HMS);
			break;
		case IDC_MINUTES_SPIN:
			theData->mDlg->CheckMinutesValue(hWnd,theData->MDY,theData->HMS);
			theData->mDlg->SpinValuesSet(hWnd,theData->MDY,theData->HMS);
			break;
		}
		break;
	 case CC_SLIDER_CHANGE:
	      {
			ISliderControl *pCustSlider = GetISlider(GetDlgItem(hWnd, IDC_TIME_SLIDER));
			if( pCustSlider )
			{
				int location = pCustSlider->GetIVal();
				theData->mDlg->SetSliderValue(location,hWnd,theData->MDY,theData->HMS);
			    ReleaseISlider( pCustSlider );
			}
		  }
      break;

	case WM_COMMAND:

		// First check for ok or cancel.
		switch (LOWORD(wParam))
		{
		case IDOK:
			EndDialog(hWnd,1);
			break;
		case IDCANCEL:
			EndDialog(hWnd,0);
			break;
		}
		break;
	
	default:
		return 0;
	}	
	return 1;
}

