#ifdef _MSC_VER
#pragma warning( disable : 4786 )
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "csaori.h"

#define WS_POPUP            0x80000000L
#define WS_CHILD            0x40000000L
#define WS_MINIMIZE         0x20000000L
#define WS_VISIBLE          0x10000000L
#define WS_DISABLED         0x08000000L
#define WS_CLIPSIBLINGS     0x04000000L
#define WS_CLIPCHILDREN     0x02000000L
#define WS_MAXIMIZE         0x01000000L
#define WS_BORDER           0x00800000L
#define WS_DLGFRAME         0x00400000L
#define WS_VSCROLL          0x00200000L
#define WS_HSCROLL          0x00100000L
#define WS_SYSMENU          0x00080000L
#define WS_THICKFRAME       0x00040000L
#define WS_GROUP            0x00020000L
#define WS_TABSTOP          0x00010000L

#define WS_MINIMIZEBOX      0x00020000L
#define WS_MAXIMIZEBOX      0x00010000L

string_t StyleToStr(DWORD style)
{
	string_t str;

	if ( (style & WS_POPUP) != 0 ) { str += L"WS_POPUP,"; }
	if ( (style & WS_MINIMIZE) != 0 ) { str += L"WS_MINIMIZE,"; }
	if ( (style & WS_VISIBLE) != 0 ) { str += L"WS_VISIBLE,"; }
	if ( (style & WS_DISABLED) != 0 ) { str += L"WS_DISABLED,"; }
	if ( (style & WS_CLIPSIBLINGS) != 0 ) { str += L"WS_CLIPSIBLINGS,"; }
	if ( (style & WS_CLIPCHILDREN) != 0 ) { str += L"WS_CLIPCHILDREN,"; }
	if ( (style & WS_MAXIMIZE) != 0 ) { str += L"WS_MAXIMIZE,"; }
	if ( (style & WS_BORDER) != 0 ) { str += L"WS_BORDER,"; }
	if ( (style & WS_DLGFRAME) != 0 ) { str += L"WS_DLGFRAME,"; }
	if ( (style & WS_VSCROLL) != 0 ) { str += L"WS_VSCROLL,"; }
	if ( (style & WS_HSCROLL) != 0 ) { str += L"WS_HSCROLL,"; }
	if ( (style & WS_SYSMENU) != 0 ) { str += L"WS_SYSMENU,"; }
	if ( (style & WS_THICKFRAME) != 0 ) { str += L"WS_THICKFRAME,"; }

	if ( (style & WS_CHILD) != 0 ) {
		str += L"WS_CHILD,";
		if ( (style & WS_GROUP) != 0 ) { str += L"WS_GROUP,"; }
		if ( (style & WS_TABSTOP) != 0 ) { str += L"WS_TABSTOP,"; }
	}
	else {
		if ( (style & WS_MINIMIZEBOX) != 0 ) { str += L"WS_MINIMIZEBOX,"; }
		if ( (style & WS_MAXIMIZEBOX) != 0 ) { str += L"WS_MAXIMIZEBOX,"; }
	}

	if ( str.size() ) {
		str.erase(str.size()-1,1);
	}
	return str;
}


#define WS_EX_DLGMODALFRAME     0x00000001L
#define WS_EX_NOPARENTNOTIFY    0x00000004L
#define WS_EX_TOPMOST           0x00000008L
#define WS_EX_ACCEPTFILES       0x00000010L
#define WS_EX_TRANSPARENT       0x00000020L
#define WS_EX_MDICHILD          0x00000040L
#define WS_EX_TOOLWINDOW        0x00000080L
#define WS_EX_WINDOWEDGE        0x00000100L
#define WS_EX_CLIENTEDGE        0x00000200L
#define WS_EX_CONTEXTHELP       0x00000400L
#define WS_EX_RIGHT             0x00001000L
//#define WS_EX_LEFT              0x00000000L
#define WS_EX_RTLREADING        0x00002000L
//#define WS_EX_LTRREADING        0x00000000L
#define WS_EX_LEFTSCROLLBAR     0x00004000L
//#define WS_EX_RIGHTSCROLLBAR    0x00000000L
#define WS_EX_CONTROLPARENT     0x00010000L
#define WS_EX_STATICEDGE        0x00020000L
#define WS_EX_APPWINDOW         0x00040000L
#define WS_EX_LAYERED           0x00080000
#define WS_EX_NOINHERITLAYOUT   0x00100000L // Disable inheritence of mirroring by children
#define WS_EX_LAYOUTRTL         0x00400000L // Right to left mirroring
#define WS_EX_COMPOSITED        0x02000000L
#define WS_EX_NOACTIVATE        0x08000000L

