#ifdef _MSC_VER
#pragma warning( disable : 4786 )
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shlobj.h>
#include "csaori.h"

/*=========================================================
	SAORI CORE
=========================================================*/

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
	実行
---------------------------------------------------------*/
static bool GetSpecialFolderPath(const std::wstring &nFolder, std::wstring &path );

void CSAORI::exec(const CSAORIInput& in,CSAORIOutput& out){
	out.result_code = SAORIRESULT_BAD_REQUEST;
	if ( in.args.size() < 1 ) { return; }
	if ( in.args[0].size() == 0 ) { return; }

	if ( wcsicmp(in.args[0].c_str(),L"get_special_folder_path") == 0 ) {
		std::wstring path;

		if ( GetSpecialFolderPath(in.args[1],path) ) {
			out.result = path;
			out.result_code = SAORIRESULT_OK;
		}
		else {
			out.result_code = SAORIRESULT_NO_CONTENT;
		}
	}
}

/*=========================================================
	処理関数群
=========================================================*/

/*---------------------------------------------------------
	get_special_folder_path
---------------------------------------------------------*/

static bool GetSpecialFolderPath(const std::wstring &nFolder, std::wstring &path )
{
	IMalloc    *pMalloc;
	ITEMIDLIST *pidl;
	
	char path_buffer[MAX_PATH*2+1];

	static const struct { int id; wchar_t *nFolder; } csidl_list[] = {
		{CSIDL_APPDATA,L"ApplicationData"},
		{CSIDL_COMMON_APPDATA,L"CommonApplicationData"},
		{CSIDL_LOCAL_APPDATA,L"LocalApplicationData"},
		{CSIDL_COOKIES,L"Cookies"},
		{CSIDL_DESKTOPDIRECTORY,L"Desktop"}, //CSIDL_DESKTOPではない！
		{CSIDL_FAVORITES,L"Favorites"},
		{CSIDL_HISTORY,L"History"},
		{CSIDL_INTERNET_CACHE,L"InternetCache"},
		{CSIDL_PROGRAMS,L"Programs"},
		//{,L"MyComputer"},
		{CSIDL_MYMUSIC,L"MyMusic"},
		{CSIDL_MYPICTURES,L"MyPictures"},
		{CSIDL_RECENT,L"Recent"},
		{CSIDL_SENDTO,L"SendTo"},
		{CSIDL_STARTMENU,L"StartMenu"},
		{CSIDL_STARTUP,L"Startup"},
		{CSIDL_SYSTEM,L"System"},
		{CSIDL_WINDOWS,L"Windows"},
		{CSIDL_TEMPLATES,L"Templates"},
		{CSIDL_DESKTOPDIRECTORY,L"DesktopDirectory"},
		{CSIDL_PERSONAL,L"Personal"},
		{CSIDL_PERSONAL,L"MyDocuments"},
		{CSIDL_PROGRAM_FILES,L"ProgramFiles"},
		{CSIDL_PROGRAM_FILES_COMMON,L"CommonProgramFiles"},
	};

	int id = 0;

	for ( unsigned int i = 0 ; i < (sizeof(csidl_list)/sizeof(csidl_list[0])) ; ++i ) {
		if ( wcsicmp(nFolder.c_str(),csidl_list[i].nFolder) == 0 ) {
			id = csidl_list[i].id;
			break;
		}
	}

	if ( id == 0 ) {
		return false;
	}
	
	//先にいろいろフィルタ
	if ( id == CSIDL_WINDOWS ) {
		::GetWindowsDirectory(path_buffer,sizeof(path_buffer)-1);
		path = SAORI_FUNC::MultiByteToUnicode(path_buffer);
		return true;
	}
	if ( id == CSIDL_SYSTEM ) {
		::GetSystemDirectory(path_buffer,sizeof(path_buffer)-1);
		path = SAORI_FUNC::MultiByteToUnicode(path_buffer);
		return true;
	}

	//メイン
	if ( NOERROR == ::SHGetMalloc( &pMalloc ) ) {
		if ( SUCCEEDED(::SHGetSpecialFolderLocation(NULL, id, &pidl )) ) {
			if ( ::SHGetPathFromIDList( pidl, path_buffer ) ) {
				path = SAORI_FUNC::MultiByteToUnicode(path_buffer);
				pMalloc->Free( pidl );
				pMalloc->Release();
				return true;
			}
			pMalloc->Free( pidl );
		}
		pMalloc->Release();
	}

	//もひとつ
	{
		typedef HRESULT (WINAPI *P_SHGetFolderPathA)(HWND hwndOwner,int nFolder,HANDLE hToken,DWORD dwFlags,LPTSTR pszPath);
		
		HMODULE hmFolder = ::LoadLibrary("shfolder");
		if ( hmFolder ) {
			P_SHGetFolderPathA getFldrPath = (P_SHGetFolderPathA)::GetProcAddress(hmFolder,"SHGetFolderPathA");

			if ( getFldrPath ) {
				if ( SUCCEEDED(getFldrPath(NULL,id,NULL,0,path_buffer)) ) { //0=SHGFP_TYPE_CURRENT
					path = SAORI_FUNC::MultiByteToUnicode(path_buffer);
					return true;
				}
			}
			::FreeLibrary(hmFolder);
		}
	}

	return false;
}
