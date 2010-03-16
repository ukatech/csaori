/*
 * Credits Block *** PLEASE DO NOT REMOVE ***
 * 
 * ScreenShot codes are taken from ReactOS screenshot.c
 * 
 * Copyright(C) ReactOS.org 1998-2009.
 * Copyright(C) Roy 2009. Some Rights Reserved.
 * Licence: GPLv2
 *
 */

#ifdef _MSC_VER
#pragma warning( disable : 4786 )
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <malloc.h>

#include <iphlpapi.h>

#include "csaori.h"

//////////DEBUG/////////////////////////
#ifdef _WINDOWS
#ifdef _DEBUG
#include <crtdbg.h>
#define new new( _NORMAL_BLOCK, __FILE__, __LINE__)
#endif
#endif
////////////////////////////////////////

typedef struct _SCREENSHOT
{
    HWND hSelf;
    HDC hDC;
    HBITMAP hBitmap;
    LPBITMAPINFO lpbi;
    LPVOID lpvBits;
    INT Width;
    INT Height;
} SCREENSHOT, *PSCREENSHOT;

DWORD dwError = 0;

VOID
GetError(VOID)
{
    dwError = GetLastError();
}
BOOL
DoWriteFile(PSCREENSHOT pScrSht,
            LPCSTR pstrFileName)
{
    BITMAPFILEHEADER bmfh;
    BOOL bSuccess;
    DWORD dwBytesWritten;
    HANDLE hFile;
    //INT PalEntries;

    hFile = CreateFileA(pstrFileName,
                       GENERIC_WRITE,
                       0,
                       NULL,
                       CREATE_ALWAYS,
                       FILE_ATTRIBUTE_NORMAL,
                       NULL);

    if (hFile == INVALID_HANDLE_VALUE)
        return FALSE;

    /* write the BITMAPFILEHEADER to file */
    bmfh.bfType = *(WORD *)"BM";  // 0x4D 0x42
    bmfh.bfReserved1 = 0;
    bmfh.bfReserved2 = 0;
    bSuccess = WriteFile(hFile,
                         &bmfh,
                         sizeof(bmfh),
                         &dwBytesWritten,
                         NULL);
    if ((!bSuccess) || (dwBytesWritten < sizeof(bmfh)))
        goto fail;

    /* write the BITMAPINFOHEADER to file */
    bSuccess = WriteFile(hFile,
                         &pScrSht->lpbi->bmiHeader,
                         sizeof(BITMAPINFOHEADER),
                         &dwBytesWritten,
                         NULL);
    if ((!bSuccess) || (dwBytesWritten < sizeof(BITMAPINFOHEADER)))
        goto fail;

    /* calculate the size of the pallete * /
    if (pScrSht->lpbi->bmiHeader.biCompression == BI_BITFIELDS)
        PalEntries = 3;
    else
    {
        if (pScrSht->lpbi->bmiHeader.biBitCount <= 8)
            PalEntries = (INT)(1 << pScrSht->lpbi->bmiHeader.biBitCount);
        else
            PalEntries = 0;
    }
    if (pScrSht->lpbi->bmiHeader.biClrUsed)
        PalEntries = pScrSht->lpbi->bmiHeader.biClrUsed;

    / * write pallete to file * /
    if (PalEntries != 0)
    {
        bSuccess = WriteFile(hFile,
                             &pScrSht->lpbi->bmiColors,
                             PalEntries * sizeof(RGBQUAD),
                             &dwBytesWritten,
                             NULL);
        if ((!bSuccess) || (dwBytesWritten < PalEntries * sizeof(RGBQUAD)))
            goto fail;
    }
*/
    /* save the current file position at the bginning of the bitmap bits */
    bmfh.bfOffBits = SetFilePointer(hFile, 0, 0, FILE_CURRENT);

    /* write the bitmap bits to file */
    bSuccess = WriteFile(hFile,
                         pScrSht->lpvBits,
                         pScrSht->lpbi->bmiHeader.biSizeImage,
                         &dwBytesWritten,
                         NULL);
    if ((!bSuccess) || (dwBytesWritten < pScrSht->lpbi->bmiHeader.biSizeImage))
        goto fail;

    /* save the current file position at the final file size */
    bmfh.bfSize = SetFilePointer(hFile, 0, 0, FILE_CURRENT);

    /* rewrite the updated file headers */
    SetFilePointer(hFile, 0, 0, FILE_BEGIN);
    bSuccess = WriteFile(hFile,
                         &bmfh,
                         sizeof(bmfh),
                         &dwBytesWritten,
                         NULL);
    if (hFile) CloseHandle(hFile);
    if ((!bSuccess) || (dwBytesWritten < sizeof(bmfh)))
        goto fail;

    return TRUE;

fail:
    DeleteFileA(pstrFileName);
    return FALSE;

}