string_t EXStyleToStr(DWORD style)
{
	string_t str;

	if ( (style & WS_EX_DLGMODALFRAME) != 0 ) { str += L"WS_EX_DLGMODALFRAME,"; }
	if ( (style & WS_EX_NOPARENTNOTIFY) != 0 ) { str += L"WS_EX_NOPARENTNOTIFY,"; }
	if ( (style & WS_EX_TOPMOST) != 0 ) { str += L"WS_EX_TOPMOST,"; }
	if ( (style & WS_EX_ACCEPTFILES) != 0 ) { str += L"WS_EX_ACCEPTFILES,"; }
	if ( (style & WS_EX_TRANSPARENT) != 0 ) { str += L"WS_EX_TRANSPARENT,"; }
	if ( (style & WS_EX_MDICHILD) != 0 ) { str += L"WS_EX_MDICHILD,"; }
	if ( (style & WS_EX_TOOLWINDOW) != 0 ) { str += L"WS_EX_TOOLWINDOW,"; }
	if ( (style & WS_EX_WINDOWEDGE) != 0 ) { str += L"WS_EX_WINDOWEDGE,"; }
	if ( (style & WS_EX_CLIENTEDGE) != 0 ) { str += L"WS_EX_CLIENTEDGE,"; }
	if ( (style & WS_EX_CONTEXTHELP) != 0 ) { str += L"WS_EX_CONTEXTHELP,"; }

	if ( (style & WS_EX_RIGHT) != 0 ) { str += L"WS_EX_RIGHT,"; }
	else { str += L"WS_EX_LEFT,"; }

	if ( (style & WS_EX_RTLREADING) != 0 ) { str += L"WS_EX_RTLREADING,"; }
	else { str += L"WS_EX_LTRREADING,"; }

	if ( (style & WS_EX_LEFTSCROLLBAR) != 0 ) { str += L"WS_EX_LEFTSCROLLBAR,"; }
	else { str += L"WS_EX_RIGHTSCROLLBAR,"; }

	if ( (style & WS_EX_CONTROLPARENT) != 0 ) { str += L"WS_EX_CONTROLPARENT,"; }
	if ( (style & WS_EX_STATICEDGE) != 0 ) { str += L"WS_EX_STATICEDGE,"; }
	if ( (style & WS_EX_APPWINDOW) != 0 ) { str += L"WS_EX_APPWINDOW,"; }
	if ( (style & WS_EX_LAYERED) != 0 ) { str += L"WS_EX_LAYERED,"; }
	if ( (style & WS_EX_NOINHERITLAYOUT) != 0 ) { str += L"WS_EX_NOINHERITLAYOUT,"; }
	if ( (style & WS_EX_LAYOUTRTL) != 0 ) { str += L"WS_EX_LAYOUTRTL,"; }
	if ( (style & WS_EX_COMPOSITED) != 0 ) { str += L"WS_EX_COMPOSITED,"; }
	if ( (style & WS_EX_NOACTIVATE) != 0 ) { str += L"WS_EX_NOACTIVATE,"; }

	if ( str.size() ) {
		str.erase(str.size()-1,1);
	}
	return str;
}

/*---------------------------------------------------------
	初期化
---------------------------------------------------------*/
bool CSAORI::load()
{
	return true;
}

