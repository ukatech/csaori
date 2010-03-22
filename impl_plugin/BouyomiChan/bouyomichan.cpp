//setlocale//
#ifdef _MSC_VER
#if _MSC_VER >= 1400
#pragma setlocale("japanese")
#endif
#endif
//setlocale end//

#ifdef _MSC_VER
#pragma warning( disable : 4786 )
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <process.h>
#include <winsock.h>
#include <new>

#include "bouyomichan.h"

//////////WINDOWS DEFINE///////////////////////////
#ifdef _WINDOWS
#ifdef _DEBUG
#include <crtdbg.h>
#define new new( _NORMAL_BLOCK, __FILE__, __LINE__)
#endif
#endif
///////////////////////////////////////////////////

/*===============================================================
	インスタンス作成（csaori_baseから呼ばれる）
===============================================================*/

CSAORIBase* CreateInstance(void)
{
	return new CBouyomiChan();
}

/*===============================================================
	初期化(DllMain縛り)
===============================================================*/

CBouyomiChan::CBouyomiChan(void) : thread_active(true)
{
}

CBouyomiChan::~CBouyomiChan()
{
}

/*===============================================================
	初期化(DllMainとは別)
===============================================================*/
static unsigned int __stdcall BYMThreadProc(void* pv)
{
	CBouyomiChan *pb = reinterpret_cast<CBouyomiChan*>(pv);
	pb->ThreadProc();
	return 0;
}

bool CBouyomiChan::load()
{
	WSADATA wsaData;
	::WSAStartup(MAKEWORD(1,0),&wsaData);

	thread_event = ::CreateEvent(NULL,FALSE,FALSE,NULL);
	while ( ! thread_queue.empty() ) {
		thread_queue.pop_front();
	}
	thread_active = true;
	thread_handle = reinterpret_cast<HANDLE>(_beginthreadex(NULL,0,BYMThreadProc,this,0,NULL));
	return true;
}

bool CBouyomiChan::unload()
{
	thread_active = false;

	::SetEvent(thread_event);
	if ( ::WaitForSingleObject(thread_handle,5000) == WAIT_TIMEOUT ) {
		::TerminateThread(thread_handle,0);
	}
	
	::CloseHandle(thread_handle);
	thread_handle = NULL;

	::CloseHandle(thread_event);
	thread_event = NULL;

	::WSACleanup();

	return true;
}

/*===============================================================
	通信部
===============================================================*/
#include <pshpack1.h>
typedef struct tagBouyomiChanOption {
	short command;
	short speed;
	short tone;
	short volume;
	short voice;
	char charset;
	long len;
} BouyomiChanOption;
#include <poppack.h>

void CBouyomiChan::ThreadProc()
{
	while ( thread_active ) {
		::WaitForSingleObject(thread_event,INFINITE);


		{ //ロック開放用ブレース
			SAORI_FUNC::CCriticalSectionLock lock(thread_lock);
			CBouyomiChanData sendstr;
			std::string utf8str;
			std::vector<char> buffer;

			while ( (!thread_queue.empty()) && thread_active ) {
				sendstr = *(thread_queue.begin());
				thread_queue.pop_front();

				SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
				if ( sock == INVALID_SOCKET ) { continue; }
				
				sockaddr_in server;
				server.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
				server.sin_port             = htons(50001);
				server.sin_family           = AF_INET;

				if ( connect(sock, (struct sockaddr *)&server, sizeof(server)) != 0 ) { continue; }
				utf8str = SAORI_FUNC::UnicodeToMultiByte(sendstr.str.c_str(),CP_UTF8);

				buffer.assign(utf8str.length()+1+15,0);

				strcpy(&(buffer[15]),utf8str.c_str());
				BouyomiChanOption *opt = reinterpret_cast<BouyomiChanOption*>(&(buffer[0]));
				opt->command = 1; //1:メッセージ読み上げ
				opt->speed = -1;
				opt->tone = -1;
				opt->volume = -1;
				opt->charset = 0;
				opt->len = utf8str.length();

				if ( sendstr.ctx == 0 ) {
					opt->voice = 1;
				}
				else if ( sendstr.ctx == 1 ) {
					opt->voice = 2;
				}
				else {
					opt->voice = 5;
				}

				send(sock,&(buffer[0]),utf8str.length()+15,0);
				closesocket(sock);
			}
		}
	}
}

