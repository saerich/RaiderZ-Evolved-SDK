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
/*===========================================================================*\
	FILE: sunlight.cpp

	DESCRIPTION: Sunlight system plugin.

	HISTORY: Created Oct.15 by John Hutchinson
			Derived from the ringarray

	Copyright (c) 1996, All Rights Reserved.
 \*==========================================================================*/
/*===========================================================================*\
 | Include Files:
\*===========================================================================*/
#include "NativeInclude.h"

#pragma managed
#include "compass.h"
#include "sunlightOverride.h"
#include "sunlight.h"
#include "natLight.h"
#include "suntypes.h"
#include "autovis.h"
#include "sunclass.h"
#include "citylist.h"
#include "DaylightSystemFactory.h"
#include "DaylightSystemFactory2.h"
#include "weatherdata.h"
#include "NatLight.h"


// russom - August 16, 2006 - 775527
#define SUBANIM_PBLOCK		0
#define SUBANIM_LOOKAT		1
#define SUBANIM_MULTIPLIER	2

// Parameter block indices
//#define PB_RAD	0
//#define PB_LAT	1
//#define PB_LONG	2
//#define PB_DATE	4
//#define PB_TIME 3
//#define PB_ZONE 5
//#define PB_DST 6

#define PB_RAD	4
#define PB_LAT	2
#define PB_LONG	3
#define PB_DATE	1
#define PB_TIME 0
#define PB_ZONE 5
#define PB_DST	6
#define PB_MANUAL 7
#define PB_SKY	8

#define MINYEAR 1583
#define MAXYEAR 3000
#define MINRADIUS 0.0f
#define MAXRADIUS 100000000.0f

// functions from GEOLOC.CPP
extern BOOL doLocationDialog(HWND hParent, IObjParam* ip, float* latitude,
							 float* longitude, char* cityName);


namespace 
{
// holdLastName's role (and holdLastCity's as well) is to keep the City picking 
// dialog's default selection up to date with the current selection.  
// Unfortunately, it doesn't work all that well - it didn't even in Vesper -
// but I'm reluctant to completely remove it in case it is necessary for 
// some reason that I didn't see.
// Note that CityRestore does not update these values. It used to, but that
// did more harm than good, since the SunMaster affected may not be active
// in the UI.  The results were that, to the user, the dialog appeared to 
// be randomly initialized.
int holdLastCity;
const int HOLD_LAST_NAME_LENGTH = 64;
char holdLastName[HOLD_LAST_NAME_LENGTH] = {0};
}

// externals from GEOLOC.CPP
extern int lastCity;
extern char* lastCityName;

static float stdang_to_compass(float stdang);
float compass_to_stdang(float compass);
static float getZRot(TimeValue t, INode* node);

class CityRestore : public RestoreObj, MaxSDK::SingleWeakRefMaker {
public:
	CityRestore(SunMaster* master, const TCHAR* name);

	virtual void Restore(int isUndo);
	virtual void Redo();

	virtual int Size() { return sizeof(*this); }
	virtual TSTR Description() { return "Undo Sunlight City Name"; }

	virtual void DeleteThis();

private:
	std::basic_string<char> _undoCity;
	std::basic_string<char> _redoCity;
	};

CityRestore::CityRestore(SunMaster* master, const TCHAR* name)
{
	SetRef(master);
	if(NULL != master && NULL != master->GetCity())
	{
		_redoCity = master->GetCity();
	}

	if(NULL != name)
	{
		_undoCity = name;
	}
	}


void CityRestore::Restore(int isUndo)
{
	SunMaster *master = static_cast<SunMaster*>(GetRef());
	if(NULL != master)
	{
		master->SetCity(_undoCity.c_str());
		master->UpdateUI(GetCOREInterface()->GetTime());
	}
	}

void CityRestore::Redo() 
{
	SunMaster *master = static_cast<SunMaster*>(GetRef());
	if(NULL != master)
	{
		master->SetCity(_redoCity.c_str());
		master->UpdateUI(GetCOREInterface()->GetTime());
	}
}

void CityRestore::DeleteThis()
{
	delete this;
}




class ManualRestore : public RestoreObj, MaxSDK::SingleWeakRefMaker {
public:
	ManualRestore(SunMaster* master, int oldManual);

	virtual void Restore(int isUndo);
	virtual void Redo();

	virtual int Size() { return sizeof(*this); }
	virtual TSTR Description() { return "Undo Set Daylight Control Type"; }

	virtual void DeleteThis(){delete this;}

private:
	int mUndoManual;
	int mRedoManual;
};

ManualRestore::ManualRestore(SunMaster* master, int oldManual)
{
	SetRef(master);
	if(master!=NULL)
	{
		TimeValue t = GetCOREInterface()->GetTime();
		mRedoManual = master->GetManual(t);
	}
	mUndoManual = oldManual;

}


void ManualRestore::Restore(int isUndo)
{
	SunMaster *master = static_cast<SunMaster*>(GetRef());
	if(master !=NULL)
	{
		master->NotifyNatLightManualChanged((IDaylightSystem2::DaylightControlType)(mUndoManual));
	}
}

void ManualRestore::Redo()
{
	SunMaster *master = static_cast<SunMaster*>(GetRef());
	if(NULL != master)
	{
		master->NotifyNatLightManualChanged((IDaylightSystem2::DaylightControlType)(mRedoManual));
	}
}



/*===========================================================================*\
 | Sun Master Methods:
\*===========================================================================*/

// This method returns a new instance of the slave controller.
Control* GetNewSlaveControl(SunMaster *master, int i);

// Initialize the class variables...
//HWND SunMaster::hMasterParams = NULL;
IObjParam *SunMaster::iObjParams = nullptr;


ISpinnerControl *SunMaster::radSpin = nullptr;
ISpinnerControl *SunMaster::latSpin = nullptr;
ISpinnerControl *SunMaster::longSpin = nullptr;
ISpinnerControl *SunMaster::hourSpin = nullptr;
ISpinnerControl *SunMaster::minSpin = nullptr;
ISpinnerControl *SunMaster::secSpin = nullptr;
ISpinnerControl *SunMaster::yearSpin = nullptr;
ISpinnerControl *SunMaster::monthSpin = nullptr;
ISpinnerControl *SunMaster::daySpin = nullptr;
ISpinnerControl *SunMaster::northSpin = nullptr;
ISpinnerControl *SunMaster::zoneSpin = nullptr;
#ifndef NO_DAYLIGHT_SKY_COVERAGE_SLIDER
ISliderControl  *SunMaster::skySlider = nullptr;
#endif // NO_DAYLIGHT_SKY_COVERAGE_SLIDER
ICustStatus	*SunMaster::altEdit = nullptr;
ICustStatus	*SunMaster::azEdit = nullptr;
ICustStatus	*SunMaster::cityDisplay = nullptr;
// no longer static 
// SYSTEMTIME SunMaster::theTime = nullptr;
HIMAGELIST SunMaster::hIconImages = NULL;

bool SunMaster::sInCreate = false;

void SunMaster::SetInCreate(bool value)
{
	sInCreate = value;
}

bool SunMaster::IsInCreate()
{
	return sInCreate;
}

float stdang_to_compass(float stdang){
	float rel =  - stdang;
	if (rel >=0.0f) 
		return rel;
	else 
		return FTWO_PI - (float)fabs(rel);
}

float compass_to_stdang(float compass){
		return  - compass;
}

// Returns the local Zrotation of the node less any winding number
// measured relative to the local z axis 
// positive values are clockwise

float getZRot(TimeValue t,INode *node){

	AffineParts lparts;
	float ang[3];
	INode *parent;
	Matrix3 nodeTM, parentTM, localTM, newTM;
	if (node ){
		nodeTM = node->GetNodeTM(t);
		parent = node->GetParentNode();
		if(parent){
			parentTM = parent->GetNodeTM(t);
			localTM = nodeTM * Inverse(parentTM);
			decomp_affine(localTM, &lparts);
			QuatToEuler(lparts.q, ang);
			float test1 = (ang[2]/FTWO_PI);
			int test2 = (int) test1;
			float temp = (float) test1-test2;
			return frtd(stdang_to_compass(temp*FTWO_PI));
		} else
			return 0.0f;
	}  else
		return 0.0f;
}


// Get the APPROXIMATE time zone from the longitude.
int getTimeZone(float longi)
{
	// Transform the supplied longitude to [-180, 180]
	float t_lon = longi;
	if (longi > 180.0 || longi < -180.0)
		 t_lon = longi - (LONG(longi)/180)*360.0;

    int tz;
	//switch t_lon to keep negative logic here.
	t_lon *= -1.0f;
	if (t_lon >= 0)
		tz = -(int)((t_lon + 7.5) / 15);
    else
		tz = (int)((-t_lon + 7.5) / 15);

	DbgAssert(tz >= -12 && tz <= 12);
    return tz;
}

void SunMaster::SetPosition(const Point3& position)
{
	Interface* ip = GetCOREInterface();
	
	//Set the position of the compass - which sets the global position of the system
	if(thePoint)
	{
		Matrix3 mat(TRUE);
		mat.SetTranslate(position);
		thePoint->SetNodeTM(ip->GetTime(), mat);
	}
}

Point3 SunMaster::GetPosition()
{
	Interface* ip = GetCOREInterface();
	
	Point3 pos(0.0f,0.0f,0.0f);
	//Get the position of the compass - which Gets the global position of the system
	if(thePoint)
	{
		Matrix3 mat(TRUE);
		mat = thePoint->GetNodeTM(ip->GetTime());
		mat.PointTransform(pos);
	}
	return pos;
}

void SunMaster::SetCompassDiameter(float compassDiameter)
{
	//Find the compass object
	CompassRoseObject* pCompass = NULL;
	Object* pObjref = thePoint->GetObjectRef();
	if (pObjref)
	{
		Object* base = pObjref->FindBaseObject();
		assert(base);
		if (base && base->ClassID() == COMPASS_CLASS_ID && base->SuperClassID() == HELPER_CLASS_ID)
			pCompass = static_cast<CompassRoseObject*>(base);
	}

	if(pCompass)
	{
		pCompass->axisLength = compassDiameter;
	}
}

float SunMaster::GetCompassDiameter()
{
	float compassDiameter = 0.0f;

	//Find the compass object
	CompassRoseObject* pCompass = NULL;
	Object* pObjref = thePoint->GetObjectRef();
	if (pObjref)
	{
		Object* base = pObjref->FindBaseObject();
		assert(base);
		if (base && base->ClassID() == COMPASS_CLASS_ID && base->SuperClassID() == HELPER_CLASS_ID)
			pCompass = static_cast<CompassRoseObject*>(base);
	}

	if(pCompass)
	{
		compassDiameter = pCompass->axisLength;
	}
	return compassDiameter;
}


void SunMaster::GetClassName(TSTR& s) {
	s = MaxSDK::GetResourceString(daylightSystem ? IDS_DAY_CLASS : IDS_SUN_CLASS);
}
Animatable* SunMaster::SubAnim(int i)
{
	switch (i) {
	case SUBANIM_PBLOCK:
		return pblock;
	case SUBANIM_LOOKAT:
		return theLookat;
	case SUBANIM_MULTIPLIER:
		return theMultiplier;
	}
	return NULL;
}

