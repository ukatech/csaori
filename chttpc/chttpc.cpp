#include "csaori.h"
#include "CInetHelper.h"

#define const_strlen(s) ((sizeof(s) / sizeof(s[0]))-1)

#if OPTION_DEBUG
#include <stdio.h>
#endif

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
	if (in.args.size() == 0) {
		out.result_code = SAORIRESULT_OK;
		out.result = (CInetHelper::checkInternet()) ? L"1" : L"0";
	} else if (in.args.size() == 1) {
		string url = SAORI_FUNC::UnicodeToMultiByte(in.args[0], CP_UTF8);
		wstring nResult = L"";
		int getResult = CInetHelper::getUrlContent(url.c_str(),NULL,nResult);
		if ( getResult == CIH_FAIL ) {
			out.result_code = SAORIRESULT_INTERNAL_SERVER_ERROR;
			out.result = nResult;
			return;
		} else {
			out.result_code = SAORIRESULT_OK;
			out.result = nResult;
		}
	} else if (in.args.size() > 1) {
		string url = SAORI_FUNC::UnicodeToMultiByte(in.args[0], CP_UTF8);
		wstring charset=L"";
		wstring saveTo=L"";
		wstring searchStart=L"";
		wstring searchEnd=L"";
		wstring id=L"";
		HWND hwnd=0;
		bool isStripTag=false;
		bool isTranslateTag=false;

		UINT idx;

		for(UINT i = 1; i < in.args.size(); i++) {
			if((idx = in.args[i].find(L"codepage=")) != string::npos) {
				charset = in.args[i].substr(idx + const_strlen(L"codepage="));
			} else if((idx = in.args[i].find(L"save=")) != string::npos) {
				saveTo = in.args[i].substr(idx + const_strlen(L"save="));
			} else if((idx = in.args[i].find(L"start=")) != string::npos) {
				searchStart = in.args[i].substr(idx + const_strlen(L"start="));
			} else if((idx = in.args[i].find(L"end=")) != string::npos) {
				searchEnd = in.args[i].substr(idx + const_strlen(L"end="));
			} else if((idx = in.args[i].find(L"id=")) != string::npos) {
				id = in.args[i].substr(idx + const_strlen(L"id="));
			} else if((idx = in.args[i].find(L"hwnd=")) != string::npos) {
				hwnd = reinterpret_cast<HWND>(_wtol(in.args[i].substr(idx + const_strlen(L"hwnd=")).c_str()));
			} else if((idx = in.args[i].find(L"strip")) != string::npos) {
				isStripTag = true;
			} else if((idx = in.args[i].find(L"translate")) != string::npos) {
				isTranslateTag = true;
			}
		}
#if OPTION_DEBUG
		printf("url=%s\ncharset=%s\nsaveTo=%s\nsearchStart=%s\nsearchEnd=%s\nid=%s\nhwnd=%d\nisStripTag=%d\nisTranslateTag=%d\n",
			url.c_str(),
			SAORI_FUNC::UnicodeToMultiByte(charset, CP_UTF8).c_str(),
			SAORI_FUNC::UnicodeToMultiByte(saveTo, CP_UTF8).c_str(),
			SAORI_FUNC::UnicodeToMultiByte(searchStart, CP_UTF8).c_str(),
			SAORI_FUNC::UnicodeToMultiByte(searchEnd, CP_UTF8).c_str(),
			SAORI_FUNC::UnicodeToMultiByte(id, CP_UTF8).c_str(),
			hwnd,
			isStripTag ? 1 : 0,
			isTranslateTag ? 1 : 0);
#endif
			out.result_code = SAORIRESULT_NO_CONTENT;
	}
}
