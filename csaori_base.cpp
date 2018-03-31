//setlocale//
#ifdef _MSC_VER
#if _MSC_VER >= 1400
#pragma setlocale("japanese")
#endif
#endif
//setlocale end//

/*
 * csaori_base.cpp
 */

#ifdef _MSC_VER
#pragma warning( disable : 4786 )
#pragma warning( disable : 4996 )
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <cstring>
#include <cstdlib>
#include <algorithm>
#include <locale>
#include <process.h>

#include "csaori_base.h"

//////////WINDOWS DEFINE///////////////////////////
#ifdef _WINDOWS
#ifdef _DEBUG
#include <crtdbg.h>
#define new new( _NORMAL_BLOCK, __FILE__, __LINE__)
#endif
#endif
///////////////////////////////////////////////////

//global CSAORIBase object
CSAORIBase* pSaori;
HINSTANCE g_hModule;

extern CSAORIBase* CreateInstance(void);

//------------------------------------------------------------------------------
// CSAORIInput
//------------------------------------------------------------------------------
bool CSAORIInput::parseString(const string_t &src)
{
	const string_t& atag = base.s_saori_argument();
	const string_t::size_type catag = atag.length();
	
	string_t::size_type pos = 0, nextpos;
	string_t::size_type ts;
	std::vector<string_t> _arg;
	std::map<string_t, string_t> _opt;
	string_t _cmd;
	string_t _id;
	
	string_t sl;
	
	pos = SAORI_FUNC::getLine(sl, src, pos);
	ts = sl.find(base.s_saori_def());
	if (ts == string_t::npos) return false;
	_cmd = sl.substr(0, ts);
	
	string_t k, v;
	int argc = 0;
	
	while(1) {
		sl = L"";
		nextpos = SAORI_FUNC::getLine(sl, src, pos);
		ts = sl.find(L": ");
		if (ts != string_t::npos) {
			k = sl.substr(0, ts);
			v = sl.substr(ts + 2);
			const char_t *pk = k.c_str();
			// _argument[n] は vector (_args) に積む
			if (k.size() > catag && _wcsnicmp(pk, atag.c_str(), catag) == 0) {
				int ord = _wtoi(pk + catag);
				if (ord > 0 || (ord == 0 && k[catag] == L'0')) {
					if (argc <= ord) {
						// _argument[n] が順番に入ってない場合は 
						// 隙間を先に埋めておく 
						for(int i=(int)(_arg.size()); i<=ord; i++) {
							_arg.push_back(L"");
						}
					}
					//_arg.push_back(v);
					_arg[ord]=v;
					argc = (int)(_arg.size());
				}
			}
			// ID: はidに
			else if ( wcsicmp(k.c_str(),L"ID") == 0 ) {
				_id = v;
			}
			// それ以外の要素は map (_opt) に積む 
			else {
				if (ts > 0) {
					//よくあるヘッダの大文字小文字統一
					if ( wcsicmp(k.c_str(),L"securitylevel") == 0 ) {
						k = L"SecurityLevel";
					}
					else if ( wcsicmp(k.c_str(),L"sender") == 0 ) {
						k = L"Sender";
					}
					else if ( wcsicmp(k.c_str(),L"charset") == 0 ) {
						k = L"Charset";
					}

					std::map<string_t,string_t>::iterator i;
					i = _opt.find(k);
					if (i == _opt.end()) {
						_opt.insert(std::pair<string_t,string_t>(k, v));
					}
					else _opt[k] = v;
				}
			}
		}
		if (nextpos == string_t::npos) break;
		pos = nextpos;
	}
	cmd=_cmd;
	args=_arg;
	opts=_opt;
	id=_id;
	return true;
}

//------------------------------------------------------------------------------
//CSAORIOutput
//------------------------------------------------------------------------------
void CSAORIOutput::setResultEmpty()
{
	if ( result_code == SAORIRESULT_OK || result_code == SAORIRESULT_FORCE_OK ) {
		result_code = SAORIRESULT_NO_CONTENT;
		result.erase();
		values.clear();
	}
}

