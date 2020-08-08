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
#ifndef _NATLIGHT_H_
#define _NATLIGHT_H_

/*===========================================================================*\
	FILE: natLight.h

	DESCRIPTION: Natural Light Assembly Class.

	HISTORY: Created July 27, 2001 by Cleve Ard

	Copyright (c) 2001, All Rights Reserved.
 \*==========================================================================*/

#pragma unmanaged
#include "NativeInclude.h"

#pragma managed
class SunMaster;
#include "weatherdata.h"

ClassDesc* GetNatLightAssemblyDesc();

 /*===========================================================================*\
 | Natural Light Assembly Class:
\*===========================================================================*/

#define NATLIGHT_HELPER_CLASS_ID Class_ID(0x4a1e6deb, 0x31c77d57)
 
class NatLightAssembly : public HelperObject, public IDaylightSystem3 {
public:
	enum {
		PBLOCK_REF = 0,
		SUN_REF = 1,
		SKY_REF = 2,
		MULT_REF = 3,
		INTENSE_REF = 4,
		SKY_COND_REF = 5,
		NUM_REFS = 6
	};

	enum {
		NUM_SUBS = 0
	};

	enum {
		MAIN_PBLOCK = 0,
		NUM_PBLOCK = 1
	};

	enum {
		SUN_PARAM = 0,
		SKY_PARAM = 1,
		MANUAL_PARAM = 2,
		WEATHER_FILE_PARAM = 3
	};

	//new ENUM for handling whether or not we are in manual, datatime, or weather file mode..
	//
	enum { 
		DATETIME_MODE = 0,
		MANUAL_MODE = 1,
		WEATHER_MODE = 2
	};
	NatLightAssembly();
	~NatLightAssembly();

	BaseInterface* GetInterface(Interface_ID id);
	
	static INode* NatLightAssembly::CreateAssembly(
		NatLightAssembly*&	natObj,
		IObjCreate*			createInterface,
		const Class_ID* sunClassID = NULL, // if NULL, use Sun type specified by market defaults
		const Class_ID* skyClassID = NULL  // if NULL, use Sky type specified by market defaults
	);

	static void AppendAssemblyNodes(
		INode*			theAssembly,
		INodeTab&		nodes,
		SysNodeContext
	);

	void SetSunAndSky(
		TimeValue			t,
		int					ref,
		ParamID				param,
		const TCHAR*		name,
		ReferenceTarget*	obj
	);

	LightObject* GetSun() const;
	LightObject* GetSky() const;

	void SetMultController(Control* c);
	void SetIntenseController(Control* c);
	void SetSkyCondController(Control* c);
	SunMaster* FindMaster() const;

	// From HelperObject

	virtual void InitNodeName(TSTR& s);

	// From Object

	virtual void GetDeformBBox(
		TimeValue	t,
		Box3&		box,
		Matrix3*	tm=NULL,
		BOOL		useSel=FALSE
	);
	virtual ObjectState Eval(TimeValue t);

	// From BaseObject

	virtual void SetExtendedDisplay(int flags);
	virtual int Display(
		TimeValue	t,
		INode*		inode,
		ViewExp*	vpt,
		int			flags
	);
	virtual int HitTest(
		TimeValue t,
		INode*		inode,
		int			type,
		int			crossing,
		int			flags,
		IPoint2*	p,
		ViewExp*	vpt
	);
	virtual void Snap(
		TimeValue	t,
		INode*		inode,
		SnapInfo*	snap,
		IPoint2*	p,
		ViewExp*	vpt
	);
	virtual void GetWorldBoundBox(
		TimeValue	t,
		INode*		inode,
		ViewExp*	vp,
		Box3&		box
	);
	virtual void GetLocalBoundBox(
		TimeValue	t,
		INode*		inode,
		ViewExp*	vp,
		Box3&		box
	);
	virtual BOOL HasViewDependentBoundingBox() { return true; }
	virtual CreateMouseCallBack* GetCreateMouseCallBack();
	virtual TCHAR *GetObjectName();
	virtual BOOL HasUVW();

	// From ReferneceTarget

	virtual RefTargetHandle Clone(RemapDir &remap = DefaultRemapDir());
	void EnumAuxFiles(NameEnumCallback& nameEnum, DWORD flags);
	// From ReferenceMaker

