#include "stdafx.h"
#include "csaori.h"

using namespace std;

DWORD WINAPI OsuwariThread(LPVOID lpParameter);
void _getDeskTopRect(RECT* rect);
bool _isEnableWindow(HWND hwnd);
void _getWorkAreaRect(RECT* rect);
void _getWindowRect(HWND hwnd,RECT* rect);
int _getPosition(string_t str);
int _getPosition2(string_t str);
BOOL CALLBACK _enumwnd(HWND hwnd,LPARAM lParam);
bool _isWindowTop(HWND hwnd);
vector<string_t> split(const string_t& s,const string_t& delim);
HWND StringToHwnd(string_t str);


int g_type;
int g_type2;
#define TYPE_ACTIVE		0
#define TYPE_FIXED		1
#define TYPE_DESKTOP	2
#define TYPE_WORKAREA	3
#define TYPE_HWND		4
#define TYPE_TITLE		5


int g_pos;
int g_pos2;
#define POS_TL		0
#define POS_TR		1	
#define POS_BL		2
#define POS_BR		3
#define POS_LEFT	4
#define POS_RIGHT	5	
#define POS_TOP		6
#define POS_BOTTOM	7
#define POS_CENTER	8

int g_dx;
int g_dy;

int g_Sleep;

bool g_isXMOVEABLE;
bool g_isYMOVEABLE;
bool g_isNOCLIP;
bool g_isCLIPEX;
int g_cex_left;
int g_cex_right;
int g_cex_top;
int g_cex_bottom;

HWND g_dstHWND;
HWND g_srcHWND;

bool g_isTopMost;

HANDLE g_hThread;
volatile bool g_isLive;

RECT g_oldRECT;
HWND g_oldHWND;

std::string g_dstTitle;

#ifdef _DEBUG
FILE* fp;
#endif

HWND StringToHwnd(string_t str){
#pragma warning( disable : 4312 ) //int -> HWND
	HWND hWnd=(HWND)_wtoi(str.c_str());
#pragma warning( default : 4312 )
	return hWnd;
}


void CSAORI::exec(const CSAORIInput& in,CSAORIOutput& out){
	if(in.args.size()==0){
		out.result=L"";
		out.result_code=SAORIRESULT_BAD_REQUEST;
		return;
	}

	if(in.args[0].find(L"START")!=string::npos){
		if(in.args.size()<7){
			out.result=L"";
			out.result_code=SAORIRESULT_BAD_REQUEST;
			return;
		}

		//arg 1
		HWND _srcHWND=StringToHwnd(in.args[1]);
		if(!IsWindow(_srcHWND)){
			out.result=L"source window is not exist";
			out.result_code=SAORIRESULT_INTERNAL_SERVER_ERROR;
			return;
		}

		//TOPMOSTフラグ保存
		if(_srcHWND!=g_srcHWND){
			if(_isWindowTop(g_srcHWND)){
				g_isTopMost=true;
			}else{
				g_isTopMost=false;
			}
		}

		g_srcHWND=_srcHWND;

		//arg 2
		if(in.args[2].find(L"ACTIVE")!=string::npos){
			g_type=TYPE_ACTIVE;
		}else if(in.args[2].find(L"FIX")!=string::npos){
			g_type=TYPE_FIXED;
		}else if(in.args[2][0]==L'#'){
			g_type=TYPE_HWND;
			g_dstHWND=StringToHwnd(in.args[2].substr(1));
			if(g_dstHWND==0){
				g_type=TYPE_ACTIVE;
			}
		}else if(in.args[2][0]==L'@'){
			g_type=TYPE_TITLE;
			g_dstTitle=SAORI_FUNC::UnicodeToMultiByte(in.args[2].substr(1));
		}else{
			out.result=L"";
			out.result_code=SAORIRESULT_BAD_REQUEST;
			return;
		}

		//arg3
		g_pos=_getPosition(in.args[3]);

		//arg4
		g_dx=_wtoi(in.args[4].c_str());

		//arg5
		g_dy=_wtoi(in.args[5].c_str());

		//arg6
		g_Sleep=_wtoi(in.args[6].c_str());
		if(g_Sleep==0){
			g_Sleep=100;
		}

		//arg7 - option
		if(in.args.size()>=8){
			g_isXMOVEABLE=false;
			g_isYMOVEABLE=false;
			g_isNOCLIP=false;
			if(in.args[7].find(L"XMOVE")!=string::npos){
				g_isXMOVEABLE=true;
			}
			if(in.args[7].find(L"YMOVE")!=string::npos){
				g_isYMOVEABLE=true;
			}
			if(in.args[7].find(L"NOCLIP")!=string::npos){
				g_isNOCLIP=true;
			}
		}

		//arg8 - on desktop position
		if(in.args.size()>=9){
			g_pos2=_getPosition2(in.args[8]);
			if(in.args[8].find(L"WORKAREA")!=string::npos){
				g_type2=TYPE_WORKAREA;
			}else{
				g_type2=TYPE_DESKTOP;
			}
		}

		//arg9 - clip ex
		if(in.args.size()>=10){
			vector<string_t> tokens=split(in.args[9],L" _");
			if(tokens.size()<=3){
				out.result=L"";
				out.result_code=SAORIRESULT_BAD_REQUEST;
				return;
			}
			g_cex_left	=_wtoi(tokens[0].c_str());
			g_cex_top	=_wtoi(tokens[1].c_str());
			g_cex_right	=_wtoi(tokens[2].c_str());
			g_cex_bottom=_wtoi(tokens[3].c_str());
		}


		//初期化
		g_oldRECT.left=g_oldRECT.right=g_oldRECT.top=g_oldRECT.bottom=0;

		//スレッド生成
		if(g_hThread==NULL){
			g_isLive=true;
			unsigned long nID;
			g_hThread=CreateThread(NULL,0,OsuwariThread,0,0,&nID);
		}
		out.result=L"";
		out.result_code=SAORIRESULT_OK;
		return;

	}else if(in.args[0].find(L"STOP")!=string::npos){
		if(g_isTopMost){
			::SetWindowPos(g_srcHWND,HWND_TOPMOST,0,0,0,0,SWP_NOSIZE|SWP_NOMOVE);
		}else{
			::SetWindowPos(g_srcHWND,HWND_NOTOPMOST,0,0,0,0,SWP_NOSIZE|SWP_NOMOVE);
		}
		unload();
		out.result=L"";
		out.result_code=SAORIRESULT_OK;
		return;
	}
	out.result=L"";
	out.result_code=SAORIRESULT_BAD_REQUEST;
	return;
}