TSTR SunMaster::SubAnimName(int i)
{
	switch (i) {
	case SUBANIM_PBLOCK:
		return MaxSDK::GetResourceString(IDS_DB_PARAMETERS);
	case SUBANIM_LOOKAT:
		return MaxSDK::GetResourceString(IDS_LOOKAT);
	case SUBANIM_MULTIPLIER:
		return MaxSDK::GetResourceString(IDS_MULTIPLIER);
	}
	return TSTR();
}

// russom - August 16, 2006 - 775527
BOOL SunMaster::CanAssignController(int subAnim)
{
	return( daylightSystem && ((subAnim == SUBANIM_LOOKAT) || (subAnim == SUBANIM_MULTIPLIER)) );
}

// russom - August 16, 2006 - 775527
BOOL SunMaster::AssignController(Animatable *control,int subAnim)
{
	if( !daylightSystem )
		return FALSE;

	if( subAnim == SUBANIM_LOOKAT )
		ReplaceReference(REF_LOOKAT,(RefTargetHandle)control);
	else if( subAnim == SUBANIM_MULTIPLIER )
		ReplaceReference(REF_MULTIPLIER,(RefTargetHandle)control);
	else
		return FALSE;

	NotifyDependents(FOREVER,0,REFMSG_CONTROLREF_CHANGE);  //fix for 894778 MZ
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);  

	return TRUE;
}

#define SUNMASTER_VERSION_SUNLIGHT 2 //2 is same as 1 except we revert the long
#define SUNMASTER_VERSION_DAYLIGHT 5 //4 is same as 5 except we revert the long

/* here's what it looked like in VIZ
	ParamBlockDesc desc[] = {
		{ TYPE_FLOAT, NULL, FALSE },//radius
		{ TYPE_FLOAT, NULL, FALSE },//lat
		{ TYPE_FLOAT, NULL, FALSE },//long
		{ TYPE_FLOAT, NULL, TRUE }, //date
		{ TYPE_INT, NULL, TRUE },//time
		{ TYPE_INT, NULL, FALSE },//zone
		{ TYPE_BOOL, NULL, FALSE },//dst
		};

*/

static ParamBlockDescID desc[] = {
		{ TYPE_FLOAT, NULL, TRUE, PB_RAD },//radius
		{ TYPE_FLOAT, NULL, TRUE, PB_LAT },//lat
		{ TYPE_FLOAT, NULL, TRUE, PB_LONG },//long
		{ TYPE_FLOAT, NULL, TRUE, PB_DATE}, //date
		{ TYPE_INT, NULL, TRUE , PB_TIME},//time
		{ TYPE_INT, NULL, FALSE, PB_ZONE },//zone
		{ TYPE_BOOL, NULL, FALSE, PB_DST},//dst
		};

	//Paramneters for MAX 2.0 version
/*
static ParamBlockDescID desc1[] = {
		{ TYPE_FLOAT, NULL, TRUE, PB_RAD },//radius
		{ TYPE_FLOAT, NULL, TRUE, PB_LAT },//lat
		{ TYPE_FLOAT, NULL, TRUE, PB_LONG },//long
		{ TYPE_INT, NULL, TRUE, PB_TIME },//time
		{ TYPE_FLOAT, NULL, TRUE , PB_DATE}, //date
		{ TYPE_INT, NULL, FALSE, PB_ZONE },//zone
		{ TYPE_BOOL, NULL, FALSE, PB_DST},//dst
		};*/
static ParamBlockDescID desc1[] = {
		{ TYPE_INT, NULL, TRUE, PB_TIME },//time
		{ TYPE_FLOAT, NULL, TRUE , PB_DATE}, //date
		{ TYPE_FLOAT, NULL, TRUE, PB_LAT },//lat
		{ TYPE_FLOAT, NULL, TRUE, PB_LONG },//long
		{ TYPE_FLOAT, NULL, TRUE, PB_RAD },//radius
		{ TYPE_INT, NULL, FALSE, PB_ZONE },//zone
		{ TYPE_BOOL, NULL, FALSE, PB_DST},//dst
		};

static ParamBlockDescID desc2Daylight[] = {
		{ TYPE_INT, NULL, TRUE, PB_TIME },//time
		{ TYPE_FLOAT, NULL, TRUE , PB_DATE}, //date
		{ TYPE_FLOAT, NULL, TRUE, PB_LAT },//lat
		{ TYPE_FLOAT, NULL, TRUE, PB_LONG },//long
		{ TYPE_FLOAT, NULL, TRUE, PB_RAD },//radius
		{ TYPE_INT, NULL, FALSE, PB_ZONE },//zone
		{ TYPE_BOOL, NULL, FALSE, PB_DST},//dst
		{ TYPE_BOOL, NULL, FALSE, PB_MANUAL},//manual override
		};

static ParamBlockDescID desc3Daylight[] = {
		{ TYPE_INT, NULL, TRUE, PB_TIME },//time
		{ TYPE_FLOAT, NULL, TRUE , PB_DATE}, //date
		{ TYPE_FLOAT, NULL, TRUE, PB_LAT },//lat
		{ TYPE_FLOAT, NULL, TRUE, PB_LONG },//long
		{ TYPE_FLOAT, NULL, TRUE, PB_RAD },//radius
		{ TYPE_INT, NULL, FALSE, PB_ZONE },//zone
		{ TYPE_BOOL, NULL, FALSE, PB_DST},//dst
		{ TYPE_BOOL, NULL, FALSE, PB_MANUAL},//manual override  //
		{ TYPE_FLOAT, NULL, TRUE, PB_SKY },//skyCondition
		};
static ParamBlockDescID desc4Daylight[] = {
		{ TYPE_INT, NULL, TRUE, PB_TIME },//time
		{ TYPE_FLOAT, NULL, TRUE , PB_DATE}, //date
		{ TYPE_FLOAT, NULL, TRUE, PB_LAT },//lat
		{ TYPE_FLOAT, NULL, TRUE, PB_LONG },//long
		{ TYPE_FLOAT, NULL, TRUE, PB_RAD },//radius
		{ TYPE_INT, NULL, FALSE, PB_ZONE },//zone
		{ TYPE_BOOL, NULL, FALSE, PB_DST},//dst
		{ TYPE_INT, NULL, FALSE, PB_MANUAL},//manual override  //
		{ TYPE_FLOAT, NULL, TRUE, PB_SKY },//skyCondition
		};

//static funcs for notifications
void NotifyMaxReset(void *ptr, NotifyInfo *info)
{
	//make sure it's a max file getting opened if we are doing an open
	DbgAssert(info != NULL);
	if (info->intcode == NOTIFY_FILE_PRE_OPEN)
		if (NULL != info->callParam && IOTYPE_MAX != *(static_cast<FileIOType*>(info->callParam)))
			return;

	//set the natLight to NULL!
	SunMaster *sl = (SunMaster*)ptr;
	sl->natLight = NULL;

}
//default data
//The latitudes and longitudes for these three cities below were obtained from the
//file "sitename.txt"

static char *JapaneseCityName ="Tokyo Japan";
static char *ChineseCityName ="Beijing China";
static char *KoreanCityName ="Seoul Korea";
static char *EnglishCityName ="San Francisco, CA";

void SunMaster::SetCityInfo(char* name, int& zone, float& lati, float& longi)
{
	WORD languageID = PRIMARYLANGID(MaxSDK::Util::GetLanguageID());

	switch(languageID)
	{
		case LANG_JAPANESE:
			name  = JapaneseCityName;
			zone  = 9;
			lati  = 35.685f;
			longi = 139.751f;
			break;

		case LANG_CHINESE:
		    name  = ChineseCityName;
			zone  = 8;
			lati  = 39.9167f;
			longi = 116.4333f;
			break;

		case LANG_KOREAN:
			name  = KoreanCityName;
			zone  = 9;
			lati  = 36.5333f;
			longi = 126.9583f;
			break;

		default:
			name  = EnglishCityName;
			zone  = -8;
			lati  = 37.795f;
			longi = -122.394f;
	}
}

// Constructor.
SunMaster::SunMaster(bool daylight) {

	pblock = NULL;
	thePoint = NULL;
	theLegacyObject = NULL;
	theLookat = NULL;
	theMultiplier = NULL;
	 theObjectMon = NULL;
	ignore_point_msgs = FALSE;
	hMasterParams = NULL;
	daylightSystem = daylight;
	controlledByWeatherFile  = false;
	natLight=NULL;

	// Create a parameter block and make a reference to it.
	if (daylightSystem)
		ReplaceReference( REF_PARAM, CreateParameterBlock( desc4Daylight, 9, SUNMASTER_VERSION_DAYLIGHT ) );
	else
		ReplaceReference( REF_PARAM, CreateParameterBlock( desc1, 7, SUNMASTER_VERSION_SUNLIGHT ) );

	 ReplaceReference(REF_OBJECT_MONITOR, (ReferenceTarget*)CreateInstance(REF_TARGET_CLASS_ID, NODEMONITOR_CLASS_ID));

	//Make the controllers linear
	Control *c = (Control *) CreateInstance(CTRL_FLOAT_CLASS_ID,Class_ID(LININTERP_FLOAT_CLASS_ID,0)); 
	pblock->SetController(PB_DATE, c, TRUE);
	c = (Control *) CreateInstance(CTRL_FLOAT_CLASS_ID,Class_ID(LININTERP_FLOAT_CLASS_ID,0)); 
	pblock->SetController(PB_TIME, c, TRUE);
	
	// Set the initial values at time 0.
	GetLocalTime(&theTime);
	tvalid = NEVER;

	//TIME_ZONE_INFORMATION tzi;
	//DWORD result = GetTimeZoneInformation(&tzi);

    CityList::GetInstance().init();

    CityList::Entry * pEntry = CityList::GetInstance().GetDefaultCity();

	// Set the city information

    char *  name    = NULL;
    int     zone    = 0;
    float   lati    = 0.0f;
    float   longi   = 0.0f;

	SetCityInfo(name, zone, lati, longi);

    if (pEntry)
    {
        name = pEntry->name;
        if ((*name == '+') || (*name == '*'))
            ++name;
        zone  = getTimeZone(pEntry->longitude);
        lati  = pEntry->latitude;
        longi = pEntry->longitude;
    }

	SetLat ( TimeValue(0), lati );
	SetLong( TimeValue(0), longi);

	SetCity(name);
	SetNorth(TimeValue(0), 0.0f);
	SetZone(TimeValue(0), zone);

	// Since we don't use the time zone of the computer anymore then default the daylight saving time to false.
	//SetDst(TimeValue(0), result == TIME_ZONE_ID_DAYLIGHT ? TRUE : FALSE);
	SetDst(TimeValue(0), FALSE);

	SetManual(0, false);
	refset=FALSE;
    mbOldAnimation = FALSE;
	timeref=0.0f;
	dateref=0;

	// Get time notification so we can update the UI
	GetCOREInterface()->RegisterTimeChangeCallback(this);


	//we need to make sure the natLight ptr is NULL before we reset. 1020318
	RegisterNotification(&NotifyMaxReset,				(void *) this, NOTIFY_FILE_PRE_OPEN);
	RegisterNotification(&NotifyMaxReset,				(void *) this, NOTIFY_SYSTEM_PRE_RESET);
	RegisterNotification(&NotifyMaxReset,				(void *) this, NOTIFY_SYSTEM_PRE_NEW);


// Since the users typically render an image per solstice/equinox, create
// the daylight system to a time that approximatly represents the summer solstice
// of the current year (i.e. June 12th of the current year, 12:00).
#ifdef DAYLIGHT_DEFAULT_TIME_SUMMER_SOLSTICE
	theTime.wMonth = 6;
	theTime.wDay = 21;

	theTime.wHour = 12;
	theTime.wMinute = 0;
	theTime.wSecond = 0;
#endif // DAYLIGHT_DEFAULT_TIME_SUMMER_SOLSTICE

	// since we know that theTime is valid we can do this
	interpJulianStruct jd = fusetime(theTime);
	SetDate( TimeValue(0),  jd );
	SetTime( TimeValue(0),  jd );
	// now that the parmeter block is up to date we can calculate
	// the dependent vars
	calculate(TimeValue(0),FOREVER);

	}

