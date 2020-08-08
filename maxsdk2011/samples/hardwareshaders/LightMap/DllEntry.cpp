/**********************************************************************
 *<
	FILE: DllEntry.cpp

	DESCRIPTION: Contains the Dll Entry stuff

	CREATED BY: 

	HISTORY: 

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/
#include "LightMap.h"
#include "IColorCorrectionMgr.h"
#include "AssetManagement/iassetmanager.h"

using namespace MaxSDK::AssetManagement;

extern ClassDesc2* GetLightMapDesc();

HINSTANCE hInstance;

BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved)
{
   if( fdwReason == DLL_PROCESS_ATTACH )
   {
	hInstance = hinstDLL;				// Hang on to this DLL's instance handle.
      DisableThreadLibraryCalls(hInstance);
	}
			
	return (TRUE);
}

__declspec( dllexport ) const TCHAR* LibDescription()
{
	return GetString(IDS_LIBDESCRIPTION);
}

//TODO: Must change this number when adding a new class
__declspec( dllexport ) int LibNumberClasses()
{
	return 1;
}

__declspec( dllexport ) ClassDesc* LibClassDesc(int i)
{
	switch(i) {
		case 0: return GetLightMapDesc();
		default: return 0;
	}
}

__declspec( dllexport ) ULONG LibVersion()
{
	return VERSION_3DSMAX;
}

TCHAR *GetString(int id)
{
	static TCHAR buf[256];

	if (hInstance)
		return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;
	return NULL;
}

BITMAPINFO *BMToDIB(Bitmap* newBM, int extraFlags)	{
	int bmw = newBM->Width();
	int bmh = newBM->Height();
	BITMAPINFO* texBMI = (BITMAPINFO *)malloc(sizeof(BITMAPINFOHEADER) + bmw * bmh * sizeof(RGBQUAD));
	BITMAPINFOHEADER *bmih = &texBMI->bmiHeader;
	bmih->biSize = sizeof (BITMAPINFOHEADER);
	bmih->biWidth = bmw;
   	bmih->biHeight = bmh;
	bmih->biPlanes = 1;
	bmih->biBitCount = 32;
	bmih->biCompression = BI_RGB;
	bmih->biSizeImage = bmih->biWidth * bmih->biHeight * sizeof(RGBQUAD);
	bmih->biXPelsPerMeter = 0;
   	bmih->biYPelsPerMeter = 0;
	bmih->biClrUsed = 0;
	bmih->biClrImportant = 0;

	DWORD *row = (DWORD *)((BYTE *)texBMI + sizeof(BITMAPINFOHEADER));
	PixelBuf l64(bmw);

	for(int y = 0; y < bmh; y++)	{
		BMM_Color_64 *p64=l64.Ptr();
		// TBD: This should be GetLinearPixels, but since the viewport isn't 
		// yet gamma corrected,  do GetPixels instead.
		DWORD dw;
 
		int r,g,b;
		newBM->GetLinearPixels(0, y, bmw, p64);
		if (extraFlags&EX_ALPHA_FROM_RGB) {
			BMM_Color_64 *pc = p64;
			for(int x = 0; x < bmw; ++x, ++pc) {
				int a = pc->r+pc->g+pc->b;
				pc->a = a/3;
				}
			}
		if (extraFlags&EX_OPAQUE_ALPHA) {
			BMM_Color_64 *pc = p64;
			for(int x = 0; x < bmw; ++x, ++pc) {
				pc->a = 0xffff;
				}
			}
		if (extraFlags&EX_RGB_FROM_ALPHA) {
			BMM_Color_64 *pc = p64;
			for(int x = 0; x < bmw; ++x, ++pc) {
				USHORT a = pc->a;
				pc->r = a;
				pc->g = a;
				pc->b = a;
				}
			}
		else if (extraFlags&EX_MULT_ALPHA) {
			BMM_Color_64 *pc = p64;
			for(int x = 0; x < bmw; ++x, ++pc) {
				USHORT a = pc->a;
				pc->r = (pc->r*a)>>16;
				pc->g = (pc->g*a)>>16;
				pc->b = (pc->b*a)>>16;
				}
			} 

		IColorCorrectionMgr* idispGamMgr = (IColorCorrectionMgr*) GetCOREInterface(COLORCORRECTIONMGR_INTERFACE);
		for(int x = 0; x < bmw; ++x, ++p64) {
         r = idispGamMgr->ColorCorrect16(p64->r, IColorCorrectionMgr::kRED_C);
         g = idispGamMgr->ColorCorrect16(p64->g, IColorCorrectionMgr::kGREEN_C); 
         b = idispGamMgr->ColorCorrect16(p64->b, IColorCorrectionMgr::kBLUE_C);  
			dw = (r << 16) | (g << 8) | b ; 
			//if ((extraFlags&EX_OPAQUE_ALPHA) || (p64->a & 0x8000)) 
			if (p64->a & 0x8000) 
				dw |= 0xff000000;
			row[x] = dw;
			}

		row += bmih->biWidth;
		}
	return texBMI;
}

BITMAPINFO * ConvertBitmap(Bitmap * bm)
{
	BITMAPINFO* texBMI;

	int newW ;
	int newH ;
	newW = getClosestPowerOf2(bm->Width());
	newH = getClosestPowerOf2(bm->Height());

	if (bm->Width()!=newW||bm->Height()!=newH)
		{ 
		Bitmap *newBM;
		BitmapInfo bi;
		bi.SetAsset(AssetUser());
		bi.SetWidth(newW);
		bi.SetHeight(newH);
		bi.SetType(BMM_TRUE_32);
		bi.SetFlags(bm->HasAlpha() ? MAP_HAS_ALPHA : 0);
		newBM = TheManager->Create(&bi);
		newBM->CopyImage(bm, COPY_IMAGE_RESIZE_LO_QUALITY, 0);
		

		texBMI = BMToDIB(newBM,EX_ALPHA_FROM_RGB);

		newBM->DeleteThis();
		}
	else  {
		texBMI = BMToDIB(bm,0);
		}
	return texBMI;
}
