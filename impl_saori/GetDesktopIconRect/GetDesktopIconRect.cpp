#ifdef _MSC_VER
#pragma warning( disable : 4786 )
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include <commctrl.h>

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
	é©ìÆfree
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
	é¿çs
---------------------------------------------------------*/
static HWND GetDesktopView(void)
{
	HWND desktopView = NULL;
	{
		HWND topWindow = ::FindWindow("Progman", "Program Manager");
		if ( topWindow ) {
			HWND childWindow = ::GetWindow(topWindow,GW_CHILD);
			if ( childWindow ) {
				desktopView = ::GetWindow(topWindow,GW_CHILD);
			}
		}
	}
	return desktopView;
}

void CSAORI::exec(const CSAORIInput& in,CSAORIOutput& out)
{
	out.result_code = SAORIRESULT_BAD_REQUEST;

	if ( in.args.size() < 1 || wcsicmp(in.args[0].c_str(),L"all") == 0 ) {
		HWND desktopView = GetDesktopView();
		if ( ! desktopView ) { return; }

		int count = ListView_GetItemCount(desktopView);

		out.result_code = SAORIRESULT_OK;
		for ( int i = 0 ; i < count ; ++i ) {
			char text_buffer[512];
			ListView_GetItemText(desktopView,i,0,text_buffer,sizeof(text_buffer)-1);
			string_t item = SAORI_FUNC::MultiByteToUnicode(text_buffer);

			RECT rect;
			ListView_GetItemRect(desktopView,i,&rect,LVIR_ICON);
			char_t rect_text[128];
			swprintf(rect_text,L",%d,%d,%d,%d",rect.left,rect.top,rect.right,rect.bottom);
			item += rect_text;

			out.values.push_back(item);
		}
	}
	else if ( in.args.size() >= 1 ) {
	}
}