static void NodesCloned(void* param, NotifyInfo*)
{
	static_cast<SunMaster*>(param)->RefThePointClone();
	UnRegisterNotification(NodesCloned, param, NOTIFY_POST_NODES_CLONED);
}

SunMaster::~SunMaster()
{
	// Just in case this gets deleted after Clone is called
	// but before the NOTIFY_POST_NODES_CLONED notification is sent.
	UnRegisterNotification(NodesCloned, this, NOTIFY_POST_NODES_CLONED);
	// Stop time notifications
	GetCOREInterface()->UnRegisterTimeChangeCallback(this);

	//notifications need to get unregistered.
	UnRegisterNotification(&NotifyMaxReset,				(void *) this, NOTIFY_FILE_PRE_OPEN);
	UnRegisterNotification(&NotifyMaxReset,				(void *) this, NOTIFY_SYSTEM_PRE_RESET);
	UnRegisterNotification(&NotifyMaxReset,				(void *) this, NOTIFY_SYSTEM_PRE_NEW);

}

void SunMaster::RefThePointClone()
{
	if (thePoint != NULL) {
		INode* p = thePoint;
		thePoint = NULL;
		ReplaceReference(REF_POINT, p);
	}
}

// This method is called to return a copy of the ring master.
RefTargetHandle SunMaster::Clone(RemapDir& remap) {
    SunMaster* newm = new SunMaster(daylightSystem);	
	newm->ReplaceReference(REF_PARAM,remap.CloneRef(this->pblock));
	newm->ReplaceReference(REF_POINT,NULL);
	newm->ReplaceReference(REF_LOOKAT,remap.CloneRef(this->theLookat));
	newm->ReplaceReference(REF_MULTIPLIER,remap.CloneRef(this->theMultiplier));
	 newm->ReplaceReference(REF_OBJECT_MONITOR, remap.CloneRef(this->theObjectMon));
	newm->thePoint = NULL;
	newm->theLegacyObject = NULL;
	newm->dateref = dateref;
	newm->SetCity(GetCity());
	remap.PatchPointer((RefTargetHandle*)&newm->thePoint,(RefTargetHandle)thePoint);
	if (newm->thePoint == NULL)
		RegisterNotification(NodesCloned, newm, NOTIFY_POST_NODES_CLONED);
	else
		newm->RefThePointClone();

	BaseClone(this, newm, remap);
	return(newm);
	}


// This method is called to update the UI parameters to reflect the
// correct values at the time passed.  Note that FALSE is passed as
// the notify parameter.  This ensure that notification message are
// not sent when the values are updated.
void SunMaster::UpdateUI(TimeValue t)
	{
	if ( hMasterParams ) {


		int manual = GetManual(t); //0 is location, 1 is manual and 2 is weather data file
		bool dateTime = (manual== NatLightAssembly::MANUAL_MODE||manual== NatLightAssembly::WEATHER_MODE) ? false: true;


		EnableWindow(GetDlgItem(hMasterParams,IDC_MANUAL_POSITION), !IsInCreate());
		EnableWindow(GetDlgItem(hMasterParams,IDC_CONTROLLER_POS), !IsInCreate());
		EnableWindow(GetDlgItem(hMasterParams,IDC_WEATHER_DATA_FILE), !IsInCreate());

		TimeValue t = GetCOREInterface()->GetTime();
		CheckDlgButton(hMasterParams, IDC_MANUAL_POSITION,
			manual== NatLightAssembly::MANUAL_MODE);
		CheckDlgButton(hMasterParams, IDC_CONTROLLER_POS,
			manual == NatLightAssembly::DATETIME_MODE);
		CheckDlgButton(hMasterParams, IDC_WEATHER_DATA_FILE,
			manual == NatLightAssembly::WEATHER_MODE);

		if(manual==NatLightAssembly::DATETIME_MODE||manual==NatLightAssembly::WEATHER_MODE) //orb scale for date time and weather file
		{
			radSpin->Enable(true);
			northSpin->Enable(true);
		}
		else
		{
			radSpin->Enable(false);
			northSpin->Enable(false);

		}
		latSpin->Enable(dateTime);
		longSpin->Enable(dateTime);
		hourSpin->Enable(dateTime);
		minSpin->Enable(dateTime);
		secSpin->Enable(dateTime);
		monthSpin->Enable(dateTime);
		daySpin->Enable(dateTime);
		yearSpin->Enable(dateTime);
		zoneSpin->Enable(dateTime);


		if(manual==NatLightAssembly::WEATHER_MODE) //only enable setup with weather file.
			EnableWindow(GetDlgItem(hMasterParams,IDC_SETUP),TRUE);
		else
			EnableWindow(GetDlgItem(hMasterParams,IDC_SETUP),FALSE);

		if (daylightSystem)
		{
#ifndef NO_DAYLIGHT_SKY_COVERAGE_SLIDER
			skySlider->Enable(dateTime);
#endif // NO_DAYLIGHT_SKY_COVERAGE_SLIDER
		}

		EnableWindow(GetDlgItem(hMasterParams,IDC_DST), dateTime);
		EnableWindow(GetDlgItem(hMasterParams,IDC_GETLOC), dateTime);

		radSpin->SetValue( GetRad(t), FALSE );
		latSpin->SetValue( GetLat(t), FALSE );
		longSpin->SetValue( GetLong(t), FALSE );
		if (daylightSystem)
		{
#ifndef NO_DAYLIGHT_SKY_COVERAGE_SLIDER
			skySlider->SetValue( GetSkyCondition(t), FALSE);
#endif // NO_DAYLIGHT_SKY_COVERAGE_SLIDER
		}

		radSpin->SetKeyBrackets(pblock->KeyFrameAtTime(PB_RAD,t));
		latSpin->SetKeyBrackets(pblock->KeyFrameAtTime(PB_LAT,t));
		longSpin->SetKeyBrackets(pblock->KeyFrameAtTime(PB_LONG,t));
		if (daylightSystem)
		{
#ifndef NO_DAYLIGHT_SKY_COVERAGE_SLIDER
			skySlider->SetKeyBrackets(pblock->KeyFrameAtTime(PB_SKY,t));
#endif // NO_DAYLIGHT_SKY_COVERAGE_SLIDER
		}

		BOOL timekey = pblock->KeyFrameAtTime(PB_TIME,t);
		BOOL datekey = pblock->KeyFrameAtTime(PB_DATE,t);

		hourSpin->SetValue( GetHour(t), FALSE );
		minSpin->SetValue( GetMin(t), FALSE );
		secSpin->SetValue( GetSec(t),FALSE);
		monthSpin->SetValue( GetMon(t), FALSE );
		daySpin->SetValue( GetDay(t), FALSE );
		yearSpin->SetValue( GetYr(t), FALSE );

		hourSpin->SetKeyBrackets(timekey);
		minSpin->SetKeyBrackets(timekey);
		secSpin->SetKeyBrackets(timekey);
		monthSpin->SetKeyBrackets(datekey);
		daySpin->SetKeyBrackets(datekey);
		yearSpin->SetKeyBrackets(datekey);

		northSpin->SetValue( GetNorth(t), FALSE);
		zoneSpin->SetValue( GetZone(t), FALSE);
		CheckDlgButton(hMasterParams,IDC_DST, GetDst(t));
		TCHAR buf[10];
	  _itot_s(static_cast<int>(rtd(az)), buf, (sizeof(buf) / sizeof(buf[0])), 10);
		if(azEdit) azEdit->SetText(buf);
	  _itot_s(static_cast<int>(rtd(alt)), buf, (sizeof(buf) / sizeof(buf[0])), 10);
		if(altEdit) altEdit->SetText(buf);
		if(cityDisplay) cityDisplay->SetText(city);
		}
	}


// The master controller of a system plug-in should implement this 
// method to give MAX a list of nodes that are part of the system.   
// The master controller should fill in the given table with the 
// INode pointers of the nodes that are part of the system. This 
// will ensure that operations like cloning and deleting affect 
// the whole system.  MAX will use GetInterface() in the 
// tmController of each selected node to retrieve the master 
// controller and then call GetSystemNodes() on the master 
// controller to get the list of nodes.
void SunMaster::GetSystemNodes(INodeTab &nodes, SysNodeContext c)
	{
	if (thePoint) {
		nodes.Append(1,&thePoint);
	}

	if (theObjectMon) 
	{
		INodeMonitor* nodeMon = static_cast<INodeMonitor*>(theObjectMon->GetInterface(IID_NODEMONITOR));
		DbgAssert(nodeMon != NULL);
		INode* n = nodeMon->GetNode();
		if (n != NULL) 
		{
			nodes.Append(1, &n);

			if (daylightSystem) {
				NatLightAssembly::AppendAssemblyNodes(n, nodes, c);
			}
		}
	}
	}

int SunMaster::NumRefs()
{
	return NUM_REF;
};

// This methods returns the ith reference - there are two: the 
// parameter block and the light (helper, actually).
RefTargetHandle SunMaster::GetReference(int i)  
{ 
	switch (i) 
	{
	case REF_PARAM:
		return pblock;
	case REF_POINT:
		return thePoint;
	case REF_LOOKAT:
		return theLookat;
	case REF_MULTIPLIER:
		return theMultiplier;
	 case REF_OBJECT_MONITOR:
		 return theObjectMon;
		}
	return NULL;
	}

// This methods sets the ith reference - there are two.
void SunMaster::SetReference(int i, RefTargetHandle rtarg) 
{
	switch (i) 
	{
	case REF_PARAM:
		pblock = static_cast<IParamBlock*>(rtarg);
		break;
	case REF_POINT:
		thePoint = static_cast<INode*>(rtarg);
		break;
	case REF_LOOKAT:
		theLookat = static_cast<Control*>(rtarg);
		break;
	case REF_MULTIPLIER:
		theMultiplier = static_cast<Control*>(rtarg);
		break;
		case REF_OBJECT_MONITOR:
			theObjectMon = rtarg;
			break;
		}
	}		

BOOL SunMaster::GetNextKeyTime(TimeValue t,DWORD flags,TimeValue &nt)
	{
	TimeValue at,tnear = 0;
	BOOL tnearInit = FALSE;
	Control *ct, *cd;
	ct = cd = NULL;
	ct = pblock->GetController(PB_TIME);
	cd = pblock->GetController(PB_DATE);

	
	if (cd && cd->GetNextKeyTime(t,flags,at)) {
		if (!tnearInit) {
			tnear = at;
			tnearInit = TRUE;
		} else 
		if (ABS(at-t) < ABS(tnear-t)) tnear = at;
		}

	if (ct && ct->GetNextKeyTime(t,flags,at)) {
		if (!tnearInit) {
			tnear = at;
			tnearInit = TRUE;
		} else 
		if (ABS(at-t) < ABS(tnear-t)) tnear = at;
		}

	
	if (tnearInit) {
		nt = tnear;
		return TRUE;
	} else {
		return FALSE;
		}
	}

