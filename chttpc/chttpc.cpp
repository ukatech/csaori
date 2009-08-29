#include "csaori.h"
#include "chttpc.h"
#include "CInetHelper.h"
#include "CSAORIDSSTP.h"
#include "CHTML2SS.h"
#include "Thread.h"

// striphtml
#include "striphtml.h"

#define const_strlen(s) ((sizeof(s) / sizeof(s[0]))-1)

#define CR_OK 0
#define CR_FAIL 1

class chttpc_runner {
	public:
		static int run(chttpc_conf* cc, wstring& out);
		static void save(string& str, wstring& filename);
};

int chttpc_runner::run(chttpc_conf* cc, wstring& out) {
	wstring nResult = L""; bool replaced = false;

	wstring fullpath = cc->module_path + cc->saveOrginal;

	int getResult = CInetHelper::getUrlContent(cc->url.c_str(), (!cc->charset.empty() ? cc->charset.c_str() : NULL), nResult, (!cc->saveOrginal.empty() ? fullpath.c_str() : NULL));
	if ( getResult == CIH_FAIL ) {
		out = nResult;
		return CR_FAIL;
	}

	if(!cc->saveUtf8.empty()) {
		string aResult = SAORI_FUNC::UnicodeToMultiByte(nResult, CP_UTF8);
		wstring fullpath = cc->module_path + cc->saveUtf8;
		save(aResult,fullpath);
	}

	if(!cc->searchStart.empty() && !cc->searchEnd.empty()) {
		UINT start, end;
		if(!cc->isMultiSearch) {
			if((start = nResult.find(cc->searchStart, 0)) != string::npos && (end = nResult.find(cc->searchEnd, start + cc->searchStart.size())) != string::npos) {
				nResult = nResult.substr(start + cc->searchStart.size(), end - start - cc->searchStart.size());
			}
		} else {
			wstring mResult = L"";
			start = 0;
			while((start = nResult.find(cc->searchStart, start)) != string::npos && (end = nResult.find(cc->searchEnd, start + cc->searchStart.size())) != string::npos) {
				mResult += nResult.substr(start + cc->searchStart.size(), end - start - cc->searchStart.size()) + L"\2";
				start++;
			}
			nResult = mResult;
		}
	}

	if(cc->isStripTag) {
		nResult = stripHTMLTags(nResult);
	}

#if TRANSLATE
	if(cc->isTranslateTag) {
		nResult = CHTML2SS::translate(nResult,cc->url,cc->stripTags);
	}
#endif

	if(!cc->saveParsed.empty()) {
		string aResult = SAORI_FUNC::UnicodeToMultiByte(nResult, cc->codepage);
		wstring fullpath = cc->module_path + cc->saveParsed;
		save(aResult,fullpath);
	}

	if(cc->isNoOutput) {
		out = L"";
	} else {
		nResult = replaceAll(nResult,L"\r\n",L"\\n");
		nResult = replaceAll(nResult,L"\n",L"\\n");
		nResult = overReplaceAll(nResult,L"\\n\\n\\n",L"\\n");
		nResult = overReplaceAll(nResult, L"  ", L" ");
		out = nResult;
	}

	return CR_OK;
}

void chttpc_runner::save(string& str, wstring& filename) {
	FILE *fp = _wfopen(filename.c_str(), L"wb");
	fwrite(str.c_str(), 1, strlen(str.c_str()), fp);
	fclose(fp);
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
		cd.refs.push_back(SAORI_FUNC::MultiByteToUnicode(m_cc->url,CP_UTF8));	// ref2
		cd.send(cd.toString());
		delete m_cc;
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
	} else {
		chttpc_conf *cc = new chttpc_conf;

		cc->url = SAORI_FUNC::UnicodeToMultiByte(in.args[0], CP_UTF8);
		cc->codepage = in.codepage;
		cc->module_path = module_path;

		UINT idx;

		for(UINT i = 1; i < in.args.size(); i++) {
			if((idx = in.args[i].find(L"codepage=")) != string::npos) {
				cc->charset = in.args[i].substr(idx + const_strlen(L"codepage="));
			} else if((idx = in.args[i].find(L"save=")) != string::npos) {
				cc->saveOrginal = in.args[i].substr(idx + const_strlen(L"save="));
			} else if((idx = in.args[i].find(L"saveUtf8=")) != string::npos) {
				cc->saveUtf8 = in.args[i].substr(idx + const_strlen(L"saveUtf8="));
			} else if((idx = in.args[i].find(L"saveParsed=")) != string::npos) {
				cc->saveParsed = in.args[i].substr(idx + const_strlen(L"saveParsed="));
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
			} else if((idx = in.args[i].find(L"multiSearch")) != string::npos) {
				cc->isMultiSearch = true;
			} else if((idx = in.args[i].find(L"translate")) != string::npos) {
				cc->isTranslateTag = true;
			} else if((idx = in.args[i].find(L"noOutput")) != string::npos) {
				cc->isNoOutput = true;
			} else if((idx = in.args[i].find(L"removeTags=")) != string::npos) {
				string tags = SAORI_FUNC::UnicodeToMultiByte(in.args[i].substr(idx + const_strlen(L"removeTags=")));
				int cutAt;
				while( (cutAt = tags.find_first_of(",")) != string::npos ) {
					if(cutAt > 0) {
						cc->stripTags[tags.substr(0,cutAt)] = true;
					}
					tags = tags.substr(cutAt+1);
				}
				if(tags.length() > 0) {
					cc->stripTags[tags] = true;
				}
			}
		}

		if(cc->hwnd != NULL && !cc->id.empty()) {
			(new chttpcThread(cc))->create();
			out.result_code = SAORIRESULT_NO_CONTENT;
		} else {
			wstring result;
			int crresult = chttpc_runner::run(cc, result);
			delete cc;
			out.result_code = crresult ? SAORIRESULT_INTERNAL_SERVER_ERROR : SAORIRESULT_OK;
			out.result = result;
		}
	}
}