string_t CSAORIOutput::toString()
{
	if ( result_code == SAORIRESULT_FORCE_OK ) {
		result_code = SAORIRESULT_OK;
	}
	//No Contentに自動補正
	else if ( result.empty() && values.empty() && opts.empty() && result_code == SAORIRESULT_OK ) {
		result_code = SAORIRESULT_NO_CONTENT;
	}

	string_t rcstr = SAORI_FUNC::getResultString(result_code);
	
	wchar_t tmptxt[32];
	swprintf(tmptxt,L"%d",result_code);

	std::wstring dest;
	dest += base.s_saori_version();
	dest += L" ";
	dest += tmptxt + std::wstring(L" ") + std::wstring(rcstr) + L"\r\n";

	dest += L"Charset: " + SAORI_FUNC::CodePagetoString(codepage) + L"\r\n";
	
//	if (!result.empty()) { //空文字列でも結果は返すべき
	dest += base.s_saori_result() + L": " + result + L"\r\n";
//	}
	if (!opts.empty()) {
		std::map<string_t,string_t>::iterator i;
		for(i=opts.begin(); i != opts.end(); i++) {
			dest += i->first + L": " + i->second + L"\r\n";
		}
	}
	if (!values.empty()) {
		int i, n = (int)(values.size());
		string_t tmp;
		for(i=0; i<n; i++) {
			swprintf(tmptxt,L"%d",i);

			dest += base.s_saori_value();
			dest += tmptxt;
			dest += std::wstring(L": ");

			tmp = values[i];
			std::string::size_type nPos = 0;
			while((nPos = tmp.find(L"\r\n", nPos)) != std::string::npos){
				tmp.replace(nPos, 2 , L"\1");
			}
			while((nPos = tmp.find(L"\r", nPos)) != std::string::npos){
				tmp.replace(nPos, 2 , L"\1");
			}
			while((nPos = tmp.find(L"\n", nPos)) != std::string::npos){
				tmp.replace(nPos, 2 , L"\1");
			}

			dest += tmp + L"\r\n";
		}
	}
	
	dest += L"\r\n";
	return dest;
}

//------------------------------------------------------------------------------
//CSAORIBase
//------------------------------------------------------------------------------
std::string CSAORIBase::request(const std::string &rq_tmp)
{
	call_id += 1;
	if ( call_id >= 0x7fffffffU ) {
		call_id = 1;
	}

	std::string cmd;
	CSAORIInput* pIn;
	CSAORIOutput* pOut;

	//未加工Requestを小文字に変換
	std::string tmp=rq_tmp;
	std::transform(tmp.begin(), tmp.end(), tmp.begin(), tolower);

	//Charset判別
	unsigned int cp = CP_SJIS;
	std::string::size_type count = tmp.find("\ncharset: ");
	if ( count != std::string::npos ) {
		cp = SAORI_FUNC::StringtoCodePage(tmp.c_str() + count + 10);
	}

	//変換
	string_t rq=SAORI_FUNC::MultiByteToUnicode(rq_tmp,cp);

	//解析処理開始
	pIn=new CSAORIInput(*this);
	pIn->codepage=cp;
	pIn->opts[L"SecurityLevel"] = L"Local";
	bool result=pIn->parseString(rq);
	
	//pOut初期化
	pOut=new CSAORIOutput(*this);
	pOut->codepage=pIn->codepage;
	pOut->result=L"";
	pOut->result_code=SAORIRESULT_INTERNAL_SERVER_ERROR;

	if(result==false){
		pOut->result_code=SAORIRESULT_INTERNAL_SERVER_ERROR;
	}else{
		if (pIn->cmd == L"GET Version") {
			pOut->result_code=SAORIRESULT_FORCE_OK;
		}
		else if ( wcsicmp(pIn->cmd.c_str(),L"EXECUTE") == 0 || wcsicmp(pIn->cmd.c_str(),L"GET") == 0 || wcsicmp(pIn->cmd.c_str(),L"NOTIFY") == 0 ) {
			string_t sec = pIn->opts[L"SecurityLevel"];

			exec_before(*pIn,*pOut);
			if ( sec.empty() || wcsicmp(sec.c_str(),L"local") == 0 ) {
				exec(*pIn,*pOut);
			}
			else {
				if ( ! exec_insecure(*pIn,*pOut) ) {
					pOut->result_code=SAORIRESULT_INTERNAL_SERVER_ERROR;
				}
			}
			exec_after(*pIn,*pOut);

			if ( wcsicmp(pIn->cmd.c_str(),L"NOTIFY") == 0 ) {
				pOut->setResultEmpty();
			}
		}
		else {
			pOut->result_code=SAORIRESULT_BAD_REQUEST;
		}
	}
	string_t res_wstr=pOut->toString();
	std::string res_str=SAORI_FUNC::UnicodeToMultiByte(res_wstr,pOut->codepage);
	delete pIn;
	delete pOut;
	return res_str;
}

