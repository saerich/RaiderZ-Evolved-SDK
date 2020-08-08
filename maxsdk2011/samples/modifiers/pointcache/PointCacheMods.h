/**********************************************************************
 *<
	FILE: PointCacheMods.h

	DESCRIPTION:	OSM and WSM

	CREATED BY:

	HISTORY:

 *>	Copyright (c) 1997, All Rights Reserved.
 **********************************************************************/

#ifndef __POINTCACHEMODS_H__
#define __POINTCACHEMODS_H__

#include "PointCache.h"

class PointCacheParamsMapDlgProc;

class PointCacheOSM : public PointCache, public IPointCache 
{
public:
	// Animatable /////////////////////////////////////////////////////////////////////////////
	SClass_ID		SuperClassID();
	Class_ID		ClassID();

	void			BeginEditParams(IObjParam* ip, ULONG flags, Animatable *prev);
	void			EndEditParams(IObjParam* ip, ULONG flags, Animatable *next);
	PointCacheParamsMapDlgProc  *dlgProc; //so we can set the file name up efficiently

	// BaseObject /////////////////////////////////////////////////////////////////////////////
	TCHAR*			GetObjectName();

	// ReferenceTarget ////////////////////////////////////////////////////////////////////////
	RefTargetHandle	Clone(RemapDir &remap);
	
	// PointCacheOSM //////////////////////////////////////////////////////////////////////////
	PointCacheOSM();

//published functions
	FPInterfaceDesc* GetDesc();
	void	fnRecord();
	void	fnSetCache();
	void	fnEnableMods();
	void	fnDisableMods();

private:
	// PointCache  /////////////////////////////////////////////////////////////////////////////
	void SetFile(const MaxSDK::AssetManagement::AssetUser& file); //used to set the filename in the UI efficiently  implemented by the OSM and WSM children.

};

class PointCacheWSM : public PointCache, public IPointCacheWSM
{
public:
	// Animatable /////////////////////////////////////////////////////////////////////////////
	SClass_ID		SuperClassID();
	Class_ID		ClassID();

	void			BeginEditParams(IObjParam* ip, ULONG flags, Animatable *prev);
	void			EndEditParams(IObjParam* ip, ULONG flags, Animatable *next);
	PointCacheParamsMapDlgProc  *dlgProc; //so we can set the file name up efficiently

	// BaseObject /////////////////////////////////////////////////////////////////////////////
	TCHAR*			GetObjectName();

	// ReferenceTarget ////////////////////////////////////////////////////////////////////////
	RefTargetHandle	Clone(RemapDir &remap);
	
	// PointCacheWSM //////////////////////////////////////////////////////////////////////////
	PointCacheWSM();

//published functions
	FPInterfaceDesc* GetDesc();
	void	fnRecord();
	void	fnSetCache();
	void	fnEnableMods();
	void	fnDisableMods();

private:
	// PointCache  /////////////////////////////////////////////////////////////////////////////
	void SetFile(const MaxSDK::AssetManagement::AssetUser& file); //used to set the filename in the UI efficiently  implemented by the OSM and WSM children.

};

extern HINSTANCE hInstance;

#endif // __POINTCACHEMODS_H__
