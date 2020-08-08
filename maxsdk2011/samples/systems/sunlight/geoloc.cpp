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
	FILE: geoloc.cpp

	DESCRIPTION:  Geographic Locator Dialog

	CREATED BY: Jack Lieberman

	HISTORY: created 9 January, 1996

 *>	Copyright (c) 19945, 1996, All Rights Reserved.
 **********************************************************************/

#pragma unmanaged
#include "NativeInclude.h"

#pragma managed
#include "sunlight.h"
#include "citylist.h"

// Geographic locator dialog stuff.
struct MapTable 
{
	int index;
	char *filename;
};

static MapTable mapTable[] = {
	IDS_ASIAN_SUBCONTINENT,	"india.map",
	IDS_AFRICA,				"africa.map",
	IDS_ASIA,				"asia.map",
	IDS_AUSTRALIA,			"aust.map",
	IDS_EUROPE,				"europe.map",
	IDS_NORTH_AMERICA,		"namer.map",
	IDS_SOUTH_AMERICA,		"samer.map",
	IDS_CANADA,				"canada.map",
	IDS_WORLD,				"world.map"
};

#define ASIAN_SUBCONTINENT	0
#define AFRICA				1
#define ASIA				2
#define AUSTRALIA			3
#define EUROPE				4
#define NORTH_AMERICA		5
#define SOUTH_AMERICA		6
#define CANADA				7
#define WORLD				8

#define LAST_MAP_NAME  8

/*static*/ int lastCity = -1;	// reset to -1 if param spinners are used
char lastCityName[64] = "";

// Forward declarations.
class LocationDialog;


// Subclass proc for map owner draw button.
LRESULT CALLBACK MapProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

// The dialog proc.
INT_PTR CALLBACK GetLocationDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

class LocationDialog
{
	friend INT_PTR CALLBACK GetLocationDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	public:
		LocationDialog();
		void initMapList(HWND h);
		void initCityList();                
		void chooseCity(HWND hDlg, int index);
		void displayMap(HWND hDlg, HWND hButton);
		void changeMap(HWND hDlg, HWND mapWnd);
		int mapClick(HWND hDlg, HWND hButton);

		int mapIndex;       // Index of current map in list.
		double updatedLatitude, updatedLongitude;

		LocationDialog* theDlg;				// THE dialog.
		IObjParam* theInterface;			// Interface pointer to MAX.
		float theLatitude;					// Incoming data.
		float theLongitude;					// Incoming data.
		WNDPROC oldMapProc;
		
	private:

		void displayCross(HWND hDlg, double latitude, double longitude);
		BOOL loadCityList(HWND hDlg);
		void imageToLatLong(double x, double y, double *latitude, double *longitude);
		HANDLE openMapFile(char* mapName);
		
		int isBig[2000];	// Note this must be increased for more cities.
		int cityIndex;      // Index of city in list box.
		double minLatitude, minLongitude, maxLatitude, maxLongitude;
		double crossLatitude, crossLongitude;
		BOOL crossVisible;
		short mapWidth, mapHeight;
		double mapLongWidth, mapLatHeight;
		double scale, xBias, yBias;
};

BOOL doLocationDialog(HWND hParent, IObjParam* ip, float* latitude, float* longitude,
						char* cityName)
// Do the dialog.
{
	LocationDialog dlg;
	dlg.theInterface = ip;
	dlg.theLatitude = *latitude;
	dlg.theLongitude = *longitude;
	
	if (DialogBoxParam(MaxSDK::GetHInstance(), MAKEINTRESOURCE(IDD_GEOGRAPHIC_LOCATION),
		hParent, GetLocationDlgProc, (LPARAM)&dlg) == 1) 
	{
		*latitude = (float)dlg.updatedLatitude;
		*longitude = (float)dlg.updatedLongitude;
		if (lastCity >= 0) 
		{
			strcpy(cityName, lastCityName);
		}
		return TRUE;
	}

	return FALSE;
}

