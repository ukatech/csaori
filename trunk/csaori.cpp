/*
 * csaori.cpp
 * 
 * written by Ukiya http://ukiya.sakura.ne.jp/
 * based by えびさわ様 "gethwnd.dll"
 */

#ifdef _MSC_VER
#pragma warning( disable : 4786 )
#endif

#include <cstring>
#include <cstdlib>
#include <algorithm>
#include <locale>
#include <sstream>

#include "csaori.h"

//////////DEBUG/////////////////////////
#ifdef _WINDOWS
#ifdef _DEBUG
#include <crtdbg.h>
#define new new( _NORMAL_BLOCK, __FILE__, __LINE__)
#endif
#endif
////////////////////////////////////////

//global CSAORI object
CSAORI* pSaori;

//------------------------------------------------------------------------------
//内部関数
//------------------------------------------------------------------------------
namespace SAORI_FUNC{
	std::string UnicodeToMultiByte(const std::wstring& Source, UINT CodePage, DWORD Flags)
	{
	  if (int Len = ::WideCharToMultiByte(CodePage, Flags, Source.c_str(), static_cast<int>(Source.size()), NULL, 0, NULL, NULL)) {
		std::vector<char> Dest(Len);
		if (Len = ::WideCharToMultiByte(CodePage, Flags, Source.c_str(), static_cast<int>(Source.size()), &Dest[0], static_cast<int>(Dest.size()), NULL, NULL)) {
		  return std::string(Dest.begin(), Dest.begin() + Len);
		}
	  }
	  return "";
	}


	std::wstring MultiByteToUnicode(const std::string& Source, UINT CodePage, DWORD Flags)
	{
	  if (int Len = ::MultiByteToWideChar(CodePage, Flags, Source.c_str(), static_cast<int>(Source.size()), NULL, 0)) {
		std::vector<wchar_t> Dest(Len);
		if (Len = ::MultiByteToWideChar(CodePage, 0, Source.c_str(), static_cast<int>(Source.size()), &Dest[0], static_cast<int>(Dest.size()))) {
		  return std::wstring(Dest.begin(), Dest.begin() + Len);
		}
	  }
	  return L"";
	}

	UINT CHARSETtoCodePage(CHARSET cset){
		switch(cset){
			case CHARSET_Shift_JIS:
				return 932;
			case CHARSET_ISO_2022_JP:
				return 50220;
			case CHARSET_EUC_JP:
				return 20932;
			case CHARSET_UTF_8:
				return CP_UTF8;
		}
		return CP_ACP;
	}

	std::wstring CHARSETtoString(CHARSET cset){
		switch(cset){
			case CHARSET_Shift_JIS:
				return L"Shift_JIS";
			case CHARSET_ISO_2022_JP:
				return L"ISO-2022-JP";
			case CHARSET_EUC_JP:
				return L"EUC-JP";
			case CHARSET_UTF_8:
				return L"UTF-8";
		}
		return L"unknown charset";
	}

	string_t getResultString(int rc)
	{
		char_t	*p;
		
		switch(rc) {
			case 200:
				p = L"OK"; break;
			case 204:
				p = L"No Content"; break;
			case 210:
				p = L"Break"; break;		// SSTP Break (SSTP session aborted)
			case 300:
				p = L"Communicate"; break; // (obsolete)
			case 311:
				p = L"Not Enough"; break; // SHIORI/2.4 TEACH...need more info.
			case 312:
				p = L"Advice"; break; // SHIORI/2.4 TEACH...drop recent reference
			case 400:
				p = L"Bad Request"; break;
			case 401:
				p = L"Unauthorized"; break;
			case 403:
				p = L"Forbidden"; break;
			case 404:
				p = L"Not Found"; break;
			case 405:
				p = L"Method Not Allowed"; break;
			case 406:
				p = L"Not Acceptable"; break;
			case 408:
				p = L"Request Timeout"; break;
			case 409:
				p = L"Conflict"; break;
			case 420:
				p = L"Refuse"; break;		// SSTP refused by ghost
			case 500:
				p = L"Internal Server Error"; break;
			case 501:
				p = L"Not Implemented"; break;
			case 503:
				p = L"Service Unavailable"; break;
			case 510:
				p = L"Not Local IP"; break;	// SSTP from Non-Local IP
			case 511:
				p = L"In Black List"; break;	// SSTP from "Black-Listed" IP
			case 512:
				p = L"Invisible"; break;	// SSTP inivisible (not proceed)
			default:
				p = L"Unknown Error";
		}
		
		return string_t(p);
	}

	//
	// string の tpos 位置からから１行（改行もしくは終端まで）取り出す 
	// 戻り値 ... 次行開始位置（次行がないときは string::npos） 
	//
	string_t::size_type getLine(string_t &sl, const string_t &src, string_t::size_type tpos)
	{
		string_t::size_type len = src.size();
		if (tpos == string_t::npos || tpos >= len) {
			sl = L"";
			return string_t::npos;
		}
		string_t::size_type  ppos = src.find_first_of(L"\r\n", tpos);
		if (ppos == string_t::npos) {
			sl = src.substr(tpos);
			return ppos;
		}
		sl = src.substr(tpos, ppos - tpos);
		ppos++;
		if (ppos >= len) return string_t::npos;
		if (src[ppos - 1] == '\r' && src[ppos] == '\n') {
			ppos++;
			if (ppos >= len) return string_t::npos;
		}
		
		return ppos;
	}

