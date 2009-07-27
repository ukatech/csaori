#include "csaori.h"
#include "chttpc.h"
#include "CInetHelper.h"
#include "CSAORIDSSTP.h"
#include "CHTML2SS.h"
#include "Thread.h"

// striphtml
#include "striphtml.h"

#define const_strlen(s) ((sizeof(s) / sizeof(s[0]))-1)

#ifdef OPTION_DEBUG
#include <stdio.h>
#endif

#define CR_OK 0
#define CR_FAIL 1

class chttpc_runner {
	public:
		static int run(chttpc_conf* cc, wstring& out);
};

int chttpc_runner::run(chttpc_conf* cc, wstring& out) {
	wstring nResult = L""; bool replaced = false;

#ifdef OPTION_DEBUG
printf("chttpc_runner::run - start\n");
#endif
	int getResult = CInetHelper::getUrlContent(cc->url.c_str(), (!cc->charset.empty() ? cc->charset.c_str() : NULL), nResult);
	if ( getResult == CIH_FAIL ) {
		out = nResult;
		return CR_FAIL;
	}

	if(!cc->saveOrginal.empty()) {
#ifdef OPTION_DEBUG
printf("chttpc_runner::run - save\n");
#endif
		string aResult = SAORI_FUNC::UnicodeToMultiByte(nResult, CP_UTF8);
		wstring fullpath = cc->module_path + cc->saveOrginal;
		FILE *fp = _wfopen(fullpath.c_str(), L"wb");
		fwrite(aResult.c_str(), 1, strlen(aResult.c_str()), fp);
		fclose(fp);
	}

#ifdef OPTION_DEBUG
wprintf(L"chttpc_runner::run - got file, data = %s\n", nResult.c_str());
#endif
	if(!cc->searchStart.empty() && !cc->searchEnd.empty()) {
#ifdef OPTION_DEBUG
printf("chttpc_runner::run - search\n");
#endif
		UINT start, end;
		if((start = nResult.find(cc->searchStart, 0)) != string::npos && (end = nResult.find(cc->searchEnd, start + cc->searchStart.size())) != string::npos) {
			out = nResult.substr(start + cc->searchStart.size(), end - start - cc->searchStart.size());
			replaced = true;
		}
	}

	if(cc->isStripTag) {
#ifdef OPTION_DEBUG
printf("chttpc_runner::run - strip\n");
#endif
		nResult = stripHTMLTags(nResult);
	}

	if(cc->isTranslateTag) {
#ifdef OPTION_DEBUG
printf("chttpc_runner::run - translate\n");
#endif
		nResult = CHTML2SS::translate(nResult);
	}

	if(!cc->saveParsed.empty()) {
#ifdef OPTION_DEBUG
printf("chttpc_runner::run - saveParsed\n");
#endif
		string aResult = SAORI_FUNC::UnicodeToMultiByte(nResult, CP_UTF8);
		wstring fullpath = cc->module_path + cc->saveParsed;
		FILE *fp = _wfopen(fullpath.c_str(), L"wb");
		fwrite(aResult.c_str(), 1, strlen(aResult.c_str()), fp);
		fclose(fp);
	}
	if(!replaced)
		out = nResult;

#ifdef OPTION_DEBUG
printf("chttpc_runner::run - done\n");
wprintf(L"chttpc_runner result = %s\n", nResult.c_str());
#endif
	return CR_OK;
}

class chttpcThread : public Thread {
	chttpc_conf	*m_cc;
	virtual	DWORD	ThreadMain() {
		CSAORIDSSTP	cd;
		wstring result;
		chttpc_runner::run(m_cc, result);
		cd.codepage = m_cc->codepage;
		cd.hwnd = m_cc->hwnd;
		cd.sender = L"chttpc";
		cd.event = L"OnchttpcNotify";
		cd.refs.push_back(m_cc->id);	// ref0
		cd.refs.push_back(result);	// ref1
		cd.send(cd.toString());
		delete this;
		return	0;
	}
public:
	chttpcThread(chttpc_conf* cc) : 
	  Thread(), m_cc(cc) {}
};

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
		chttpc_conf *cc = new chttpc_conf;

		cc->url = SAORI_FUNC::UnicodeToMultiByte(in.args[0], CP_UTF8);
		cc->codepage = in.codepage;
		cc->module_path = module_path;