LocationDialog::LocationDialog()
{
	cityIndex = -1;
	mapIndex = -1;
	crossVisible = FALSE;
	scale = 100.;
	xBias = -2.6;
	yBias = .5;
}


void LocationDialog::initCityList()
{
	CityList::GetInstance().init();
}

void LocationDialog::initMapList(HWND h)
{
	// Add map names to combo box.
	for (UINT i=0; i<=LAST_MAP_NAME; i++)
	{
		const TCHAR* mapName = MaxSDK::GetResourceString(mapTable[i].index);
		if(nullptr != mapName)
		{
			SendMessage(h, CB_ADDSTRING, 0, (LPARAM)mapName);
		}
	}

	// Set current map
	// provide a sensible default.

	WORD languageID = PRIMARYLANGID(MaxSDK::Util::GetLanguageID());

	// Check if an Asia-Pacific language is being used
	if (languageID == LANG_JAPANESE || languageID == LANG_CHINESE || languageID == LANG_KOREAN)
	{
		// ASIA
		mapIndex = ASIA;
	}
	else if (languageID == LANG_FRENCH || languageID == LANG_GERMAN)
	{
		// EUROPE: French or German is being used
		mapIndex = EUROPE;
	}
	else
	{
		// NORTH AMERICA: English is being used
		mapIndex = NORTH_AMERICA;
	}

	// make sure that we have initialised the cityList
	CityList::GetInstance().init();

	// try and calculate the map we need based on the default city...
	float defaultCityLatitude = theLatitude;
	float defaultCityLongitude = theLongitude;
	CityList::Entry * defaultCity = CityList::GetInstance().GetDefaultCity();
	if (defaultCity != NULL)
	{
		defaultCityLatitude = defaultCity->latitude;
		defaultCityLongitude = defaultCity->longitude;
	} 

	if (defaultCityLatitude != 0.0f && defaultCityLongitude != 0.0f)
	{
		for (int j=0; j<=LAST_MAP_NAME; ++j)
		{
			HANDLE hMap = openMapFile(mapTable[j].filename);
			if (hMap != INVALID_HANDLE_VALUE)
			{
				// Get the control dimensions.
				RECT mapRect;
				GetWindowRect(GetDlgItem(GetParent(h), IDC_MAP_PICTURE), &mapRect);

				// Read the map file header and record values.
				DWORD bytesRead = 0;
				double latitude = 0.0, longitude = 0.0, width = 0.0;
				if (ReadFile(hMap, &latitude, sizeof(double), &bytesRead, NULL) == FALSE
					|| bytesRead == 0) 
				{
					CloseHandle(hMap);
					break;
				}
				if (ReadFile(hMap, &longitude, sizeof(double), &bytesRead, NULL) == FALSE
					|| bytesRead == 0) 
				{
					CloseHandle(hMap);
					break;
				}
				if (ReadFile(hMap, &width, sizeof(double), &bytesRead, NULL) == FALSE
					|| bytesRead == 0) 
				{
					CloseHandle(hMap);
					break;
				}
				double aspectRatio = ((double)(mapRect.bottom - mapRect.top))/((double)mapRect.right - mapRect.left);
				double height = width * aspectRatio;

				if (  (defaultCityLongitude > (longitude - width / 2.)) //signs reversed
					&&(defaultCityLongitude < (longitude + width / 2.)) //signs reversed
					&&( defaultCityLatitude > (latitude - height / 2.))
					&&( defaultCityLatitude < (latitude + height / 2.)))
				{
					mapIndex = j;
					CloseHandle(hMap);
					break;
				}
				CloseHandle(hMap);
			}
		}
	}
	SendMessage(h, CB_SETCURSEL, (WPARAM)mapIndex, 0);
}

