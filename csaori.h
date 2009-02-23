/*
 * csaori.h
 * 
 * written by Ukiya http://ukiya.sakura.ne.jp/
 * based by えびさわ様 "gethwnd.dll"
 */

#pragma once

#define SAORIAPI extern "C" __declspec(dllexport)

#define SAORICDECL __cdecl

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <string>
#include <vector>
#include <map>

//charset hack
#ifdef CP_UTF8
#undef CP_UTF8
#endif

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
typedef std::basic_string<char_t> string_t;

typedef enum {CP_SJIS=932,CP_EUCJP=20932,CP_ISO2022JP=50220,CP_UTF8=65001} CODEPAGE;
typedef enum {SAORIRESULT_OK=200,SAORIRESULT_NO_CONTENT=204,SAORIRESULT_BAD_REQUEST=400,SAORIRESULT_INTERNAL_SERVER_ERROR=500} SAORIRESULT;

//global functions
namespace SAORI_FUNC {
	string_t intToString(int num);
	string_t::size_type  getLine(string_t &, const string_t &, string_t::size_type);
	string_t getResultString(int);

	std::string UnicodeToMultiByte(const wchar_t *Source, unsigned int CodePage=CP_OEMCP, DWORD Flags=0);
	inline std::string UnicodeToMultiByte(const std::wstring& Source, unsigned int CodePage=CP_OEMCP, DWORD Flags=0) {
		return UnicodeToMultiByte(Source.c_str(),CodePage,Flags);
	}

	std::wstring MultiByteToUnicode(const char *Source, unsigned int CodePage=CP_OEMCP, DWORD Flags=0);
	inline std::wstring MultiByteToUnicode(const std::string& Source, unsigned int CodePage=CP_OEMCP, DWORD Flags=0) {
		return MultiByteToUnicode(Source.c_str(),CodePage,Flags);
	}

	std::wstring CodePagetoString(unsigned int cp);
	unsigned int StringtoCodePage(const char *str);
}

//Classes
class CSAORIInput{
public:
	unsigned int codepage;
	string_t cmd;
	std::vector<string_t> args;
	std::map<string_t,string_t> opts;

	bool parseString(const string_t &src);
};

class CSAORIOutput{
public:
	unsigned int codepage;
	SAORIRESULT result_code;
	string_t result;
	std::vector<string_t> values;
	std::map<string_t,string_t> opts;

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
	//内部関数
	void setModulePath(const std::string &str);
	std::string request(const std::string &req);
	
	//相対パスかどうかをチェックして全部絶対パスに変換
	string_t checkAndModifyPath(const string_t &path);

	//以下が実装すべき関数
	void exec(const CSAORIInput& in,CSAORIOutput& out);
	bool unload();
	bool load();
};
