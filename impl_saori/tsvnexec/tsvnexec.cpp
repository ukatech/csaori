#ifdef _MSC_VER
#pragma warning( disable : 4786 )
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <process.h>
#include <comdef.h>
#include <shellapi.h>
#include <mbstring.h>
#include <commctrl.h>

#include "csaori.h"
#include "util/cs_threadcall.h"

#import "SubWCRevCOM.exe" named_guids

//////////WINDOWS DEFINE///////////////////////////
#ifdef _WINDOWS
#ifdef _DEBUG
#include <crtdbg.h>
#define new new( _NORMAL_BLOCK, __FILE__, __LINE__)
#endif
#endif
///////////////////////////////////////////////////

//data struct
class CTSVNSAORI;

class ExecuteTortoiseProcData {
public:
	std::string tsvn;
	std::string arg;
	std::string notify_event;
	std::string msg;
	string_t path;

	void *hwnd;
	CTSVNSAORI *saori;
	bool minimize;
};

class SubWCRevData {
public:
    bool     HasModifications;
    bool     IsSvnItem;
    bool     NeedsLocking;
    bool     IsLocked;
    DWORD    Revision;
    DWORD    MinRev;
    DWORD    MaxRev;
    string_t Date;
    string_t Url;
    string_t Author;
    string_t LockCreationDate;
    string_t LockOwner;
    string_t LockComment;

	string_t path;

	SubWCRevData(void) {
		HasModifications = false;
		IsSvnItem = false;
		NeedsLocking = false;
		IsLocked = 0;
		Revision = 0;
		MinRev = 0;
		MaxRev = 0;
	}
	void Set(LibSubWCRev::ISubWCRev &i) {
		HasModifications = i.HasModifications != 0;
		IsSvnItem = i.IsSvnItem != 0;
		NeedsLocking = i.NeedsLocking != 0;
		IsLocked = i.IsLocked != 0;
		Revision = wcstoul(static_cast<_bstr_t>(i.Revision),NULL,10);
		MinRev = wcstoul(static_cast<_bstr_t>(i.MinRev),NULL,10);
		MaxRev = wcstoul(static_cast<_bstr_t>(i.MaxRev),NULL,10);
		Date = static_cast<_bstr_t>(i.Date);
		Url = static_cast<_bstr_t>(i.Url);
		Author = static_cast<_bstr_t>(i.Author);
		LockCreationDate = static_cast<_bstr_t>(i.LockCreationDate);
		LockOwner = static_cast<_bstr_t>(i.LockOwner);
		LockComment = static_cast<_bstr_t>(i.LockComment);
	}
};

#ifndef IID_PPV_ARGS
extern "C++" {
    template<typename T> void** IID_PPV_ARGS_Helper(T** pp) {
        static_cast<IUnknown*>(*pp);    // make sure everyone derives from IUnknown
        return reinterpret_cast<void**>(pp);
    }
}

#define IID_PPV_ARGS(ppType) __uuidof(**(ppType)), IID_PPV_ARGS_Helper(ppType)
#endif //IID_PPV_ARGS

//Prototypes
static void _cdecl ExecuteTortoiseProcThreadFunc(void *d);
static void _cdecl SubWCRevThreadFunc(void *d);

//Class Definitions
class CTSVNSAORI : public CSAORI, public CSThreadCallBase {
private:
	void  *m_hwnd;

	volatile HANDLE m_hThread;
	HANDLE m_hThreadExitEvent;

	HWND   m_hTSVNWindow;

	SubWCRevData *m_pRev;
	LibSubWCRev::ISubWCRev *m_pSubWCRev;

protected:
	virtual void CSTC_Start(void);
	virtual void CSTC_Exit(void);
	virtual void CSTC_Call(void *p1,void *p2);

public:
	CTSVNSAORI();
	virtual ~CTSVNSAORI();

	bool GetTortoiseProcPath(std::string &path);

	DWORD ExecuteTortoiseProc(ExecuteTortoiseProcData &d);
	void  ExecuteTortoiseProcThread(ExecuteTortoiseProcData *d);

	SubWCRevData* SubWCRev(const string_t& path);

	//以下が実装すべき関数
	virtual void exec(const CSAORIInput& in,CSAORIOutput& out);
	virtual bool unload();
	virtual bool load();
};

//CreateInstance
CSAORIBase* CreateInstance(void)
{
	return new CTSVNSAORI();
}

CTSVNSAORI::CTSVNSAORI(void) : m_hwnd(NULL) , m_hThread(NULL) , m_hTSVNWindow(NULL) , m_pRev(NULL) , m_pSubWCRev(NULL)
{
	m_hThreadExitEvent = ::CreateEvent(NULL,TRUE,FALSE,NULL);
}

