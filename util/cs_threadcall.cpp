/*-------------------------------------------------
	cs_threadcall.cpp
	別スレッドで関数を一時実行
-------------------------------------------------*/

#define  WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <objbase.h>

#include <process.h>

#include "cs_threadcall.h"

//////////WINDOWS DEFINE///////////////////////////
#ifdef _WINDOWS
#ifdef _DEBUG
#include <crtdbg.h>
#define new new( _NORMAL_BLOCK, __FILE__, __LINE__)
#endif
#endif
///////////////////////////////////////////////////

#define CSTF_START 1
#define CSTF_EXIT  2
#define CSTF_CALL  0

CSThreadCall& GetCSThreadCall(void)
{
	static CSThreadCall t;
	return t;
}

CSThreadCall::CSThreadCall() : m_hThread(NULL) , m_call(NULL)
{
	m_hEvent = ::CreateEvent(NULL,TRUE,FALSE,NULL);
}
CSThreadCall::~CSThreadCall()
{
	Exit();
	::CloseHandle(m_hEvent);
}

static void __cdecl CSTF(void *p)
{
	::CoInitialize(NULL);

	CSThreadCall *pc = reinterpret_cast<CSThreadCall*>(p);
	pc->ThreadProc();

	::CoUninitialize();
}

bool CSThreadCall::Start(void)
{
	if ( ! m_hThread ) {
		unsigned long result = _beginthread(CSTF,0,reinterpret_cast<void*>(this));
		if ( result != static_cast<unsigned long>(-1) ) {
			m_hThread = reinterpret_cast<HANDLE>(result);
		}
		::WaitForSingleObject(m_hEvent,INFINITE);
		::ResetEvent(m_hEvent);
	}
	return m_hThread != NULL;
}
bool CSThreadCall::Exit(void)
{
	if ( m_hThread ) {
		::PostThreadMessage(m_ThreadID,WM_QUIT,0,0);
		::WaitForSingleObject(m_hThread,INFINITE);
		m_ThreadID = 0;
		m_hThread = NULL;
	}
	return m_hThread == NULL;
}

#define CSTF_MESSAGE (WM_APP+184)

class CSThreadCallData {
public:
	bool is_sync;
	void *p1;
	void *p2;
};

void CSThreadCall::ThreadProc(void)
{
	//Queueを強制的に作成
	MSG msg;
	::PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);

	m_ThreadID = ::GetCurrentThreadId();
	::SetEvent(m_hEvent);

	if ( m_call ) {
		m_call->CSTC_Start();
	}

	while ( true ) {
		if ( ::GetMessage(&msg,NULL,0,0) <= 0 ) {
			break;
		}
		if ( msg.message == CSTF_MESSAGE ) {
			if ( msg.wParam == CSTF_START ) {
				CSThreadCallBase *call_new = reinterpret_cast<CSThreadCallBase*>(msg.lParam);

				if ( call_new != m_call ) {
					if ( m_call ) {
						m_call->CSTC_Exit();
					}
					if ( call_new ) {
						call_new->CSTC_Start();
					}
					m_call = call_new;
				}
			}
			else if ( msg.wParam == CSTF_EXIT ) {
				break;
			}
			else if ( msg.wParam == CSTF_CALL ) {
				CSThreadCallData *pd = reinterpret_cast<CSThreadCallData*>(msg.lParam);
				m_call->CSTC_Call(pd->p1,pd->p2);
				if ( pd->is_sync ) {
					::SetEvent(m_hEvent);
				}
				delete pd;
			}
		}
	}

	if ( m_call ) {
		m_call->CSTC_Exit();
		m_call = NULL;
	}

	return;
}

void CSThreadCall::Set(CSThreadCallBase *call)
{
	if ( m_hThread ) {
		::PostThreadMessage(m_ThreadID,CSTF_MESSAGE,CSTF_START,reinterpret_cast<LPARAM>(call));
	}
	else {
		m_call = call;
	}
}

bool CSThreadCall::Call(void *p1,void *p2,bool isSync)
{
	Start();

	CSThreadCallData *pd = new CSThreadCallData;
	pd->p1 = p1;
	pd->p2 = p2;
	pd->is_sync = isSync;

	bool result = ::PostThreadMessage(m_ThreadID,CSTF_MESSAGE,CSTF_CALL,reinterpret_cast<LPARAM>(pd)) != 0;

	if ( isSync ) {
		if ( result ) {
			::WaitForSingleObject(m_hEvent,INFINITE);
		}
		::ResetEvent(m_hEvent);
	}

	return result;
}
