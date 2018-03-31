/*
 * csaori_util.h
 */

#pragma once

#pragma warning(disable : 4786)

#define SAORIAPI extern "C" __declspec(dllexport)

#define SAORICDECL __cdecl

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <string>
#include <vector>
#include <map>
#include <sstream>

//charset hack
#ifdef CP_UTF8
#undef CP_UTF8
#endif

typedef wchar_t char_t;
typedef std::basic_string<char_t> string_t;

typedef enum {
	CP_SJIS=932,
	CP_EUCJP=20932,
	CP_ISO2022JP=50220,
	CP_UTF8=65001
} CODEPAGE;

//global functions
namespace SAORI_FUNC {
	template<typename _Type> string_t numToString(_Type num){
		char_t buf[32];
		swprintf(buf,L"%d",num);
		return buf;
	}
	template<> string_t numToString<unsigned char>(unsigned char num);
	template<> string_t numToString<unsigned int>(unsigned int num);
	template<> string_t numToString<unsigned long>(unsigned long num);

	string_t inline intToString(int num){return numToString(num);}

	string_t::size_type  getLine(string_t &, const string_t &, string_t::size_type);
	string_t getResultString(int);

	string_t replaceAll(string_t string, const string_t &find, const string_t &replace);
	std::string replaceAll(std::string string, const std::string &find, const std::string &replace);

	std::string UnicodeToMultiByte(const wchar_t *Source, unsigned int CodePage=CP_OEMCP, DWORD Flags=0);
	inline std::string UnicodeToMultiByte(const string_t& Source, unsigned int CodePage=CP_OEMCP, DWORD Flags=0) {
		return UnicodeToMultiByte(Source.c_str(),CodePage,Flags);
	}

	string_t MultiByteToUnicode(const char *Source, unsigned int CodePage=CP_OEMCP, DWORD Flags=0);
	inline string_t MultiByteToUnicode(const std::string& Source, unsigned int CodePage=CP_OEMCP, DWORD Flags=0) {
		return MultiByteToUnicode(Source.c_str(),CodePage,Flags);
	}

	string_t CodePagetoString(unsigned int cp);
	unsigned int StringtoCodePage(const char *str);

	void AsyncMessageBox(void *hwnd,char_t *message,char_t *title,unsigned int flags);

	//CriticalSection
	class CCriticalSection {
	private:
		CRITICAL_SECTION c;
		bool init;
	public:
		CCriticalSection(void);
		~CCriticalSection();

		void Enter(void);
		void Leave(void);
	};

	//CriticalSection開放ラッパー
	class CCriticalSectionLock {
	private:
		CCriticalSection &c;
	public:
		CCriticalSectionLock(CCriticalSection &pc) : c(pc) {
			c.Enter();
		}
		~CCriticalSectionLock() {
			c.Leave();
		}
	};
}

