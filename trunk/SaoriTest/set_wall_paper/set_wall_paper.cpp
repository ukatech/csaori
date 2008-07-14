#ifdef _MSC_VER
#pragma warning( disable : 4786 )
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wininet.h>
#include <atlbase.h>
#include <shlobj.h>
#include <comdef.h>
#include "csaori.h"

static bool SetWallpaper(const char_t *pszFile, DWORD dwStyle);

bool CSAORI::load()
{
	return true;
}

bool CSAORI::unload()
{
	return false;
}

void CSAORI::exec(const CSAORIInput &in, CSAORIOutput &out)
{
	DWORD dwStyle = 3;
	if (in.args.empty())
	{
		out.result_code = SAORIRESULT_BAD_REQUEST;
		return;
	}
	if (in.args.size() > 1)
	{
		dwStyle = _wtoi(in.args[1].c_str());
	}
	if (SetWallpaper(in.args[0].c_str(), dwStyle))
	{
		out.result_code = SAORIRESULT_OK;
	}
	else
	{
		out.result_code = SAORIRESULT_INTERNAL_SERVER_ERROR;
	}
}

bool SetWallpaper(const char_t *pszFile, DWORD dwStyle)
{
	CComPtr<IActiveDesktop> pActiveDesktop;
	if (FAILED(pActiveDesktop.CoCreateInstance(CLSID_ActiveDesktop, NULL, CLSCTX_INPROC_SERVER)) || pActiveDesktop == NULL)
	{
		return false;
	}
	
	COMPONENTSOPT  copt;
	ZeroMemory(&copt, sizeof(copt));
	copt.dwSize = sizeof(copt);
	pActiveDesktop->GetDesktopItemOptions(&copt, 0);

	if ( ! copt.fActiveDesktop ) {
		copt.fActiveDesktop = true;
		if ( FAILED( pActiveDesktop->SetDesktopItemOptions(&copt,0) ) ) {
			return false;
		}
	}

	if (FAILED(pActiveDesktop->SetWallpaper(pszFile, 0)))
	{
		return false;
	}
	
	if ( dwStyle < WPSTYLE_MAX ) {
		WALLPAPEROPT opt;
		opt.dwSize = sizeof(WALLPAPEROPT);
		opt.dwStyle = dwStyle;
		if (FAILED(pActiveDesktop->SetWallpaperOptions(&opt, 0)))
		{
			return false;
		}
		if (FAILED(pActiveDesktop->ApplyChanges(AD_APPLY_ALL)))
		{
			return false;
		}
	}
	return true;
}