bool CSAORI::unload(){
	g_isLive=false;
	if(g_hThread!=NULL){
		if(WaitForSingleObject(g_hThread,g_Sleep*10+1000)==WAIT_TIMEOUT){
			TerminateThread(g_hThread,-1);
		}
		//Sleep(g_Sleep*1.5);
		CloseHandle(g_hThread);
		g_hThread=NULL;
	}
#ifdef _DEBUG
	fclose(fp);
#endif
	return true;
}

bool CSAORI::load(){
	g_hThread=NULL;
#ifdef _DEBUG
	fopen_s(&fp,"d:\\saori.log","at+");//C4996
#endif
	g_cex_left=g_cex_right=g_cex_top=g_cex_bottom=0;
	return true;
}

//------------------------------------------
DWORD WINAPI OsuwariThread(LPVOID lpParameter){
	bool canosuwari;
	HWND dsthwnd;
	bool isForceTopWnd;
	bool isRecalced;

	while(true){
		if(g_isLive==false){return 0;}
		RECT dstrect;

		canosuwari=true;
		dsthwnd=NULL;
		isRecalced=false;

		//対象矩形決定
		switch(g_type){
			case TYPE_ACTIVE:
				dsthwnd=::GetForegroundWindow();
				if(_isEnableWindow(dsthwnd)){
					_getWindowRect(dsthwnd,&dstrect);
				}else{//座れない
					if(_isEnableWindow(g_oldHWND)){//前のウィンドウを試す
						_getWindowRect(g_oldHWND,&dstrect);
						dsthwnd=g_oldHWND;
					}else{//それでも座れない
						canosuwari=false;
					}
				}
				break;
			case TYPE_FIXED:
				canosuwari=false;//座れないとして扱う→option8が有効になる
				break;
			case TYPE_HWND:
				if(_isEnableWindow(g_dstHWND)){
					_getWindowRect(g_dstHWND,&dstrect);
				}else{//座れない
					canosuwari=false;
				}
				break;
			case TYPE_TITLE:
				g_dstHWND=NULL;
				EnumWindows(_enumwnd,0);
				if(_isEnableWindow(g_dstHWND)){
					_getWindowRect(g_dstHWND,&dstrect);
				}else{//座れない
					canosuwari=false;
				}
				break;
		}

RECALC:
		//座れなかった場合
		if(canosuwari==false){
			//タイプ２によって割り振る
			if(g_type2==TYPE_WORKAREA){
				_getWorkAreaRect(&dstrect);
			}else{
				_getDeskTopRect(&dstrect);
			}
			//仮想矩形決定
			switch(g_pos2){
				case POS_LEFT:
					dstrect.right=dstrect.left;
					dstrect.left=dstrect.right-100;
					break;
				case POS_RIGHT:
					dstrect.left=dstrect.right;
					dstrect.right=dstrect.left+100;
					break;
				case POS_TOP:
					dstrect.bottom=dstrect.top;
					dstrect.top=dstrect.bottom-100;
					break;
				case POS_BOTTOM:
					dstrect.top=dstrect.bottom;
					dstrect.bottom=dstrect.top+100;
					break;
			}
		}

		//移動判定
		int px,py;
		int cx,cy;
		int ex,ey;
		RECT myrect;
		_getWindowRect(g_srcHWND,&myrect);
		//原点計算
		switch(g_pos){
			case POS_TL:
				px=dstrect.left;
				py=dstrect.top;
				cx=px;
				cy=py;
				break;
			case POS_TR:
				px=dstrect.right;
				py=dstrect.top;
				cx=px-(myrect.right-myrect.left);
				cy=py;
				break;
			case POS_BL:
				px=dstrect.left;
				py=dstrect.bottom;
				cx=px;
				cy=py-(myrect.bottom-myrect.top);
				break;
			case POS_BR:
				px=dstrect.right;
				py=dstrect.bottom;
				cx=px-(myrect.right-myrect.left);
				cy=py-(myrect.bottom-myrect.top);
				break;
		}
		ex=cx+g_dx;
		ey=cy+g_dy;
		if(g_oldRECT.left==dstrect.left && g_oldRECT.right ==dstrect.right && g_oldRECT.top ==dstrect.top  && g_oldRECT.bottom==dstrect.bottom){
			//対象矩形が変わってなければ
			if(g_isXMOVEABLE){
				int x=myrect.left;
				if(x<dstrect.left){
					x=dstrect.left;
				}
				if(x>dstrect.right-(myrect.right-myrect.left)){
					x=dstrect.right-(myrect.right-myrect.left);
				}
				g_dx=x-cx;
				ex=x;
				ey=cy+g_dy;
			}
			if(g_isYMOVEABLE){
				int y=myrect.top;
				if(y<dstrect.top){
					y=dstrect.top;
				}
				if(y>dstrect.bottom-(myrect.bottom-myrect.top)){
					y=dstrect.bottom-(myrect.bottom-myrect.top);
				}
				g_dy=y-cy;
				ex=cx+g_dx;
				ey=y;
			}
		}else{
			//対象矩形が変わっていたら
			//isForceTopWnd=true;//何故かここにはこない？
		}

		if(myrect.top!=ey || myrect.left!=ex){
			//移動した場合は強制的にTOPMOSTをかける
			isForceTopWnd=true;
		}else{
			isForceTopWnd=false;
		}

		//見切れ判定
		if(g_isNOCLIP==false && (g_type!=TYPE_FIXED && canosuwari==true)){
			RECT srect;
			_getDeskTopRect(&srect);
			bool isOnScreen=true;
			if(ex<srect.left								- g_cex_left)	{isOnScreen=false;}
			if(ex+(myrect.right-myrect.left)>srect.right	+ g_cex_right)	{isOnScreen=false;}
			if(ey<srect.top									- g_cex_top)	{isOnScreen=false;}
			if(ey+(myrect.bottom-myrect.top)>srect.bottom	+ g_cex_bottom)	{isOnScreen=false;}
			if(isOnScreen==false && isRecalced==false){
				isRecalced=true;
				canosuwari=false;
				goto RECALC;
			}
		}

		//Zオーダー決定
		HWND tophwnd=0;
/*
		if(g_followHWND!=0){
			//追従するHWND
			tophwnd=g_followHWND;
		}else{
*/
			tophwnd=::FindWindowA("#32768",NULL);
			if(tophwnd!=NULL && IsWindowVisible(tophwnd)){
				//メニューが出てる場合
				tophwnd=HWND_NOTOPMOST;
			}else{
				tophwnd=HWND_TOPMOST;
			}
			if((g_type==TYPE_HWND || g_type==TYPE_TITLE) && canosuwari==true){
				if(::GetForegroundWindow()!=g_dstHWND){
					//座れてかつそのウィンドウがトップでない場合
					tophwnd=HWND_NOTOPMOST;
				}
			}
//		}

#pragma warning(disable:4313)
#ifdef _DEBUG
//		fprintf(fp,"hwnd=%d\tex=%d ey=%d dx=%d dy=%d canosuwari=%d tophwnd=%d\n",g_srcHWND,ex,ey,g_dx,g_dy,canosuwari,tophwnd);
#endif
#pragma warning(default:4313)

		//移動
		if(tophwnd==HWND_TOPMOST && _isWindowTop(g_srcHWND) && isForceTopWnd==false){
			::SetWindowPos(g_srcHWND,tophwnd,ex,ey,0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_ASYNCWINDOWPOS|SWP_NOACTIVATE);
		}else{
			::SetWindowPos(g_srcHWND,tophwnd,ex,ey,0,0,SWP_NOSIZE|SWP_ASYNCWINDOWPOS|SWP_NOACTIVATE);
		}

		g_oldRECT=dstrect;
		g_oldHWND=dsthwnd;

		//スリープ
		if(g_isLive==false){return 0;}
		::Sleep(g_Sleep);
		if(g_isLive==false){return 0;}
	}

	return 0;
}

