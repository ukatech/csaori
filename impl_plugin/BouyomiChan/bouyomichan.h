#include "cplugin.h"
#include <deque>

#pragma once

#ifndef BOUYOMICHAN_PLUGIN_INCLUDED
#define BOUYOMICHAN_PLUGIN_INCLUDED

class CBouyomiChanData {
public:
	CBouyomiChanData(void) : ctx(0) {
	}
	CBouyomiChanData(const string_t &s,int context) : str(s),ctx(context) {
	}
	string_t str;
	int ctx;
};

class CBouyomiChan : public CPLUGIN
{
private:
	SAORI_FUNC::CCriticalSection thread_lock;
	volatile int thread_active;
	std::deque<CBouyomiChanData> thread_queue;
	HANDLE thread_event;
	HANDLE thread_handle;
	int speak_type;
	std::string ini_file_path;

public:
	CBouyomiChan(void);
	~CBouyomiChan();

	void ThreadProc(void);

	void ShowMenu(CSAORIOutput& out);

	virtual void exec(const CSAORIInput& in,CSAORIOutput& out);
	virtual bool unload();
	virtual bool load();
};

#endif //BOUYOMICHAN_PLUGIN_INCLUDED


