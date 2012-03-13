#pragma once

#ifndef __CPUUSAGE_H
#define __CPUUSAGE_H
#include <windows.h>

class CPUUsage {
private:
	bool m_is_osvi_got;
	OSVERSIONINFO m_osvi;
	unsigned int m_usage;
	unsigned char m_usage_array[100];

	volatile int m_thread_exit;
	HANDLE m_h_event;
	HANDLE m_h_thread;

	void CPUUsageThread9X();
	void CPUUsageThreadNT();

public:
	CPUUsage(void);
	~CPUUsage(void);

	void Load(void);
	void Unload(void);

	void CPUUsageThread();

	inline unsigned int GetCPUUsage(void) { return m_usage; }
	inline unsigned char* GetCPUUsageHistory(void) { return m_usage_array; }
};

#endif /* __CPUUSAGE_H */