bool _isEnableWindow(HWND hwnd){
	if(hwnd==NULL){
		return false;
	}
	if(hwnd==g_srcHWND){
		return false;
	}
	if(::GetParent(hwnd)!=NULL){
		return false;
	}
	return true;
}

bool _isWindowTop(HWND hwnd){
	if(::GetWindowLong(g_srcHWND,GWL_EXSTYLE) & WS_EX_TOPMOST){
		return true;
	}else{
		return false;
	}
}

void _getDeskTopRect(RECT* rect){
	rect->left=0;
	rect->right=::GetSystemMetrics(SM_CXSCREEN);
	rect->top=0;
	rect->bottom=::GetSystemMetrics(SM_CYSCREEN);
	return;
}

void _getWorkAreaRect(RECT* rect){
	::SystemParametersInfo( SPI_GETWORKAREA, 0, rect, 0 );
	return;
}

void _getWindowRect(HWND hwnd,RECT* rect){
	::GetWindowRect(hwnd,rect);
	return;
}

int _getPosition(string_t str){
	if(str.find(L"TL")!=string::npos)	{return POS_TL;}
	if(str.find(L"TR")!=string::npos)	{return POS_TR;}
	if(str.find(L"BL")!=string::npos)	{return POS_BL;}
	if(str.find(L"BR")!=string::npos)	{return POS_BR;}
	return POS_TL;
}

