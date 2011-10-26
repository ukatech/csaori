#ifdef _MSC_VER
#pragma warning( disable : 4786 )
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include <commctrl.h>

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
#define BUFFER_SIZE 4096

void CSAORI::exec(const CSAORIInput& in,CSAORIOutput& out)
{
	out.result_code = SAORIRESULT_BAD_REQUEST;

	if ( in.args.size() < 0 ) { return; }

	if ( wcsicmp(in.args[0].c_str(),L"hwnd") == 0 ) {
		out.result_code = SAORIRESULT_OK;
		return;
	}
	else if ( wcsicmp(in.args[0].c_str(),L"get") == 0 ) {
		out.result_code = SAORIRESULT_INTERNAL_SERVER_ERROR;

		//デスクトップウインドウ (左上0,0)
		HWND hWnd = ::FindWindow("Progman", "Program Manager");  
		if ( hWnd ) {
			hWnd = ::FindWindowEx(hWnd, NULL, "SHELLDLL_DefView", NULL);
			if ( hWnd ) {
				hWnd = ::FindWindowEx(hWnd, NULL, "SysListView32", NULL);
			}
		}
		if ( ! hWnd ) { return; }

		//検索対象と検索結果
		string_t find_string;
		if ( in.args.size() >= 2 ) {
			find_string = in.args[1];
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
			swprintf(rect_text,L",%d,%d",pt.x,pt.y);
			item += rect_text;

			swprintf(rect_text,L",%d,%d,%d,%d",rect.left,rect.top,rect.right,rect.bottom);
			item += rect_text;

			out.values.push_back(item);
		}

		::VirtualFreeEx(ph,ptr,0,MEM_RELEASE);
		::CloseHandle(ph);

		out.result_code = SAORIRESULT_OK;

		if ( find_pos >= 0 ) {
			char_t rect_text[128];

			swprintf(rect_text,L"%d,",find_pos);
			out.result = rect_text;

			out.result += find_text;

			swprintf(rect_text,L",%d,%d",find_point.x,find_point.y);
			out.result += rect_text;

			swprintf(rect_text,L",%d,%d,%d,%d",find_rect.left,find_rect.top,find_rect.right,find_rect.bottom);
			out.result += rect_text;
		}
		else {
			out.result = L"";
		}
	}
}

