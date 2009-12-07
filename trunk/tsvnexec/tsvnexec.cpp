#ifdef _MSC_VER
#pragma warning( disable : 4786 )
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wininet.h>
#include <atlbase.h>
#include <shlobj.h>
#include <comdef.h>
#include "csaori.h"

#import "SubWCRevCOM.exe" named_guids

static bool GetTortoiseProcPath(std::string &path);

bool CSAORI::load()
{
	return true;
}

bool CSAORI::unload()
{
	return false;
}

void CSAORI::exec(const CSAORIInput &in, CSAORIOutput &out)
{
	//パラメータ必須 Arg0:パス Arg1:コマンド Arg2以降:サブコマンド
	if ( in.args.size() < 2 ) {
		out.result_code = SAORIRESULT_BAD_REQUEST;
		return;
	}

	const std::wstring &path = in.args[0];
	const std::wstring &cmd = in.args[1];

	out.result_code = SAORIRESULT_OK;

	if ( wcsnicmp(cmd.c_str(),L"info",4) == 0 ) {
		if ( in.args.size() < 3 ) {
			out.result_code = SAORIRESULT_BAD_REQUEST;
			return;
		}

		CComPtr<LibSubWCRev::ISubWCRev> pRev;
		if ( FAILED(pRev.CoCreateInstance(LibSubWCRev::CLSID_SubWCRev,NULL,CLSCTX_ALL)) || pRev == NULL ) {
			out.result = L"!ERROR!|TSVN_NOT_FOUND|ISubWCRev";
			return;
		}

		if ( FAILED(pRev->GetWCInfo(path.c_str(),true,true)) ) {
			out.result = L"!ERROR!|PATH_INVALID|ISubWCRev";
			return;
		}
		
		out.result = L"OK";

		size_t n = in.args.size();
		for ( size_t i = 2 ; i < n ; ++i ) {
			const std::wstring &c = in.args[i];

			if ( wcsicmp(c.c_str(),L"Revision") == 0 ) {
				out.values.push_back(std::wstring(static_cast<_bstr_t>(pRev->Revision)));
			}
			else if ( wcsicmp(c.c_str(),L"MinRev") == 0 ) {
				out.values.push_back(std::wstring(static_cast<_bstr_t>(pRev->MinRev)));
			}
			else if ( wcsicmp(c.c_str(),L"MaxRev") == 0 ) {
				out.values.push_back(std::wstring(static_cast<_bstr_t>(pRev->MaxRev)));
			}
			else if ( wcsicmp(c.c_str(),L"Date") == 0 ) {
				out.values.push_back(std::wstring(static_cast<_bstr_t>(pRev->Date)));
			}
			else if ( wcsicmp(c.c_str(),L"Url") == 0 ) {
				out.values.push_back(std::wstring(static_cast<_bstr_t>(pRev->Url)));
			}
			else if ( wcsicmp(c.c_str(),L"Author") == 0 ) {
				out.values.push_back(std::wstring(static_cast<_bstr_t>(pRev->Author)));
			}
			else if ( wcsicmp(c.c_str(),L"HasModifications") == 0 ) {
				out.values.push_back(std::wstring(pRev->HasModifications ? L"true" : L"false"));
			}
			else if ( wcsicmp(c.c_str(),L"IsSvnItem") == 0 ) {
				out.values.push_back(std::wstring(pRev->IsSvnItem ? L"true" : L"false"));
			}
			else if ( wcsicmp(c.c_str(),L"NeedsLocking") == 0 ) {
				out.values.push_back(std::wstring(pRev->NeedsLocking ? L"true" : L"false"));
			}
			else if ( wcsicmp(c.c_str(),L"IsLocked") == 0 ) {
				out.values.push_back(std::wstring(pRev->IsLocked ? L"true" : L"false"));
			}
			else if ( wcsicmp(c.c_str(),L"LockCreationDate") == 0 ) {
				out.values.push_back(std::wstring(static_cast<_bstr_t>(pRev->LockCreationDate)));
			}
			else if ( wcsicmp(c.c_str(),L"LockOwner") == 0 ) {
				out.values.push_back(std::wstring(static_cast<_bstr_t>(pRev->LockOwner)));
			}
			else if ( wcsicmp(c.c_str(),L"LockComment") == 0 ) {
				out.values.push_back(std::wstring(static_cast<_bstr_t>(pRev->LockComment)));
			}
			else {
				out.values.push_back(std::wstring(L"!ERROR!|PARAMETER_INVALID"));
			}
		}
	}
	else {
		std::string tsvn;
		if ( ! GetTortoiseProcPath(tsvn) ) {
			out.result = L"!ERROR!|TSVN_NOT_FOUND|TortoiseProc";
			return;
		}

		std::string path_a = SAORI_FUNC::UnicodeToMultiByte(path);
		if ( ::GetFileAttributes(path_a.c_str()) == INVALID_FILE_ATTRIBUTES ) {
			out.result = L"!ERROR!|PATH_INVALID|TortoiseProc";
			return;
		}

		std::string arg;
		arg += "/command:" + SAORI_FUNC::UnicodeToMultiByte(cmd) + " ";

		arg += "/path:\"" + path_a + "\" ";

		size_t n = in.args.size();

		bool close_on_end_found = false;
		bool wait_found = false;

		for ( size_t i = 2 ; i < n ; ++i ) {
			const std::wstring &c = in.args[i];

			if ( wcsnicmp(c.c_str(),L"wait",4) || wcsnicmp(c.c_str(),L"/wait",5) ) {
				wait_found = true;
			}
			else {
				if ( c[0] != L'/' ) {
					arg += "/";
				}
				SAORI_FUNC::UnicodeToMultiByte(c) + " ";

				if ( wcsnicmp(c.c_str(),L"closeonend",10) || wcsnicmp(c.c_str(),L"/closeonend",11) ) {
					close_on_end_found = true;
				}
			}
		}

		if ( ! close_on_end_found ) {
			arg += "/closeonend:1 ";
		}

		PROCESS_INFORMATION pi;
		STARTUPINFO si;

		ZeroMemory(&si,sizeof(si));
		si.cb=sizeof(si);

		const char *pAppName = (const char*)_mbsrchr((const unsigned char*)tsvn.c_str(),'\\');
		std::string param;
		if ( pAppName ) {
			param = pAppName + 1;
		}
		else {
			param = tsvn;
		}
		param += arg;

		char *paramptr = (char*)malloc(param.size()+1);
		memcpy(paramptr,param.c_str(),param.size()+1);

		if ( ::CreateProcess(tsvn.c_str(),paramptr,NULL,NULL,FALSE,
			::GetPriorityClass(::GetCurrentProcess()),
			NULL,NULL,&si,&pi) ) {

			::CloseHandle(pi.hThread);
			if ( wait_found ) {
				::WaitForSingleObject(pi.hProcess,INFINITE);
			}
			::CloseHandle(pi.hProcess);

			out.result = L"OK";
		}
		else {
			out.result = L"!ERROR!|TSVN_EXEC_FAIL|TortoiseProc";
		}

		free(paramptr);
	}
}

/*-----------------------------------------------------
	SVNのパスを取る
------------------------------------------------------*/
bool GetTortoiseProcPath(std::string &path)
{
	HKEY hKeyTSVN;
	int result = ::RegOpenKeyEx(HKEY_LOCAL_MACHINE,"Software\\TortoiseSVN",0,KEY_READ,&hKeyTSVN);
	if ( result != ERROR_SUCCESS || hKeyTSVN == NULL ) {
		return FALSE;
	}
	
	char data[MAX_PATH];
	DWORD dataLen = sizeof(data);

	result = ::RegQueryValueEx(hKeyTSVN,"ProcPath",0,NULL,(BYTE*)data,&dataLen);
	::RegCloseKey(hKeyTSVN);

	if ( result != ERROR_SUCCESS || data[0] == 0 ) {
		return FALSE;
	}

	path = data;
	return TRUE;
}
