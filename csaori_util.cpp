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

//------------------------------------------------------------------------------
// Utility functions
//------------------------------------------------------------------------------
namespace SAORI_FUNC{
	template<> string_t numToString<unsigned char>(unsigned char num){
		char_t buf[32];
		swprintf(buf,L"%u",num);
		return buf;
	}
	template<> string_t numToString<unsigned int>(unsigned int num){
		char_t buf[32];
		swprintf(buf,L"%u",num);
		return buf;
	}
	template<> string_t numToString<unsigned long>(unsigned long num){
		char_t buf[32];
		swprintf(buf,L"%u",num);
		return buf;
	}

	std::string UnicodeToMultiByte(const wchar_t *Source, unsigned int CodePage, DWORD Flags)
	{
		if ( Source && *Source ) {
			if (int Len = ::WideCharToMultiByte(CodePage, Flags, Source, (int)wcslen(Source), NULL, 0, NULL, NULL)) {
				std::vector<char> Dest(Len);
				if (Len = ::WideCharToMultiByte(CodePage, Flags, Source, (int)wcslen(Source), &Dest[0], static_cast<int>(Dest.size()), NULL, NULL)) {
					return std::string(Dest.begin(), Dest.begin() + Len);
				}
			}
		}
		return "";
	}
	
	
	std::wstring MultiByteToUnicode(const char* Source, unsigned int CodePage, DWORD Flags)
	{
		if ( Source && *Source ) {
			if (int Len = ::MultiByteToWideChar(CodePage, Flags, Source, (int)strlen(Source), NULL, 0)) {
				std::vector<wchar_t> Dest(Len);
				if (Len = ::MultiByteToWideChar(CodePage, 0, Source, (int)strlen(Source), &Dest[0], static_cast<int>(Dest.size()))) {
					return std::wstring(Dest.begin(), Dest.begin() + Len);
				}
			}
		}
		return L"";
	}
	
	std::wstring CodePagetoString(unsigned int cset){
		switch(cset){
			case CP_SJIS:
				return L"Shift_JIS";
			case CP_ISO2022JP:
				return L"ISO-2022-JP";
			case CP_EUCJP:
				return L"EUC-JP";
			case CP_UTF8:
				return L"UTF-8";
		}
		return L"unknown charset";
	}

	UINT StringtoCodePage(const char *str)
	{
		if ( str && *str ) {
			if ( strnicmp(str,"shift_jis",9) == 0 ) {
				return CP_SJIS;
			}
			if ( strnicmp(str,"x-sjis",6) == 0 ) {
				return CP_SJIS;
			}
			if ( strnicmp(str,"iso-2022-jp",11) == 0 ) {
				return CP_ISO2022JP;
			}
			if ( strnicmp(str,"euc-jp",6) == 0 ) {
				return CP_EUCJP;
			}
			if ( strnicmp(str,"x-euc-jp",8) == 0 ) {
				return CP_EUCJP;
			}
			if ( strnicmp(str,"utf-8",5) == 0 ) {
				return CP_UTF8;
			}
		}
		return CP_SJIS;
	}

	string_t getResultString(int rc)
	{
		const char_t	*p;
		
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

	// cut one line
	// return ... first position of next line or string::npos if not found.

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

	string_t replaceAll(string_t string, const string_t &find, const string_t &replace) {
		string_t::size_type found = std::string::npos;
		while((found = string.find(find, found)) != std::string::npos) {
			string.replace(found, find.length(), replace);
			found += replace.size();
		}
		return string;
	}

	std::string replaceAll(std::string string, const std::string &find, const std::string &replace) {
		std::string::size_type found = std::string::npos;
		while((found = string.find(find, found)) != std::string::npos) {
			string.replace(found, find.length(), replace);
			found += replace.size();
		}
		return string;
	}

	typedef struct tagMessageBoxInfo {
		void *hwnd;
		string_t message;
		string_t title;
		unsigned int flags;
	} MessageBoxInfo;

	static void __cdecl AsyncMessageBoxProc(void *p)
	{
		MessageBoxInfo *pInf = reinterpret_cast<MessageBoxInfo*>(p);
		::MessageBoxW((HWND)pInf->hwnd,pInf->message.c_str(),pInf->title.c_str(),pInf->flags);
		delete pInf;
	}

	void AsyncMessageBox(void *hwnd,char_t *message,char_t *title,unsigned int flags)
	{
		MessageBoxInfo *pMsg = new MessageBoxInfo;
		pMsg->hwnd = hwnd;
		pMsg->title = title ? title : L"";
		pMsg->message = message ? message : L"";
		pMsg->flags = flags;

		_beginthread(AsyncMessageBoxProc,0,pMsg);
	}

	CCriticalSection::CCriticalSection(void) : init(false)
	{
		//Vista以降のメモリリークとマルチスレッドのパフォーマンス対策
		typedef BOOL (WINAPI *FInitializeCriticalSectionEx)(LPCRITICAL_SECTION lpCriticalSection,DWORD dwSpinCount,DWORD Flags);
		typedef BOOL (WINAPI *FInitializeCriticalSectionAndSpinCount)(LPCRITICAL_SECTION lpCriticalSection,DWORD dwSpinCoun);

		static FInitializeCriticalSectionEx SInitializeCriticalSectionEx = NULL;
		static FInitializeCriticalSectionAndSpinCount SInitializeCriticalSectionAndSpinCount = NULL;

		if ( ! init ) {
			OSVERSIONINFO inf;
			inf.dwOSVersionInfoSize = sizeof(inf);
			::GetVersionEx(&inf);

			init = true;
			if ( inf.dwMajorVersion >= 6 ) {
				SInitializeCriticalSectionEx = reinterpret_cast<FInitializeCriticalSectionEx>(
					::GetProcAddress(::GetModuleHandleA("kernel32"),"InitializeCriticalSectionEx") );
			}

			if ( inf.dwMajorVersion >= 5 ) {
				SInitializeCriticalSectionAndSpinCount = reinterpret_cast<FInitializeCriticalSectionAndSpinCount>(
					::GetProcAddress(::GetModuleHandleA("kernel32"),"InitializeCriticalSectionAndSpinCount") );
			}
		}

		BOOL result;
		if ( SInitializeCriticalSectionEx ) {
			result = SInitializeCriticalSectionEx(&c,1500,0x01000000 /*CRITICAL_SECTION_NO_DEBUG_INFO*/);
		}
		else if ( SInitializeCriticalSectionAndSpinCount ) {
			result = SInitializeCriticalSectionAndSpinCount(&c,1500);
		}
		else {
			::InitializeCriticalSection(&c);
			result = 1;
		}
		init = (result != 0);
	}
	CCriticalSection::~CCriticalSection()
	{
		if ( init ) {
			::DeleteCriticalSection(&c);
		}
	}
	void CCriticalSection::Enter(void)
	{
		if ( init ) {
			::EnterCriticalSection(&c);
		}
	}
	void CCriticalSection::Leave(void)
	{
		if ( init ) {
			::LeaveCriticalSection(&c);
		}
	}

}