#define CROSS_SIZE 10
#define CROSS_COLOR (RGB(0, 255, 0))
void LocationDialog::displayCross(HWND hDlg, double latitude, double longitude)
{
	short xImage = 0, yImage = 0;
	HWND hMap = GetDlgItem(hDlg, IDC_MAP_PICTURE);

	HDC hDC = GetDC(hMap);
	if (hDC == NULL)
	{
		return;
	}
	int oRop = SetROP2(hDC, R2_XORPEN);
	HPEN pen = CreatePen(PS_SOLID, 3, CROSS_COLOR);
	HPEN oPen = (HPEN)SelectObject(hDC, pen);

	HRGN hrgn = CreateRectRgn(1, 1, mapWidth, mapHeight - 1);
	if (hrgn != NULL)
	{
		SelectClipRgn(hDC, hrgn);
		DeleteObject(hrgn);
	}
								 
	// calculation of x-coordinate differs in sign from original map,
	//   which uses different convention for longitude.
	xImage = (int)((longitude-minLongitude)*mapWidth/mapLongWidth);
	yImage = (int)((latitude-minLatitude)*mapHeight/mapLatHeight);
	if (crossVisible)
	{
		short xOld = 0, yOld = 0;
		
		xOld = (int) ((crossLongitude-minLongitude)*mapWidth/mapLongWidth);
		yOld = (int) ((crossLatitude-minLatitude)*mapHeight/mapLatHeight);

		// erase.
		MoveToEx(hDC, xOld-CROSS_SIZE, mapHeight-yOld, NULL);
		LineTo(hDC, xOld+CROSS_SIZE, mapHeight-yOld);
		MoveToEx(hDC, xOld, mapHeight-yOld-CROSS_SIZE, NULL);
		LineTo(hDC, xOld,mapHeight-yOld+CROSS_SIZE);
	}
	MoveToEx(hDC, xImage-CROSS_SIZE, mapHeight-yImage, NULL);
	LineTo(hDC, xImage+CROSS_SIZE, mapHeight-yImage);
	MoveToEx(hDC, xImage, mapHeight-yImage-CROSS_SIZE, NULL);
	LineTo(hDC, xImage, mapHeight-yImage+CROSS_SIZE);
	crossVisible = TRUE;
	
	crossLatitude = latitude;
	crossLongitude = longitude;
	
	SetROP2(hDC, oRop);
	SelectObject(hDC, oPen);
	DeleteObject(pen);
	
	ReleaseDC(hMap, hDC);    
}

void LocationDialog::chooseCity(HWND hDlg, int index)
// Handle a change of city.
{
	double latitude = 0.0, longitude = 0.0;

	// Get the cityList index of the city chosen.
	int itemIndex = SendMessage(GetDlgItem(hDlg, IDC_CITYLIST),
								LB_GETITEMDATA, index, 0);

	// Find that city's lat and long.
	latitude = CityList::GetInstance()[itemIndex].latitude;
	longitude = CityList::GetInstance()[itemIndex].longitude;

	// Get that city's name, too.
	strcpy(lastCityName, CityList::GetInstance()[itemIndex].name);
	
	updatedLatitude = latitude;
	updatedLongitude = longitude;
	displayCross(hDlg, latitude, longitude);
	lastCity = cityIndex = index;
}

#define SGEOLOC _T("\\GeoLoc\\")
#define GEOLOC _T("GeoLoc\\")