// This function converts a position on the unit sphere,
// given in azimuth altitude, to xyz coordinates
Point3 az_to_xyz(float az, float alt){
	double x,y,z;
	x = cos(alt)*sin(az);
	y = cos(alt)*cos(az);
	z =	sin(alt);
	Point3 xyzp(x,y,z);
	return Normalize(xyzp);
}


// ======= This method is the crux of the system plug-in ==========
// This method gets called by each slave controller and based on the slaves
// ID, it is free to do whatever it wants. In the current system there is only 
// the slave controller of the light.
void SunMaster::GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method,
 int id) {
	Matrix3 tmat;
	Matrix3 *mat;
	tmat.IdentityMatrix();
	Point3 unitdir;
	float* mult;

	switch(id){
		case LIGHT_TM:

			if (theLookat != NULL && GetManual(t)==NatLightAssembly::MANUAL_MODE) {
				theLookat->GetValue(t, val, valid, method);
			} else {
				float radius = GetRad(t,valid);

				if(controlledByWeatherFile&&natLight!=NULL)
				{
					static int notGetting = true; //just to make sure we hit no infinite loops
												  //we shouldn't
					if(notGetting)
					{
						notGetting = false;
						natLight->SetSunLocationBasedOnWeatherFile(this,t,valid);
						notGetting = true;
					};
				}
				// calculate the controllers dependent variables: the azimuth and altitude
				calculate(t,valid);

				// Calculate the translation and rotation of the node 
				//CAUTION slop data types
				mat = (Matrix3*)val;
				tmat.RotateX(PI/2.0f - float(alt));
				tmat.RotateZ(PI - float(az));
				unitdir = az_to_xyz(float(az), float(alt));
				tmat.SetTrans(radius*unitdir);
				if (theLookat != NULL) {
					SuspendAnimate();
					AnimateOff();
					SetXFormPacket xform(tmat);
					theLookat->SetValue(t, &xform, 0);
					ResumeAnimate();
				}

				Matrix3 tm = *mat;
				(*mat) = (method==CTRL_RELATIVE) ? tmat*(*mat) : tmat;

				if (theLookat != NULL) {
					// Lookat controllers setup some of their state in
					// the GetValue method, so it needs to be called.
					Interval v;
					theLookat->GetValue(t, &tm, v, method);
				}

				// Make sure spinners track when animating and in Motion Panel
				// the limits on the day spinner may be wrong
				if ( hMasterParams ) {
					int year, month, day ,leap, modays;
					month = GetMon(t);
					year = GetYr(t);
					day = GetDay(t);
					leap = isleap(year);
					if (month == 12) modays = 31;
					else modays = GetMDays(leap, month)-GetMDays(leap, month-1);
					daySpin->SetLimits(1,modays,FALSE);
				}

//				UpdateUI(t);
			}
			break;

		case LIGHT_INTENSE:
		case LIGHT_MULT:
			mult=(float*)val;
			if (theMultiplier != NULL && GetManual(t)==NatLightAssembly::MANUAL_MODE) {
				theMultiplier->GetValue(t, val, valid, method);
			} else {
				// calculate the controllers dependent variables: the azimuth and altitude
				calculate(t,valid);
				*mult = calcIntensity(t, valid);
				if (theMultiplier != NULL) {
					SuspendAnimate();
					AnimateOff();
					theMultiplier->SetValue(t, mult, 0);
					ResumeAnimate();
				}
			}
			if (id == LIGHT_MULT) {
				float factor = 1500.0f;
				ToneOperatorInterface* pI = static_cast<ToneOperatorInterface*>(
					GetCOREInterface(TONE_OPERATOR_INTERFACE));
				if (pI != NULL) {
					ToneOperator* p = pI->GetToneOperator();
					if (p != NULL) {
						factor = p->GetPhysicalUnit(t, valid);
					}
				}
				*mult /= factor;
			}
			break;

		case LIGHT_SKY:
			mult=(float*)val;
			*mult = GetSkyCondition(t, valid);
			break;
	}
}

static inline int getJulian(int mon, int day)
{
	static int days[] = {
		0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334
	};

	return days[mon - 1] + day;
}

float SunMaster::calcIntensity(TimeValue t, Interval& valid)
{
	const float Esc = 127500;	// Solar Illumination Constant in lx
	const float kPi = 3.14159265358979323846f;
	const float k2Pi365 = (2.0f * kPi) / 365.0f;
	float cloudy;

	cloudy = GetSkyCondition(t, valid);

	if (cloudy > 0.75f || alt <= 0.0f) {
		return 0.0f;
	}

	int julian = getJulian(GetMon(t, valid), GetDay(t, valid));

	float Ext = Esc * (1.0f + .034f * cosf(k2Pi365 * float(julian - 2)));
	float opticalAirMass = 1.0f / sin(alt);
	if (cloudy < 0.35f)
		opticalAirMass *= -0.21f;
	else
		opticalAirMass *= -0.80f;

	return Ext * exp(opticalAirMass);
}

void SunMaster::SetValue(TimeValue t, void *val, int commit, GetSetMethod method, int id)
{
	switch (id) {
	case LIGHT_TM:
		// Always set the lookat controller. To keep
		// the lookat and time somewhat in sync.
		if (GetManual(t)==NatLightAssembly::MANUAL_MODE && theLookat != NULL)
			theLookat->SetValue(t, val, commit, method);
		break;

	case LIGHT_INTENSE:
		// Always set the lookat controller. To keep
		// the lookat and time somewhat in sync.
		if (GetManual(t)==NatLightAssembly::MANUAL_MODE && theMultiplier != NULL)
			theMultiplier->SetValue(t, val, commit, method);
		break;

	case LIGHT_MULT:
		// Always set the lookat controller. To keep
		// the lookat and time somewhat in sync.
		if (GetManual(t)==NatLightAssembly::MANUAL_MODE && theMultiplier != NULL) {
			float factor = 1500.0f;
			ToneOperatorInterface* pI = static_cast<ToneOperatorInterface*>(
				GetCOREInterface(TONE_OPERATOR_INTERFACE));
			if (pI != NULL) {
				ToneOperator* p = pI->GetToneOperator();
				if (p != NULL) {
					factor = p->GetPhysicalUnit(t);
				}
			}
			factor *= *static_cast<float*>(val);
			theMultiplier->SetValue(t, &factor, commit, method);
		}
		break;

	case LIGHT_SKY:
		SetSkyCondition(t, *static_cast<float*>(val));
		break;
	}
}

// This is the method that recieves change notification messages
RefResult SunMaster::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
     PartID& partID, RefMessage message ) 
    {
	switch (message) {
		case REFMSG_GET_PARAM_DIM: { 
			// The ParamBlock needs info to display in the track view.
			GetParamDim *gpd = (GetParamDim*)partID;
			switch (gpd->index) {
				case PB_RAD: gpd->dim = stdWorldDim;break;
				case PB_TIME: gpd->dim = &mTimeDimension;break;
				}
			return REF_HALT; 
			}

		case REFMSG_GET_PARAM_NAME: {
			GetParamName *gpn = (GetParamName*)partID;
			switch (gpn->index) {
				case PB_DATE: gpn->name = MaxSDK::GetResourceString(IDS_SOL_DATE);break;
				case PB_TIME: gpn->name = MaxSDK::GetResourceString(IDS_SOL_TIME);break;
				case PB_RAD: gpn->name = MaxSDK::GetResourceString(IDS_RAD);break;
				case PB_LAT: gpn->name = MaxSDK::GetResourceString(IDS_LAT);break;
				case PB_LONG: gpn->name = MaxSDK::GetResourceString(IDS_LONG);break;
				case PB_SKY: gpn->name = MaxSDK::GetResourceString(IDS_SKY_COVER);break;
//				case PB_ZONE: gpn->name = MaxSDK::GetResourceString(IDS_ZONE);break;
				}
			return REF_HALT; 
			}

		case REFMSG_TARGET_DELETED:
			if (hTarget==thePoint) {
				thePoint = NULL;
				break;
				}
			return REF_STOP;

		case REFMSG_CHANGE:
			if ( hTarget==thePoint && !ignore_point_msgs ) {
				if (hMasterParams) 
					UpdateUI(iObjParams->GetTime());
				break;
				}
			else if (hTarget == pblock){
				if (hMasterParams) 
					UpdateUI(iObjParams->GetTime());
				break;
				}
			else if (hTarget == theLookat && GetManual(GetCOREInterface()->GetTime())==NatLightAssembly::MANUAL_MODE)
				break;
			else if (hTarget == theMultiplier && GetManual(GetCOREInterface()->GetTime())==NatLightAssembly::MANUAL_MODE)
				break;
			return REF_STOP;
		}
	return(REF_SUCCEED);
	}

// Sets the city name.
void SunMaster::SetCity(const char* name)
{
	const char* nm = name;
	if (nm != NULL)
	{
		if ((*nm == '+') || ((*nm == '*')))
			++nm;
		strcpy(city, nm);
	}
	else
	{
		*city = '\0';
		lastCity = -1;
	}
}

void SunMaster::calculateCity(TimeValue t)
{
	if(controlledByWeatherFile==false)
	{
		//Search through the city list to find a match if we can.
		CityList& citylist = CityList::GetInstance();
		citylist.init();
		char* cityName = NULL;
		//There may be minor differences in the latitude and longitude.
		const float eps = 0.01f;
		for (int i = 0; i < citylist.Count(); ++i)
		{
			if (fabs(GetLat(t) - citylist[i].latitude) < eps && 
				fabs(GetLong(t) - citylist[i].longitude) < eps)
			{
				cityName = citylist[i].name;
				if (cityName[0] == '+' || cityName[0] == '*')
				{
					cityName = &cityName[1];
				}
				break;
			}
		}
		SetCity(cityName);
	}
}

// The following methods set and get the radius value in the parameter block.
void SunMaster::SetRad(TimeValue t, float r) { 
	pblock->SetValue( PB_RAD, t, r );
	}

float SunMaster::GetRad(TimeValue t, Interval& valid ) { 	
	float f;
	pblock->GetValue( PB_RAD, t, f, valid );
	return f;
	}

// The following methods set and get the lat/long in the parameter block.
void SunMaster::SetLat(TimeValue t, float r) { 
	pblock->SetValue( PB_LAT, t, r );
	}

float SunMaster::GetLat(TimeValue t, Interval& valid ) { 	
	float f;
	pblock->GetValue( PB_LAT, t, f, valid );
	return f;
	}

void SunMaster::SetLong(TimeValue t, float r) { 
	pblock->SetValue( PB_LONG, t, r );
	}

float SunMaster::GetLong(TimeValue t, Interval& valid ) { 	
	float f;
	pblock->GetValue( PB_LONG, t, f, valid );
	return f;
	}

// Other pblock access methods

void SunMaster::SetZone(TimeValue t, int h) { 
	pblock->SetValue( PB_ZONE, t, h );
	}

