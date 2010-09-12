#ifdef _MSC_VER
#pragma warning( disable : 4786 )
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <process.h>
#include <atlbase.h>
#include <comdef.h>
#include <shellapi.h>
#include "csaori.h"

#import "SubWCRevCOM.exe" named_guids

//data struct
class ExecuteTortoiseProcData {
public:
	std::string tsvn;
	std::string arg;
	std::wstring path;
	std::string notify_event;
	void *hwnd;
};

//Prototypes
static bool GetTortoiseProcPath(std::string &path);
static DWORD ExecuteTortoiseProc(const ExecuteTortoiseProcData &d);

static void _cdecl ExecuteTortoiseProcThread(void *d);

void *g_hwnd = NULL;

bool CSAORI::load()
{
	//CoInitialize(NULL);
	return true;
}

bool CSAORI::unload()
{
	return false;
}

void CSAORI::exec(const CSAORIInput &in, CSAORIOutput &out)
{
	//パラメータ必須 Arg0:コマンド
	if ( in.args.size() < 1 ) {
		out.result_code = SAORIRESULT_BAD_REQUEST;
		return;
	}

	const std::wstring &cmd = in.args[0];

	//***** installed *****
	if ( wcsnicmp(cmd.c_str(),L"install",7) == 0 ) {
		std::string path;
	
		out.result_code = SAORIRESULT_OK;
		out.result = L"";

		if ( GetTortoiseProcPath(path) ) {
			DWORD tmp;
			DWORD dwVersionSize = ::GetFileVersionInfoSize(path.c_str(),&tmp);

			if ( dwVersionSize ) {
				void *pBuf = malloc(dwVersionSize + 16); //念のため16バイトほどおまけしてみる

				if ( ::GetFileVersionInfo(path.c_str(),NULL,dwVersionSize + 16,pBuf) ) {
					UINT verInfoLen;
					VS_FIXEDFILEINFO *verInfo;
					::VerQueryValue(pBuf,"\\",(void**)&verInfo,&verInfoLen);

					if ( verInfoLen ) {
						wchar_t buf[70];

						swprintf(buf,L"%u.%u.%u.%u",
						verInfo->dwFileVersionMS >> 16,
						verInfo->dwFileVersionMS & 0xFFFFU,
						verInfo->dwFileVersionLS >> 16,
						verInfo->dwFileVersionLS & 0xFFFFU);

						out.result = buf;
					}
				}

				free(pBuf);
			}
		}

		return;
	}

	//***** helpコマンド / settingsコマンドなど *****
	if ( wcsnicmp(cmd.c_str(),L"help",4) == 0 || wcsnicmp(cmd.c_str(),L"setting",7) == 0 ||
		wcsnicmp(cmd.c_str(),L"about",5) == 0 || wcsnicmp(cmd.c_str(),L"rtfm",4) == 0 ) {
		std::string path;
		if ( GetTortoiseProcPath(path) ) {
			std::string command;

			if ( wcsnicmp(cmd.c_str(),L"help",4) == 0 ) {
				command = "/command:help";
			}
			else if ( wcsnicmp(cmd.c_str(),L"about",5) == 0 ) {
				command = "/command:about";
			}
			else if ( wcsnicmp(cmd.c_str(),L"rtfm",4) == 0 ) {
				command = "/command:rtfm";
			}
			else {
				command = "/command:settings";
			}

			::ShellExecuteA(NULL,"open",path.c_str(),command.c_str(),NULL,SW_SHOWNORMAL);
		}
		return;
	}

	//パラメータ必須 Arg0:コマンド Arg1:パス Arg2以降:サブコマンド
	if ( in.args.size() < 2 ) {
		out.result_code = SAORIRESULT_BAD_REQUEST;
		return;
	}

	const std::wstring &path = in.args[1];

	out.result_code = SAORIRESULT_OK;

	//***** hwndコマンド *****
	if ( wcsnicmp(cmd.c_str(),L"hwnd",4) == 0 ) {
		//このコマンドのみ特殊：Arg1はhwnd
		g_hwnd = reinterpret_cast<void*>(wcstoul(path.c_str(),NULL,10));
		return;
	}

	//***** infoコマンド -> SubWCRev/COMを呼び出し *****
	if ( wcsnicmp(cmd.c_str(),L"info",4) == 0 ) {
		if ( in.args.size() < 3 ) {
			out.result_code = SAORIRESULT_BAD_REQUEST;
			return;
		}

		CComPtr<LibSubWCRev::ISubWCRev> pRev;
		if ( FAILED(pRev.CoCreateInstance(LibSubWCRev::CLSID_SubWCRev,NULL,CLSCTX_LOCAL_SERVER )) || pRev == NULL ) {
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
		return;
	}

	//***** info/install以外 *****
	/*if*/ {
		std::string tsvn;
		if ( ! GetTortoiseProcPath(tsvn) ) {
			out.result = L"!ERROR!|TSVN_NOT_FOUND|TortoiseProc";
			return;
		}

		//指定パス存在チェック
		std::string path_a;
		if ( path.find(L"*") == std::wstring::npos ) {
			path_a = SAORI_FUNC::UnicodeToMultiByte(path);
			if ( ::GetFileAttributes(path_a.c_str()) == INVALID_FILE_ATTRIBUTES ) {
				out.result = L"!ERROR!|PATH_INVALID|TortoiseProc";
				return;
			}
		}

		//スイッチ構築
		std::string arg;
		arg += "/command:" + SAORI_FUNC::UnicodeToMultiByte(cmd) + " ";

		arg += "/path:\"" + path_a + "\" ";

		size_t n = in.args.size();

		bool close_on_end_found = false;
		void *hwnd = NULL;
		bool notify_found = false;
		std::string notify_event;

		for ( size_t i = 2 ; i < n ; ++i ) {
			const std::wstring &c = in.args[i];

			if ( wcsnicmp(c.c_str(),L"notify",6) || wcsnicmp(c.c_str(),L"/notify",7) ) {
				const wchar_t *pParam;
				if ( c[0] != L'/' ) {
					pParam = c.c_str() + 6;
				}
				else {
					pParam = c.c_str() + 7;
				}
				notify_found = true;

				if ( pParam[0] == L':' ) {
					pParam += 1;
					notify_event = SAORI_FUNC::UnicodeToMultiByte(pParam);
				}
			}
			else {
				if ( c[0] != L'/' ) { //スラッシュ省略されたときには補う
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

		if ( g_hwnd && notify_found ) {
			ExecuteTortoiseProcData *pData = new ExecuteTortoiseProcData;
			pData->arg = arg;
			pData->hwnd = g_hwnd;
			pData->tsvn = tsvn;
			pData->path = path;
			pData->notify_event = notify_event;

			_beginthread(ExecuteTortoiseProcThread,0,pData);
		}
		else {
			ExecuteTortoiseProcData data;
			data.arg = arg;
			data.hwnd = NULL;
			data.tsvn = tsvn;
			data.path = path;
			data.notify_event = notify_event;

			if ( ExecuteTortoiseProc(data) == 0 ) {
				out.result = L"OK";
			}
			else {
				out.result = L"!ERROR!|TSVN_EXEC_FAIL|TortoiseProc";
			}
		}
		return;
	}
}

/*-----------------------------------------------------
	スレッドによるTortoiseProc実行部
------------------------------------------------------*/
void _cdecl ExecuteTortoiseProcThread(void *ptr)
{
	::CoInitialize(NULL);

	ExecuteTortoiseProcData *pData = reinterpret_cast<ExecuteTortoiseProcData*>(ptr);
	const ExecuteTortoiseProcData &d = *pData;

	CComPtr<LibSubWCRev::ISubWCRev> pRev;
	if ( FAILED(pRev.CoCreateInstance(LibSubWCRev::CLSID_SubWCRev,NULL,CLSCTX_LOCAL_SERVER )) || pRev == NULL ) {
		pRev = NULL;
	}

	long before = -1;
	if ( pRev && ! FAILED(pRev->GetWCInfo(d.path.c_str(),true,true)) ) {
		before = pRev->Revision;
	}

	DWORD result = ExecuteTortoiseProc(*pData);

	long after = -1;
	if ( pRev && ! FAILED(pRev->GetWCInfo(d.path.c_str(),true,true)) ) {
		after = pRev->Revision;
	}

	std::string sstp = "NOTIFY SSTP/1.1\r\nCharset: UTF-8\r\nSender: tsvnexec SAORI\r\n";
	sstp += "HWnd: 0\r\nEvent: ";
	if ( strncmp(d.notify_event.c_str(),"On",2) == 0 ) {
		sstp += d.notify_event;
	}
	else {
		sstp += "OnTSVNExecComplete";
	}
	sstp += "\r\n";

	sstp += "Reference0: ";
	sstp += d.notify_event;
	sstp += "\r\n";

	char data[128];
	sprintf(data,"Reference1: %d\r\nReference2: %d\r\nReference3: %d\r\n",static_cast<long>(result),before,after);
	sstp += data;
	sstp += "\r\n";

	DWORD sstpresult;
	
	COPYDATASTRUCT c;
	c.dwData = 9801;
	c.cbData = sstp.size();
	c.lpData = const_cast<char*>(sstp.c_str());

	SendMessageTimeout(reinterpret_cast<HWND>(d.hwnd),
		WM_COPYDATA,
		reinterpret_cast<WPARAM>(d.hwnd),
		reinterpret_cast<LPARAM>(&c),
		SMTO_ABORTIFHUNG,1000,&sstpresult);

	delete pData;
}

/*-----------------------------------------------------
	TortoiseProc実行部
------------------------------------------------------*/
DWORD ExecuteTortoiseProc(const ExecuteTortoiseProcData &d)
{
	PROCESS_INFORMATION pi;
	STARTUPINFO si;

	ZeroMemory(&si,sizeof(si));
	si.cb=sizeof(si);

	const char *pAppName = (const char*)_mbsrchr((const unsigned char*)d.tsvn.c_str(),'\\');
	std::string param;
	if ( pAppName ) {
		param = pAppName + 1;
	}
	else {
		param = d.tsvn;
	}
	param += " ";
	param += d.arg;

	char *paramptr = (char*)malloc(param.size()+1);
	memcpy(paramptr,param.c_str(),param.size()+1);

	DWORD result = 0xffffffffU;
	if ( ::CreateProcess(d.tsvn.c_str(),paramptr,NULL,NULL,FALSE,
		::GetPriorityClass(::GetCurrentProcess()),
		NULL,NULL,&si,&pi) ) {

		result = true;

		::CloseHandle(pi.hThread);
		if ( d.hwnd ) {
			::WaitForSingleObject(pi.hProcess,INFINITE);
			::GetExitCodeProcess(pi.hProcess,&result);
		}
		::CloseHandle(pi.hProcess);
	}

	free(paramptr);
	return result;
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