int _getPosition2(string_t str){
	if(str.find(L"LEFT")!=string::npos)		{return POS_LEFT;}
	if(str.find(L"RIGHT")!=string::npos)	{return POS_RIGHT;}
	if(str.find(L"TOP")!=string::npos)		{return POS_TOP;}
	if(str.find(L"BOTTOM")!=string::npos)	{return POS_BOTTOM;}
	return POS_CENTER;
}

BOOL CALLBACK _enumwnd(HWND hwnd,LPARAM lParam){
	char buf[255];
	int res=GetWindowTextA(hwnd,buf,254);
	if(res!=0){
		if(strstr(buf,g_dstTitle.c_str())!=NULL){
			g_dstHWND=hwnd;
			return FALSE;
		}
	}
	return TRUE;
}

/**
* split関数
* @param string str 分割したい文字列
* @param string delim デリミタ
* @return list<string> 分割された文字列
*/
vector<string_t> split(const string_t& s,const string_t& delim)
{
	vector<string_t> ret;
	typedef string_t::size_type string_size;
	string_size i = 0;
	while (i != s.size()){
		// 最初の空白を無視
		while(i != s.size() && delim.find(s[i])!=string::npos){
			++i;
		}
		string_size j = i;
		while (j != s.size() && delim.find(s[j])==string::npos){
		    ++j;
		}
		if (i != j){
			ret.push_back(s.substr(i, j-i));
			i = j;
		}
	}
	return ret;
}