CTSVNSAORI::~CTSVNSAORI()
{
	if ( m_pRev ) {
		delete m_pRev;
	}
	::CloseHandle(m_hThreadExitEvent);
}

bool CTSVNSAORI::load()
{
	GetCSThreadCall().Set(this);
	return true;
}

bool CTSVNSAORI::unload()
{
	::SetEvent(m_hThreadExitEvent);
	if ( m_hThread ) {
		::WaitForSingleObject(m_hThread,INFINITE);
	}

	GetCSThreadCall().Exit();
	return true;
}

void CTSVNSAORI::exec(const CSAORIInput &in, CSAORIOutput &out)
{
	//パラメータ必須 Arg0:コマンド
	if ( in.args.size() < 1 ) {
		out.result_code = SAORIRESULT_BAD_REQUEST;
		return;
	}

	const string_t &cmd = in.args[0];

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

		out.result_code = SAORIRESULT_OK;
		out.result = L"";
		
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

	//***** cancel *****
	if ( wcsnicmp(cmd.c_str(),L"cancel",6) == 0 ) {
		out.result_code = SAORIRESULT_OK;
		out.result = L"";

		if ( m_hTSVNWindow ) {
			::PostMessage(m_hTSVNWindow,WM_COMMAND,MAKEWPARAM(IDCANCEL,BN_CLICKED),NULL);
		}
		return;
	}


	//パラメータ必須 Arg0:コマンド Arg1:パス Arg2以降:サブコマンド
	if ( in.args.size() < 2 ) {
		out.result_code = SAORIRESULT_BAD_REQUEST;
		return;
	}

	const string_t &path = in.args[1];

	out.result_code = SAORIRESULT_OK;

	//***** hwndコマンド *****
	if ( wcsnicmp(cmd.c_str(),L"hwnd",4) == 0 ) {
		//このコマンドのみ特殊：Arg1はhwnd
		m_hwnd = reinterpret_cast<void*>(wcstoul(path.c_str(),NULL,10));
		return;
	}

	//***** infoコマンド -> SubWCRev/COMを呼び出し *****
	if ( wcsnicmp(cmd.c_str(),L"info",4) == 0 ) {
		if ( in.args.size() < 3 ) {
			out.result_code = SAORIRESULT_BAD_REQUEST;
			return;
		}

		SubWCRevData *pWCRev = SubWCRev(path);
		if ( pWCRev == NULL ) {
			out.result = L"!ERROR!|TSVN_NOT_FOUND|ISubWCRev";
			return;
		}
		
		out.result = L"OK";
		char_t buffer[32];

		size_t n = in.args.size();
		for ( size_t i = 2 ; i < n ; ++i ) {
			const string_t &c = in.args[i];

			if ( wcsicmp(c.c_str(),L"Revision") == 0 ) {
				swprintf(buffer,L"%d",pWCRev->Revision);
				out.values.push_back(buffer);
			}
			else if ( wcsicmp(c.c_str(),L"MinRev") == 0 ) {
				swprintf(buffer,L"%d",pWCRev->MinRev);
				out.values.push_back(buffer);
			}
			else if ( wcsicmp(c.c_str(),L"MaxRev") == 0 ) {
				swprintf(buffer,L"%d",pWCRev->MaxRev);
				out.values.push_back(buffer);
			}
			else if ( wcsicmp(c.c_str(),L"Date") == 0 ) {
				out.values.push_back(pWCRev->Date);
			}
			else if ( wcsicmp(c.c_str(),L"Url") == 0 ) {
				out.values.push_back(pWCRev->Url);
			}
			else if ( wcsicmp(c.c_str(),L"Author") == 0 ) {
				out.values.push_back(pWCRev->Author);
			}
			else if ( wcsicmp(c.c_str(),L"HasModifications") == 0 ) {
				out.values.push_back(string_t(pWCRev->HasModifications ? L"true" : L"false"));
			}
			else if ( wcsicmp(c.c_str(),L"IsSvnItem") == 0 ) {
				out.values.push_back(string_t(pWCRev->IsSvnItem ? L"true" : L"false"));
			}
			else if ( wcsicmp(c.c_str(),L"NeedsLocking") == 0 ) {
				out.values.push_back(string_t(pWCRev->NeedsLocking ? L"true" : L"false"));
			}
			else if ( wcsicmp(c.c_str(),L"IsLocked") == 0 ) {
				out.values.push_back(string_t(pWCRev->IsLocked ? L"true" : L"false"));
			}
			else if ( wcsicmp(c.c_str(),L"LockCreationDate") == 0 ) {
				out.values.push_back(pWCRev->LockCreationDate);
			}
			else if ( wcsicmp(c.c_str(),L"LockOwner") == 0 ) {
				out.values.push_back(pWCRev->LockOwner);
			}
			else if ( wcsicmp(c.c_str(),L"LockComment") == 0 ) {
				out.values.push_back(pWCRev->LockComment);
			}
			else {
				out.values.push_back(string_t(L"!ERROR!|PARAMETER_INVALID"));
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
		if ( path.find(L"*") == string_t::npos ) {
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
		bool minimize_found = false;

		std::string notify_event;
		std::string close_on_end_arg;

		for ( size_t i = 2 ; i < n ; ++i ) {
			const string_t &c = in.args[i];

			if ( wcsnicmp(c.c_str(),L"notify",6) == 0 || wcsnicmp(c.c_str(),L"/notify",7) == 0 ) {
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
			else if ( wcsnicmp(c.c_str(),L"minimize",8) == 0 || wcsnicmp(c.c_str(),L"/minimize",9) == 0 ) {
				minimize_found = true;
			}
			else {
				if ( wcsnicmp(c.c_str(),L"closeonend",10) == 0 || wcsnicmp(c.c_str(),L"/closeonend",11) == 0 ) {
					close_on_end_arg.erase();
					if ( c[0] != L'/' ) { //スラッシュ省略されたときには補う
						close_on_end_arg += "/";
					}
					close_on_end_arg += SAORI_FUNC::UnicodeToMultiByte(c);
				}
				else {
					if ( c[0] != L'/' ) { //スラッシュ省略されたときには補う
						arg += "/";
					}
					arg += SAORI_FUNC::UnicodeToMultiByte(c) + " ";
				}
			}
		}

		if ( minimize_found ) {
			arg += "/closeonend:0 ";
		}
		else {
			if ( close_on_end_arg.size() ) {
				arg += close_on_end_arg;
			}
		}

		if ( m_hwnd && notify_found ) {
			if ( m_hThread ) {
				out.result = L"!ERROR!|TSVN_DOUBLE_EXEC|TortoiseProc";
			}
			else {
				ExecuteTortoiseProcData *pData = new ExecuteTortoiseProcData;
				pData->arg = arg;
				pData->hwnd = m_hwnd;
				pData->tsvn = tsvn;
				pData->path = path;
				pData->notify_event = notify_event;
				pData->minimize = minimize_found;
				pData->saori = this;

				unsigned long result = _beginthread(ExecuteTortoiseProcThreadFunc,0,pData);
				if ( result != static_cast<unsigned long>(-1) ) {
					m_hThread = reinterpret_cast<HANDLE>(result);
				}
			}
		}
		else {
			ExecuteTortoiseProcData data;
			data.arg = arg;
			data.hwnd = NULL;
			data.tsvn = tsvn;
			data.path = path;
			data.notify_event = notify_event;
			data.minimize = false;
			data.saori = this;

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
	COM例外吐き対策
------------------------------------------------------*/
void CTSVNSAORI::CSTC_Start(void)
{
	; //NOOP
}
void CTSVNSAORI::CSTC_Exit(void)
{
	try {
		if ( m_pSubWCRev ) {
			m_pSubWCRev->Release();
			m_pSubWCRev = NULL;
		}
	}
	catch ( ... ) {
		; //NOOP
	}
}
void CTSVNSAORI::CSTC_Call(void *p1,void *p2)
{
	try {
		if ( ! m_pSubWCRev ) {
			HRESULT hr = ::CoCreateInstance(LibSubWCRev::CLSID_SubWCRev,NULL,
				CLSCTX_LOCAL_SERVER|CLSCTX_INPROC_HANDLER|CLSCTX_INPROC_SERVER ,IID_PPV_ARGS(&m_pSubWCRev));

			if ( ! SUCCEEDED(hr) ) {
				m_pSubWCRev = NULL;
			}
		}
		if ( m_pSubWCRev ) {
			SubWCRevData *pData = reinterpret_cast<SubWCRevData*>(p1);
			if ( SUCCEEDED(m_pSubWCRev->GetWCInfo(pData->path.c_str(),true,true)) ) {
				pData->Set(*m_pSubWCRev);
			}
		}
	}
	catch ( ... ) {
		; //NOOP
	}
}

SubWCRevData* CTSVNSAORI::SubWCRev(const string_t& path)
{
	SubWCRevData *pData = new SubWCRevData;
	pData->path = path;

	GetCSThreadCall().Call(pData,NULL,true);

	if ( m_pRev ) {
		delete m_pRev;
	}
	m_pRev = pData;
	return pData;
}

/*-----------------------------------------------------
	スレッドによるTortoiseProc実行部
------------------------------------------------------*/
void _cdecl ExecuteTortoiseProcThreadFunc(void *ptr)
{
	ExecuteTortoiseProcData *pData = reinterpret_cast<ExecuteTortoiseProcData*>(ptr);
	pData->saori->ExecuteTortoiseProcThread(pData);
}

void CTSVNSAORI::ExecuteTortoiseProcThread(ExecuteTortoiseProcData *pData)
{
	const ExecuteTortoiseProcData &d = *pData;

	long before = -1;
	
	SubWCRevData *pWCRev = pData->saori->SubWCRev(d.path.c_str());
	if ( pWCRev ) {
		before = pWCRev->Revision;
	}

	DWORD result = pData->saori->ExecuteTortoiseProc(*pData);

	long after = -1;

	pWCRev = pData->saori->SubWCRev(d.path.c_str());
	if ( pWCRev ) {
		after = pWCRev->Revision;
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
	sstp += "Reference4: ";
	sstp += pData->msg;
	sstp += "\r\n";
	sstp += "\r\n";
	
	DWORD sstpresult;

	COPYDATASTRUCT c;
	c.dwData = 9801;
	c.cbData = sstp.size();
	c.lpData = const_cast<char*>(sstp.c_str());

	m_hThread = NULL;
	m_hTSVNWindow = NULL;

	::SendMessageTimeout(reinterpret_cast<HWND>(d.hwnd),
		WM_COPYDATA,
		reinterpret_cast<WPARAM>(d.hwnd),
		reinterpret_cast<LPARAM>(&c),
		SMTO_ABORTIFHUNG,5000,&sstpresult);

	delete pData;
}

/*-----------------------------------------------------
	TortoiseProc実行部
------------------------------------------------------*/
BOOL CALLBACK EXTP_FindWindowProc(HWND hWnd,LPARAM lParam)
{
	char cls[256];
	::GetClassName(hWnd,cls,sizeof(cls));
	
	if ( stricmp(cls,"#32770") == 0 ) {
		*reinterpret_cast<HWND*>(lParam) = hWnd;
	}

	return true;
}

DWORD CTSVNSAORI::ExecuteTortoiseProc(ExecuteTortoiseProcData &d)
{
	PROCESS_INFORMATION pi;
	STARTUPINFO si;

	ZeroMemory(&si,sizeof(si));
	si.cb=sizeof(si);
	si.dwFlags = STARTF_USESHOWWINDOW;

	if ( d.hwnd && d.minimize ) {
		si.wShowWindow = SW_SHOWMINNOACTIVE;
	}
	else {
		si.wShowWindow = SW_SHOWNORMAL;
	}

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

		if ( d.hwnd ) {
			HWND hWndDlg = NULL;
			HWND hOK = NULL;
			//HWND hLV = NULL;

			while ( true ) {
				::Sleep(500);

				if ( ! hWndDlg ) {
					::EnumThreadWindows(pi.dwThreadId,EXTP_FindWindowProc,reinterpret_cast<LPARAM>(&hWndDlg));
					if ( ! hWndDlg ) { continue; }
					m_hTSVNWindow = hWndDlg;
				}

				if ( ! ::IsWindow(hWndDlg) ) {
					break;
				}

				if ( ! hOK ) {
					hOK = ::GetDlgItem(hWndDlg,IDOK);
					if ( ! hOK ) { continue; }
				}

				if ( ! ::IsWindowEnabled(hOK) ) { continue; }

				//hLV = ::FindWindowEx(hWndDlg,NULL,"SysListView32",NULL);
				//if ( ! hLV ) {
				//	hLV = ::FindWindowEx(hWndDlg,NULL,"SysListView64",NULL);
				//}

				char buf[512];
				::GetWindowText(hWndDlg,buf,sizeof(buf)-1);
				char *f = strrchr(buf,' ');
				if ( ! f ) { f = buf; }
				d.msg = f;

				::GetWindowText(::FindWindowEx(hWndDlg,NULL,"Static",NULL),buf,sizeof(buf)-1);
				if ( buf[0] ) {
					d.msg += " ";
					d.msg += buf;
				}
				break;
			}
			::Sleep(1000);

			if ( d.minimize ) {
				::SendMessage(hWndDlg,WM_COMMAND,MAKEWPARAM(IDOK,BN_CLICKED),reinterpret_cast<LPARAM>(hOK));
			}

			HANDLE ha[] = {pi.hProcess,m_hThreadExitEvent};
			::WaitForMultipleObjects(sizeof(ha)/sizeof(ha[0]),ha,FALSE,INFINITE);

			::GetExitCodeProcess(pi.hProcess,&result);
		}
		::CloseHandle(pi.hThread);
		::CloseHandle(pi.hProcess);
	}
	else {
		result = 0;
	}

	free(paramptr);
	return result;
}

/*-----------------------------------------------------
	SVNのパスを取る
------------------------------------------------------*/
bool CTSVNSAORI::GetTortoiseProcPath(std::string &path)
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