BOOL
CaptureScreen(PSCREENSHOT pScrSht)
{
    HDC ScreenDC;
    RECT rect;

    /* get window resolution */
    //pScrSht->Width = GetSystemMetrics(SM_CXSCREEN);
    //pScrSht->Height = GetSystemMetrics(SM_CYSCREEN);

    GetWindowRect(pScrSht->hSelf, &rect);
    pScrSht->Width = rect.right - rect.left;
    pScrSht->Height = rect.bottom - rect.top;

    /* get a DC for the screen */
    if (!(ScreenDC = GetDC(pScrSht->hSelf)))
        return FALSE;

    /* get a bitmap handle for the screen
     * needed to convert to a DIB */
    pScrSht->hBitmap = CreateCompatibleBitmap(ScreenDC,
                                              pScrSht->Width,
                                              pScrSht->Height);
    if (pScrSht->hBitmap == NULL)
    {
        GetError();
        ReleaseDC(pScrSht->hSelf, ScreenDC);
        return FALSE;
    }

    /* get a DC compatable with the screen DC */
    if (!(pScrSht->hDC = CreateCompatibleDC(ScreenDC)))
    {
        GetError();
        ReleaseDC(pScrSht->hSelf, ScreenDC);
        return FALSE;
    }

    /* select the bitmap into the DC */
    SelectObject(pScrSht->hDC,
                 pScrSht->hBitmap);

    /* copy the screen DC to the bitmap */
    BitBlt(pScrSht->hDC,
           0,
           0,
           pScrSht->Width,
           pScrSht->Height,
           ScreenDC,
           0,
           0,
           SRCCOPY | CAPTUREBLT);

    /* we're finished with the screen DC */
    ReleaseDC(pScrSht->hSelf, ScreenDC);

    return TRUE;
}


BOOL
ConvertDDBtoDIB(PSCREENSHOT pScrSht)
{
    INT Ret;
    BITMAP bitmap;
    WORD cClrBits;


/*
    / * can't call GetDIBits with hBitmap selected * /
    //SelectObject(hDC, hOldBitmap);

    / * let GetDIBits fill the lpbi structure by passing NULL pointer * /
    Ret = GetDIBits(hDC,
                    hBitmap,
                    0,
                    Height,
                    NULL,
                    lpbi,
                    DIB_RGB_COLORS);
    if (Ret == 0)
    {
        GetError();
        ReleaseDC(hwnd, hDC);
        HeapFree(GetProcessHeap(), 0, lpbi);
        return -1;
    }
*/

////////////////////////////////////////////////////

	if (!GetObjectW(pScrSht->hBitmap,
                    sizeof(BITMAP),
                    (LPTSTR)&bitmap))
    {
        GetError();
		return FALSE;
	}

	cClrBits = (WORD)(bitmap.bmPlanes * bitmap.bmBitsPixel);
	if (cClrBits == 1)
		cClrBits = 1;
	else if (cClrBits <= 4)
		cClrBits = 4;
	else if (cClrBits <= 8)
		cClrBits = 8;
	else if (cClrBits <= 16)
		cClrBits = 16;
	else if (cClrBits <= 24)
		cClrBits = 24;
	else cClrBits = 32;

	if (cClrBits != 24)
        pScrSht->lpbi = (PBITMAPINFO) HeapAlloc(GetProcessHeap(),
                                                0,
                                                sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * (1 << cClrBits));
    else
        pScrSht->lpbi = (PBITMAPINFO) HeapAlloc(GetProcessHeap(),
                                                0,
                                                sizeof(BITMAPINFOHEADER));

	if (!pScrSht->lpbi)
	{
		GetError();
		return FALSE;
	}

	pScrSht->lpbi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	pScrSht->lpbi->bmiHeader.biWidth = bitmap.bmWidth;
	pScrSht->lpbi->bmiHeader.biHeight = bitmap.bmHeight;
	pScrSht->lpbi->bmiHeader.biPlanes = bitmap.bmPlanes;
	pScrSht->lpbi->bmiHeader.biBitCount = bitmap.bmBitsPixel;

	if (cClrBits < 24)
		pScrSht->lpbi->bmiHeader.biClrUsed = (1 << cClrBits);

	pScrSht->lpbi->bmiHeader.biCompression = BI_RGB;
	pScrSht->lpbi->bmiHeader.biSizeImage = ((pScrSht->lpbi->bmiHeader.biWidth * cClrBits +31) & ~31) /8
                                           * pScrSht->lpbi->bmiHeader.biHeight;

	pScrSht->lpbi->bmiHeader.biClrImportant = 0;

//////////////////////////////////////////////////////

    /* reserve memory to hold the screen bitmap */
    pScrSht->lpvBits = HeapAlloc(GetProcessHeap(),
                                 0,
                                 pScrSht->lpbi->bmiHeader.biSizeImage);
    if (pScrSht->lpvBits == NULL)
    {
        GetError();
        return FALSE;
    }

    /* convert the DDB to a DIB */
    Ret = GetDIBits(pScrSht->hDC,
                    pScrSht->hBitmap,
                    0,
                    pScrSht->Height,
                    pScrSht->lpvBits,
                    pScrSht->lpbi,
                    DIB_RGB_COLORS);
    if (Ret == 0)
    {
        GetError();
        return FALSE;
    }

    return TRUE;

}

