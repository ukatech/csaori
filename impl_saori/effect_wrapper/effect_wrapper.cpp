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
#include "effect.hpp"

static effect* g_effect = NULL;

bool CSAORI::load()
{
	char gProgDir[MAX_PATH + 1];
	char* ptrEnd;
	::GetModuleFileName(g_hModule, gProgDir, MAX_PATH);
	if ( (ptrEnd = strrchr(gProgDir, '\\')) != NULL ) {
		*ptrEnd = '\0';
	} else {
		gProgDir[0] = '\0';
	}

	if ( ! g_effect ) {
		g_effect = new effect(gProgDir,"effect.dll");
	}
	return true;
}

bool CSAORI::unload()
{
	if ( g_effect ) {
		delete g_effect;
		g_effect = NULL;
	}
	return false;
}

void CSAORI::exec(const CSAORIInput &in, CSAORIOutput &out)
{
	if (in.args.size() < 2)
	{
		out.result_code = SAORIRESULT_BAD_REQUEST;
		return;
	}
	if (wcsicmp(in.args[0].c_str(),L"hwnd")==0)
	{
		out.result_code = SAORIRESULT_OK;
		return;
	}

	std::string plugin = SAORI_FUNC::UnicodeToMultiByte(in.args[0].c_str());

	long param[9];
	long param_num = in.args.size()-1;

	for ( int i = 0 ; i < 9 ; ++i ) {
		if (in.args.size() >= i+2) {
			param[i] = _wtoi(in.args[i+1].c_str());
		}
	}

	g_effect->_run(plugin.c_str(), param, param_num);

	out.result_code = SAORIRESULT_OK;
}
