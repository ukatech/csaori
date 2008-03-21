/*
saorimnp.h

2008/03/18 version 1.0 êVãKçÏê¨

written by Ukiya http://ukiya.sakura.ne.jp/
based by Ç¶Ç—Ç≥ÇÌól "gethwnd.dll"
*/

#pragma once

#ifdef SAORI_EXPORTS
#define SAORIAPI extern "C" __declspec(dllexport)
#else
#define SAORIAPI extern "C" __declspec(dllimport) 
#endif

#define SAORICDECL __cdecl

#include <windows.h>
#include <string>
#include <vector>
#include <map>
#include <cstring>
#include <cstdlib>
#include <algorithm>
#include <locale>

using namespace std;

//SAORI INTERFACES
SAORIAPI BOOL SAORICDECL load(HGLOBAL h,long len);
SAORIAPI BOOL SAORICDECL unload();
SAORIAPI HGLOBAL SAORICDECL request(HGLOBAL h,long* len);

//DLLMain
BOOL APIENTRY DllMain( HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved);

#define SAORI_VERSIONSTRING_10		L"SAORI/1.0"
#define SAORI_VERSIONSTRING			SAORI_VERSIONSTRING_10
#define SAORI_SIZEOFVERSIONSTRING	(sizeof(SAORI_VERSIONSTRING) - 1)

typedef wchar_t char_t;
typedef basic_string<char_t> string_t;

typedef enum {CHARSET_Shift_JIS,CHARSET_ISO_2022_JP,CHARSET_EUC_JP,CHARSET_UTF_8} CHARSET;
typedef enum {SAORIRESULT_OK=200,SAORIRESULT_NO_CONTENT=204,SAORIRESULT_BAD_REQUEST=400,SAORIRESULT_INTERNAL_SERVER_ERROR=500} SAORIRESULT;

//global functions
namespace SAORI_FUNC{
	static string_t numToString(unsigned num, unsigned base=10, char_t sign=L'\0');
	static string_t intToString(int num, unsigned base=10);
	static string_t::size_type  getLine(string_t &, const string_t &, string_t::size_type);
	static string_t getResultString(int);

	static std::string UnicodeToMultiByte(const std::wstring& Source, UINT CodePage=CP_ACP, DWORD Flags=0);
	static std::wstring MultiByteToUnicode(const std::string& Source, UINT CodePage=CP_ACP, DWORD Flags=0);
	static UINT CHARSETtoCodePage(CHARSET cset);
	static wstring CHARSETtoString(CHARSET cset);
}

//Classes
class CSAORIInput{
public:
	CHARSET charset;
	string_t cmd;
	vector<string_t> args;
	map<string_t,string_t> opts;

	bool parseString(const string_t src);
};

class CSAORIOutput{
public:
	CHARSET charset;
	SAORIRESULT result_code;
	string_t result;
	vector<string_t> values;
	map<string_t,string_t> opts;

	string_t toString();
};

class CSAORI{
public:
	CSAORI(){
		setlocale( LC_ALL, "Japanese");
	}

private:
	string_t module_path;

public:
	void setModulePath(string str);
	string request(string req);

	//à»â∫Ç™é¿ëïÇ∑Ç◊Ç´ä÷êî
	void exec(const CSAORIInput& in,CSAORIOutput& out);
	bool unload();
	bool load();
};