void CSAORIBase::setModulePath(const std::string &str){
	module_path=SAORI_FUNC::MultiByteToUnicode(str);
}

void CSAORIBase::setModuleHandle(HANDLE hMod){
	module_handle=hMod;
}

string_t CSAORIBase::checkAndModifyPathW(const string_t &p)
{
	string_t filepath = p;

	if ( p.size() >= 3 ) {
#ifdef _WINDOWS
		//環境変数展開用特殊コード
		string_t::size_type len=filepath.size() < MAX_PATH ? MAX_PATH : filepath.size();
		len *= 2;

		{
			void *pBuf = malloc(len+1);
#ifdef _UNICODE
			std::string::size_type realLen = ::ExpandEnvironmentStrings(filepath.c_str(),(wchar_t*)pBuf,(DWORD)len);
#else
			std::string filepatha = SAORI_FUNC::UnicodeToMultiByte(p);
			std::string::size_type realLen = ::ExpandEnvironmentStrings(filepatha.c_str(),(char*)pBuf,(DWORD)len);
#endif
			if ( realLen > len ) {
				free(pBuf);
				pBuf = malloc(realLen+1);
#ifdef _UNICODE
				realLen = ::ExpandEnvironmentStrings(filepath.c_str(),(wchar_t*)pBuf,(DWORD)realLen);
#else
				realLen = ::ExpandEnvironmentStrings(filepatha.c_str(),(char*)pBuf,(DWORD)realLen);
#endif
			}

			if ( realLen ) {
#ifdef _UNICODE
				filepath = (wchar_t*)pBuf;
#else
				filepatha = (char*)pBuf;
				filepath = SAORI_FUNC::MultiByteToUnicode(filepatha);
#endif
			}
			free(pBuf);
		}

		//Windows 絶対パス
		if ( wcsncmp(filepath.c_str(),L"\\\\",2) == 0 || wcsncmp(filepath.c_str()+1,L":\\",2) == 0 ||
			wcsncmp(filepath.c_str(),L"//",2) == 0 || wcsncmp(filepath.c_str()+1,L":/",2) == 0 ) {
			return filepath;
		}
#else
		//Unix
		if ( filepath[0] == L'/' ) {
			return filepath;
		}
#endif
	}
	string_t fullpath = module_path;
#ifdef _WINDOWS
	if ( fullpath[fullpath.size()-1] != L'\\' ) {
		fullpath += L"\\";
	}
#else
	if ( fullpath[fullpath.size()-1] != L'/' ) {
		fullpath += L"/";
	}
#endif
	fullpath += filepath;
	return fullpath;
}



