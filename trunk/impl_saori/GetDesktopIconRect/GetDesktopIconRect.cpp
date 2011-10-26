#ifdef _MSC_VER
#pragma warning( disable : 4786 )
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include <commctrl.h>
#include <shlobj.h>
#include <shellapi.h>
#include <process.h>

#include "csaori.h"

/*---------------------------------------------------------
	初期化
---------------------------------------------------------*/
bool CSAORI::load(){
	return true;
}

/*---------------------------------------------------------
	解放
---------------------------------------------------------*/
bool CSAORI::unload(){
	return true;
}

/*---------------------------------------------------------
	自動free
---------------------------------------------------------*/
class CAutoFree {
private:
	CAutoFree() { }

	void *m_pBuf;
public:
	CAutoFree(void* pBuf) : m_pBuf(pBuf) { }
	~CAutoFree() { if ( m_pBuf ) { free(m_pBuf); } }

	operator void*() { return m_pBuf; }
};

/*---------------------------------------------------------
	実行
---------------------------------------------------------*/
static HWND GetDesktopListView(void)
{
	HWND hWnd = ::FindWindow("Progman", "Program Manager");  
	if ( hWnd ) {
		hWnd = ::FindWindowEx(hWnd, NULL, "SHELLDLL_DefView", NULL);
		if ( hWnd ) {
			hWnd = ::FindWindowEx(hWnd, NULL, "SysListView32", NULL);
		}
	}
	return hWnd;
}

#define BUFFER_SIZE 4096

HWND g_hWnd = NULL;

static void __cdecl EmptyTrashThread(void*);
static void QueryTrash(DWORD &size,DWORD &item);

void CSAORI::exec(const CSAORIInput& in,CSAORIOutput& out)
{
	out.result_code = SAORIRESULT_BAD_REQUEST;

	if ( in.args.size() < 0 ) { return; }

	if ( wcsicmp(in.args[0].c_str(),L"hwnd") == 0 ) {
		if ( in.args.size() >= 2 ) {
			g_hWnd = reinterpret_cast<HWND>(wcstoul(in.args[1].c_str(),NULL,10));
			out.result_code = SAORIRESULT_OK;
		}
		return;
	}
	else if ( wcsicmp(in.args[0].c_str(),L"get") == 0 ) {
		out.result_code = SAORIRESULT_INTERNAL_SERVER_ERROR;

		//デスクトップウインドウ (左上0,0)
		HWND hWnd = GetDesktopListView();
		if ( ! hWnd ) { return; }

		//検索対象と検索結果
		string_t find_string;
		if ( in.args.size() >= 2 ) {
			find_string = in.args[1];
			if ( wcsnicmp(find_string.c_str(),L"system:",7) == 0 ) {
				int nFolder = CSIDL_PERSONAL;
				const char_t *str = find_string.c_str()+7;
				if ( wcsicmp(str,L"bitbucket") == 0 || wcsicmp(str,L"trash") == 0 ) {
					nFolder = CSIDL_BITBUCKET;
				}
				else if ( wcsicmp(str,L"internet") == 0 ) {
					nFolder = CSIDL_INTERNET;
				}
				else if ( wcsicmp(str,L"drives") == 0 || wcsicmp(str,L"computer") == 0 || wcsicmp(str,L"mycomputer") == 0 ) {
					nFolder = CSIDL_DRIVES;
				}

				LPITEMIDLIST pidl;

				IMalloc *pMalloc;
				::SHGetMalloc( &pMalloc );
				::SHGetSpecialFolderLocation(NULL,nFolder,&pidl);

				SHFILEINFOW fileinfo;
				::SHGetFileInfoW(reinterpret_cast<wchar_t*>(pidl),0,&fileinfo,sizeof(fileinfo),SHGFI_PIDL|SHGFI_DISPLAYNAME);

				pMalloc->Free(pidl);
				pMalloc->Release();

				find_string = fileinfo.szDisplayName;
			}
		}
		int find_pos = -1;
		POINT find_point;
		RECT find_rect;
		string_t find_text;

		//PIDを取って、内部に領域確保
		DWORD pid;  
		::GetWindowThreadProcessId(hWnd,&pid);

		HANDLE ph = ::OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE, false, pid);
		if ( ! ph ) { return; }

		void *ptr = ::VirtualAllocEx(ph,NULL,BUFFER_SIZE,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);
		if ( ! ptr ) {
			::CloseHandle(ph);
			return;
		}

		//要素取得ループ
		int count = ListView_GetItemCount(hWnd);

		for ( int i = 0 ; i < count ; ++i ) {
			//プロセス内メモリにLVITEMを書いて取得、残りの領域に文字列を書いてもらう
			LVITEM li;
			ZeroMemory(&li,sizeof(li));
			li.mask = LVIF_TEXT;  
			li.iItem = i;
			li.iSubItem = 0;
			li.cchTextMax = BUFFER_SIZE-sizeof(LVITEM)-1;
			li.pszText = reinterpret_cast<char*>(ptr)+sizeof(LVITEM);
			
			DWORD numRead;
			char text_buffer[BUFFER_SIZE-sizeof(LVITEM)];

			::WriteProcessMemory(ph, ptr, &li, sizeof(LVITEM), &numRead );
			::SendMessage(hWnd, LVM_GETITEM , 0, (LPARAM)ptr);
			::ReadProcessMemory(ph, reinterpret_cast<char*>(ptr)+sizeof(LVITEM), text_buffer, BUFFER_SIZE-sizeof(LVITEM)-1, &numRead );
			
			string_t text = SAORI_FUNC::MultiByteToUnicode(text_buffer);
			string_t item = text;

			ListView_GetItemPosition(hWnd,i,ptr);

			int cmd = LVIR_ICON;
			::WriteProcessMemory(ph,reinterpret_cast<char*>(ptr)+sizeof(POINT),&cmd,sizeof(cmd),&numRead);
			::SendMessage(hWnd,LVM_GETITEMRECT,i,reinterpret_cast<LPARAM>(reinterpret_cast<char*>(ptr)+sizeof(POINT)));

			POINT pt;
			RECT rect;
			::ReadProcessMemory(ph, ptr, &pt, sizeof(POINT), &numRead );
			::ReadProcessMemory(ph, reinterpret_cast<char*>(ptr)+sizeof(POINT), &rect, sizeof(RECT), &numRead );

			if ( find_pos < 0 ) {
				if ( wcsstr(text.c_str(),find_string.c_str()) ) {
					find_pos = i;
					find_point = pt;
					find_rect = rect;
					find_text = text;
				}
			}

			char_t rect_text[128];
			swprintf(rect_text,L"\1%d,%d",pt.x,pt.y);
			item += rect_text;

			swprintf(rect_text,L"\1%d,%d,%d,%d",rect.left,rect.top,rect.right,rect.bottom);
			item += rect_text;

			out.values.push_back(item);
		}

		::VirtualFreeEx(ph,ptr,0,MEM_RELEASE);
		::CloseHandle(ph);

		out.result_code = SAORIRESULT_OK;

		if ( find_pos >= 0 ) {
			char_t rect_text[128];

			swprintf(rect_text,L"%d\1",find_pos);
			out.result = rect_text;

			out.result += find_text;

			swprintf(rect_text,L"\1%d,%d",find_point.x,find_point.y);
			out.result += rect_text;

			swprintf(rect_text,L"\1%d,%d,%d,%d",find_rect.left,find_rect.top,find_rect.right,find_rect.bottom);
			out.result += rect_text;
		}
		else {
			out.result = L"";
		}
	}
	else if ( wcsicmp(in.args[0].c_str(),L"empty_trash") == 0 ) {
		_beginthread(EmptyTrashThread,0,NULL);
		out.result_code = SAORIRESULT_OK;
	}
	else if ( wcsicmp(in.args[0].c_str(),L"query_trash") == 0 ) {
		DWORD totalSize = 0;
		DWORD totalItems = 0;

		QueryTrash(totalSize,totalItems);

		char_t size_text[32];
		char_t item_text[32];
		swprintf(size_text,L"%u",totalSize);
		swprintf(item_text,L"%u",totalItems);

		out.result = item_text;
		out.values.push_back(item_text);
		out.values.push_back(size_text);

		out.result_code = SAORIRESULT_OK;
	}
}

