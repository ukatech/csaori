#include "csaori.h"
#include "chttpc.h"
#include "CInetHelper.h"
#include "CSAORIDSSTP.h"
#include "CHTML2SS.h"
#include "Thread.h"

// striphtml
#include "striphtml.h"

#define const_strlen(s) ((sizeof(s) / sizeof(s[0]))-1)

#if OPTION_DEBUG
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

	int getResult = CInetHelper::getUrlContent(cc->url.c_str(),(cc->charset != L"" ? cc->charset.c_str() : NULL),nResult);
	if ( getResult == CIH_FAIL ) {
		out = nResult;
		return CR_FAIL;
	}

	if(cc->searchStart != L"" && cc->searchEnd != L"") {
		UINT start, end;
		if((start = nResult.find(cc->searchStart, 0)) != string::npos && (end = nResult.find(cc->searchEnd, start)) != string::npos) {
			out = nResult.substr(start, end);
			replaced = true;
		}
	}

	if(cc->isStripTag) {
		nResult = stripHTMLTags(nResult);
	}

	if(cc->isTranslateTag) {
		nResult = CHTML2SS::translate(nResult);
	}

	if(cc->saveTo != L"") {
		string aResult = SAORI_FUNC::UnicodeToMultiByte(cc->charset, CP_UTF8);
		FILE *fp = _wfopen(cc->saveTo.c_str(), L"wb");
		fwrite(aResult.c_str(), strlen(aResult.c_str()), 1, fp);
		fclose(fp);
	}
	if(!replaced)
		out = nResult;
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
		cc->codepage=in.codepage;
		cc->charset=L"";
		cc->saveTo=L"";
		cc->searchStart=L"";
		cc->searchEnd=L"";
		cc->id=L"";
		cc->hwnd=0;
		cc->isStripTag=false;
		cc->isTranslateTag=false;

		UINT idx;

		for(UINT i = 1; i < in.args.size(); i++) {
			if((idx = in.args[i].find(L"codepage=")) != string::npos) {
				cc->charset = in.args[i].substr(idx + const_strlen(L"codepage="));
			} else if((idx = in.args[i].find(L"save=")) != string::npos) {
				cc->saveTo = in.args[i].substr(idx + const_strlen(L"save="));
			} else if((idx = in.args[i].find(L"start=")) != string::npos) {
				cc->searchStart = in.args[i].substr(idx + const_strlen(L"start="));
			} else if((idx = in.args[i].find(L"end=")) != string::npos) {
				cc->searchEnd = in.args[i].substr(idx + const_strlen(L"end="));
			} else if((idx = in.args[i].find(L"id=")) != string::npos) {
				cc->id = in.args[i].substr(idx + const_strlen(L"id="));
			} else if((idx = in.args[i].find(L"hwnd=")) != string::npos) {
				cc->hwnd = reinterpret_cast<HWND>(_wtol(in.args[i].substr(idx + const_strlen(L"hwnd=")).c_str()));
			} else if((idx = in.args[i].find(L"strip")) != string::npos) {
				cc->isStripTag = true;
			} else if((idx = in.args[i].find(L"translate")) != string::npos) {
				cc->isTranslateTag = true;
			}
		}
#if OPTION_DEBUG
		printf("url=%s\ncharset=%s\nsaveTo=%s\nsearchStart=%s\nsearchEnd=%s\nid=%s\nhwnd=%d\nisStripTag=%d\nisTranslateTag=%d\n",
			cc->url.c_str(),
			SAORI_FUNC::UnicodeToMultiByte(cc->charset, CP_UTF8).c_str(),
			SAORI_FUNC::UnicodeToMultiByte(cc->saveTo, CP_UTF8).c_str(),
			SAORI_FUNC::UnicodeToMultiByte(cc->searchStart, CP_UTF8).c_str(),
			SAORI_FUNC::UnicodeToMultiByte(cc->searchEnd, CP_UTF8).c_str(),
			SAORI_FUNC::UnicodeToMultiByte(cc->id, CP_UTF8).c_str(),
			cc->hwnd,
			cc->isStripTag ? 1 : 0,
			cc->isTranslateTag ? 1 : 0);
#endif

		if(cc->hwnd != NULL && cc->id != L"") {
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