std::string CSAORIBase::checkAndModifyPath(const std::string &p)
{
	std::string filepath = p;

	if ( p.size() >= 3 ) {
#ifdef _WINDOWS
		//環境変数展開用特殊コード
		std::string::size_type len = filepath.size() < MAX_PATH ? MAX_PATH : filepath.size();
		len *= 2;

		{
			void *pBuf = malloc(len+1);
#ifdef _UNICODE
			string_t filepathw = SAORI_FUNC::MultiByteToUnicode(p);
			std::string::size_type realLen = ::ExpandEnvironmentStrings(filepathw.c_str(),(wchar_t*)pBuf,(DWORD)len);
#else
			std::string::size_type realLen = ::ExpandEnvironmentStrings(filepath.c_str(),(char*)pBuf,(DWORD)len);
#endif
			if ( realLen > len ) {
				free(pBuf);
				pBuf = malloc(realLen+1);
#ifdef _UNICODE
				realLen = ::ExpandEnvironmentStrings(filepathw.c_str(),(wchar_t*)pBuf,(DWORD)realLen);
#else
				realLen = ::ExpandEnvironmentStrings(filepath.c_str(),(char*)pBuf,(DWORD)realLen);
#endif
			}

			if ( realLen ) {
#ifdef _UNICODE
				filepathw = (wchar_t*)pBuf;
				filepath = SAORI_FUNC::UnicodeToMultiByte(filepathw);
#else
				filepath = (char*)pBuf;
#endif
			}
			free(pBuf);
		}

		//Windows 絶対パス
		if ( strncmp(filepath.c_str(),"\\\\",2) == 0 || strncmp(filepath.c_str()+1,":\\",2) == 0 ||
			strncmp(filepath.c_str(),"//",2) == 0 || strncmp(filepath.c_str()+1,":/",2) == 0 ) {
			return filepath;
		}
#else
		//Unix
		if ( filepath[0] == '/' ) {
			return filepath;
		}
#endif
	}
	std::string fullpath = SAORI_FUNC::UnicodeToMultiByte(module_path);
#ifdef _WINDOWS
	if ( fullpath[fullpath.size()-1] != '\\' ) {
		fullpath += "\\";
	}
#else
	if ( fullpath[fullpath.size()-1] != '/' ) {
		fullpath += "/";
	}
#endif
	fullpath += filepath;
	return fullpath;
}

//------------------------------------------------------------------------------
//SAORI INTERFACES
//------------------------------------------------------------------------------
BOOL APIENTRY DllMain(
	HANDLE hModule,
	DWORD  ul_reason_for_call, 
	LPVOID lpReserved
){
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		g_hModule = (HINSTANCE)hModule;
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		unload();
		break;
	}
    return TRUE;
}

SAORIAPI
HGLOBAL
SAORICDECL 
request(HGLOBAL h, long *len)
{
	std::string rq((char *)h, *len);
	GlobalFree(h);
	
	std::string re;

#ifndef _DEBUG
	try {
#endif
		re = pSaori->request(rq);
#ifndef _DEBUG
	}
	catch(...) {
		re = SAORI_FUNC::UnicodeToMultiByte(pSaori->s_saori_version() + string_t(L" 500 Internal Server Error\r\n\r\n"));
	}
#endif

	*len = (long)(re.size());
	h = GlobalAlloc(GMEM_FIXED, *len+1);
	if (h) {
		memcpy(h,re.c_str(),*len+1);
	}
	
	return h;
}

SAORIAPI
BOOL
SAORICDECL
load(HGLOBAL h, long len)
{
#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_DELAY_FREE_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetBreakAlloc(829);
#endif
	if(pSaori!=NULL){
		unload();
	}
 	pSaori = CreateInstance();
	if (h) {
		std::string mpath;
		mpath.assign((char*)h,len);
		GlobalFree(h);
		pSaori->setModulePath(mpath);
		pSaori->setModuleHandle(g_hModule);
	}

	BOOL re;

#ifndef _DEBUG
	try {
#endif
		re = pSaori->load();
#ifndef _DEBUG
	}
	catch(...) {
		re = FALSE;
	}
#endif

	return re;
}

SAORIAPI
BOOL
SAORICDECL
unload()
{
	if(pSaori==NULL){
		return TRUE;
	}

	BOOL re;

#ifndef _DEBUG
	try {
#endif
		re=pSaori->unload();
#ifndef _DEBUG
	}
	catch(...) {
		re = 0;
	}
#endif
	delete pSaori;
	pSaori=NULL;

	return re;
}