/*		cc->charset = L"";
		cc->saveTo = L"";
		cc->searchStart = L"";
		cc->searchEnd = L"";
		cc->id = L"";
		cc->hwnd = 0;
		cc->isStripTag = false;
		cc->isTranslateTag = false;
*/
		UINT idx;

		for(UINT i = 1; i < in.args.size(); i++) {
			if((idx = in.args[i].find(L"codepage=")) != string::npos) {
#ifdef OPTION_DEBUG
		printf("in codepage\n");
#endif
				cc->charset = in.args[i].substr(idx + const_strlen(L"codepage="));
			} else if((idx = in.args[i].find(L"save=")) != string::npos) {
#ifdef OPTION_DEBUG
		printf("in save\n");
#endif
				cc->saveOrginal = in.args[i].substr(idx + const_strlen(L"save="));
			} else if((idx = in.args[i].find(L"saveParsed=")) != string::npos) {
#ifdef OPTION_DEBUG
		printf("in saveParsed\n");
#endif
				cc->saveParsed = in.args[i].substr(idx + const_strlen(L"saveParsed="));
			} else if((idx = in.args[i].find(L"start=")) != string::npos) {
#ifdef OPTION_DEBUG
		printf("in start\n");
#endif
				cc->searchStart = in.args[i].substr(idx + const_strlen(L"start="));
			} else if((idx = in.args[i].find(L"end=")) != string::npos) {
#ifdef OPTION_DEBUG
		printf("in end\n");
#endif
				cc->searchEnd = in.args[i].substr(idx + const_strlen(L"end="));
			} else if((idx = in.args[i].find(L"id=")) != string::npos) {
#ifdef OPTION_DEBUG
		printf("in id\n");
#endif
				cc->id = in.args[i].substr(idx + const_strlen(L"id="));
			} else if((idx = in.args[i].find(L"hwnd=")) != string::npos) {
#ifdef OPTION_DEBUG
		printf("in hwnd\n");
#endif
				cc->hwnd = reinterpret_cast<HWND>(_wtol(in.args[i].substr(idx + const_strlen(L"hwnd=")).c_str()));
			} else if((idx = in.args[i].find(L"strip")) != string::npos) {
#ifdef OPTION_DEBUG
		printf("in strip\n");
#endif
				cc->isStripTag = true;
			} else if((idx = in.args[i].find(L"translate")) != string::npos) {
#ifdef OPTION_DEBUG
		printf("in translate\n");
#endif
				cc->isTranslateTag = true;
			}
		}
#ifdef OPTION_DEBUG
		printf("url=%s\ncharset=%s\nsaveOrginal=%s\nsearchStart=%s\nsearchEnd=%s\nid=%s\nhwnd=%d\nisStripTag=%d\nisTranslateTag=%d\n",
			cc->url.c_str(),
			SAORI_FUNC::UnicodeToMultiByte(cc->charset, CP_UTF8).c_str(),
			SAORI_FUNC::UnicodeToMultiByte(cc->saveOrginal, CP_UTF8).c_str(),
			SAORI_FUNC::UnicodeToMultiByte(cc->searchStart, CP_UTF8).c_str(),
			SAORI_FUNC::UnicodeToMultiByte(cc->searchEnd, CP_UTF8).c_str(),
			SAORI_FUNC::UnicodeToMultiByte(cc->id, CP_UTF8).c_str(),
			cc->hwnd,
			cc->isStripTag ? 1 : 0,
			cc->isTranslateTag ? 1 : 0);
#endif

		if(cc->hwnd != NULL && !cc->id.empty()) {
			(new chttpcThread(cc))->create();
			out.result_code = SAORIRESULT_NO_CONTENT;
		} else {
			wstring result;
			int crresult = chttpc_runner::run(cc, result);
			out.result_code = crresult ? SAORIRESULT_INTERNAL_SERVER_ERROR : SAORIRESULT_OK;
			out.result = result;
		}
	}
}