int SunMaster::GetZone(TimeValue t, Interval& valid ) { 	
	int i;
	pblock->GetValue( PB_ZONE, t, i, valid );
	return i;
	}

void SunMaster::SetDst(TimeValue t, BOOL h) { 
	pblock->SetValue( PB_DST, t, h );
	}

BOOL  SunMaster::GetDst(TimeValue t, Interval& valid ) { 	
	BOOL b;
	pblock->GetValue( PB_DST, t, b, valid );
	return b;
	}

void SunMaster::SetManual(TimeValue t, int h) { 
	if (daylightSystem) {
		int oldManual; 
		Interval valid = FOREVER;
		pblock->GetValue( PB_MANUAL, t, oldManual, valid );
		if(oldManual!=h)
		{
			pblock->SetValue( PB_MANUAL, t, h ); //we could not call this
			if(theHold.Holding())
				theHold.Put(new ManualRestore(this, oldManual));	
			NotifyNatLightManualChanged((IDaylightSystem2::DaylightControlType)(h));
		}
	}
}

int  SunMaster::GetManual(TimeValue t, Interval& valid ) {
	if (daylightSystem && pblock != NULL) {
		int b;
		pblock->GetValue( PB_MANUAL, t, b, valid );
		return b;
		}
	return false;
	}

void SunMaster::SetSkyCondition(TimeValue t, float sky)
{
	if (daylightSystem)
		pblock->SetValue( PB_SKY, t, sky );
}

float SunMaster::GetSkyCondition(TimeValue t, Interval& valid)
{
	if (daylightSystem) {
		float f;
		pblock->GetValue( PB_SKY, t, f, valid );
		return f;
	}
	return 1.0f;
}

void SunMaster::SetNorth(TimeValue t, float r) { 
	if (thePoint) align_north(t,r);
	}

float SunMaster::GetNorth(TimeValue t, Interval& valid ) { 	
	return getZRot(t, thePoint);
	}


// This method actually places a representation of the current Julian date
// into the parameter block where it is interpolated. Because pblocks
// can only handle floats, two values are interpolated separately.
// The number of days goes into PB_DATE and is interpolated as a float.
//
// Furthermore because the number of JulianDays can be such a large number,
// prior to sticking it in the pblock we save a local epoch and measure date
// changes relative to this benchmark
//
void SunMaster::SetDate(TimeValue t, const interpJulianStruct& jd) { 
	double t1;

	// if were not animating we save a local epoch and measure relative to it
//	if(!Animating()){
	if(!refset){
		dateref=(long)jd.days;
		refset = TRUE;
		t1=0.0;
	}
	else {
		tvalid.SetEmpty();
		t1 = jd.days - dateref;
	}

	pblock->SetValue( PB_DATE, t, (float) t1 ); //caution possible data loss
	}

// This method puts the time of day into the pblock.
// The number of seconds in the current day goes into PB_TIME and
// is interpolated as an int.
void SunMaster::SetTime(TimeValue t, const interpJulianStruct& jd) { 
	pblock->SetValue( PB_TIME, t, jd.subday );
	}

// GetTime pulls the interpolated parameter block values out and passes them through
// fracturetime which adds the local epoch back in and explodes the value into a gregorian
// representation in the static var theTime.

// We implement a validity mechanism for the local time representation

void SunMaster::GetTime(TimeValue t, Interval& valid ) {
	if(0){
//	if(tvalid.InInterval(t)){
		valid &= tvalid;
		return;
	}
	else{
		tvalid = FOREVER;
		interpJulianStruct jd;
		float date;
		int time;
		pblock->GetValue( PB_DATE, t, date, tvalid );
		pblock->GetValue( PB_TIME, t, time, tvalid );
		valid &= tvalid;
		if (mbOldAnimation) {
			jd.days=date;
			jd.subday=time;
		}
		else {
			jd.days=floor(date + 0.5);
			jd.subday=time % (24 * 60 * 60);	// Truncate time to a day
			if (jd.subday < 0)					// No negative times.
				jd.subday += 24 * 60 * 60;
		}
		jd.epoch=dateref;
		fracturetime(jd,theTime);
	}
	}

//////////////////////////////////////////////////////////////
// The Pseudo pblock methods for getting time and setting time
//////////////////////////////////////////////////////////////

void SunMaster::SetHour(TimeValue t, int h) { 
	theTime.wHour = h;
	SetTime(t,fusetime(theTime));
	}

int SunMaster::GetHour(TimeValue t, Interval& valid ) { 	
	GetTime(t,valid);//throw away return, we want side-effect
	return theTime.wHour;
	}

void SunMaster::SetMin(TimeValue t, int h) { 
	theTime.wMinute = h;
	SetTime(t,fusetime(theTime));
	}

int SunMaster::GetMin(TimeValue t, Interval& valid ) { 	
	GetTime(t,valid);//throw away return, we want side-effect
	return theTime.wMinute;
	}

void SunMaster::SetSec(TimeValue t, int h) { 
	theTime.wSecond = h;
	SetTime(t,fusetime(theTime));
	}

int SunMaster::GetSec(TimeValue t, Interval& valid ) { 	
	GetTime(t,valid);//throw away return, we want side-effect
	return theTime.wSecond;
	}

void SunMaster::SetMon(TimeValue t, int h) { 
	theTime.wMonth = h;
	SetDate(t,fusetime(theTime));
	}

int SunMaster::GetMon(TimeValue t, Interval& valid ) { 	
	GetTime(t,valid);//throw away return, we want side-effect
	return theTime.wMonth;
	}

void SunMaster::SetDay(TimeValue t, int h) { 
	theTime.wDay = h;
	SetDate(t,fusetime(theTime));
	}

int SunMaster::GetDay(TimeValue t, Interval& valid ) { 	
	GetTime(t,valid);//throw away return, we want side-effect
	return theTime.wDay;
	}

void SunMaster::SetYr(TimeValue t, int h) { 
	theTime.wYear = h;
	SetDate(t,fusetime(theTime));
	}

int SunMaster::GetYr(TimeValue t, Interval& valid ) { 	
	GetTime(t,valid);//throw away return, we want side-effect
	return theTime.wYear;
	}

void SunMaster::SetWeatherMDYHMS(TimeValue t,int month,int day, int year, int hour, int minute, int second,BOOL dst)
{
	theTime.wMonth = month;
	theTime.wDay = day;
	theTime.wYear = year;
	theTime.wHour = hour;
	theTime.wMinute = minute;
	theTime.wSecond = second;
	interpJulianStruct jd = fusetime(theTime);
	SetDst(t, dst);
	SetDate(t,  jd );
	SetTime(t,  jd );
}



/////////////////////////////////////////////////////////////
//Methods for getting the private, dependent variables
///////////////////////////////////////////////////////////////

Point2 SunMaster::GetAzAlt(TimeValue t, Interval& valid ) {
	Point2 result;
	GetTime(t,valid);
	calculate(t,FOREVER);
	result.x = (float) az;
	result.y = (float) alt;
	return result;
}

// calculate the dependent variables at the given time
void SunMaster::calculate(TimeValue t, Interval& valid){
	double latitude,longitude;
	long hour,min,sec,month,day,year,zone;
	BOOL dst;

	// Retrieve the values of the UI parameters at the time passed in.
	zone = GetZone(t,valid);
	dst = GetDst(t,valid);
	latitude = GetLat(t,valid);
	longitude = GetLong(t,valid);
	hour = GetHour(t,valid);
	min = GetMin(t,valid);
	sec = GetSec(t,valid);
	month = GetMon(t,valid);
	day = GetDay(t,valid);
	year = GetYr(t,valid);


	double st;
	long zonedst = zone;
	if(dst) zonedst++;
	sunLocator(dtr(latitude), dtr(-longitude), month, day,  //mz new daylight fixes autovis functions expect longtitude to be wrong
		year, hour - zonedst, min,sec, 0,
		   &alt, &az, &st);

	

#ifdef _DEBUG
	FILE *stream;
	stream=fopen("round.log","a");
	float date;
	pblock->GetValue( PB_DATE, t, date, valid );
	float time;
	pblock->GetValue( PB_TIME, t, time, valid );
	fprintf(stream, "Yr: %d\tMon: %d\tDay: %d\tHr: %d\tMin: %d\tSec: %d\n", year, month, day, hour, min, sec );
/*	printf( "UYr: %d\tUMon: %d\tUDay: %d\tUHr: %d\tUMin: %d\tUSec: %d\n", hourSpin->GetIVal(),\
		monthSpin->GetIVal(), daySpin->GetIVal(), hourSpin->GetIVal(),\
		minSpin->GetIVal(), secSpin->GetIVal() );*/
	fprintf(stream, "Date: %f\tOffset: %d\tTime: %f\tOffset: %f\n", date, dateref, time, timeref );
  	fprintf(stream, "Lat: %f\tLong: %f\tZone: %ld\n\n", latitude, longitude, zone );
	fprintf(stream, "Az: %f\tAlt: %f\tSt: %f\n\n\n", az, alt, st );
	fclose(stream);
#endif

}


//method to rotate the helper object from the UI of the master controller
void SunMaster::align_north(TimeValue now, float north){
	assert(thePoint);

	// animate off and suspend msg processing
	ignore_point_msgs = TRUE;
	SuspendAnimate();
	AnimateOff();

	AffineParts lparts;
	float ang[3];
	INode *parent;
	Matrix3 nodeTM, parentTM, localTM, newTM;
  	parent = thePoint->GetParentNode();
	nodeTM = thePoint->GetNodeTM(now);
	parentTM = parent->GetNodeTM(now);
	localTM = nodeTM * Inverse(parentTM);
	decomp_affine(localTM, &lparts);
	QuatToEuler(lparts.q, ang);

	int turns = (int) (ang[2]/FTWO_PI);
	float subturn = ang[2] - turns;//we only affect the scrap
	ang[2]= turns + fdtr(compass_to_stdang(north));
	
	// build a new matrix
	EulerToQuat(ang, lparts.q);
	lparts.q.MakeMatrix(newTM);
	newTM.SetTrans(lparts.t);

	// animate back on and msg processing back to normal
	thePoint->SetNodeTM(now, newTM);
	ResumeAnimate();
	ignore_point_msgs = FALSE;
}

void SunMaster::NotifyNatLightManualChanged(IDaylightSystem2::DaylightControlType type)
{
	TimeValue t = GetCOREInterface()->GetTime();
	Interface* ip = GetCOREInterface();
	INodeTab nodes;
	GetSystemNodes(nodes,kSNCFileSave);
	for(int i=0;i<nodes.Count();++i)
	{
		if(nodes[i])
		{
			Object* daylightAssemblyObj = nodes[i]->GetObjectRef();
			BaseInterface* bi = daylightAssemblyObj->GetInterface(IID_DAYLIGHT_SYSTEM2);
			if(bi)
			{
				NatLightAssembly *nLA = dynamic_cast<NatLightAssembly*>(daylightAssemblyObj);
				nLA->SunMasterManualChanged(type);
			}
		}
	}
	
}
void SunMaster::ValuesUpdated()
{

	 INodeMonitor* nodeMon = NULL;
	 if (NULL != theObjectMon && 
			 NULL != (nodeMon = static_cast<INodeMonitor*>(theObjectMon->GetInterface(IID_NODEMONITOR))) &&
			 NULL != nodeMon->GetNode()) 
	 {
		 NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	 }
}