/*===============================================================
	実行部
===============================================================*/
static char_t* SplitCharTalk(char_t *scr,int &ctx)
{
	char_t* ptr = NULL;
	char_t* next = NULL;

	ctx = 0;

	char_t* s = wcsstr(scr,L"\\0");
	if ( s && (!ptr || ptr > s) ) {
		ptr = s;
		next = s + 2;
		ctx = 0;
	}

	s = wcsstr(scr,L"\\1");
	if ( s && (!ptr || ptr > s) ) {
		ptr = s;
		next = s + 2;
		ctx = 1;
	}

	s = wcsstr(scr,L"\\h");
	if ( s && (!ptr || ptr > s) ) {
		ptr = s;
		next = s + 2;
		ctx = 0;
	}

	s = wcsstr(scr,L"\\u");
	if ( s && (!ptr || ptr > s) ) {
		ptr = s;
		next = s + 2;
		ctx = 1;
	}

	s = wcsstr(scr,L"\\p");
	if ( s && (!ptr || ptr > s) ) {
		ptr = s;
		next = s + 2;
		if ( *next == L'[' ) {
			ctx = wcstoul(next+1,NULL,10);
			char_t *end = wcschr(next+1,L']');
			if ( end ) {
				next = end+1;
			}
			else {
				next = NULL;
			}
		}
		else if ( *next >= L'0' && *next <= L'9' ) {
			ctx = (*next - L'0');
			++next;
		}
	}

	if ( next && *next == 0 ) {
		next = NULL;
	}
	if ( ptr ) {
		*ptr = 0;
	}
	return next;
}

static void ClearTag(char_t *t)
{
	int len = wcslen(t);

	for ( int i = 0 ; i < len ; ++i ) {
		if ( t[i] == L'\\' ) {
			if ( t[i+1] == L'\\' ) {
				memcpy(t+i,t+i+1,(len-i/*-1+1 ZEROも*/)*sizeof(char_t));
				len -= 1;
			}
			else {
				size_t j = i+1;
				while ( t[j] == L'_' ) { ++j; } //先行アンダースコア

				char_t tagc = t[j];
				if ( iswalnum(tagc) || tagc == '!' || tagc == '?' ) { //タグ文字列
					++j;
					if ( t[j] == L'[' ) { //パラメータもあった
						++j;
						bool in_quote = false;
						while ( true ) {
							if ( t[j] == 0 ) { break; }
							if ( in_quote ) {
								if ( t[j] == L'"' ) {
									in_quote = false;
								}
							}
							else {
								if ( t[j] == L'"' ) {
									in_quote = true;
								}
								else if ( t[j] == L']' ) {
									++j;
									break;
								}
							}
							++j;
						}
					}
					else if ( t[j] >= L'0' && t[j] <= L'9' ) {
						if ( tagc == L'w' || tagc == L's' ) {
							++j;
						}
					}
				}
				if ( len == j ) {
					*(t+i) = 0;
					len = 0;
				}
				else {
					memcpy(t+i,t+j,(len-j+1/*+1してZEROも*/)*sizeof(char_t));
					len -= (j-i);
					i -= 1;
				}
			}
		}
	}
}

void CBouyomiChan::exec(const CSAORIInput& in,CSAORIOutput& out)
{
	//--------------------------------------------------------
	if ( wcsicmp(in.id.c_str(),L"OnMenuExec") == 0 ) {
		::MessageBoxW(NULL,L"棒読みちゃん\r\nhttp://chi.usamimi.info/\r\nにゴーストの喋りを送信するプラグインです。",L"棒読みちゃん送信",MB_OK);
		return;
	}
	//--------------------------------------------------------
	if ( wcsicmp(in.id.c_str(),L"OnOtherGhostTalk") == 0 ) {
		//棒読みちゃん起動チェック
		OSVERSIONINFO inf;
		inf.dwOSVersionInfoSize = sizeof(inf);
		::GetVersionEx(&inf);

		HANDLE hMutex;
		if ( inf.dwMajorVersion >= 5 ) {
			hMutex = ::CreateMutex(NULL,TRUE,"Local\\棒読みちゃん");
		}
		else {
			hMutex = ::CreateMutex(NULL,TRUE,"棒読みちゃん");
		}

		if ( ! hMutex ) {
			return;
		}
		if ( ::GetLastError() != ERROR_ALREADY_EXISTS ) {
			return;
		}

		//スクリプト＝Ref4
		if ( in.args.size() < 5 ) {
			return;
		}
		const string_t &str = in.args[4];

		char_t *buf = (char_t*)malloc((str.size()+1)*sizeof(char_t));
		wcscpy(buf,str.c_str());

		char_t *split = buf;
		char_t *next = split;
		size_t count = 0;
		int ctx = 0;
		int lastctx = 0;

		do {
			next = SplitCharTalk(split,ctx);
			ClearTag(split);
			if ( *split == 0 ) {
				if ( next ) {
					split = next;
					lastctx = ctx;
					continue;
				}
				else {
					break;
				}
			}
			{ //開放用ブレース
				SAORI_FUNC::CCriticalSectionLock lock(thread_lock);
				thread_queue.push_back(CBouyomiChanData(string_t(split),lastctx));
				++count;
			}
			split = next;
			lastctx = ctx;
		} while ( split );

		free(buf);

		if ( count ) {
			::SetEvent(thread_event);
		}
		return;
	}
	//--------------------------------------------------------
}