static void QueryTrash(DWORD &totalSize,DWORD &totalItems)
{
	totalSize = 0;
	totalItems = 0;

	char pathName[1024];
	if ( ! ::GetLogicalDriveStrings(sizeof(pathName)-2,pathName) ) {
		return;
	}

	SHQUERYRBINFO rbInfo;
	rbInfo.cbSize = sizeof(rbInfo);

	char *rootPath = pathName;

	while ( rootPath[0] ) {
		UINT type = ::GetDriveType(rootPath);
		if ( type == DRIVE_FIXED || type == DRIVE_RAMDISK ) {
			if ( ::SHQueryRecycleBin(rootPath,&rbInfo) == S_OK ) {
				totalItems += (DWORD)rbInfo.i64NumItems;
				totalSize += (DWORD)rbInfo.i64Size;
			}
		}
		rootPath += strlen(rootPath) + 1;
	}
}

void __cdecl EmptyTrashThread(void*)
{
	DWORD totalSize = 0;
	DWORD totalItems = 0;

	QueryTrash(totalSize,totalItems);

	if ( totalItems ) {
		::SHEmptyRecycleBin(NULL,NULL,SHERB_NOCONFIRMATION | SHERB_NOPROGRESSUI | SHERB_NOSOUND);
	}

	if ( g_hWnd && ::IsWindow(g_hWnd) ) {
		string_t sstp(L"NOTIFY SSTP/1.1\r\nEvent: OnRecycleBinEmptied\r\nCharset: UTF-8\r\nHWnd: 0\r\n");

		char_t tmp[256];

		swprintf(tmp,L"Reference0: %u\r\nReference1: %u\r\n\r\n",totalItems,totalSize);

		sstp += tmp;

		std::string sstp_a = SAORI_FUNC::UnicodeToMultiByte(sstp.c_str(),CP_UTF8);

		DWORD result;
		COPYDATASTRUCT cds;
		cds.dwData = 9801;
		cds.cbData = sstp_a.size();
		cds.lpData = const_cast<char*>(sstp_a.c_str());

		::SendMessageTimeout(g_hWnd,WM_COPYDATA,reinterpret_cast<WPARAM>(g_hWnd),reinterpret_cast<LPARAM>(&cds),SMTO_ABORTIFHUNG|SMTO_BLOCK,2000,&result);
	}
}