// The Dialog Proc

INT_PTR CALLBACK MasterParamDialogProc( HWND hDlg, UINT message, 
	WPARAM wParam, LPARAM lParam )
	{
	TCHAR buf[10];
	SunMaster *mc = DLGetWindowLongPtr<SunMaster *>( hDlg);
	if ( !mc && message != WM_INITDIALOG ) return FALSE;
	TimeValue now = mc->iObjParams->GetTime();

	assert(mc->iObjParams);
	switch ( message ) {
		int year, month,day,leap,modays;// locals for handling date changes
		case WM_INITDIALOG: {
			mc = (SunMaster *)lParam;
			DLSetWindowLongPtr( hDlg, mc);
			SetDlgFont( hDlg, mc->iObjParams->GetAppHFont() );

			// set up the date locals based on the current UI
			year = mc->GetYr(now);
			month = mc->GetMon(now);
			day = mc->GetDay(now);
			leap = isleap(year);
			if (month == 12) modays = 31;
			else modays = GetMDays(leap, month)-GetMDays(leap, month-1);

			// reset these copies
			holdLastCity = -1;
			strncpy(holdLastName, mc->GetCity(), sizeof(holdLastName)/ sizeof(holdLastName[0]));
			
			mc->radSpin  = GetISpinner(GetDlgItem(hDlg,IDC_RADSPINNER));
			mc->latSpin  = GetISpinner(GetDlgItem(hDlg,IDC_LATSPINNER));
			mc->longSpin  = GetISpinner(GetDlgItem(hDlg,IDC_LONGSPINNER));
			mc->yearSpin  = GetISpinner(GetDlgItem(hDlg,IDC_YEARSPINNER));
			mc->monthSpin  = GetISpinner(GetDlgItem(hDlg,IDC_MONTHSPINNER));
			mc->daySpin  = GetISpinner(GetDlgItem(hDlg,IDC_DAYSPINNER));
			mc->secSpin  = GetISpinner(GetDlgItem(hDlg,IDC_SECSPINNER));
			mc->hourSpin  = GetISpinner(GetDlgItem(hDlg,IDC_HOURSPINNER));
			mc->minSpin  = GetISpinner(GetDlgItem(hDlg,IDC_MINSPINNER));
			mc->northSpin  = GetISpinner(GetDlgItem(hDlg,IDC_NORTHSPINNER));
			mc->zoneSpin  = GetISpinner(GetDlgItem(hDlg,IDC_ZONESPINNER));
#ifndef NO_DAYLIGHT_SKY_COVERAGE_SLIDER
			mc->skySlider = mc->daylightSystem ? GetISlider(GetDlgItem(hDlg,IDC_SKY_COVER_SLIDER)) : NULL;
#endif // NO_DAYLIGHT_SKY_COVERAGE_SLIDER
			mc->azEdit  = GetICustStatus(GetDlgItem(hDlg,IDC_AZ));
			mc->altEdit  = GetICustStatus(GetDlgItem(hDlg,IDC_ALT));
			mc->cityDisplay  = GetICustStatus(GetDlgItem(hDlg,IDC_CITY));

			mc->radSpin->SetLimits( MINRADIUS,MAXRADIUS, FALSE );
			mc->latSpin->SetLimits( -90.0f, 90.0f, FALSE );
			mc->longSpin->SetLimits( -180.0f, 180.0f, FALSE );
			mc->yearSpin->SetLimits(MINYEAR,MAXYEAR, FALSE );
			mc->zoneSpin->SetLimits(-12,12, FALSE );
			mc->monthSpin->SetLimits(1,12, FALSE );
			mc->daySpin->SetLimits( 1, modays, FALSE );
			mc->hourSpin->SetLimits(0,23, FALSE );
			mc->minSpin->SetLimits( 0, 59, FALSE );
			mc->secSpin->SetLimits( 0, 59, FALSE );
			mc->northSpin->SetLimits(0.0f, 359.99f, FALSE );

			mc->radSpin->SetAutoScale( TRUE );
			mc->latSpin->SetScale(float(0.1) );
			mc->longSpin->SetScale(float(0.1) );
			mc->yearSpin->SetScale(float(1.0) );
			mc->monthSpin->SetScale(float(1.0) );
			mc->daySpin->SetScale(float(1.0) );
			mc->hourSpin->SetScale(float(1.0) );
			mc->minSpin->SetScale(float(1.0) );
			mc->secSpin->SetScale(float(1.0) );
			mc->northSpin->SetScale(float(1.0) );
			mc->zoneSpin->SetScale(float(1.0) );

			SetupFloatSlider(hDlg, IDC_SKY_COVER_SLIDER, 0, 0, 1,
				mc->GetSkyCondition(now), 2);

			mc->latSpin->SetValue( mc->GetLat(now), FALSE );
			mc->longSpin->SetValue( mc->GetLong(now), FALSE );
			mc->radSpin->SetValue( mc->GetRad(now), FALSE );
			mc->hourSpin->SetValue( mc->GetHour(now), FALSE );
			mc->minSpin->SetValue( mc->GetMin(now), FALSE );
			mc->secSpin->SetValue( mc->GetSec(now), FALSE );
			mc->monthSpin->SetValue( month, FALSE );
			mc->daySpin->SetValue( mc->GetDay(now), FALSE );
			mc->yearSpin->SetValue( year , FALSE );
			mc->northSpin->SetValue( mc->GetNorth(now), FALSE );
			mc->zoneSpin->SetValue( mc->GetZone(now), FALSE );
			CheckDlgButton(hDlg,IDC_DST, mc->GetDst(now));
		 _itot_s(static_cast<int>(rtd(mc->Getaz())), buf, (sizeof(buf) / sizeof(buf[0])), 10);
			mc->azEdit->SetText(buf);
		 _itot_s(static_cast<int>(rtd(mc->Getalt())), buf, (sizeof(buf) / sizeof(buf[0])), 10);
			mc->altEdit->SetText(buf);
			mc->cityDisplay->SetText(mc->GetCity());

			mc->radSpin->LinkToEdit( GetDlgItem(hDlg,IDC_RADIUS), EDITTYPE_POS_UNIVERSE );			
			mc->latSpin->LinkToEdit( GetDlgItem(hDlg,IDC_LAT), EDITTYPE_FLOAT );			
			mc->longSpin->LinkToEdit( GetDlgItem(hDlg,IDC_LONG), EDITTYPE_FLOAT );			
			mc->yearSpin->LinkToEdit( GetDlgItem(hDlg,IDC_YEAR), EDITTYPE_INT );			
			mc->monthSpin->LinkToEdit( GetDlgItem(hDlg,IDC_MONTH), EDITTYPE_INT );			
			mc->daySpin->LinkToEdit( GetDlgItem(hDlg,IDC_DAY), EDITTYPE_INT );			
			mc->hourSpin->LinkToEdit( GetDlgItem(hDlg,IDC_HOUR), EDITTYPE_INT );			
			mc->minSpin->LinkToEdit( GetDlgItem(hDlg,IDC_MIN), EDITTYPE_INT );			
			mc->secSpin->LinkToEdit( GetDlgItem(hDlg,IDC_SEC), EDITTYPE_INT );			
			mc->northSpin->LinkToEdit( GetDlgItem(hDlg,IDC_NORTH), EDITTYPE_FLOAT );			
 			mc->zoneSpin->LinkToEdit( GetDlgItem(hDlg,IDC_ZONE), EDITTYPE_INT );			

			// Set up icon	
			ICustButton *pBut = GetICustButton(GetDlgItem(hDlg,IDC_SETUP));
			if (pBut)
			{
				if (!mc->hIconImages)
				{
					mc->hIconImages  = ImageList_Create(16, 15, ILC_COLOR24 | ILC_MASK, 6, 0);
					LoadMAXFileIcon("daylight", mc->hIconImages, kBackground, FALSE);
				}	
				if(mc->hIconImages)
				{
					pBut->SetImage(mc->hIconImages, 4,4,5,5,12,11);
					ReleaseICustButton(pBut);
				}
				//pBut->SetTooltip(TRUE, MaxSDK::GetResourceString(IDS_SETUP));
			}

			mc->iObjParams->RedrawViews(now, REDRAW_INTERACTIVE, mc);
			}

			return FALSE;	// DB 2/27

		case WM_DESTROY:
			ReleaseISpinner( mc->radSpin );
			ReleaseISpinner( mc->latSpin );
			ReleaseISpinner( mc->longSpin );
			ReleaseISpinner( mc->daySpin );
			ReleaseISpinner( mc->monthSpin );
			ReleaseISpinner( mc->yearSpin );
			ReleaseISpinner( mc->hourSpin );
			ReleaseISpinner( mc->minSpin );
			ReleaseISpinner( mc->secSpin );
			ReleaseISpinner( mc->northSpin );
			ReleaseISpinner( mc->zoneSpin );
			ReleaseICustStatus( mc->azEdit);
			ReleaseICustStatus( mc->altEdit);
			ReleaseICustStatus( mc->cityDisplay);
			mc->radSpin  = NULL;
			mc->latSpin  = NULL;
			mc->longSpin  = NULL;
			mc->yearSpin  = NULL;
			mc->monthSpin  = NULL;
			mc->daySpin  = NULL;
			mc->hourSpin  = NULL;
			mc->minSpin  = NULL;
			mc->secSpin  = NULL;
			mc->azEdit  = NULL;
			mc->altEdit  =NULL;
			mc->cityDisplay  = NULL;
			mc->northSpin  =NULL;
			mc->zoneSpin  =NULL;


			return FALSE;

		case CC_SPINNER_BUTTONDOWN:
			theHold.Begin();
			return TRUE;


		case CC_SPINNER_CHANGE:
			if (!theHold.Holding())
				theHold.Begin();
			year = mc->GetYr(now);
			month = mc->GetMon(now);
			day = mc->GetDay(now);
			leap = isleap(year);
			if (month == 12) modays = 31;
			else modays = GetMDays(leap, month)-GetMDays(leap, month-1);


			switch ( LOWORD(wParam) ) {
				case IDC_RADSPINNER: mc->SetRad(now,  mc->radSpin->GetFVal() );  break;
				case IDC_LATSPINNER:
					mc->SetLat(now,  mc->latSpin->GetFVal() );
					mc->SetCity(NULL);
					break;
				case IDC_LONGSPINNER:
					mc->SetLong(now,  mc->longSpin->GetFVal() );
					mc->SetCity(NULL);
					break;
				case IDC_HOURSPINNER: mc->SetHour(now,  mc->hourSpin->GetIVal() );  break;
				case IDC_MINSPINNER: mc->SetMin(now,  mc->minSpin->GetIVal() );  break;
				case IDC_SECSPINNER: mc->SetSec(now,  mc->secSpin->GetIVal() );  break;
				case IDC_MONTHSPINNER:
					month =  mc->monthSpin->GetIVal();
					if (month == 12) modays = 31;
					else modays = GetMDays(leap, month)-GetMDays(leap, month-1);
					if (day > modays){
						day=modays;
						mc->SetDay(now,  day );}
					mc->daySpin->SetLimits(1,modays,FALSE);
					mc->SetMon(now,  month );
					break;
				case IDC_DAYSPINNER: mc->SetDay(now,  mc->daySpin->GetIVal() );  break;
				case IDC_YEARSPINNER: 
					year =  mc->yearSpin->GetIVal();
					leap = isleap(year);
					if (month == 12) modays = 31;
					else modays = GetMDays(leap, month)-GetMDays(leap, month-1);
					if (day > modays){
						day=modays;
						mc->SetDay(now,  day );}
					mc->daySpin->SetLimits(1,modays,FALSE);
					mc->SetYr(now,  year );
					break;
				case IDC_NORTHSPINNER:mc->SetNorth(now,  mc->northSpin->GetFVal() );  break;
				case IDC_ZONESPINNER: mc->SetZone(now,  mc->zoneSpin->GetIVal() );  break;
				}
			//Notify dependents: they will call getvalue which will update UI
			//else : calculate and Update UI directly
				 {
					 INodeMonitor* nodeMon = NULL;
					 if (NULL != mc->theObjectMon && 
							 NULL != (nodeMon = static_cast<INodeMonitor*>(mc->theObjectMon->GetInterface(IID_NODEMONITOR))) &&
							 NULL != nodeMon->GetNode()) 
					 {
						 mc->NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
					 }
			else {
				mc->calculate(now,FOREVER);
				mc->UpdateUI(now);
			}
				 }
			assert(mc->iObjParams);
			mc->iObjParams->RedrawViews(now, REDRAW_INTERACTIVE, mc);
			return TRUE;

		case WM_CUSTEDIT_ENTER:
		case CC_SPINNER_BUTTONUP:
			if (!HIWORD(wParam) && message != WM_CUSTEDIT_ENTER)
			{
				theHold.Cancel();
				switch (LOWORD(wParam))
				{
					case IDC_LATSPINNER:
					case IDC_LONGSPINNER:
						// reset if spinner operation is cancelled
						lastCity = holdLastCity;
						mc->SetCity(holdLastName);
						mc->calculate(now,FOREVER);
						mc->UpdateUI(now);
						break;
				}
			} else {
				if ((holdLastCity != -1 || *holdLastName != '\0') && theHold.Holding())
					theHold.Put(new CityRestore(mc, holdLastName));
				strncpy(holdLastName, mc->GetCity(), sizeof(holdLastName)/ sizeof(holdLastName[0]));
				holdLastCity = -1;
//				mc->iObjParams->RedrawViews(now, REDRAW_END, mc);
				theHold.Accept(const_cast<TCHAR*>(MaxSDK::GetResourceString(IDS_UNDO_PARAM)));
			}
			return TRUE;
			 break;

		case CC_SLIDER_CHANGE:
			{
			if (!theHold.Holding())
				theHold.Begin();
#ifndef NO_DAYLIGHT_SKY_COVERAGE_SLIDER
			switch ( LOWORD(wParam) ) {
				case IDC_SKY_COVER_SLIDER:
					if (mc->daylightSystem)
						mc->SetSkyCondition(now, mc->skySlider->GetFVal());
					break;
			}
#endif // NO_DAYLIGHT_SKY_COVERAGE_SLIDER
			//Notify dependents: they will call getvalue which will update UI
			//else : calculate and Update UI directly
				 INodeMonitor* nodeMon = NULL;
				 if (NULL != mc->theObjectMon && 
						 NULL != (nodeMon = static_cast<INodeMonitor*>(mc->theObjectMon->GetInterface(IID_NODEMONITOR))) &&
						 NULL != nodeMon->GetNode()) 
				 {
					mc->NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
				 }
			else {
				mc->UpdateUI(now);
			}

			assert(mc->iObjParams);
			mc->iObjParams->RedrawViews(now, REDRAW_INTERACTIVE, mc);
			return TRUE;
			}
			break;
		case CC_SLIDER_BUTTONDOWN:
			theHold.Begin();
			return TRUE;

		case CC_SLIDER_BUTTONUP:
			if (! HIWORD(wParam)) {
				theHold.Cancel();
			} else {
				theHold.Accept(const_cast<TCHAR*>(MaxSDK::GetResourceString(IDS_UNDO_PARAM)));
			}
			return TRUE;

		case WM_MOUSEACTIVATE:
			mc->iObjParams->RealizeParamPanel();
			return FALSE;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:
			mc->iObjParams->RollupMouseMessage(hDlg,message,wParam,lParam);
			return FALSE;

		case WM_COMMAND:
			{
				switch(LOWORD(wParam)) { // Switch on ID
				default:
					return FALSE;
				case IDC_MANUAL_POSITION: {
					theHold.Begin();
					mc->SetManual(now,NatLightAssembly::MANUAL_MODE);
					theHold.Accept(const_cast<TCHAR*>(MaxSDK::GetResourceString(IDS_UNDO_PARAM)));
				} break;

				case IDC_CONTROLLER_POS: {
					theHold.Begin();
					mc->SetManual(now,NatLightAssembly::DATETIME_MODE);
					theHold.Accept(const_cast<TCHAR*>(MaxSDK::GetResourceString(IDS_UNDO_PARAM)));
				} break;

				case IDC_WEATHER_DATA_FILE:{
					theHold.Begin();
					mc->SetManual(now,NatLightAssembly::WEATHER_MODE);
					theHold.Accept(const_cast<TCHAR*>(MaxSDK::GetResourceString(IDS_UNDO_PARAM)));
				} break;
				case IDC_SETUP:
					{
						TimeValue t = GetCOREInterface()->GetTime();
						if (HIWORD(wParam) == BN_CLICKED&&mc->GetManual(t)==NatLightAssembly::WEATHER_MODE)
						{
							Interface* ip = GetCOREInterface();
							INodeTab nodes;
							mc->GetSystemNodes(nodes,kSNCFileSave);
							for(int i=0;i<nodes.Count();++i)
							{
								if(nodes[i])
								{
									Object* daylightAssemblyObj = nodes[i]->GetObjectRef();
									BaseInterface* bi = daylightAssemblyObj->GetInterface(IID_DAYLIGHT_SYSTEM2);
									if(bi)
									{
										IDaylightSystem2* ds = static_cast<IDaylightSystem2*>(bi);
										ds->OpenWeatherFileDlg();
										break;
									}
								}
							}
						}
					}
					break;
				// The user clicked the daylight savings checkbox.
				case IDC_DST:
					theHold.Begin();
					mc->SetDst(now,IsDlgButtonChecked(mc->hMasterParams,IDC_DST));
					theHold.Accept(const_cast<TCHAR*>(MaxSDK::GetResourceString(IDS_UNDO_PARAM)));
					break;
				case IDC_GETLOC: 
					float lat = mc->GetLat(now);
					float longi = mc->GetLong(now);
					char city[256] = "";
					if (doLocationDialog(hDlg, mc->iObjParams, &lat, &longi, city)) {
						// the "holdLastCity" string may be out of sync with the master
						// since it only shows the currently selected master regardless
						// of which one the undo action will modify
						std::basic_string<TCHAR> oldCity;
						if(NULL != mc->GetCity())
						{
							oldCity = mc->GetCity();
						}
						int tz = getTimeZone(longi);
						theHold.Begin();
						mc->SetCity(city);		// Set this first so updates triggered by
						mc->SetLat(now,lat);	// setting other values will update the UI.
						mc->SetLong(now,longi);
						mc->SetZone(now,tz);
						theHold.Put(new CityRestore(mc, oldCity.c_str()));
						theHold.Accept(const_cast<TCHAR*>(MaxSDK::GetResourceString(IDS_UNDO_PARAM)));
						// set these copies
						holdLastCity = lastCity;
						strcpy(holdLastName, city);
					}
					break;
				}
				 INodeMonitor* nodeMon = NULL;
				 if (NULL != mc->theObjectMon && 
						 NULL != (nodeMon = static_cast<INodeMonitor*>(mc->theObjectMon->GetInterface(IID_NODEMONITOR))) &&
						 NULL != nodeMon->GetNode()) 
				 {
					 mc->NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
				 }
			else {
				mc->calculate(now,FOREVER);
				mc->UpdateUI(now);
			}
			mc->iObjParams->RedrawViews(now, REDRAW_INTERACTIVE, mc);
			return TRUE;
			}
			break;
		default:
			return FALSE;
		}
	}



