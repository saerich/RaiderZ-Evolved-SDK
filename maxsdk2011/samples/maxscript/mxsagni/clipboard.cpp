/**********************************************************************
*<
FILE: clipboard.cpp

DESCRIPTION: 

CREATED BY: Larry Minton

HISTORY: Created 15 April 2007

*>	Copyright (c) 2007, All Rights Reserved.
**********************************************************************/

#include "MAXScrpt.h"
#include "Numbers.h"
#include "Strings.h"
#include "MXSAgni.h"

#include "bitmaps.h"

#ifdef ScripterExport
#undef ScripterExport
#endif
#define ScripterExport __declspec( dllexport )

#include <maxscript\macros\define_instantiation_functions.h>

// ============================================================================

#include <maxscript\macros\define_external_functions.h>
#	include "namedefs.h"

#include <maxscript\macros\define_instantiation_functions.h>
#	include "clipboard_wraps.h"

// -----------------------------------------------------------------------------------------

Value*
setclipboardText_cf(Value** arg_list, int count)
{
	// setclipboardText <string>
	check_arg_count(setclipboardText, 1, count);
	TSTR workString = arg_list[0]->to_string();
	Replace_LF_with_CRLF(workString);
	int len = workString.Length();
	HGLOBAL hGlobalMemory = GlobalAlloc (GHND, len+1);
	if (!hGlobalMemory) 
		return Integer::intern(-1);
	TCHAR* pGlobalMemory = (TCHAR*)GlobalLock (hGlobalMemory);
	for (int i = 0; i < len; i++)
		*pGlobalMemory++ = workString[i];
	*pGlobalMemory = 0;
	GlobalUnlock (hGlobalMemory);
	BOOL res = OpenClipboard (MAXScript_interface->GetMAXHWnd());
	if (!res) 
	{
		GlobalFree (hGlobalMemory);
		return Integer::intern(-2);
	}
	res = EmptyClipboard();
	if (!res) 
	{
		GlobalFree (hGlobalMemory);
		CloseClipboard();
		return Integer::intern(-3);
	}
	HANDLE hres = SetClipboardData (CF_TEXT, hGlobalMemory);
	if (!hres) 
	{
		GlobalFree (hGlobalMemory);
		CloseClipboard();
		return Integer::intern(-4);
	}
	CloseClipboard();
	return &true_value;
}

Value*
getclipboardText_cf(Value** arg_list, int count)
{
	// getclipboardText()
	check_arg_count(getclipboardText, 0, count);
	BOOL res = IsClipboardFormatAvailable(CF_TEXT);
	if (!res) return &undefined;
	res = OpenClipboard (MAXScript_interface->GetMAXHWnd());
	if (!res) return &undefined;
	HANDLE hClipMem = GetClipboardData (CF_TEXT);
	if (!hClipMem) 
	{
		CloseClipboard();
		return &undefined;
	}
	one_typed_value_local(String* result);
	TCHAR* pClipMem = (TCHAR*)GlobalLock (hClipMem);
	TSTR workString = pClipMem;
	GlobalUnlock (hClipMem);
	CloseClipboard();
	Replace_CRLF_with_LF(workString);
	vl.result = new String(workString);
	return_value(vl.result);
}

//
// ----- This code was stolen from the dll/Abrowser project
//

