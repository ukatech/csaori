/*-------------------------------------------------
	cs_threadcall.cpp
	別スレッドで関数を一時実行
-------------------------------------------------*/

#pragma once

#ifndef CS_THREADCALL_INCLUDED
#define CS_THREADCALL_INCLUDED

class CSThreadCallBase {
public:
	virtual void CSTC_Start(void) { }
	virtual void CSTC_Exit(void) { }
	virtual void CSTC_Call(void *p1,void *p2) { }

	CSThreadCallBase() { }
	virtual ~CSThreadCallBase() { }
};

class CSThreadCall {
private:
	HANDLE m_hThread;
	HANDLE m_hEvent;
	volatile DWORD m_ThreadID;
	CSThreadCallBase *m_call;

public:
	CSThreadCall();
	~CSThreadCall();

	void ThreadProc(void);

	bool Start(void);
	bool Exit(void);

	void Set(CSThreadCallBase *call);
	bool Call(void *p1 = NULL,void *p2 = NULL,bool isSync = true);
};

CSThreadCall& GetCSThreadCall(void);

#endif //CS_THREADCALL_INCLUDED