// This method is called when the sun masters parameters may be edited
// in the motion branch of the command panel.  
void SunMaster::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev )
	{
	// Save the interface pointer passed in.  This pointer is only valid
	// between BeginEditParams() and EndEditParams().
	iObjParams = ip;
	SetInCreate( (flags & BEGIN_EDIT_CREATE) != 0 );

	if ( !hMasterParams  ) {
		// Add the rollup page to the command panel. This method sets the
		// dialog proc used to manage the user interaction with the dialog
		// controls.


		hMasterParams = ip->AddRollupPage( 
				MaxSDK::GetHInstance(), 
				MAKEINTRESOURCE(daylightSystem ? IDD_DAYPARAM : IDD_SUNPARAM),
				MasterParamDialogProc,
				const_cast<TCHAR*>(MaxSDK::GetResourceString(IDS_SUN_DLG_NAM)), 
				(LPARAM)this
#if defined(NO_MOTION_PANEL) || defined(NO_DAYLIGHT_MOTION_PANEL)
				, 0,
				(ROLLUP_CAT_STANDARD + 2 * ROLLUP_CAT_CUSTATTRIB) / 3
#endif	// defined(NO_MOTION_PANEL) || defined(NO_DAYLIGHT_MOTION_PANEL)
				);
		
	} else {
		DLSetWindowLongPtr( hMasterParams, this);    
/*
		// Init the dialog to our values.
		radSpin->SetValue(GetRad(ip->GetTime()),FALSE);
		latSpin->SetValue(GetLat(ip->GetTime()),FALSE);
		longSpin->SetValue(GetLong(ip->GetTime()),FALSE);
		hourSpin->SetValue(GetHour(ip->GetTime()),FALSE);
		minSpin->SetValue(GetMin(ip->GetTime()),FALSE);
		secSpin->SetValue(GetSec(ip->GetTime()),FALSE);
		monthSpin->SetValue(GetMon(ip->GetTime()),FALSE);
		daySpin->SetValue(GetDay(ip->GetTime()),FALSE);
		yearSpin->SetValue(GetYr(ip->GetTime()),FALSE);
		BOOL temp = GetDst(ip->GetTime());
		CheckDlgButton(hMasterParams,IDC_DST,temp );
		*/
		}
	//JH 11/20/00
	//FIx for 136634 Commented out the manual updates to the UI and just call update Ui in both cases.
	UpdateUI(ip->GetTime());
	
	}
		
// This method is called when the user is finished editing the sun masters
// parameters in the command panel.		
void SunMaster::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next )
{
	if (hMasterParams==NULL) {
		return;
		}

	if ( flags&END_EDIT_REMOVEUI ) 
	 {    
		ip->UnRegisterDlgWnd(hMasterParams);
		ip->DeleteRollupPage(hMasterParams);
		hMasterParams = NULL;
		}
	else 
	 {      
		DLSetWindowLongPtr( hMasterParams, 0);
		}


	hMasterParams = NULL;
	iObjParams = NULL;
}

#define VERSION_CHUNK 0x130
#define CITY_CHUNK		0x120
#define NODE_ID_CHUNK 0x110 // obsolete data
#define EPOCH_CHUNK 0x100
#define NEW_ANIM_CHUNK 0xf0