////////////////////////////////////////////////////
// CSaori Main Part

bool CSAORI::load()
{
	return true;
}

bool CSAORI::unload()
{
	return true;
}

void CSAORI::exec(const CSAORIInput& in,CSAORIOutput& out)
{
    PSCREENSHOT pScrSht;
    BOOL captureResult = 0;

	//parameter check
	if ( in.args.size() == 0 ) {
		out.result_code = SAORIRESULT_BAD_REQUEST;
		out.result = L"NG";
		out.values.push_back(L"Invalid arguments");
		return;
	}

	BOOL bFullScreen = TRUE;

    pScrSht = (PSCREENSHOT)HeapAlloc(GetProcessHeap(),
                        0,
                        sizeof(SCREENSHOT));
    if (pScrSht == NULL)
	{
		out.result_code = SAORIRESULT_INTERNAL_SERVER_ERROR;
		out.result = L"NG";
		out.values.push_back(string_t(L"Error. RetCode=" + SAORI_FUNC::intToString(dwError) ));
	}

    if (bFullScreen)
    {
        pScrSht->hSelf = GetDesktopWindow();
    }
    else
    {
        pScrSht->hSelf = GetForegroundWindow();
    }

    if (pScrSht->hSelf == NULL)
    {
        HeapFree(GetProcessHeap(),
                 0,
                 pScrSht);

		out.result_code = SAORIRESULT_INTERNAL_SERVER_ERROR;
		out.result = L"NG";
		out.values.push_back(string_t(L"Error. RetCode=" + SAORI_FUNC::intToString(dwError) ));
    }

	std::string fname = checkAndModifyPath(SAORI_FUNC::UnicodeToMultiByte(in.args[0]));

	if (captureResult = CaptureScreen(pScrSht))
    {
        /* convert the DDB image to DIB */
        if(captureResult = ConvertDDBtoDIB(pScrSht))
        {
            /* build the headers and write to file */
            if(!(captureResult = DoWriteFile(pScrSht, fname.c_str())))
			{
	        	GetError();
			}
        }
        else
        {
        	GetError();
        }
    }
	else
	{
        GetError();
	}

    /* cleanup */
    if (pScrSht->hSelf != NULL)
        ReleaseDC(pScrSht->hSelf, pScrSht->hDC);
    if (pScrSht->hBitmap != NULL)
        DeleteObject(pScrSht->hBitmap);
    if (pScrSht->lpbi != NULL)
        HeapFree(GetProcessHeap(),
                 0,
                 pScrSht->lpbi);
    if (pScrSht->lpvBits != NULL)
        HeapFree(GetProcessHeap(),
                 0,
                 pScrSht->lpvBits);
    if (pScrSht != NULL)
        HeapFree(GetProcessHeap(),
                 0,
                 pScrSht);

	if ( captureResult ) {
		out.result_code=SAORIRESULT_OK;
		out.result = L"OK";
	} else {
		out.result_code = SAORIRESULT_INTERNAL_SERVER_ERROR;
		out.result = L"NG";
		out.values.push_back(string_t(L"Error. RetCode=" + SAORI_FUNC::intToString(dwError) ));
	}
}
