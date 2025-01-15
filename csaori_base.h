/*
 * csaori_base.h
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

#include "csaori_util.h"

//SAORI INTERFACES
SAORIAPI BOOL SAORICDECL load(HGLOBAL h,long len);
SAORIAPI BOOL SAORICDECL unload();
SAORIAPI HGLOBAL SAORICDECL request(HGLOBAL h,long* len);

//DLLMain
BOOL APIENTRY DllMain( HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved);

extern HINSTANCE g_hModule;

typedef enum {
	SAORIRESULT_FORCE_OK=0,
	SAORIRESULT_OK=200,
	SAORIRESULT_NO_CONTENT=204,
	SAORIRESULT_BAD_REQUEST=400,
	SAORIRESULT_INTERNAL_SERVER_ERROR=500
} SAORIRESULT;

//Classes
class CSAORIBase;

typedef std::map<string_t,string_t> map_strpair;
typedef std::vector<string_t> vector_str;

class CSAORIInput{
private:
	const CSAORIBase &base;
	
	CSAORIInput(void); //DUMMY
public:
	CSAORIInput(const CSAORIBase &b) : base(b) {
	}
	
	unsigned int codepage;
	string_t cmd;
	string_t id;
	vector_str args;
	map_strpair opts;

	bool parseString(const string_t &src);
};

class CSAORIOutput{
private:
	const CSAORIBase &base;
	
	CSAORIOutput(void); //DUMMY
public:
	CSAORIOutput(const CSAORIBase &b) : base(b) {
	}

	unsigned int codepage;
	SAORIRESULT result_code;
	string_t result;
	vector_str values;
	map_strpair opts;

	string_t toString();
	void setResultEmpty();

	bool parseString(const string_t &src);
};

class CSAORIBase {
public:
	CSAORIBase() : module_handle(NULL) , call_id(0) {
		setlocale( LC_ALL, "Japanese");
	}
	virtual ~CSAORIBase() {
		; //NOOP
	}

private:
	string_t module_path;
	HANDLE module_handle;
	unsigned int call_id;

public:
	//Internal Functions
	void setModulePath(const std::string &str,bool isUTF8);
	void setModuleHandle(HANDLE hMod);

	const string_t& getModulePath(void) const { return module_path; }
	HANDLE getModuleHandle(void) { return module_handle; }

	std::string request(const std::string &req);
	
	//Check relative path and convert to full path
	std::string checkAndModifyPath(const std::string &path);
	string_t checkAndModifyPathW(const string_t &path);

	//exec call unique id
	unsigned int getLastCallID() { return call_id; }

	//Interface specific constant string functions to override
	virtual const string_t& s_saori_version(void) const = 0;
	virtual const string_t& s_saori_def(void) const = 0;
	virtual const string_t& s_saori_argument(void) const = 0;
	virtual const string_t& s_saori_value(void) const = 0;
	virtual const string_t& s_saori_result(void) const = 0;

	//Prologue / Epilogue
	virtual void exec_before(const CSAORIInput& in,CSAORIOutput& out) { }
	virtual void exec_after(const CSAORIInput& in,CSAORIOutput& out) { }

	//Public functions to implement.
	virtual void exec(const CSAORIInput& in,CSAORIOutput& out) = 0;
	virtual bool exec_insecure(const CSAORIInput& in,CSAORIOutput& out) { return false; } //SecLevel Remote : Optional

	virtual bool unload() = 0;
	virtual bool load() = 0;
};