	virtual RefResult NotifyRefChanged(
		Interval		changeInt,
		RefTargetHandle	hTarget,
		PartID&			partID,
		RefMessage		message
	);
	virtual int NumRefs();
	virtual RefTargetHandle GetReference(int i);
	virtual void SetReference(int i, RefTargetHandle rtarg);

	virtual IOResult Save(ISave *isave);
	virtual IOResult Load(ILoad *iload);

	// From Animatable

	virtual void GetClassName(TSTR& s);
	virtual Class_ID ClassID();
	virtual void DeleteThis();

	virtual void BeginEditParams(
		IObjParam*	ip,
		ULONG		flags,
		Animatable*	prev=NULL
	);
	virtual void EndEditParams(
		IObjParam*	ip,
		ULONG		flags,
		Animatable*	next=NULL
	);

	virtual int NumSubs();
	virtual Animatable* SubAnim(int i);
	virtual TSTR SubAnimName(int i);
	virtual BOOL IsAnimated();

	virtual int NumParamBlocks();
	virtual IParamBlock2* GetParamBlock(int i);
	virtual IParamBlock2* GetParamBlockByID(short id);

	virtual int SetProperty(
		ULONG	id,
		void*	data
	);
	virtual void* GetProperty(ULONG id);


	//From IDaylightSystem
	virtual void   SetPosition(const Point3& position);
	virtual Point3 GetPosition() const;
	virtual void   SetOrbitalScale(float orbScale);
	virtual float  GetOrbitalScale() const;
	virtual void   SetNorthDirection(float angle);
	virtual float  GetNorthDirection() const;
	virtual void   SetCompassDiameter(float compassDiameter);
	virtual float  GetCompassDiameter() const;
	virtual void   SetTimeOfDay(const Point3& time);
	virtual Point3 GetTimeOfDay() const;
	virtual void   SetDate(const Point3& date);
	virtual Point3 GetDate() const;
	virtual void   SetLatLong(float latitude, float longitude);
	virtual float  GetLatitude() const;
	virtual float  GetLongitude() const;
	virtual void   SetDaylightSavingTime(BOOL isDaylightSavingTime);
	virtual BOOL   GetDaylightSavingTime() const;

	//From IDaylightSystem2
	virtual int GetTimeZone(float longitude) const;
	virtual LightObject* SetSun(const Class_ID& sunClassID);
	virtual LightObject* SetSky(const Class_ID& skyClassID);
	TSTR GetLocation();
	TSTR GetWeatherFileName()const;
	void SetWeatherFileName(TSTR &val);
	IDaylightSystem2::DaylightControlType GetDaylightControlType()const;
	void SetDaylightControlType(IDaylightSystem2::DaylightControlType val);
	void OpenWeatherFileDlg();
	void MatchTimeLine();
	void GetAltitudeAzimuth(TimeValue t, float &altidue,float &azimuth);
	bool GetTemps(float &dryBulbTemperature, float &dewPointTemperature);
	bool GetExtraTerrestialRadiation(float &extraterrestrialHorizontalRadiation, float &extraterrestrialDirectNormalRadiation);
	bool GetRadiation(float &globalHorizontalRadiation, float &directNormalRadiation,
											 float &diffuseHorizontalRadiation);
	bool GetIlluminance(float &globalHorizontalIlluminance, float &directNormalIlluminance,
											 float &diffuseHorizontalIlluminance, float &zenithLuminance);

	//From IDaylightSystem3
	void GetAltAz(TimeValue t, float &altidue,float &azimuth);

	//weather file function publishing  (not all of these guys are in the sdk which  is fine for now).
	int fpGetDaylightControlType();
	void fpSetDaylightControlType(int type);
	TCHAR * fpGetWeatherFileName();
	void LoadWeatherFile(TCHAR *filename);
	void SetWeatherFileAnimated(BOOL val);
	BOOL GetWeatherFileAnimated();
	void SetWeatherFileSpecificTimeAndDate(Point3 &time,Point3 &date);
	void GetWeatherFileSpecificTimeAndDate(Point3 &time,Point3 &date);
	void SetWeatherFileAnimStartTimeAndDate(Point3 &time,Point3 &date);
	void GetWeatherFileAnimStartTimeAndDate(Point3 &time,Point3 &date);
	void SetWeatherFileAnimEndTimeAndDate(Point3 &time,Point3 &date);
	void GetWeatherFileAnimEndTimeAndDate(Point3 &time,Point3 &date);
	void SetWeatherFileSkipHours(BOOL skipHours,int hourStart, int hourEnd);
	void GetWeatherFileSkipHours(BOOL &skipHours, int &hourStart, int  &hourEnd);
	void SetWeatherFileSkipWeekend(BOOL weekend);
	BOOL GetWeatherFileSkipWeekend();
	void SetWeatherFileFramePer(int frameper);
	int GetWeatherFileFramePer();