BOOL ConvertToRGB(BITMAPINFO **ptr)
{
	BOOL res = false;
	BITMAPINFO *bOld = *ptr;
	int depth = bOld->bmiHeader.biBitCount;
	int w	  = bOld->bmiHeader.biWidth;
	int h	  = bOld->bmiHeader.biHeight;
	int i;

	if(bOld->bmiHeader.biCompression != BI_RGB)
		return false;

	if(depth>=32)
		return true;

	int clr = bOld->bmiHeader.biClrUsed;
	if(!clr)
		clr = 2<<(depth-1);								

	RGBQUAD *rgb= (RGBQUAD*) bOld->bmiColors;

	BYTE *buf = new BYTE[sizeof(BITMAPINFOHEADER) + w*h*sizeof(RGBQUAD)];
	if(!buf)
		return false;


	BITMAPINFO *bInfo = (BITMAPINFO*)buf;
	bInfo->bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
	bInfo->bmiHeader.biWidth= w;
	bInfo->bmiHeader.biHeight=h;
	bInfo->bmiHeader.biPlanes=1; 
	bInfo->bmiHeader.biBitCount=32;
	bInfo->bmiHeader.biCompression=BI_RGB;
	bInfo->bmiHeader.biSizeImage=0;
	bInfo->bmiHeader.biClrUsed=0;
	bInfo->bmiHeader.biXPelsPerMeter=27306;
	bInfo->bmiHeader.biYPelsPerMeter=27306;
	bInfo->bmiHeader.biClrImportant=0;

	BYTE *data = (BYTE*) ((BYTE*)rgb + clr*sizeof(RGBQUAD));

	switch(depth)
	{
	case 24:
		{
			for(int j=0;j<h;j++)
			{
				int padW = ((3*w+ 3) & ~3);
				BYTE *line = (BYTE*)rgb + j*padW;
				for(i=0;i<w;i++)
				{
					bInfo->bmiColors[j*w+i].rgbBlue		= line[3*i];
					bInfo->bmiColors[j*w+i].rgbGreen	= line[3*i+1];
					bInfo->bmiColors[j*w+i].rgbRed		= line[3*i+2];
					bInfo->bmiColors[j*w+i].rgbReserved = 0;
				}
			}
		}
		break;


	case 8:
		{
			for(int j=0;j<h;j++)
			{
				int padW = ((w+ 3) & ~3);
				BYTE *line = data + j*padW;
				for(i=0;i<w;i++)
				{
					bInfo->bmiColors[j*w+i].rgbBlue	 = rgb[line[i]].rgbBlue;	//rgb[line[i]].rgbRed;
					bInfo->bmiColors[j*w+i].rgbGreen = rgb[line[i]].rgbGreen;
					bInfo->bmiColors[j*w+i].rgbRed   = rgb[line[i]].rgbRed;		//rgb[line[i]].rgbBlue;
					bInfo->bmiColors[j*w+i].rgbReserved = rgb[line[i]].rgbReserved;
				}
			}
		}
		break;

	case 4:
		{
			for(int j=0;j<h;j++)
			{
				int padW = ((w/2)+ (w%2?1:0)+ 3) & ~3;
				BYTE *line = data + j*padW;
				for(i=0;i<w/2;i++)
				{
					BYTE b = line[i];					
					bInfo->bmiColors[j*w+2*i].rgbBlue		= rgb[b&0x0f].rgbBlue;
					bInfo->bmiColors[j*w+2*i].rgbGreen		= rgb[b&0x0f].rgbGreen;
					bInfo->bmiColors[j*w+2*i].rgbRed		= rgb[b&0x0f].rgbRed;
					bInfo->bmiColors[j*w+2*i].rgbReserved	= rgb[b&0x0f].rgbReserved;

					bInfo->bmiColors[j*w+2*i+1].rgbBlue		= rgb[(b>>4)&0x0f].rgbBlue;
					bInfo->bmiColors[j*w+2*i+1].rgbGreen	= rgb[(b>>4)&0x0f].rgbGreen;
					bInfo->bmiColors[j*w+2*i+1].rgbRed		= rgb[(b>>4)&0x0f].rgbRed;
					bInfo->bmiColors[j*w+2*i+1].rgbReserved	= rgb[(b>>4)&0x0f].rgbReserved;
				}
			}
		}
		break;

	case 1:
		{
			for(int j=0;j<h;j++)
			{
				int padW = ((w/8)+ (w%8?1:0) + 3) & ~3;
				BYTE *line = data + j*padW;
				for(i=0;i<w/8;i++)
				{
					BYTE b = line[i];					
					for(int k=0;k<8;k++)
					{
						bInfo->bmiColors[j*w+8*i+k].rgbBlue		= rgb[(b&(0x80>>k))?1:0].rgbBlue;
						bInfo->bmiColors[j*w+8*i+k].rgbGreen	= rgb[(b&(0x80>>k))?1:0].rgbGreen;
						bInfo->bmiColors[j*w+8*i+k].rgbRed		= rgb[(b&(0x80>>k))?1:0].rgbRed;
						bInfo->bmiColors[j*w+8*i+k].rgbReserved	= rgb[(b&(0x80>>k))?1:0].rgbReserved;
					}
				}
			}
		}
		break;

	default: goto Done;
	}

	res = true;

Done:
	if(!res)
	{
		delete bInfo;
	}	
	else
	{
		delete *ptr;
		*ptr = bInfo;
	}
	return res;
}

BMM_Color_64 local_c;
Bitmap* CreateBitmapFromBInfo(void **ptr, const int cx, const int cy)
{
	Bitmap		*map=NULL;
	Bitmap		*mapTmp=NULL;
	BitmapInfo  bi,biTmp;
	float		w,h;
	BITMAPINFO  *bInfo;

	if(!ConvertToRGB((BITMAPINFO**)ptr))
		goto Done;

	bInfo = (BITMAPINFO*) *ptr;

	biTmp.SetName(_T(""));
	biTmp.SetType(BMM_TRUE_32);

	if(!bInfo)
		goto Done;

	w = (float)bInfo->bmiHeader.biWidth;
	h = (float)bInfo->bmiHeader.biHeight;

	biTmp.SetWidth((int)w);
	biTmp.SetHeight((int)h);

	mapTmp = CreateBitmapFromBitmapInfo(biTmp);
	if(!mapTmp) goto Done;
	mapTmp->FromDib(bInfo);

	bi.SetName(_T(""));
	bi.SetType(BMM_TRUE_32);
	if(w>h)
	{
		bi.SetWidth(cx);
		bi.SetHeight((int)(cx*h/w));
	}
	else
	{
		bi.SetHeight(cy);
		bi.SetWidth((int)(cy*w/h));
	}
	map = CreateBitmapFromBitmapInfo(bi);
	if(!map) goto Done;

	map->CopyImage(mapTmp,COPY_IMAGE_RESIZE_HI_QUALITY,local_c,&bi);

Done:
	if(mapTmp) mapTmp->DeleteThis();
	return map;
}


