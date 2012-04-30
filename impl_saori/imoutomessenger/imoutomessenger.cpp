#ifdef _MSC_VER
#pragma warning( disable : 4786 )
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include "csaori.h"

/*---------------------------------------------------------
	èâä˙âª
---------------------------------------------------------*/
bool CSAORI::load(){
	return true;
}

/*---------------------------------------------------------
	âï˙
---------------------------------------------------------*/
bool CSAORI::unload(){
	return true;
}

/*---------------------------------------------------------
	é¿çs
---------------------------------------------------------*/
class ILWindowsDef {
public:
	ILWindowsDef(HWND h,const char *t) : hwnd(h), title(t) { }

	HWND hwnd;
	std::string title;
};

typedef std::vector<ILWindowsDef> ILWindowsList;

static BOOL CALLBACK EnumILWindowsProc(HWND hwnd,LPARAM lParam)
{
	char buf[512];
	::GetWindowText(hwnd,buf,sizeof(buf)-1);

	if ( strncmp(buf,"IMOUTOMAIN_",11) == 0 ) {
		ILWindowsList *pList = reinterpret_cast<ILWindowsList*>(lParam);

		pList->push_back(ILWindowsDef(hwnd,buf+11));
	}

	return TRUE;
}

void CSAORI::exec(const CSAORIInput& in,CSAORIOutput& out){
	out.result_code = SAORIRESULT_BAD_REQUEST;
	if ( in.args.size() < 1 ) { return; }

	DWORD flag = SND_ASYNC;
	std::string filepath;

	if ( wcsnicmp(in.args[0].c_str(),L"list",4) == 0 ) {
		ILWindowsList list;
		::EnumWindows(EnumILWindowsProc,reinterpret_cast<LPARAM>(&list));

		if ( list.size() == 0 ) {
			out.result_code = SAORIRESULT_NO_CONTENT;
			return;
		}

		out.result_code = SAORIRESULT_OK;

		string_t txt;

		size_t n = list.size();
		string_t title;

		for ( size_t i = 0 ; i < n ; ++i ) {
			title = SAORI_FUNC::MultiByteToUnicode(list[i].title.c_str());
			out.values.push_back(title);
			txt += title;
			txt += L"\1";
		}

		if ( txt.size() ) {
			txt.erase(txt.size()-1,1);
		}
		out.result = txt;
	}
	else if ( wcsnicmp(in.args[0].c_str(),L"send",4) == 0 ) {
		ILWindowsList list;
		::EnumWindows(EnumILWindowsProc,reinterpret_cast<LPARAM>(&list));

		if ( list.size() == 0 ) {
			out.result_code = SAORIRESULT_NO_CONTENT;
			return;
		}

		HWND hWndToSend = NULL;
		if ( in.args.size() >= 2 ) {
			size_t n = list.size();
			std::string find_str = SAORI_FUNC::UnicodeToMultiByte(in.args[1]);

			for ( size_t i = 0 ; i < n ; ++i ) {
				if ( strstr(list[i].title.c_str(),find_str.c_str()) ) {
					hWndToSend = list[i].hwnd;
					break;
				}
			}
		}
		if ( hWndToSend == NULL ) {
			hWndToSend = list[0].hwnd;
		}

		out.result_code = SAORIRESULT_OK;
	}
}