	// Function Publishing stuff
	enum 
	{ 
		eGetSun,
		eSetSun,
		eGetSky,
		eSetSky,
		/* TODO
		eSetPosition, 
		eGetPosition,
		eSetOrbitalScale, 
		eGetOrbitalScale,
		eSetNorthDirection,
		eGetNorthDirection,
		eSetCompassDiameter,
		eGetCompassDiameter,
		eSetTimeOfDay,
		eGetTimeOfDay,
		eSetDate,
		eGetDate,
		eSetLatitude,
		eGetLatitude,
		eSetLongitude,
		eGetLongitude,
		eEnableDaylightSavingsTime,
		eIsDaylightSavingsTime,
		eGetTimeZone,
		*/
		//following is added for weather file functionality
		eGetDaylightControlType,
		eSetDaylightControlType,
		eGetLocation,
		eGetAltitudeAzimuth,
		eGetWeatherFileName,
		eOpenWeatherFileDlg,
		eMatchTimeLine,
		eLoadWeatherFile,
		eSetWeatherFileFilterAnimated,
		eGetWeatherFileFilterAnimated,
		eSetWeatherFileSpecificTimeAndDate,
		eGetWeatherFileSpecificTimeAndDate,
		eSetWeatherFileAnimStartTimeAndDate,
		eGetWeatherFileAnimStartTimeAndDate,
		eSetWeatherFileAnimEndTimeAndDate,
		eGetWeatherFileAnimEndTimeAndDate,
		eSetWeatherFileSkipHours,
		eGetWeatherFileSkipHours,
		eSetWeatherFileSkipWeekend,
		eGetWeatherFileSkipWeekend,
		eSetWeatherFileSkipFramePer,
		eGetWeatherFileSkipFramePer
	}; 


	BEGIN_FUNCTION_MAP 
		FN_0(eGetSun, TYPE_OBJECT, GetSun);
		FN_1(eSetSun, TYPE_OBJECT, fpSetSun, TYPE_CLASS);
		FN_0(eGetSky, TYPE_OBJECT, GetSky);
		FN_1(eSetSky, TYPE_OBJECT, fpSetSky, TYPE_CLASS);
		
		FN_0(eGetDaylightControlType,TYPE_INT,fpGetDaylightControlType);
		VFN_1(eSetDaylightControlType,fpSetDaylightControlType,TYPE_INT);
		FN_0(eGetLocation,TYPE_TSTR_BV,GetLocation);
		VFN_3(eGetAltitudeAzimuth,GetAltitudeAzimuth,TYPE_TIMEVALUE,TYPE_FLOAT_BR,TYPE_FLOAT_BR);
		FN_0(eGetWeatherFileName,TYPE_FILENAME,fpGetWeatherFileName);
		VFN_0(eOpenWeatherFileDlg,OpenWeatherFileDlg);
		VFN_0(eMatchTimeLine, MatchTimeLine);
		VFN_1(eLoadWeatherFile,LoadWeatherFile,TYPE_FILENAME);
		VFN_1(eSetWeatherFileFilterAnimated,SetWeatherFileAnimated,TYPE_BOOL);
		FN_0(eGetWeatherFileFilterAnimated,TYPE_BOOL, GetWeatherFileAnimated);

		VFN_2(eSetWeatherFileSpecificTimeAndDate,SetWeatherFileSpecificTimeAndDate,TYPE_POINT3_BR,TYPE_POINT3_BR);
		VFN_2(eGetWeatherFileSpecificTimeAndDate,GetWeatherFileSpecificTimeAndDate,TYPE_POINT3_BR,TYPE_POINT3_BR);
		VFN_2(eSetWeatherFileAnimStartTimeAndDate,SetWeatherFileAnimStartTimeAndDate,TYPE_POINT3_BR,TYPE_POINT3_BR);
		VFN_2(eGetWeatherFileAnimStartTimeAndDate,GetWeatherFileAnimStartTimeAndDate,TYPE_POINT3_BR,TYPE_POINT3_BR);
		VFN_2(eSetWeatherFileAnimEndTimeAndDate,SetWeatherFileAnimEndTimeAndDate,TYPE_POINT3_BR,TYPE_POINT3_BR);
		VFN_2(eGetWeatherFileAnimEndTimeAndDate,GetWeatherFileAnimEndTimeAndDate,TYPE_POINT3_BR,TYPE_POINT3_BR);