	string_t intToString(int num)
	{
		std::wostringstream stream;
		stream << num;
		string_t result = stream.str();
		return result;
	}
}

//------------------------------------------------------------------------------
//CSAORIInput
//------------------------------------------------------------------------------
bool CSAORIInput::parseString(const string_t &src)
{
	const char_t atag[] = L"Argument";
	const string_t::size_type catag = 8;//"Argument"の長さ
	string_t::size_type pos = 0, nextpos;
	string_t::size_type ts;
	std::vector<string_t> _arg;
	std::map<string_t, string_t> _opt;
	string_t _cmd;
	
	string_t sl;
	
	pos = SAORI_FUNC::getLine(sl, src, pos);
	ts = sl.find(L" SAORI/1.");
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
			// _argument[n] は vector (_args) に積み、それ以外の要素は 
			// map (_opt) に積む 
			if (k.size() > catag && _memicmp(pk, atag, catag) == 0) {
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
			else {
				if (ts > 0) {
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
	return true;
}

//------------------------------------------------------------------------------
//CSAORIOutput
//------------------------------------------------------------------------------
string_t CSAORIOutput::toString()
{
	string_t rcstr = SAORI_FUNC::getResultString(result_code);
	
	std::wostringstream dest;
	dest << SAORI_VERSIONSTRING L" ";
	dest << result_code << L" " << rcstr << L"\r\n";

	dest << L"Charset: " << SAORI_FUNC::CHARSETtoString(charset) << L"\r\n";
	
//	if (!result.empty()) { //空文字列でも結果は返すべき
		dest << L"Result: " << result << L"\r\n";
//	}
	if (!values.empty()) {
		int i, n = (int)(values.size());
		string_t tmp;
		for(i=0; i<n; i++) {
			dest << L"Value" << i << L": ";

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

			dest << values[i] << L"\r\n";
		}
	}
	if (!opts.empty()) {
		std::map<string_t,string_t>::iterator i;
		for(i=opts.begin(); i != opts.end(); i++) {
			dest << i->first << L": " << i->second << L"\r\n";
		}
	}
	
	dest << L"\r\n";
	return dest.str();
}

//------------------------------------------------------------------------------
//CSAORI
//------------------------------------------------------------------------------
std::string CSAORI::request(const std::string &rq_tmp)
{
	std::string cmd;
	CSAORIInput* pIn;
	CSAORIOutput* pOut;

	//未加工Requestを小文字に変換
	std::string tmp=rq_tmp;
	std::transform(tmp.begin(), tmp.end(), tmp.begin(), tolower);

	//Charset判別
	CHARSET cset=CHARSET_Shift_JIS;
	if(tmp.find("\ncharset: shift_jis")!=std::string::npos){
		cset=CHARSET_Shift_JIS;
	}else if(tmp.find("\ncharset: iso-2022-jp")!=std::string::npos){
		cset=CHARSET_ISO_2022_JP;
	}else if(tmp.find("\ncharset: euc-jp")!=std::string::npos){
		cset=CHARSET_EUC_JP;
	}else if(tmp.find("\ncharset: utf-8")!=std::string::npos){
		cset=CHARSET_UTF_8;
	}

	//変換
	string_t rq=SAORI_FUNC::MultiByteToUnicode(rq_tmp,SAORI_FUNC::CHARSETtoCodePage(cset));

	//解析処理開始
	pIn=new CSAORIInput();
	pIn->charset=cset;
	pIn->opts[L"SecurityLevel"] = L"Local";
	bool result=pIn->parseString(rq);
	
	//pOut初期化
	pOut=new CSAORIOutput();
	pOut->charset=pIn->charset;
	pOut->result=L"";
	pOut->result_code=SAORIRESULT_INTERNAL_SERVER_ERROR;

	if(result==false){
		pOut->result_code=SAORIRESULT_INTERNAL_SERVER_ERROR;
	}else{
		if (pIn->cmd == L"GET Version") {
			pOut->result_code=SAORIRESULT_OK;
		}else if (pIn->cmd == L"EXECUTE") {
			string_t sec = pIn->opts[L"SecurityLevel"];
			if (sec==L"Local" || sec==L"local") {
				exec(*pIn,*pOut);
			}
			else {
				pOut->result_code=SAORIRESULT_INTERNAL_SERVER_ERROR;
			}
		}else {
			pOut->result_code=SAORIRESULT_BAD_REQUEST;
		}
	}
	string_t res_wstr=pOut->toString();
	std::string res_str=SAORI_FUNC::UnicodeToMultiByte(res_wstr,SAORI_FUNC::CHARSETtoCodePage(pOut->charset));
	delete pIn;
	delete pOut;
	return res_str;
}

void CSAORI::setModulePath(const std::string &str){
	module_path=SAORI_FUNC::MultiByteToUnicode(str);
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
	
	std::string re=pSaori->request(rq);

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
	if(pSaori!=NULL){
		unload();
	}
	pSaori=new CSAORI();
	if (h) {
		std::string mpath;
		mpath.assign((char*)h,len);
		GlobalFree(h);
		pSaori->setModulePath(mpath);
	}
	return pSaori->load();
}

SAORIAPI
BOOL
SAORICDECL
unload()
{
	if(pSaori==NULL){
		return TRUE;
	}
	BOOL re=pSaori->unload();
	delete pSaori;
	pSaori=NULL;
#ifdef _DEBUG
	_CrtDumpMemoryLeaks();
#endif
	return re;
}