// Fetch the map file full path name.
HANDLE LocationDialog::openMapFile(char* mapName)
{
	HANDLE hMap = NULL;
	TCHAR fileName[MAX_PATH] = {0};
	int len = 0;

	// Get subdir containing this plugin.
	GetModuleFileName(MaxSDK::GetHInstance(), fileName, MAX_PATH);

	// Append subdir string containing maps.
	TCHAR *slash = _tcsrchr(fileName, '\\');
	if (slash)
	{
		_tcscpy(slash, SGEOLOC);
	}
	else
	{
		// If the plugin filename contains no path information, look in plugcfg.
		_tcscpy(fileName,theInterface->GetDir(APP_PLUGCFG_DIR));
		len = static_cast<int>(_tcslen(fileName));   // SR DCAST64: Downcast to 2G limit.
		if (len)
		{
			if (_tcscmp(&fileName[len-1],_T("\\")))
			{
				_tcscat(fileName,_T("\\"));
			}
		}
	}

	_tcscat(fileName,mapName);
	hMap = CreateFile(fileName, GENERIC_READ, FILE_SHARE_READ, NULL,
						OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hMap == INVALID_HANDLE_VALUE)
	{
		// If that fails, look in the MaxStart dir.
		_tcscpy(fileName,theInterface->GetDir(APP_MAXSTART_DIR));
		len = static_cast<int>(_tcslen(fileName));   // SR DCAST64: Downcast to 2G limit.
		if (len)
		{
			if (_tcscmp(&fileName[len-1],_T("\\")))
			{
				_tcscat(fileName,_T("\\"));
			}
			_tcscat(fileName, GEOLOC);
		}   
		_tcscat(fileName,mapName);
		hMap = CreateFile(fileName, GENERIC_READ, FILE_SHARE_READ, NULL,
							OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
		if (hMap == INVALID_HANDLE_VALUE)
		{
			// If THAT fails, look beneath the main exe dir.
			_tcscpy(fileName,theInterface->GetDir(APP_MAX_SYS_ROOT_DIR));
			len = static_cast<int>(_tcslen(fileName));   // SR DCAST64: Downcast to 2G limit.
			if (len)
			{
				if (_tcscmp(&fileName[len-1],_T("\\")))
				{
					_tcscat(fileName,_T("\\"));
				}
			}
			_tcscat(fileName, GEOLOC);
			_tcscat(fileName,mapName);
			hMap = CreateFile(fileName, GENERIC_READ, FILE_SHARE_READ, NULL,
							OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
			if (hMap == INVALID_HANDLE_VALUE)
			{
				// Last try: If THAT fails, look IN the main exe dir.
				_tcscpy(fileName,theInterface->GetDir(APP_MAX_SYS_ROOT_DIR));
				len = static_cast<int>(_tcslen(fileName));   // SR DCAST64: Downcast to 2G limit.
				if (len)
				{
					if (_tcscmp(&fileName[len-1],_T("\\")))
					{
						_tcscat(fileName,_T("\\"));
					}
				}
				_tcscat(fileName,mapName);
				hMap = CreateFile(fileName, GENERIC_READ, FILE_SHARE_READ, NULL,
								  OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
			}
		}
	}

	return hMap;
}



void LocationDialog::displayMap(HWND hDlg, HWND hButton)
// Draw the current map.
{
	double latitude = 0.0, longitude = 0.0, width = 0.0, height = 0.0;
	double aspectRatio = 0.0;
	double x1 = 0.0, x2 = 0.0, y1 = 0.0, y2 = 0.0;
	
	minLongitude = -180.;
	minLatitude = -90.;
	maxLongitude = 180.;
	maxLatitude = 90.;

	// Get the map file name.
	char mapName[32] = {0};
	lstrcpy(mapName, mapTable[mapIndex].filename);
   
	HANDLE hMap = openMapFile(mapName);
	if (hMap == INVALID_HANDLE_VALUE)
	{
		return;
	}

	// Get the control dimensions.
	RECT mapRect;
	GetWindowRect(GetDlgItem(hDlg, IDC_MAP_PICTURE), &mapRect);
	mapWidth = (short)(mapRect.right - mapRect.left);
	mapHeight = (short)(mapRect.bottom - mapRect.top);    

	// Read the map file header and record values.
	DWORD bytesRead = 0;
	if (ReadFile(hMap, &latitude, sizeof(double), &bytesRead, NULL) == FALSE
		|| bytesRead == 0)
	{
		CloseHandle(hMap);
		return;
	}
	if (ReadFile(hMap, &longitude, sizeof(double), &bytesRead, NULL) == FALSE
		|| bytesRead == 0)
	{
		CloseHandle(hMap);
		return;
	}
	if (ReadFile(hMap, &width, sizeof(double), &bytesRead, NULL) == FALSE
		|| bytesRead == 0)
	{
		CloseHandle(hMap);
		return;
	}
	aspectRatio = ((double)mapHeight)/((double)mapWidth);
	height = width * aspectRatio;
	minLongitude = longitude - width / 2.;
	minLatitude = latitude - height / 2.;
	maxLongitude = longitude + width / 2.;
	maxLatitude = latitude + height / 2.;

	crossVisible = FALSE;

	HDC hDC = GetDC(hDlg);
	COLORREF bkColor = GetPixel(hDC, 2, 2);
	ReleaseDC(hDlg, hDC);
	
	// Read the rest of the file and draw.
	hDC = GetDC(hButton);    
	if (hDC != NULL)
	{
		RECT rect;
		rect.left = rect.top = 0;
		rect.right = mapWidth;
		rect.bottom = mapHeight;        
		HBRUSH bkBrush = CreateSolidBrush(bkColor);
		HBRUSH oBrush = (HBRUSH)SelectObject(hDC, bkBrush);
		GetClientRect(hButton, &mapRect);
		Rectangle(hDC, mapRect.left - 1, mapRect.top, mapRect.right + 2
			, mapRect.bottom);
		while (TRUE)
		{
			if (ReadFile(hMap, &x1, sizeof(double), &bytesRead, NULL) == FALSE
				|| bytesRead == 0)
			{
				break;
			}
			if (ReadFile(hMap, &y1, sizeof(double), &bytesRead, NULL) == FALSE
				|| bytesRead == 0)
			{
				break;
			}
			if (ReadFile(hMap, &x2, sizeof(double), &bytesRead, NULL) == FALSE
				|| bytesRead == 0)
			{
				break;
			}
			if (ReadFile(hMap, &y2, sizeof(double), &bytesRead, NULL) == FALSE
				|| bytesRead == 0)
			{
				break;
			}

			// only if they have the same sign
			if (x1 * x2 > 0.0)
			{
				short mx = 0, my = 0, lx = 0, ly = 0;
				mx = (short)((x1 - minLongitude) * mapWidth / width);
				my = (short)(mapHeight - ((y1 - minLatitude) * mapHeight / height));
#ifdef DEBUG
				if (mx > mapWidth)
				{
					break;//mx = mapWidth;
				}
				if (mx < 0)
				{
					break;//mx = 0;
				}
				if (my > mapHeight)
				{
					break;//my = mapHeight;
				}
				if (my < 0)
				{
					break;//my = 0;
				}
#endif

				lx = (short)((x2 - minLongitude) * mapWidth / width);
				ly = (short)(mapHeight - ((y2 - minLatitude) * mapHeight / height));
#ifdef DEBUG
				if (lx > mapWidth)
				{
					break;//lx = mapWidth;
				}
				if (lx < 0)
				{
					break;//lx = 0;
				}
				if (ly > mapHeight)
				{
					break;//ly = mapHeight;
				}
				if (ly < 0)
				{
					break;//ly = 0;
				}
#endif

				MoveToEx(hDC, mx, my, NULL);
				LineTo(hDC, lx, ly);
			}
		}
		SelectObject(hDC, oBrush);
		DeleteObject(bkBrush);
		ReleaseDC(hButton, hDC);
	}
	CloseHandle(hMap);

	// calculate for imageToLatLong.
	xBias = longitude - (width/2.);
	yBias = latitude - ((width/2.)*((double)mapHeight/(double)mapWidth));
	scale = (double) mapWidth / width;
		
	mapLongWidth = width;
	mapLatHeight = height;
}

BOOL LocationDialog::loadCityList(HWND hDlg)
// Update the list of cities.
{
	CityList::Entry* cl = NULL;
	UINT numCities = CityList::GetInstance().Count();    
	HWND h = GetDlgItem(hDlg, IDC_CITYLIST);
	int index = -1;

	// Empty the list box and add only those cities in the current map.
	SendMessage(h, LB_RESETCONTENT, 0, 0);
	for (UINT i=0; i<numCities; i++)
	{
		cl = &(CityList::GetInstance()[i]);
		if ((double)cl->latitude > minLatitude && 
			(double)cl->latitude < maxLatitude &&
			(double)cl->longitude > minLongitude && /* signs are */
			(double)cl->longitude < maxLongitude)   /* are no longer reversed  */
		{ 

			if ((cl->name[0] == '+') || (cl->name[0] == '*')) 
			{
				isBig[i] = TRUE;
				index = SendMessage(h, LB_ADDSTRING, 0, (LPARAM)(cl->name+1));
			}
			else
			{
				isBig[i] = FALSE;                
				index = SendMessage(h, LB_ADDSTRING, 0, (LPARAM)(cl->name));
			}

			// Store city list index in item data.
			SendMessage(h, LB_SETITEMDATA, index, (LPARAM)i);

			// Check if the current cl in the cityList is the one to be 
			// highlighted in the Geographic Location dialog. ---Fix defect 888888
			if(cl->latitude == theLatitude &&
			   cl->longitude == theLongitude)
			{
				lastCity = cityIndex = index;
			}
		}
	}

	if (lastCity >= 0)
	{
		SendMessage(h, LB_SETCURSEL, (WPARAM)lastCity, 0);    
		chooseCity(hDlg, lastCity);
	}

	return TRUE;
}

void LocationDialog::changeMap(HWND hDlg, HWND mapWnd)
// Draw a new map and change the cities list to accomodate.
{
	displayMap(hDlg, mapWnd);
	loadCityList(hDlg);
}

void LocationDialog::imageToLatLong(double x, double y, double *latitude
	, double *longitude)
{
	*longitude = x / scale + xBias;
	*latitude = (mapHeight - y) / scale + yBias;
}

int LocationDialog::mapClick(HWND hDlg, HWND hButton)
{
	CityList::Entry* cl = NULL;
	
	// Get the cursor position in client coordinates.
	POINT curPos;    
	GetCursorPos(&curPos);
	ScreenToClient(hButton, &curPos);

	double latitude = 0.0, longitude = 0.0;
	imageToLatLong((double)curPos.x, (double)curPos.y, &latitude, &longitude);
	longitude = longitude;   // no longer reversed .
	if (latitude<minLatitude ||
		latitude>maxLatitude ||
		longitude<minLongitude || /* signs are */
		longitude>maxLongitude ) /* no longer reversed  */
	{
		return 0;
	}

	UINT numCities = CityList::GetInstance().Count();
	HWND list = GetDlgItem(hDlg, IDC_CITYLIST);

	// get nearest big city.
	if (IsDlgButtonChecked(hDlg, IDC_NEAREST) != 0)
	{ 
		double distance = 360.0/* large value */, tempDistance = 0.0; 
		UINT i = 0, closestCity = -1; 
		bool foundClosest = false;
		for (i=0; i<numCities; ++i)
		{
			cl = &(CityList::GetInstance()[i]);            
			if (cl->latitude>minLatitude &&
				cl->latitude<maxLatitude &&
				cl->longitude>minLongitude && /* signs are */
				cl->longitude<maxLongitude && /* no longer reversed  */
				isBig[i])
			{
				tempDistance = sqrt(
				 (cl->latitude-latitude)*
				 (cl->latitude-latitude)+
				 (cl->longitude-longitude)*
				 (cl->longitude-longitude));

				if (tempDistance < distance)
				{
					distance = tempDistance;
					closestCity = i;
					foundClosest = true;
				}
			}
		}
		if (foundClosest)
		{
			updatedLatitude = (double)CityList::GetInstance()[closestCity].latitude;
			updatedLongitude = (double)CityList::GetInstance()[closestCity].longitude;
			strcpy(lastCityName, CityList::GetInstance()[closestCity].name);

			// Select the city in the list box.
			int count = SendMessage(list, LB_GETCOUNT, 0, 0);
			for (int j = 0; j<count; j++)
			{
				if (SendMessage(list, LB_GETITEMDATA, j, 0) == closestCity)
				{
					SendMessage(list, LB_SETCURSEL, (WPARAM)j, 0);
					lastCity = cityIndex = j;
				}
			}
			
			displayCross(hDlg,
				(double)updatedLatitude, (double)updatedLongitude);
		}
	}
	else
	{
		updatedLatitude = latitude;
		updatedLongitude = longitude;
		*lastCityName = '\0';

		// Select no city in the list box.
		SendMessage(list, LB_SETCURSEL, (WPARAM)-1, 0);
		lastCity = cityIndex = -1;
		displayCross(hDlg, latitude, longitude);
	}
	return 0;
}

static INT_PTR CALLBACK GetLocationDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LocationDialog *theDlg = DLGetWindowLongPtr<LocationDialog *>( hWnd);

	Interval valid = FOREVER;
	HWND hMap = GetDlgItem(hWnd, IDC_MAP_PICTURE);
	
	
	switch (msg)
	{
	case WM_INITDIALOG:
		theDlg = (LocationDialog *)lParam;
		DLSetWindowLongPtr( hWnd, theDlg);

		// Set the initial lat and long.
		theDlg->updatedLatitude = (double)theDlg->theLatitude;
		theDlg->updatedLongitude = (double)theDlg->theLongitude;

		// Subclass the map owner draw button.
		theDlg->oldMapProc = DLGetWindowProc(hMap);
		DLSetWindowLongPtr(hMap, MapProc);
		DLSetWindowLongPtr(hMap, theDlg);

		// Initialize items.
		theDlg->initCityList();
		theDlg->initMapList(GetDlgItem(hWnd, IDC_MAP_NAME));
		theDlg->changeMap(hWnd, hMap);
		CenterWindow(hWnd,GetParent(hWnd));

#ifdef DAYLIGHT_CITY_PICKER_BIG_CITY_DEFAULT
		SetCheckBox(hWnd, IDC_NEAREST, true);
#endif // DAYLIGHT_CITY_PICKER_BIG_CITY_DEFAULT

		break;

	case WM_DRAWITEM:
		theDlg->displayMap(hWnd, ((LPDRAWITEMSTRUCT)lParam)->hwndItem);
		theDlg->displayCross(hWnd, theDlg->theLatitude, theDlg->theLongitude);
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

		// Then check for notifications.
		switch(HIWORD(wParam))
		{
		case LBN_SELCHANGE:
			 // User chose a city from the list box or map from combo box.
			if (GetDlgItem(hWnd, IDC_CITYLIST) == (HWND)lParam)
			{
				theDlg->chooseCity(hWnd, SendMessage((HWND)lParam, LB_GETCURSEL,0,0));
			}
			else
			{
				int sel = SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0);
				if (theDlg->mapIndex != sel)
				{
					theDlg->mapIndex = sel;
					lastCity = 0;
					theDlg->changeMap(hWnd, GetDlgItem(hWnd, IDC_MAP_PICTURE));
				}
			}
			break;

		case LBN_DBLCLK:
			// User may have double-clicked a city; if so, accept it and end dialog.
			if (GetDlgItem(hWnd, IDC_CITYLIST) == (HWND)lParam)
			{
				theDlg->chooseCity(hWnd, SendMessage((HWND)lParam, LB_GETCURSEL,0,0));
				EndDialog(hWnd,1);
			}
			break;
		default:
			break;
		}
		break;
		
	default:
		return 0;
	}
	
	return 1;
}

LRESULT CALLBACK MapProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
// Handle a message for map window.
{
	LocationDialog *theDlg = DLGetWindowLongPtr<LocationDialog *>( hwnd);
	// These are the messages we handle specially.
	switch (msg)
	{
	case WM_LBUTTONDOWN:
	case WM_MOUSEMOVE:

		// Eat these messages.
		return TRUE;
	case WM_LBUTTONUP:
		theDlg->mapClick(GetParent(hwnd), hwnd);
		return TRUE;
	}

	// Pass it on.
	return (CallWindowProc(theDlg->oldMapProc, hwnd, msg, wparam, lparam));
}