		VFN_3(eSetWeatherFileSkipHours,SetWeatherFileSkipHours,TYPE_BOOL,TYPE_INT,TYPE_INT);
		VFN_3(eGetWeatherFileSkipHours,GetWeatherFileSkipHours,TYPE_BOOL_BR,TYPE_INT_BR,TYPE_INT_BR);
		VFN_1(eSetWeatherFileSkipWeekend,SetWeatherFileSkipWeekend,TYPE_BOOL);
		FN_0(eGetWeatherFileSkipWeekend,TYPE_BOOL,GetWeatherFileSkipWeekend);
		VFN_1(eSetWeatherFileSkipFramePer,SetWeatherFileFramePer,TYPE_INT);
		FN_0(eGetWeatherFileSkipFramePer,TYPE_INT,GetWeatherFileFramePer);



	END_FUNCTION_MAP 

	// From FPMixinInterface
	virtual FPInterfaceDesc* GetDesc() { return &mFPInterfaceDesc; }
	static FPInterfaceDesc mFPInterfaceDesc;

	//misc
	void WeatherFileMissing(){mWeatherFileMissing = true;}
	//for sunmaster interaction key for daylight weather control.
	void SetEnergyValuesBasedOnWeatherFile(TimeValue t, Interval &valid);
	void SetSunLocationBasedOnWeatherFile(SunMaster *themaster, TimeValue t, Interval &valid);
	void SetUpBasedOnWeatherFile();
	void SunMasterManualChanged( IDaylightSystem2::DaylightControlType type);

	//Render each Frame
	static void RenderFrame(void *param, NotifyInfo *nInfo);

private:
	class CreateCallback;
	class ParamDlgProc;
	friend class ParamDlgProc;
	class Accessor;
	class FindAssemblyHead;
	class ReplaceSunAndSkyNodes;
	class AnySunAndSkyNodes;
	class FindManual;
	class SetManual;
	class SunMesh;
	class OpenAssembly;
	class CloseAssembly;

	static Accessor			sAccessor;
	static ParamDlgProc		sEditParams;
	static CreateCallback	sCreateCallback;
	static ParamBlockDesc2	sMainParams;
	static SunMesh			sSunMesh;

	IParamBlock2*			mpParams;
	LightObject*			mpSunObj;
	LightObject*			mpSkyObj;
	Control*				mpMult;
	Control*				mpIntense;
	Control*				mpSkyCond;
	int						mExtDispFlags;
	bool					mWeatherFileMissing;
	bool					mErrorLoadingWeatherFile;
	// Helper methods for function publishing
	Object* fpSetSun(ClassDesc* sunClass);
	Object* fpSetSky(ClassDesc* skyClass);

	WeatherUIData			mWeatherData; //stored UI data for the weather file.
	WeatherFileCacheFiltered mWeatherFileCache; //stores cached weather file data for easy access.
	//launch the weather file dialog.
	void LaunchWeatherDialog(HWND hDlg);
	TSTR GetResolvedWeatherFileName();
	BOOL GetWeatherFileDST(int month, int day); //get DST based upon weather file
	void SetDaylightControlledIfNeeded(ReferenceTarget *obj, bool controlled);
	bool GetTempsLocal(DaylightWeatherData &data,float &dryBulbTemperature, float &dewPointTemperature);
	bool GetExtraTerrestialRadiationLocal(DaylightWeatherData &data,float &extraterrestrialHorizontalRadiation, float &extraterrestrialDirectNormalRadiation);
	bool GetRadiationLocal(DaylightWeatherData &data,float &globalHorizontalRadiation, float &directNormalRadiation,
											 float &diffuseHorizontalRadiation);
	bool GetIlluminanceLocal(DaylightWeatherData &data,float &globalHorizontalIlluminance, float &directNormalIlluminance,
											 float &diffuseHorizontalIlluminance, float &zenithLuminance);

};

void SetUpSun(SClass_ID sid, const Class_ID& systemClassID, Interface* ip, 
	GenLight* obj, const Point3& sunColor);

#endif //_NATLIGHT_H_