Value*
setclipboardBitmap_cf(Value** arg_list, int count)
{
	// setclipboardBitmap <bitmap>
	check_arg_count(setclipboardBitmap, 1, count);

	MAXBitMap* mbm = (MAXBitMap*)arg_list[0];
	type_check(mbm, MAXBitMap, _T("setclipboardBitmap"));
	PBITMAPINFO bmi = mbm->bm->ToDib(24);
	HDC hdc = GetDC(GetDesktopWindow());
	HBITMAP new_map = CreateDIBitmap(hdc, &bmi->bmiHeader, CBM_INIT, bmi->bmiColors, bmi, DIB_RGB_COLORS); 
	ReleaseDC(GetDesktopWindow(), hdc);	
	LocalFree(bmi);
	if (!new_map)
		return Integer::intern(-1);
	BOOL res = OpenClipboard (MAXScript_interface->GetMAXHWnd());
	if (!res) 
	{
		DeleteObject(new_map);
		return Integer::intern(-2);
	}
	res = EmptyClipboard();
	if (!res) 
	{
		DeleteObject(new_map);
		CloseClipboard();
		return Integer::intern(-3);
	}
	HANDLE hres = SetClipboardData (CF_BITMAP, new_map);
	if (!hres) 
	{
		DeleteObject(new_map);
		CloseClipboard();
		return Integer::intern(-4);
	}
	CloseClipboard();
	return &true_value;
}

Value*
getclipboardBitmap_cf(Value** arg_list, int count)
{
	// getclipboardBitmap()
	check_arg_count(getclipboardBitmap, 0, count);
	BOOL res = IsClipboardFormatAvailable(CF_BITMAP);
	if (!res) return &undefined;
	res = OpenClipboard (MAXScript_interface->GetMAXHWnd());
	if (!res) return &undefined;
	HBITMAP hBitmap = (HBITMAP)GetClipboardData (CF_BITMAP);
	if (!hBitmap) 
	{
		CloseClipboard();
		return &undefined;
	}

	BYTE *buf = NULL;
	BITMAPINFO	bInfo;
	BITMAP bm;
	GetObject (hBitmap, sizeof (BITMAP), (PSTR) &bm);

	bInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bInfo.bmiHeader.biWidth = bm.bmWidth; 
	bInfo.bmiHeader.biHeight = bm.bmHeight; 
	bInfo.bmiHeader.biPlanes = bm.bmPlanes; 
	bInfo.bmiHeader.biBitCount = bm.bmBitsPixel;
	bInfo.bmiHeader.biCompression = BI_RGB;
	bInfo.bmiHeader.biSizeImage = 0;
	buf = new BYTE[bm.bmWidth * bm.bmHeight * (bm.bmBitsPixel / 8) + sizeof(BITMAPINFOHEADER)];

	if (buf)
	{
		HDC hdc = GetDC(GetDesktopWindow());
		if (!GetDIBits(hdc, hBitmap, 0, bInfo.bmiHeader.biHeight, &buf[sizeof(BITMAPINFOHEADER)], &bInfo, DIB_RGB_COLORS))
		{
			delete [] buf;
			CloseClipboard();
			ReleaseDC(GetDesktopWindow(), hdc);	
			return &undefined;
		}			
		memcpy(buf, &bInfo, sizeof(BITMAPINFOHEADER));
		ReleaseDC(GetDesktopWindow(), hdc);	
	}

	CloseClipboard();
	Bitmap *map = CreateBitmapFromBInfo((void**)&buf, bInfo.bmiHeader.biWidth, bInfo.bmiHeader.biHeight);
	delete [] buf;
	one_typed_value_local(MAXBitMap* mbm);
	vl.mbm = new MAXBitMap ();
	vl.mbm->bi.CopyImageInfo(&map->Storage()->bi);
	vl.mbm->bi.SetFirstFrame(0);
	vl.mbm->bi.SetLastFrame(0);
	vl.mbm->bi.SetName("");
	vl.mbm->bi.SetDevice("");
	if (vl.mbm->bi.Type() > BMM_TRUE_64)
		vl.mbm->bi.SetType(BMM_TRUE_64);
	vl.mbm->bm = map;
	return_value(vl.mbm);
}