/*---------------------------------------------------------
	解放
---------------------------------------------------------*/
bool CSAORI::unload()
{
	return true;
}

/*---------------------------------------------------------
	実行
---------------------------------------------------------*/
BOOL CALLBACK EnumWindowProc(HWND hwnd, LPARAM lp)
{
	CSAORIOutput *pOut = (CSAORIOutput*)lp;
	
	DWORD style = ::GetWindowLong(hwnd, GWL_STYLE);
	if ( ::IsWindow(hwnd) && ::IsWindowVisible(hwnd) && (style&WS_CAPTION) && !(style&WS_POPUP) ) {
		char buf[65536];
		::GetWindowText(hwnd, buf, 65535);
		if ( buf[0] != '\0' ) {
			pOut->values.push_back(SAORI_FUNC::MultiByteToUnicode(buf));
		}
	}
	return TRUE;
}

bool CSAORI_exec_info(const CSAORIInput& in,CSAORIOutput& out);

void CSAORI::exec(const CSAORIInput& in,CSAORIOutput& out)
{
	if ( in.args.size() ) {
		if ( in.args[0] == L"info" && in.args.size() >= 2 ) {
			if ( CSAORI_exec_info(in,out) ) {
				out.result_code = SAORIRESULT_OK;
			}
			else {
				out.result_code = SAORIRESULT_BAD_REQUEST;
			}
			return;
		}
	}

	//デフォルト動作
	out.result_code = SAORIRESULT_OK;
	::EnumWindows(EnumWindowProc, (LPARAM)&out);
	out.result = SAORI_FUNC::intToString(out.values.size());
}

bool CSAORI_exec_info(const CSAORIInput& in,CSAORIOutput& out)
{
	HWND hWnd = (HWND)wcstoul(in.args[1].c_str(),NULL,0);

	WINDOWINFO inf;
	inf.cbSize = sizeof(inf);
	if ( ! ::GetWindowInfo(hWnd,&inf) ) {
		return false;
	}

	MONITORINFO mon;
	mon.cbSize = sizeof(mon);
	::GetMonitorInfo(::MonitorFromWindow(hWnd,MONITOR_DEFAULTTONEAREST),&mon);

	//Val0 = Title Text
	char buf_a[512];
	::GetWindowTextA(hWnd,buf_a,sizeof(buf_a));
	out.values.push_back(SAORI_FUNC::MultiByteToUnicode(buf_a));

	//Val1 = Class Text
	::GetClassNameA(hWnd,buf_a,sizeof(buf_a));
	out.values.push_back(SAORI_FUNC::MultiByteToUnicode(buf_a));

	//Val2 = Window Rect
	char_t buf[64];
	swprintf(buf,L"%d,%d,%d,%d",inf.rcWindow.left,inf.rcWindow.top,inf.rcWindow.right,inf.rcWindow.bottom);

	out.values.push_back(buf);

	//Val3 = Client Rect
	swprintf(buf,L"%d,%d,%d,%d",inf.rcClient.left,inf.rcClient.top,inf.rcClient.right,inf.rcClient.bottom);

	out.values.push_back(buf);

	//Val4 = Work Rect
	swprintf(buf,L"%d,%d,%d,%d",mon.rcWork.left,mon.rcWork.top,mon.rcWork.right,mon.rcWork.bottom);

	out.values.push_back(buf);

	//Val5 = Style
	out.values.push_back(StyleToStr(inf.dwStyle));

	//Val6 = EXStyle
	out.values.push_back(EXStyleToStr(inf.dwExStyle));

	//Val7 = Parent
	swprintf(buf,L"%u",(UINT)::GetParent(hWnd));
	out.values.push_back(buf);

	//Val8 = State
	if ( ::IsIconic(hWnd) ) {
		out.values.push_back(L"minimized");
	}
	else if ( ::IsZoomed(hWnd) ) {
		out.values.push_back(L"maximized");
	}
	else {
		out.values.push_back(L"normal");
	}

	return true;
}