IOResult SunMaster::Save(ISave *isave) 
{
	ULONG nb;
	IOResult res;

	isave->BeginChunk(CITY_CHUNK);
		res = isave->WriteCString(city);
		assert(res == IO_OK);
	isave->EndChunk();

	isave->BeginChunk(EPOCH_CHUNK);
		isave->Write(&dateref,sizeof(ULONG), &nb);
	isave->EndChunk();

	if (!mbOldAnimation) 
	{
	    isave->BeginChunk(NEW_ANIM_CHUNK);
	    isave->EndChunk();
    }

	return IO_OK;
	}

class SunMasterPostLoad : public PostLoadCallback {
public:
    SunMaster *bo;
    SunMasterPostLoad(SunMaster *b) {bo=b;}
	void replaceSunlightWithDaylight();
    void proc(ILoad *iload) {
		int sunMasterVersion = bo->daylightSystem
			? SUNMASTER_VERSION_DAYLIGHT : SUNMASTER_VERSION_SUNLIGHT;
		int nParams = bo->daylightSystem ? 9 : 7;
		ParamBlockDescID* pDesc = bo->daylightSystem ? desc4Daylight : desc1;
        if (bo->pblock->GetVersion() != sunMasterVersion) {
            switch (bo->pblock->GetVersion()) {
            case 0:
                bo->ReplaceReference(REF_PARAM,
                                     UpdateParameterBlock(
                                         desc, 7, bo->pblock,
                                         pDesc, nParams, sunMasterVersion));
				bo->SetZone(TimeValue(0),-1 * bo->GetZone(TimeValue(0)));
				bo->SetLong(TimeValue(0),-1 * bo->GetLong(TimeValue(0)));
                iload->SetObsolete();
                break;

            case 1:
				bo->ReplaceReference(REF_PARAM,
										 UpdateParameterBlock(
											 desc1, 7, bo->pblock,
											 pDesc, nParams, sunMasterVersion));
				//if sun no change to any reference, just reset long still
				bo->SetLong(TimeValue(0),-1 * bo->GetLong(TimeValue(0)));
                iload->SetObsolete();
                break;

            case 2:
                bo->ReplaceReference(REF_PARAM,
                                     UpdateParameterBlock(
                                         desc2Daylight, 8, bo->pblock,
                                         desc3Daylight, 9, SUNMASTER_VERSION_DAYLIGHT));
				bo->daylightSystem = true;
				bo->SetLong(TimeValue(0),-1 * bo->GetLong(TimeValue(0)));
                iload->SetObsolete();
                break;
			   case 3:
                bo->ReplaceReference(REF_PARAM,
                                     UpdateParameterBlock(
                                         desc3Daylight, 9, bo->pblock,
                                         desc4Daylight, 9, SUNMASTER_VERSION_DAYLIGHT));
				bo->daylightSystem = true;
				bo->SetLong(TimeValue(0),-1 * bo->GetLong(TimeValue(0)));
                iload->SetObsolete();
                break;

            case 4:
				 bo->ReplaceReference(REF_PARAM,
                                     UpdateParameterBlock(
                                         desc3Daylight, 9, bo->pblock,
                                         desc4Daylight, 9, SUNMASTER_VERSION_DAYLIGHT));
				bo->daylightSystem = true;
				bo->SetLong(TimeValue(0),-1 * bo->GetLong(TimeValue(0)));
                iload->SetObsolete();
                break;
            default:
                assert(0);
                break;
            }
        }

				// Transform the persisted pointer to the Sun\Daylight node, into a node monitor
				if (bo->theLegacyObject != NULL) 
				{
					INodeMonitor* nodeMon = static_cast<INodeMonitor*>(bo->theObjectMon->GetInterface(IID_NODEMONITOR));
					DbgAssert(nodeMon != NULL);
					nodeMon->SetNode(bo->theLegacyObject);
					bo->theLegacyObject = NULL;
				}
        delete this;
    }
};

IOResult SunMaster::Load(ILoad *iload) 
{
	// Legacy max files do not contain a RingMaster version number
	IOResult res = IO_OK;
	USHORT next = iload->PeekNextChunkID();
	if (next != VERSION_CHUNK) {
		res = DoLoadLegacy(iload);
	}
	else {
		res = DoLoad(iload);
	}
	return res;
}


IOResult SunMaster::DoLoad(ILoad *iload)
{
	IOResult res;

	while (IO_OK == (res = iload->OpenChunk())) 
	{
		switch (iload->CurChunkID()) 
		{
			case VERSION_CHUNK:
			{
				ULONG 	nb;
				iload->Read(&mVerLoaded, sizeof(unsigned short), &nb);
				if (kVERSION_CURRENT > mVerLoaded) {
					iload->SetObsolete();
				}
			}
			break;
		}
		iload->CloseChunk();
	}
	return IO_OK;
}

IOResult SunMaster::DoLoadLegacy(ILoad *iload) 
{
	ULONG nb;
	TCHAR	*cp;
	IOResult res;

    mbOldAnimation = TRUE;
	iload->RegisterPostLoadCallback(new SunMasterPostLoad(this));
	while (IO_OK == (res = iload->OpenChunk())) 
	{
		switch (iload->CurChunkID())  
		{
			case CITY_CHUNK:
					res = iload->ReadCStringChunk(&cp);
				if (res == IO_OK) {
						SetCity(cp);
				}
				break;
			case NODE_ID_CHUNK:
					ULONG id;
					iload->Read(&id,sizeof(ULONG), &nb);
				if (id != 0xffffffff) {
					iload->RecordBackpatch(id, (void**)&theLegacyObject);
				}
				break;
			case EPOCH_CHUNK:
					iload->Read(&dateref,sizeof(LONG), &nb);
				if (dateref != 0) {
						refset = TRUE;
				}
				break;
			case NEW_ANIM_CHUNK:
					mbOldAnimation = FALSE;
				break;
			}

		iload->CloseChunk();
		if (res != IO_OK) {
			return res;
		}
	} // while

	tvalid = NEVER;

	return IO_OK;
	}

void SunMaster::TimeChanged(TimeValue t)
{
	if(controlledByWeatherFile&&natLight!=NULL)
	{
		Interval valid;
		natLight->SetEnergyValuesBasedOnWeatherFile(t,valid);
	}
	UpdateUI(t);
}

/*===========================================================================*\
 | Sun Slave Controller  Methods:
\*===========================================================================*/


// This method returns a new instance of the slave controller.
Control* GetNewSlaveControl(SunMaster *master, int i) {
	return new SlaveControl(master,i);
	}

// Constructor.  
SlaveControl::SlaveControl(const SlaveControl& ctrl) {
	daylightSystem = ctrl.daylightSystem;
	master = ctrl.master;
	id = ctrl.id;
	}

void SlaveControl::GetClassName(TSTR& s)
{
   s = MaxSDK::GetResourceString(id == LIGHT_TM ? IDS_SLAVE_POS_CLASS : IDS_SLAVE_FLOAT_CLASS);
}

// This constructor creates a reference from the slave controller to 
// the sun master object.
SlaveControl::SlaveControl(const SunMaster* m, int i) {
	daylightSystem = m->daylightSystem;
	id = i;
	master = NULL;
	ReplaceReference( 0, (ReferenceTarget *)m);
	}

SClass_ID SlaveControl::SuperClassID()
{
	switch (id) {
	case LIGHT_TM:
		return CTRL_POSITION_CLASS_ID;
	}
	return CTRL_FLOAT_CLASS_ID;
}

// This method is called to create a copy of the slave controller.
RefTargetHandle SlaveControl::Clone(RemapDir& remap) {
	SlaveControl *sl = new SlaveControl(daylightSystem);
	sl->id = id;
	sl->ReplaceReference(0, remap.CloneRef(master));
	BaseClone(this, sl, remap);
	return sl;
	}

SlaveControl& SlaveControl::operator=(const SlaveControl& ctrl) {
	daylightSystem = ctrl.daylightSystem;
	master = ctrl.master;
	id = ctrl.id;
	return (*this);
	}

// ========= This method is used to retrieve the value of the 
// controller at the specified time. =========

// This is a important aspect of the system plug-in - this method 
// calls the master object to get the value.

void SlaveControl::GetValue(TimeValue t, void *val, Interval &valid, 
	GetSetMethod method) {
	// Ensure the ring master exists.
	assert(master);
	master->GetValue(t,val,valid,method,id);	
	}

void SlaveControl::SetValue(TimeValue t, void *val, int commit, 
	GetSetMethod method)
{
	master->SetValue(t, val, commit, method, id);
	}

void* SlaveControl::GetInterface(ULONG id) {
	if (id==I_MASTER) 
		return (ReferenceTarget*)master;
	else 
		return Control::GetInterface(id);
	}

const int kSlaveIDChunk = 0x4444;
IOResult SlaveControl::Save(ISave *isave)
{
	isave->BeginChunk(kSlaveIDChunk);
	ULONG nb;
	IOResult res = isave->Write(&id, sizeof(id), &nb);
	if (res == IO_OK && nb != sizeof(id))
		res = IO_ERROR;
	isave->EndChunk();
	return res;
}

class SlaveControlPostLoad : public PostLoadCallback {
public:
	SlaveControlPostLoad(SlaveControl* slave) : mpSlave(slave) {}

	void proc(ILoad* iload) {
		if (mpSlave->master != NULL) {
			if (mpSlave->daylightSystem != mpSlave->master->daylightSystem) {
				mpSlave->daylightSystem = mpSlave->master->daylightSystem;
				iload->SetObsolete();
			}
		}
		delete this;
	}

private:
	SlaveControl*	mpSlave;
};

IOResult SlaveControl::Load(ILoad *iload)
{
	ULONG nb;
	IOResult res;

	iload->RegisterPostLoadCallback(new SlaveControlPostLoad(this));

	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
		case kSlaveIDChunk:
			res = iload->Read(&id, sizeof(id), &nb);
			if (res != IO_OK && nb != sizeof(id))
				res = IO_ERROR;
			break;
		}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
	}

	return IO_OK;
}

// These are the API methods
void SlaveControl::GetSunTime( TimeValue t, SYSTEMTIME&	sunt ){
	master->GetTime(t,FOREVER);
	int hours = master->GetZone(t,FOREVER);
	if(master->GetDst(t,FOREVER)) hours--;

	
	sunt.wYear = master->theTime.wYear;
	sunt.wMonth = master->theTime.wMonth;
	sunt.wDay = master->theTime.wDay;

	sunt.wHour = (master->theTime.wHour + hours)%24;
	sunt.wMinute = master->theTime.wMinute;
	sunt.wSecond = master->theTime.wSecond;
}


 void SlaveControl::GetSunLoc(TimeValue t, Point2& origin, Point2& orient){

//	master.GetTime(t, FOREVER);
//	master.Calculate(t,FOREVER);
	
	origin.x = master->GetLat(t, FOREVER);
	origin.y = master->GetLong(t, FOREVER);
	Point2 temp =master->GetAzAlt(t,FOREVER); 
	orient.x = temp.x;
	orient.y = temp.y;
}


#if defined(NO_MOTION_PANEL) || defined(NO_DAYLIGHT_MOTION_PANEL)
bool SunMaster::IsCreateModeActive()
{
	return SunMasterCreateMode::GetInstance()->IsActive();
}
#endif	// defined(NO_MOTION_PANEL) || defined(NO_DAYLIGHT_MOTION_PANEL)
